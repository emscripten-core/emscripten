# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

TAG = 'release-2.0.1'
HASH = '81fac757bd058adcb3eb5b2cc46addeaa44cee2cd4db653dad5d9666bdc0385cdc21bf5b72872e6dd6dd8eb65812a46d7752298827d6c61ad5ce2b6c963f7ed0'


def get(ports, settings, shared):
  if settings.USE_SDL_MIXER != 2:
    return []

  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_mixer'
  ports.fetch_project('sdl2_mixer', 'https://github.com/emscripten-ports/SDL2_mixer/archive/' + TAG + '.zip', 'SDL2_mixer-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libSDL2_mixer')

  def create():
    logging.info('building port: sdl2_mixer')

    source_path = os.path.join(ports.get_dir(), 'sdl2_mixer', 'SDL2_mixer-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2_mixer')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)
    os.symlink(dest_path, os.path.join(dest_path, 'SDL2'))

    final = os.path.join(dest_path, libname)
    ports.build_port(os.path.join(dest_path, 'src'), final, [], ['-DOGG_MUSIC'], ['dynamic_flac', 'dynamic_fluidsynth', 'dynamic_mod', 'dynamic_modplug', 'dynamic_mp3',
                                                                     'dynamic_ogg', 'fluidsynth', 'load_mp3', 'music_cmd', 'music_flac', 'music_mad',
                                                                     'music_mod', 'music_modplug'])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libSDL2_mixer'))


def process_dependencies(settings):
  if settings.USE_SDL_MIXER == 2:
    settings.USE_SDL = 2
    settings.USE_VORBIS = 1


def process_args(ports, args, settings, shared):
  if settings.USE_SDL_MIXER == 2:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2_mixer')]
  return args


def show():
  return 'SDL2_mixer (USE_SDL_MIXER=2; zlib license)'
