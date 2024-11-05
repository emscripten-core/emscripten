# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '2b147ffef10ec541d3eace326eafe11a54e635f8'
HASH = 'f39f1f50a039a1667fe92b87d28548d32adcf0eb8526008656de5315039aa21f29d230707caa47f80f6b3a412a577698cd4bbfb9458bb92ac47e6ba993b8efe6'

deps = ['sdl2']


def needed(settings):
  return settings.USE_SDL_GFX == 2


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_gfx'
  ports.fetch_project('sdl2_gfx', f'https://github.com/svn2github/sdl2_gfx/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('sdl2_gfx', 'sdl2_gfx-' + TAG)
    ports.build_port(source_path, final, 'sdl2_gfx', exclude_dirs=['test'], flags=['-sUSE_SDL=2'])
    ports.install_headers(source_path, target='SDL2')

  return [shared.cache.get_lib('libSDL2_gfx.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libSDL2_gfx.a')


def process_dependencies(settings):
  settings.USE_SDL = 2


def show():
  return 'sdl2_gfx (-sUSE_SDL_GFX=2 or --use-port=sdl2_gfx; zlib license)'
