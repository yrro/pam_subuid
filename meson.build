# SPDX-License-Identifier: ISC

project('pam_subuid', 'c',
  version: '1',
  license: 'LGPLv2+',
  default_options : [
    'c_std=gnu11',
    'warning_level=3',
    'buildtype=debugoptimized',
  ],
  meson_version : '>= 0.46',
)

add_global_arguments('-fstack-protector-strong', '-Werror=format-security', '-Wno-unused-parameter', language: 'c')

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
    'subxid.c',
    'pam.c',
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

ctest = executable('ctest',
  sources : [
    'ctest.c',
    'subxid.c',
    'test_find_free_range.c',
    'test_logindef_uint.c',
    'test_subxid_entry_compare.c',
  ],
)
test('ctest', ctest)

