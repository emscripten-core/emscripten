# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'release-68-2'
VERSION = '68_2'
HASH = '12c3db5966c234c94e7918fb8acc8bd0838edc36a620f3faa788e7ff27b06f1aa431eb117401026e3963622b9323212f444b735d5c9dd3d0b82d772a4834b993'

libname_libicu_common = 'libicu_common.a'
libname_libicu_stubdata = 'libicu_stubdata.a'
libname_libicu_i18n = 'libicu_i18n.a'


def needed(settings):
  return settings.USE_ICU


def get(ports, settings, shared):
  url = 'https://github.com/unicode-org/icu/releases/download/%s/icu4c-%s-src.zip' % (TAG, VERSION)
  ports.fetch_project('icu', url, 'icu', sha512hash=HASH)
  icu_source_path = os.path.join(ports.get_build_dir(), 'icu', 'source')

  def prepare_build():
    source_path = os.path.join(ports.get_dir(), 'icu', 'icu') # downloaded icu4c path
    dest_path = os.path.join(ports.get_build_dir(), 'icu') # icu build path
    logging.debug(f'preparing for icu build: {source_path} -> {dest_path}')
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

  def build_lib(lib_output, lib_src, other_includes, build_flags):
    logging.debug('building port: icu- ' + lib_output)

    additional_build_flags = [
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
    ports.build_port(lib_src, lib_output, other_includes, build_flags + additional_build_flags)

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

  return [
      shared.Cache.get_lib(libname_libicu_common, create_libicu_common), # this also prepares the build
      shared.Cache.get_lib(libname_libicu_stubdata, create_libicu_stubdata),
      shared.Cache.get_lib(libname_libicu_i18n, create_libicu_i18n)
  ]


def clear(ports, settings, shared):
  shared.Cache.erase_lib(libname_libicu_common)
  shared.Cache.erase_lib(libname_libicu_stubdata)
  shared.Cache.erase_lib(libname_libicu_i18n)


def process_args(ports):
  return []


def show():
  return 'icu (USE_ICU=1; Unicode License)'
