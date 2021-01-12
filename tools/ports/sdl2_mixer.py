# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

TAG = 'release-2.0.2'
HASH = 'b9d03061d177f20f4e03f3e3553afd7bfe0c05da7b9a774312b389318e747cf9724e0475e9afff6a64ce31bab0217e2afb2619d75556753fbbb6ecafa9775219'


def needed(settings):
  return settings.USE_SDL_MIXER == 2


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_mixer'
  ports.fetch_project('sdl2_mixer', 'https://github.com/emscripten-ports/SDL2_mixer/archive/' + TAG + '.zip', 'SDL2_mixer-' + TAG, sha512hash=HASH)

  settings.SDL2_MIXER_FORMATS.sort()
  formats = '-'.join(settings.SDL2_MIXER_FORMATS)

  libname = 'libSDL2_mixer'
  if formats != '':
    libname += '_' + formats
  libname += '.a'

  def create():
    logging.info('building port: sdl2_mixer')

    source_path = os.path.join(ports.get_dir(), 'sdl2_mixer', 'SDL2_mixer-' + TAG)
    dest_path = os.path.join(ports.get_build_dir(), 'sdl2_mixer')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    flags = [
      '-s', 'USE_SDL=2',
      '-O2',
      '-DMUSIC_WAV',
    ]

    if "ogg" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-s', 'USE_VORBIS=1',
        '-DMUSIC_OGG',
      ]

    if "mp3" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-s', 'USE_MPG123=1',
        '-DMUSIC_MP3_MPG123',
      ]

    final = os.path.join(dest_path, libname)
    ports.build_port(
      dest_path,
      final,
      includes=[],
      flags=flags,
      exclude_files=[
        'playmus.c',
        'playwave.c',
      ],
      exclude_dirs=[
        'native_midi',
        'timidity',
        'external',
      ]
    )

    # copy header to a location so it can be used as 'SDL2/'
    ports.install_headers(source_path, pattern='SDL_*.h', target='SDL2')
    return final

  return [shared.Cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libSDL2_mixer.a')


def process_dependencies(settings):
  global deps
  deps = ['sdl2']
  settings.USE_SDL = 2
  if "ogg" in settings.SDL2_MIXER_FORMATS:
    deps.append('vorbis')
    settings.USE_VORBIS = 1
  if "mp3" in settings.SDL2_MIXER_FORMATS:
    deps.append('mpg123')
    settings.USE_MPG123 = 1


def process_args(ports):
  return []


def show():
  return 'SDL2_mixer (USE_SDL_MIXER=2; zlib license)'
