# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-2.8.0'
HASH = '494ccd74540f74e717f7e4f1dc7f96398c0f4b1883ab00c4a76b0c7239bd2c185cb4358a35ef47819c49e7c14dac7c37b98a29c7b5237478121571f5e7ac4dfc'

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
    source_path = ports.get_dir('sdl2_mixer', 'SDL_mixer-' + TAG)
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
    include_path = os.path.join(source_path, 'include')
    includes = [
      include_path,
      os.path.join(source_path, 'src'),
      os.path.join(source_path, 'src', 'codecs')
    ]
    ports.build_port(
      source_path,
      final,
      build_dir,
      flags=flags,
      exclude_files=[
        'playmus.c',
        'playwave.c',
        'main.c',
      ],
      exclude_dirs=[
        'native_midi',
        'external',
        'Xcode',
      ],
      includes=includes,
    )

    ports.install_headers(include_path, target='SDL2')

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


def show():
  return 'sdl2_mixer (-sUSE_SDL_MIXER=2 or --use-port=sdl2_mixer; zlib license)'
