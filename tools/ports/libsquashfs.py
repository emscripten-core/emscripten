# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = '1.3.2'
HASH = '4a3a194af80aa9ed689cf541106906945f546fa15a5b30feff9df95998105298aa919757b1f17bf8387da0bdb05b70857ce818ce8572411cd5ef25e2b93c2022'

deps = ['zlib']
#variants = {
#  'libpng-mt': {'PTHREADS': 1},
#  'libpng-legacysjlj': {'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 1},
#  'libpng-mt-legacysjlj': {'PTHREADS': 1, 'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 1},
#}

def needed(settings):
  return settings.USE_LIBSQUASHFS



def get(ports, settings, shared):
  # TODO: This is an emscripten-hosted mirror of the libsquashfs repo from Sourceforge.
  ports.fetch_project('libsquashfs', f'https://infraroot.at/pub/squashfs/squashfs-tools-ng-{TAG}.tar.gz', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('libsquashfs', 'squashfs-tools-ng-' + TAG)
    squashfsconf_h = os.path.join(os.path.dirname(__file__), 'libsquashfs/config.h')
    shutil.copyfile(squashfsconf_h, os.path.join(source_path, 'config.h'))
    ports.install_headers(os.path.join(source_path,'include','sqfs'), target='sqfs')
    flags = ['-sUSE_ZLIB', '-DWITH_GZIP', '-Wno-error=incompatible-pointer-types', '-Wno-error=format', '-D_GNU_SOURCE']
    ports.make_pkg_config('libsquashfs', TAG, flags)
    includes = [ os.path.join(source_path, 'include')]
    exclude_dirs = ['bin', 'extras', 'common', 'compat', 'fstree', 'io', 'tar', 'win32', 'gensquashfs', 'libio', 'libtar']
    exclude_files = ['lz4.c', 'lzma.c', 'xz.c', 'zstd.c']



#    ports.install_headers(os.path.join(source_path, 'include', 'ogg'), target='ogg')
#    ports.make_pkg_config('ogg', TAG, '-sUSE_OGG')
#    ports.build_port(os.path.join(source_path, 'src'), final, 'ogg')


#    ports.build_port(source_path, final, 'libpng', flags=flags, exclude_files=['pngtest'], exclude_dirs=['scripts', 'contrib'])
    ports.build_port(source_path, final, 'libsquashfs', flags=flags, includes=includes, exclude_dirs=exclude_dirs, exclude_files=exclude_files)

  return [shared.cache.get_lib('libsquashfs.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libsquashfs.a')


def process_dependencies(settings):
  settings.USE_ZLIB = 1


def show():
  return 'libsquashfs (-sUSE_LIBSQUASHFS or --use-port=libsquashfs; LGPL-3.0-or-later license)'
