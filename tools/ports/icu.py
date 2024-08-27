# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-68-2'
VERSION = '68_2'
HASH = '12c3db5966c234c94e7918fb8acc8bd0838edc36a620f3faa788e7ff27b06f1aa431eb117401026e3963622b9323212f444b735d5c9dd3d0b82d772a4834b993'

variants = {'icu-mt': {'PTHREADS': 1}}

libname_libicu_common = 'libicu_common'
libname_libicu_stubdata = 'libicu_stubdata'
libname_libicu_i18n = 'libicu_i18n'
libname_libicu_io = 'libicu_io'


def needed(settings):
  return settings.USE_ICU


def get_lib_name(base_name, settings):
  return base_name + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('icu', f'https://github.com/unicode-org/icu/releases/download/{TAG}/icu4c-{VERSION}-src.zip', sha512hash=HASH)
  icu_source_path = None

  def prepare_build():
    nonlocal icu_source_path
    source_path = ports.get_dir('icu', 'icu') # downloaded icu4c path
    icu_source_path = os.path.join(source_path, 'source')

  def build_lib(lib_output, lib_src, other_includes, build_flags):
    additional_build_flags = [
        # TODO: investigate why this is needed and remove
        '-Wno-macro-redefined',
        '-Wno-deprecated-declarations',
        # usage of 'using namespace icu' is deprecated: icu v61
        '-DU_USING_ICU_NAMESPACE=0',
        # make explicit inclusion of utf header: ref utf.h
        '-DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1',
        # mark UnicodeString constructors explicit : ref unistr.h
        '-DUNISTR_FROM_CHAR_EXPLICIT=explicit',
        '-DUNISTR_FROM_STRING_EXPLICIT=explicit',
        # generate static
        '-DU_STATIC_IMPLEMENTATION',
        # CXXFLAGS
        '-std=c++11'
    ]
    if settings.PTHREADS:
      additional_build_flags.append('-pthread')

    ports.build_port(lib_src, lib_output, 'icu', includes=other_includes, flags=build_flags + additional_build_flags)

  # creator for libicu_common
  def create_libicu_common(lib_output):
    prepare_build()
    lib_src = os.path.join(icu_source_path, 'common')
    ports.install_headers(os.path.join(lib_src, 'unicode'), target='unicode')
    build_lib(lib_output, lib_src, [], ['-DU_COMMON_IMPLEMENTATION=1'])

  # creator for libicu_stubdata
  def create_libicu_stubdata(lib_output):
    lib_src = os.path.join(icu_source_path, 'stubdata')
    other_includes = [os.path.join(icu_source_path, 'common')]
    build_lib(lib_output, lib_src, other_includes, [])

  # creator for libicu_i18n
  def create_libicu_i18n(lib_output):
    lib_src = os.path.join(icu_source_path, 'i18n')
    ports.install_headers(os.path.join(lib_src, 'unicode'), target='unicode')
    other_includes = [os.path.join(icu_source_path, 'common')]
    build_lib(lib_output, lib_src, other_includes, ['-DU_I18N_IMPLEMENTATION=1'])

  # creator for libicu_io
  def create_libicu_io(lib_output):
    prepare_build()
    lib_src = os.path.join(icu_source_path, 'io')
    ports.install_headers(os.path.join(lib_src, 'unicode'), target='unicode')
    other_includes = [os.path.join(icu_source_path, 'common'), os.path.join(icu_source_path, 'i18n')]
    build_lib(lib_output, lib_src, other_includes, ['-DU_IO_IMPLEMENTATION=1'])

  return [
      shared.cache.get_lib(get_lib_name(libname_libicu_common, settings), create_libicu_common), # this also prepares the build
      shared.cache.get_lib(get_lib_name(libname_libicu_stubdata, settings), create_libicu_stubdata),
      shared.cache.get_lib(get_lib_name(libname_libicu_i18n, settings), create_libicu_i18n),
      shared.cache.get_lib(get_lib_name(libname_libicu_io, settings), create_libicu_io)
  ]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(libname_libicu_common, settings))
  shared.cache.erase_lib(get_lib_name(libname_libicu_stubdata, settings))
  shared.cache.erase_lib(get_lib_name(libname_libicu_i18n, settings))
  shared.cache.erase_lib(get_lib_name(libname_libicu_io, settings))


def show():
  return 'icu (-sUSE_ICU=1 or --use-port=icu; Unicode License)'
