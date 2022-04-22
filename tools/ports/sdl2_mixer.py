# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

TAG = 'release-2.0.4'
HASH = '5ba387f997219a1deda868f380bf7ee8bc0842261dd54772ad2d560f5282fcbe7bc130e8d16dccc259eeb8cda993a0f34cd3be103fc38f8c6a68428a10e5db4c'

deps = ['sdl2']


def needed(settings):
  return settings.USE_SDL_MIXER == 2


def get_lib_name(settings):
  settings.SDL2_MIXER_FORMATS.sort()
  formats = '-'.join(settings.SDL2_MIXER_FORMATS)

  libname = 'libSDL2_mixer'
  if formats != '':
    libname += '_' + formats
  libname += '.a'

  return libname


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_mixer'
  ports.fetch_project('sdl2_mixer', 'https://github.com/libsdl-org/SDL_mixer/archive/' + TAG + '.zip', 'SDL2_mixer-' + TAG, sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    logging.info('building port: sdl2_mixer')

    source_path = os.path.join(ports.get_dir(), 'sdl2_mixer', 'SDL_mixer-' + TAG)
    dest_path = os.path.join(ports.get_build_dir(), 'sdl2_mixer')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    flags = [
      '-sUSE_SDL=2',
      '-O2',
      '-DMUSIC_WAV',
    ]

    if "ogg" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-sUSE_VORBIS=1',
        '-DMUSIC_OGG',
      ]

    if "mp3" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-sUSE_MPG123=1',
        '-DMUSIC_MP3_MPG123',
      ]

    if "mod" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-sUSE_MODPLUG=1',
        '-DMUSIC_MOD_MODPLUG',
      ]

    if "mid" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-DMUSIC_MID_TIMIDITY',
      ]

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
        'external',
      ]
    )

    # copy header to a location so it can be used as 'SDL2/'
    ports.install_headers(source_path, pattern='SDL_*.h', target='SDL2')

  return [shared.Cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 2
  if "ogg" in settings.SDL2_MIXER_FORMATS:
    deps.append('vorbis')
    settings.USE_VORBIS = 1
  if "mp3" in settings.SDL2_MIXER_FORMATS:
    deps.append('mpg123')
    settings.USE_MPG123 = 1
  if "mod" in settings.SDL2_MIXER_FORMATS:
    deps.append('libmodplug')
    settings.USE_MODPLUG = 1


def process_args(ports):
  return []


def show():
  return 'SDL2_mixer (USE_SDL_MIXER=2; zlib license)'
