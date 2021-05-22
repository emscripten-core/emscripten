# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
from tools import system_libs
from ..utils import path_from_root

TAG = 'VER-2-10-4'
HASH = '795dfc6b3f14b25cda4f2e76637e7453390ef775bd6b7dc380d5aa03d923890bdff46d3f40e52b54a49c05207efa4e9cbdc9281cb5babb51750919bbda1c80b9'


def needed(settings):
  return settings.USE_FREETYPE


def get(ports, settings, shared):
  ports.fetch_project('freetype', 'http://git.savannah.gnu.org/cgit/freetype/freetype2.git/snapshot/freetype2-' + TAG + '.tar.gz', 'FreeType-' + TAG, sha512hash=HASH)

  def create(final):
    ports.clear_project_build('freetype')

    source_path = os.path.join(ports.get_dir(), 'freetype', 'freetype2-' + TAG)
    dest_path = os.path.join(ports.get_build_dir(), 'freetype')

    shutil.rmtree(dest_path, ignore_errors=True)
    os.makedirs(dest_path)

    cmake_cmd = [
      # this should be shared.EMCMAKE to match newer emsdk versions
      path_from_root('emcmake'),
      'cmake',
      '-B' + dest_path,
      '-H' + source_path,
      '-DCMAKE_BUILD_TYPE=Release',
      '-DCMAKE_INSTALL_PREFIX=' + dest_path
    ]

    extra_cflags = []

    if settings.RELOCATABLE:
      extra_cflags.append('-fPIC')

    if settings.USE_PTHREADS:
      extra_cflags.append('-pthread')

    if len(extra_cflags):
      cmake_cmd += ['-DCMAKE_CXX_FLAGS="{}"'.format(' '.join(extra_cflags))]
      cmake_cmd += ['-DCMAKE_C_FLAGS="{}"'.format(' '.join(extra_cflags))]

    shared.run_process(cmake_cmd, env=clean_env())
    shared.run_process(['cmake', '--build', dest_path, '--target', 'install'])
    
    ports.install_header_dir(os.path.join(dest_path, 'include', 'freetype2'),
                             target=os.path.join('freetype2', 'freetype'))

    shutil.copyfile(os.path.join(dest_path, 'libfreetype.a'), final)

  return [shared.Cache.get_lib('libfreetype.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libfreetype.a')


def process_args(ports):
  return ['-I' + os.path.join(ports.get_include_dir(), 'freetype2', 'freetype')]


def show():
  return 'freetype (USE_FREETYPE=1; freetype license)'
