# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_1'
HASH = '99bee75beb662f8520bbb18ad6dbf8590d30eb3a7360899f0ac4764ca72fe8013da37c9df21e525f9d2dc5632827d4b4cea558cbc938e7fbed0c41a29a7a2dc5'

deps = ['ogg']


def needed(settings):
  return settings.USE_VORBIS


def get(ports, settings, shared):
  ports.fetch_project('vorbis', f'https://github.com/emscripten-ports/vorbis/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('vorbis', 'Vorbis-' + TAG)
    ports.install_headers(os.path.join(source_path, 'include', 'vorbis'), target='vorbis')
    ports.build_port(os.path.join(source_path, 'lib'), final, 'vorbis',
                     flags=['-sUSE_OGG'],
                     exclude_files=['psytune', 'barkmel', 'tone', 'misc'])

  return [shared.cache.get_lib('libvorbis.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libvorbis.a')


def process_dependencies(settings):
  settings.USE_OGG = 1


def show():
  return 'vorbis (-sUSE_VORBIS or --use-port=vorbis; zlib license)'
