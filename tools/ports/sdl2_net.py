# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_2'
HASH = '317b22ad9b6b2f7b40fac7b7c426da2fa2da1803bbe58d480631f1e5b190d730763f2768c77c72affa806c69a1e703f401b15a1be3ec611cd259950d5ebc3711'

deps = ['sdl2']


def needed(settings):
  return settings.USE_SDL_NET == 2


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_net'
  ports.fetch_project('sdl2_net', f'https://github.com/emscripten-ports/SDL2_net/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    src_dir = ports.get_dir('sdl2_net', 'SDL2_net-' + TAG)
    ports.install_headers(src_dir, target='SDL2')
    excludes = ['chatd.c', 'chat.cpp', 'showinterfaces.c']
    ports.build_port(src_dir, final, 'sdl2_net', exclude_files=excludes, flags=['-sUSE_SDL=2'])

  return [shared.cache.get_lib('libSDL2_net.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libSDL2_net.a')


def process_dependencies(settings):
  settings.USE_SDL = 2


def show():
  return 'sdl2_net (-sUSE_SDL_NET=2 or --use-port=sdl2_net; zlib license)'
