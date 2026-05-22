# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = '1.6.56'
HASH = 'e9b7c90e5b29d877e0c0888fe35e5498ae513619943728d7a05269b261786c476808df06de460ec27f6d045cf7193a5e3656b95c553539b4edcdd2fd0c5fa422'

deps = ['zlib']
variants = {
  'libpng-mt': {'PTHREADS': 1},
  'libpng-legacysjlj': {'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 1},
  'libpng-mt-legacysjlj': {'PTHREADS': 1, 'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 1},
}


def needed(settings):
  return settings.USE_LIBPNG


def get_lib_name(settings):
  suffix = ''
  if settings.PTHREADS:
    suffix += '-mt'
  if settings.SUPPORT_LONGJMP == 'wasm':
    suffix += '-legacysjlj'
  return f'libpng{suffix}.a'


def get(ports, settings, shared):
  # This is an emscripten-hosted mirror of the libpng repo from Sourceforge.
  ports.fetch_project('libpng', f'https://storage.googleapis.com/webassembly/emscripten-ports/libpng-{TAG}.tar.gz', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('libpng', 'libpng-' + TAG)
    pnglibconf_h = os.path.join(os.path.dirname(__file__), 'libpng/pnglibconf.h')
    shutil.copyfile(pnglibconf_h, os.path.join(source_path, 'pnglibconf.h'))
    ports.install_headers(source_path)

    flags = ['-sUSE_ZLIB']
    if settings.PTHREADS:
      flags += ['-pthread']
    if settings.SUPPORT_LONGJMP == 'wasm':
      flags.append('-sSUPPORT_LONGJMP=wasm')

    ports.build_port(source_path, final, 'libpng', flags=flags, exclude_files=['pngtest'], exclude_dirs=['scripts', 'contrib'])

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_ZLIB = 1


def show():
  return 'libpng (-sUSE_LIBPNG or --use-port=libpng; zlib license)'
