# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '38fcb695276ed794f879d5d9c5ef4e5286a5200d' # Latest as of 24 November 2020
HASH = '4c1ac5d27439d28c6d84593dd15dd80c825d68c6bf1020ab4317f2bce1efe16401b5b3280a181047c8317c38a19bbeeae8d52862e6b2c9776d5809758ee7aaa6'

deps = ['freetype', 'sdl2', 'harfbuzz']


def needed(settings):
  return settings.USE_SDL_TTF == 2


def get(ports, settings, shared):
  ports.fetch_project('sdl2_ttf', 'https://github.com/libsdl-org/SDL_ttf/archive/' + TAG + '.zip', 'SDL_ttf-' + TAG, sha512hash=HASH)

  def create(final):
    src_root = os.path.join(ports.get_dir(), 'sdl2_ttf', 'SDL_ttf-' + TAG)
    ports.install_headers(src_root, target='SDL2')

    srcs = ['SDL_ttf.c']
    commands = []
    o_s = []

    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'sdl2_ttf', src + '.o')
      command = [shared.EMCC,
                 '-c', os.path.join(src_root, src),
                 '-O2', '-DTTF_USE_HARFBUZZ=1', '-s', 'USE_SDL=2', '-s', 'USE_FREETYPE=1', '-s', 'USE_HARFBUZZ=1', '-o', o, '-w']
      commands.append(command)
      o_s.append(o)

    shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
    ports.run_commands(commands)
    ports.create_lib(final, o_s)

  return [shared.Cache.get_lib('libSDL2_ttf.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_lib('libSDL2_ttf.a')


def process_dependencies(settings):
  settings.USE_SDL = 2
  settings.USE_FREETYPE = 1
  settings.USE_HARFBUZZ = 1


def process_args(ports):
  return ['-DTTF_USE_HARFBUZZ=1']


def show():
  return 'SDL2_ttf (USE_SDL_TTF=2; zlib license)'
