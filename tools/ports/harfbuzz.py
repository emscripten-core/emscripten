# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import logging
from tools import building

TAG = '1.7.5'
HASH = 'c2c13fc97bb74f0f13092b07804f7087e948bce49793f48b62c2c24a5792523acc0002840bebf21829172bb2e7c3df9f9625250aec6c786a55489667dd04d6a0'

deps = ['freetype']


def needed(settings):
  return settings.USE_HARFBUZZ


def get(ports, settings, shared):
  ports.fetch_project('harfbuzz', 'https://github.com/harfbuzz/harfbuzz/releases/download/' +
                      TAG + '/harfbuzz-' + TAG + '.tar.bz2', 'harfbuzz-' + TAG, is_tarbz2=True, sha512hash=HASH)

  def create():
    logging.info('building port: harfbuzz')
    ports.clear_project_build('harfbuzz')

    source_path = os.path.join(ports.get_dir(), 'harfbuzz', 'harfbuzz-' + TAG)
    dest_path = os.path.join(ports.get_build_dir(), 'harfbuzz')

    freetype_lib = shared.Cache.get_path('libfreetype.a')
    freetype_include = os.path.join(ports.get_include_dir(), 'freetype2', 'freetype')
    freetype_include_dirs = freetype_include + ';' + os.path.join(freetype_include, 'config')

    configure_args = [
      'cmake',
      '-G', 'Unix Makefiles',
      '-B' + dest_path,
      '-H' + source_path,
      '-DCMAKE_BUILD_TYPE=Release',
      '-DCMAKE_INSTALL_PREFIX=' + dest_path,
      '-DFREETYPE_INCLUDE_DIRS=' + freetype_include_dirs,
      '-DFREETYPE_LIBRARY=' + freetype_lib,
      '-DHB_HAVE_FREETYPE=ON'
    ]

    extra_cxx_flags = []

    if settings.RELOCATABLE:
      extra_cxx_flags.append('-fPIC')

    if settings.USE_PTHREADS:
      extra_cxx_flags.append('-pthread')

    if len(extra_cxx_flags):
      configure_args += ['-DCMAKE_CXX_FLAGS="{}"'.format(' '.join(extra_cxx_flags))]

    building.configure(configure_args)
    building.make(['make', '-j%d' % building.get_num_cores(), '-C' + dest_path, 'install'])

    ports.install_header_dir(os.path.join(dest_path, 'include', 'harfbuzz'))

    return os.path.join(dest_path, 'libharfbuzz.a')

  return [shared.Cache.get('libharfbuzz' + ('-mt' if settings.USE_PTHREADS else '') + '.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libharfbuzz.a')


def process_dependencies(settings):
  settings.USE_FREETYPE = 1


def process_args(ports):
  return ['-I' + os.path.join(ports.get_build_dir(), 'harfbuzz', 'include', 'harfbuzz')]


def show():
  return 'harfbuzz (USE_HARFBUZZ=1; MIT license)'
