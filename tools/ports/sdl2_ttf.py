# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging

TAG = 'version_1'

def get(ports, settings, shared):
  if settings.USE_SDL_TTF == 2:
    ports.fetch_project('sdl2-ttf', 'https://github.com/emscripten-ports/SDL2_ttf/archive/' + TAG + '.zip', 'SDL2_ttf-' + TAG)
    def create():
      sdl_ttf_h = os.path.join(ports.get_dir(), 'sdl2-ttf', 'SDL2_ttf-' + TAG, 'SDL_ttf.h')

      shutil.copy2(sdl_ttf_h, os.path.join(ports.get_build_dir(), 'include'))
      shutil.copy2(sdl_ttf_h, os.path.join(ports.get_build_dir(), 'sdl2', 'include'))
      shutil.copy2(sdl_ttf_h, os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL2'))

      srcs = ['SDL_ttf.c']
      commands = []
      o_s = []

      for src in srcs:
        o = os.path.join(ports.get_build_dir(), 'sdl2-ttf', src + '.o')
        command = [shared.PYTHON, shared.EMCC]
        command += [os.path.join(ports.get_dir(), 'sdl2-ttf', 'SDL2_ttf-' + TAG, src)]
        command += ['-O2', '-s', 'USE_SDL=2', '-s', 'USE_FREETYPE=1', '-o', o, '-w']
        commands.append(command)
        o_s.append(o)

      shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'sdl2-ttf', 'libsdl2_ttf.bc')
      shared.Building.link(o_s, final)
      return final
    return [shared.Cache.get('sdl2-ttf', create, what='port')]
  else:
    return []

def process_dependencies(settings):
  if settings.USE_SDL_TTF == 2:
    settings.USE_SDL = 2
    settings.USE_FREETYPE = 1

def process_args(ports, args, settings, shared):
  if settings.USE_SDL_TTF == 2:
    get(ports, settings, shared)
  return args

def show():
  return 'SDL2_ttf (USE_SDL_TTF=2; zlib license)'
