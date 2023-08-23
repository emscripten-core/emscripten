# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-2.0.4'
HASH = '5ba387f997219a1deda868f380bf7ee8bc0842261dd54772ad2d560f5282fcbe7bc130e8d16dccc259eeb8cda993a0f34cd3be103fc38f8c6a68428a10e5db4c'

deps = ['sdl2']
variants = {
  'sdl2_mixer_mp3': {'SDL2_MIXER_FORMATS': ["mp3"]},
  'sdl2_mixer_none': {'SDL2_MIXER_FORMATS': []},
}


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
  ports.fetch_project('sdl2_mixer', f'https://github.com/libsdl-org/SDL_mixer/archive/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    source_path = os.path.join(ports.get_dir(), 'sdl2_mixer', 'SDL_mixer-' + TAG)
    flags = [
      '-sUSE_SDL=2',
      '-O2',
      '-DMUSIC_WAV',
    ]

    if "ogg" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-sUSE_VORBIS',
        '-DMUSIC_OGG',
      ]

    if "mp3" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-Wno-incompatible-function-pointer-types',
        '-sUSE_MPG123',
        '-DMUSIC_MP3_MPG123',
      ]

    if "mod" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-sUSE_MODPLUG',
        '-DMUSIC_MOD_MODPLUG',
      ]

    if "mid" in settings.SDL2_MIXER_FORMATS:
      flags += [
        '-DMUSIC_MID_TIMIDITY',
      ]

    build_dir = ports.clear_project_build('sdl2_mixer')
    ports.build_port(
      source_path,
      final,
      build_dir,
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

  return [shared.cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


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
