# PAM LUKS Keyring Unlock

A PAM module that seamlessly unlocks your Gnome Keyring and KDE Wallet using your LUKS encryption key.

This module retrieves your LUKS passphrase from the kernel keyring, where it's cached by `sd-encrypt`, and uses it to unlock your desktop's keyring, providing a smooth, passwordless login experience after you've unlocked your encrypted drive.

## Features

- **Automatic unlocking:** Unlock your Gnome Keyring and KDE Wallet without entering your password again.
- **Seamless integration:** Works with your existing LUKS setup and desktop environment.
- **Secure:** Retrieves the LUKS key from the kernel keyring, avoiding the need to store your password in plaintext.
- **Lightweight:** A small and efficient C-based PAM module.

## How it works

1. When you boot your system, `sd-encrypt` prompts for your LUKS passphrase to unlock your encrypted drive.
2. After a successful unlock, `sd-encrypt` caches the passphrase in the kernel's "user" keyring.
3. When you log in, this PAM module is invoked.
4. The module retrieves the cached LUKS passphrase from the kernel keyring.
5. It then passes the passphrase to `pam_gnome_keyring.so` and `pam_kwallet5.so` to unlock your Gnome Keyring and KDE Wallet.

## Installation

### Prerequisites

- A Linux system with a LUKS-encrypted root partition.
- `systemd` as the init system.
- `rd.luks.options=password-cache=yes` kernel parameter set.
- `meson` and `ninja` for building the module.
- `libkeyutils-devel` (or equivalent) for keyring access.
- `pam-devel` (or equivalent) for PAM module development.

### Building and Installing

1. **Clone the repository:**

    ```bash
    git clone https://github.com/cubic3d/pam-luks-keyring-unlock.git
    cd pam-luks-keyring-unlock
    ```

2. **Build and install the module:**

    ```bash
    meson setup build
    ninja -C build install
    ```

### AUR Package (Arch Linux)

An Arch User Repository (AUR) package named `pam-luks-keyring-unlock` is available for easy installation on Arch Linux and its derivatives.

Install with an AUR helper (e.g., `yay`):

```bash
yay -S pam-luks-keyring-unlock
```

Or, if you prefer to build manually:

```bash
git clone https://aur.archlinux.org/pam-luks-keyring-unlock.git
cd pam-luks-keyring-unlock
makepkg -si
```

The AUR package installs the `pam_luks_keyring_unlock.so` module to `/usr/lib/security/`.

## Configuration

To enable the module, you need to add it to your PAM configuration. The exact file depends on your login manager (e.g., greetd).

**Important:** Always back up your PAM configuration files before making changes. An incorrect configuration can lock you out of your system.

Add the following line to the appropriate file in `/etc/pam.d/` (e.g., `/etc/pam.d/greetd`):

```text
session    optional     pam_luks_keyring_unlock.so
```

This line should be placed **before** the lines that unlock the keyrings, for example:

```bash
# /etc/pam.d/greetd

#%PAM-1.0

auth       sufficient   pam_fprintd.so
auth       required     pam_securetty.so
auth       requisite    pam_nologin.so
auth       include      system-local-login
account    include      system-local-login
session    optional     pam_luks_keyring_unlock.so #<-
session    optional     pam_gnome_keyring.so auto_start #<-
session    include      system-local-login
```

Make sure you have enabled `rd.luks.options=password-cache=yes` in the kernel parameters to allow `sd-encrypt` writing the password into the keyring.

If you built the module manually and installed it to `/usr/local/lib/security/pam_luks_keyring_unlock.so` (default), the full path needs to be used in the config.

## Contributing

Contributions are welcome! If you find a bug or have an idea for a new feature, please open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENCE](LICENCE) file for details.
