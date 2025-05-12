# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = 'version_1'
HASH = '929e8d6003c06ae09593021b83323c8f1f54532b67b8ba189f4aedce52c25dc182bac474de5392c46ad5b0dea5a24928e4ede1492d52f4dd5cd58eea9be4dba7'


def needed(settings):
  return settings.USE_OGG


def get(ports, settings, shared):
  ports.fetch_project('ogg', f'https://github.com/emscripten-ports/ogg/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('ogg', 'Ogg-' + TAG)
    config_types_h = os.path.join(os.path.dirname(__file__), 'ogg/config_types.h')
    shutil.copyfile(config_types_h, os.path.join(source_path, 'include/ogg/config_types.h'))
    ports.install_headers(os.path.join(source_path, 'include', 'ogg'), target='ogg')
    ports.build_port(os.path.join(source_path, 'src'), final, 'ogg')

  return [shared.cache.get_lib('libogg.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libogg.a')


def show():
  return 'ogg (-sUSE_OGG=1 or --use-port=ogg; zlib license)'
