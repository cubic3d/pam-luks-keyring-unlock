#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <keyutils.h>

#define PAM_LOG_INFO(...) pam_syslog(pamh, LOG_AUTH | LOG_INFO, __VA_ARGS__)
#define PAM_LOG_ERROR(...) pam_syslog(pamh, LOG_AUTH | LOG_ERR, __VA_ARGS__)

void cleanup_callback(pam_handle_t *pamh, void *data, int error_status) {
  (void)pamh;
  (void)error_status;

  explicit_bzero(data, strlen((char *)data));
  free(data);
}

int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;

  // Attempt to find the cryptsetup key for the user
  key_serial_t key_serial = find_key_by_type_and_desc("user", "cryptsetup", 0);
  if (key_serial == -1) {
    if (errno == ENOKEY) {
      PAM_LOG_ERROR("No cryptsetup key found for user.");
    } else if (errno == EKEYEXPIRED) {
      PAM_LOG_ERROR("Cryptsetup key has expired.");
    } else {
      PAM_LOG_ERROR("Error retrieving cryptsetup key: %d.", errno);
    }

    return PAM_SUCCESS;
  }

  // Read the key data
  char *buffer = NULL;
  int len = keyctl_read_alloc(key_serial, (void **)&buffer);
  if (len < 0) {
    PAM_LOG_ERROR("Failed to read cryptsetup data: %d.", errno);
    return PAM_SUCCESS;
  }

  // Mistyped passwords are also cached in the keyring, so we need to extract the last string
  const char *password = NULL;
  for (const char *i = buffer; i < buffer + len && *i != '\0'; i += strlen(i) + 1) {
    password = i;
  }

  // Set the password for Gnome Keyring
  int ret = pam_set_data(pamh, "gkr_system_authtok", strdup(password), cleanup_callback);
  if (ret != PAM_SUCCESS) {
    PAM_LOG_ERROR("Failed to set Gnome Keyring password: %d.", ret);
  } else {
    PAM_LOG_INFO("Gnome Keyring password set successfully.");
  }

  // Set the password for KDE Wallet
  ret = pam_set_data(pamh, "kwallet5_key", strdup(password), cleanup_callback);
  if (ret != PAM_SUCCESS) {
    PAM_LOG_ERROR("Failed to set KDE Wallet password: %d.", ret);
  } else {
    PAM_LOG_INFO("KDE Wallet password set successfully.");
  }

  explicit_bzero(buffer, len);
  free(buffer);
  return PAM_SUCCESS;
}

int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;

  // Unset data
  pam_set_data(pamh, "gkr_system_authtok", NULL, NULL);
  pam_set_data(pamh, "kwallet5_key", NULL, NULL);

  return PAM_SUCCESS;
}
