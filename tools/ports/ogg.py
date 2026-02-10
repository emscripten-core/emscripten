# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '1.3.5'
HASH = 'e7b5f5d469090b66dbb33634591ae6de41af3a5644c10488e59ba7428ed78912e208a2c0fbb5003ec5b7eb2a0102a2f85cecb21fa9512d790139ecc45b6d03f4'


def needed(settings):
  return settings.USE_OGG


def get(ports, settings, shared):
  ports.fetch_project('ogg', f'https://github.com/xiph/ogg/releases/download/v{VERSION}/libogg-{VERSION}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('ogg', 'libogg-' + VERSION)
    config_types_h = os.path.join(os.path.dirname(__file__), 'ogg/config_types.h')
    shutil.copyfile(config_types_h, os.path.join(source_path, 'include/ogg/config_types.h'))
    ports.install_headers(os.path.join(source_path, 'include', 'ogg'), target='ogg')
    ports.make_pkg_config('ogg', VERSION, '-sUSE_OGG')
    ports.build_port(os.path.join(source_path, 'src'), final, 'ogg')

  return [shared.cache.get_lib('libogg.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libogg.a')


def show():
  return 'ogg (-sUSE_OGG or --use-port=ogg; zlib license)'
