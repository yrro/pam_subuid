# SPDX-License-Identifier: LGPL-2.1+

project('pam_subuid', 'c',
  version: '1',
  license: 'LGPLv2+',
  default_options : [
    'c_std=gnu11',
    'warning_level=3',
  ],
  meson_version : '>= 0.46',
)

prefixdir = get_option('prefix')
if not prefixdir.startswith('/')
  error('Prefix is not absolute: "@0@"'.format(prefixdir))
endif

libdir = join_paths(prefixdir, get_option('libdir'))

pamlibdir = join_paths(libdir, 'security')

cc = meson.get_compiler('c')

libpam = cc.find_library('pam', required: true)

pam_subuid = shared_library('pam_subuid',
  sources : [
    'pam_subuid.c',
  ],
  name_prefix : '',
  c_args : [
    '-fvisibility=internal',
  ],
  dependencies : [
    libpam,
  ],
  install : true,
  install_dir : pamlibdir,
)
