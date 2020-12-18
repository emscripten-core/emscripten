# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '703ebc7c66fd' # Latest as of 24 November 2020
HASH = '362c95f08170a90fe026b264a1053692edea581f5a9908c84aa42d89315cf8c8f74719a80ae45fa16ed7944b12e1b930e5add1d3ad58de04012ed63f535dfb04'

deps = ['freetype', 'sdl2', 'harfbuzz']


def needed(settings):
  return settings.USE_SDL_TTF == 2


def get(ports, settings, shared):
  ports.fetch_project('sdl2_ttf', 'https://hg.libsdl.org/SDL_ttf/archive/' + TAG + '.zip', 'SDL_ttf-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libSDL2_ttf')

  def create():
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
    final = os.path.join(ports.get_build_dir(), 'sdl2_ttf', libname)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file(ports.get_lib_name('libSDL2_ttf'))


def process_dependencies(settings):
  settings.USE_SDL = 2
  settings.USE_FREETYPE = 1
  settings.USE_HARFBUZZ = 1


def process_args(ports):
  return ['-DTTF_USE_HARFBUZZ=1']


def show():
  return 'SDL2_ttf (USE_SDL_TTF=2; zlib license)'
