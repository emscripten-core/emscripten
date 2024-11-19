# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

TAG = 'release-2.20.2' # Latest as of 21 February 2023
HASH = '8a625d29bef2ab7cbfe2143136a303c0fdb066ecd802d6c725de1b73ad8b056908cb524fe58f38eaee9f105471d2af50bbcb17911d46506dbcf573db218b3685'

deps = ['freetype', 'sdl2', 'harfbuzz']

variants = {'sdl2_ttf-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SDL_TTF == 2


def get_lib_name(settings):
  return 'libSDL2_ttf' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('sdl2_ttf', f'https://github.com/libsdl-org/SDL_ttf/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    src_root = ports.get_dir('sdl2_ttf', 'SDL_ttf-' + TAG)
    ports.install_headers(src_root, target='SDL2')
    flags = ['-DTTF_USE_HARFBUZZ=1', '-sUSE_SDL=2', '-sUSE_FREETYPE', '-sUSE_HARFBUZZ']
    if settings.PTHREADS:
      flags += ['-pthread']
    ports.build_port(src_root, final, 'sdl2_ttf', flags=flags, srcs=['SDL_ttf.c'])

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 2
  settings.USE_FREETYPE = 1
  settings.USE_HARFBUZZ = 1


def process_args(ports):
  return ['-DTTF_USE_HARFBUZZ=1']


def show():
  return 'sdl2_ttf (-sUSE_SDL_TTF=2 or --use-port=sdl2_ttf; zlib license)'
