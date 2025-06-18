# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-3.2.2'
HASH = 'c5f34d1b79492e0341c91687cde9ec315f5d6544c7ebaa7ef5d092e77ccfc363a0e702ba9c43bfa0926c54420843ccfb98b81362985fd7b4a67d09a7852b90ba'

deps = ['freetype', 'sdl3', 'harfbuzz']

variants = {'sdl3_ttf-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SDL_TTF == 3


def get_lib_name(settings):
  return 'libSDL3_ttf' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('sdl3_ttf', f'https://github.com/libsdl-org/SDL_ttf/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    src_root = ports.get_dir('sdl3_ttf', 'SDL_ttf-' + TAG)
    ports.install_header_dir(os.path.join(src_root, 'include'), target='.')
    flags = ['-DTTF_USE_HARFBUZZ=1', '-sUSE_SDL=3', '-sUSE_FREETYPE', '-sUSE_HARFBUZZ']
    if settings.PTHREADS:
      flags += ['-pthread']

    srcs = [
      'src/SDL_gpu_textengine.c',
      'src/SDL_hashtable.c',
      'src/SDL_hashtable_ttf.c',
      'src/SDL_renderer_textengine.c',
      'src/SDL_surface_textengine.c',
      'src/SDL_ttf.c',
    ]

    ports.build_port(src_root, final, 'sdl3_ttf', flags=flags, srcs=srcs)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 3
  settings.USE_FREETYPE = 1
  settings.USE_HARFBUZZ = 1


def process_args(ports):
  return ['-DTTF_USE_HARFBUZZ=1']


def show():
  return 'sdl3_ttf (-sUSE_SDL_TTF=3 or --use-port=sdl3_ttf; zlib license)'
