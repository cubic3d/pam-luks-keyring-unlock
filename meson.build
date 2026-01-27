project('pam-luks-keyring-unlock', 'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'warning_level=3',
    'c_std=c2x',
    'buildtype=release',
  ],
)

inc_dirs = include_directories('src')

src_files = files(
  'src/pam_luks_keyring_unlock.c',
)

module = library(
  'pam_luks_keyring_unlock',
  src_files,
  name_prefix: '',
  dependencies: [
    dependency('pam'),
    dependency('libkeyutils'),
  ],
  include_directories: inc_dirs,
  install: true,
  install_dir: get_option('libdir') / 'security',
)
