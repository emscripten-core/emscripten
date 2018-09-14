# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging

TAG = 'version_2'

def get(ports, settings, shared):
  if settings.USE_SDL_NET == 2:
    sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
    assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_net'
    ports.fetch_project('sdl2-net', 'https://github.com/emscripten-ports/SDL2_net/archive/' + TAG + '.zip', 'SDL2_net-' + TAG)
    def create():
      logging.info('building port: sdl2-net')
      shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2-net', 'SDL2_net-' + TAG, 'SDL_net.h'), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL_net.h'))
      shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2-net', 'SDL2_net-' + TAG, 'SDL_net.h'), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL2', 'SDL_net.h'))
      srcs = 'SDLnet.c SDLnetselect.c SDLnetTCP.c SDLnetUDP.c'.split(' ')
      commands = []
      o_s = []
      for src in srcs:
        o = os.path.join(ports.get_build_dir(), 'sdl2-net', src + '.o')
        commands.append([shared.PYTHON, shared.EMCC, os.path.join(ports.get_dir(), 'sdl2-net', 'SDL2_net-' + TAG, src), '-O2', '-s', 'USE_SDL=2', '-o', o, '-w'])
        o_s.append(o)
      shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'sdl2-net', 'libsdl2_net.bc')
      shared.Building.link(o_s, final)
      return final
    return [shared.Cache.get('sdl2-net', create, what='port')]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_SDL_NET == 2:
    get(ports, settings, shared)
  return args

def show():
  return 'SDL2_net (zlib license)'

