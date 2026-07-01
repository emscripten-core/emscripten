# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

VERSION = '1.3.7'
HASH = '10edd193f44f2b2d6085672e257b5be11066f5db27348d4cab53789bf9e8739c2df2cd7f42b83e4edbab53e10b5dc84320d6aab1deac0d8e18196a8a88b19b77'

deps = ['ogg']


def needed(settings):
  return settings.USE_VORBIS


def get(ports, settings, shared):
  ports.fetch_project('vorbis', f'https://github.com/xiph/vorbis/releases/download/v{VERSION}/libvorbis-{VERSION}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('vorbis', 'libvorbis-' + VERSION)
    ports.install_headers(os.path.join(source_path, 'include', 'vorbis'), target='vorbis')
    ports.make_pkg_config('vorbis', VERSION, '-sUSE_VORBIS')
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
