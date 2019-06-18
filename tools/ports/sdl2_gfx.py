# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

TAG = '2b147ffef10ec541d3eace326eafe11a54e635f8'
HASH = 'f39f1f50a039a1667fe92b87d28548d32adcf0eb8526008656de5315039aa21f29d230707caa47f80f6b3a412a577698cd4bbfb9458bb92ac47e6ba993b8efe6'


def get(ports, settings, shared):
  if settings.USE_SDL_GFX != 2:
    return []

  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_gfx'
  ports.fetch_project('sdl2_gfx', 'https://github.com/svn2github/sdl2_gfx/archive/' + TAG + '.zip', 'sdl2_gfx-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libSDL2_gfx')

  def create():
    logging.info('building port: sdl2_gfx')

    source_path = os.path.join(ports.get_dir(), 'sdl2_gfx', 'sdl2_gfx-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2_gfx')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    for header in ['SDL2_framerate.h', 'SDL2_gfxPrimitives_font.h', 'SDL2_gfxPrimitives.h', 'SDL2_imageFilter.h', 'SDL2_rotozoom.h']:
      shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2_gfx', 'sdl2_gfx-' + TAG, header), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL2', header))

    final = os.path.join(dest_path, libname)
    ports.build_port(dest_path, final, [dest_path], exclude_dirs=['test'])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libSDL2_gfx'))


def process_args(ports, args, settings, shared):
  if settings.USE_SDL_GFX == 2:
    get(ports, settings, shared)
  return args


def show():
  return 'SDL2_gfx (zlib license)'
