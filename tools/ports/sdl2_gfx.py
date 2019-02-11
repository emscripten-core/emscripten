# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging

TAG = '2b147ffef10ec541d3eace326eafe11a54e635f8'

def get(ports, settings, shared):
  if settings.USE_SDL_GFX == 2:
    sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
    assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_gfx'
    ports.fetch_project('sdl2-gfx', 'https://github.com/svn2github/sdl2_gfx/archive/' + TAG + '.zip', 'sdl2_gfx-' + TAG)
    def create():
      logging.info('building port: sdl2-gfx')

      source_path = os.path.join(ports.get_dir(), 'sdl2-gfx', 'sdl2_gfx-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2-gfx')

      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)

      for header in ['SDL2_framerate.h', 'SDL2_gfxPrimitives_font.h', 'SDL2_gfxPrimitives.h', 'SDL2_imageFilter.h', 'SDL2_rotozoom.h']:
        shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2-gfx', 'sdl2_gfx-' + TAG, header), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL2', header))

      final = os.path.join(dest_path, 'libsdl2_gfx.bc')
      ports.build_port(dest_path, final, [dest_path], exclude_dirs=['test'])
      return final
    return [shared.Cache.get('sdl2-gfx', create)]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_SDL_GFX == 2:
    get(ports, settings, shared)
  return args

def show():
  return 'SDL2_gfx (zlib license)'

