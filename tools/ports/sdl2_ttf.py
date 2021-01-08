# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_1'
HASH = '6ce426de0411ba51dd307027c4ef00ff3de4ee396018e524265970039132ab20adb29c2d2e61576c393056374f03fd148dd96f0c4abf8dcee51853dd32f0778f'

deps = ['freetype', 'sdl2']


def needed(settings):
  return settings.USE_SDL_TTF == 2


def get(ports, settings, shared):
  ports.fetch_project('sdl2_ttf', 'https://github.com/emscripten-ports/SDL2_ttf/archive/' + TAG + '.zip', 'SDL2_ttf-' + TAG, sha512hash=HASH)
  libname = 'libSDL2_ttf.a'

  def create():
    src_root = os.path.join(ports.get_dir(), 'sdl2_ttf', 'SDL2_ttf-' + TAG)
    ports.install_headers(src_root, target='SDL2')

    srcs = ['SDL_ttf.c']
    commands = []
    o_s = []

    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'sdl2_ttf', src + '.o')
      command = [shared.EMCC,
                 '-c', os.path.join(src_root, src),
                 '-O2', '-s', 'USE_SDL=2', '-s', 'USE_FREETYPE=1', '-o', o, '-w']
      commands.append(command)
      o_s.append(o)

    shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'sdl2_ttf', libname)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libSDL2_ttf.a')


def process_dependencies(settings):
  settings.USE_SDL = 2
  settings.USE_FREETYPE = 1


def process_args(ports):
  return []


def show():
  return 'SDL2_ttf (USE_SDL_TTF=2; zlib license)'
