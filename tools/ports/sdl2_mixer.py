# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'release-2.8.0'
HASH = '494ccd74540f74e717f7e4f1dc7f96398c0f4b1883ab00c4a76b0c7239bd2c185cb4358a35ef47819c49e7c14dac7c37b98a29c7b5237478121571f5e7ac4dfc'

deps = ['sdl2']
variants = {
  'sdl2_mixer-mp3': {'SDL2_MIXER_FORMATS': ['mp3']},
  'sdl2_mixer-none': {'SDL2_MIXER_FORMATS': []},
  'sdl2_mixer-mp3-mt': {'SDL2_MIXER_FORMATS': ['mp3'], 'PTHREADS': 1},
  'sdl2_mixer-none-mt': {'SDL2_MIXER_FORMATS': [], 'PTHREADS': 1},
}

OPTIONS = {
  'formats': 'A comma separated list of formats (ex: --use-port=sdl2_mixer:formats=ogg,mp3)',
}

SUPPORTED_FORMATS = {'ogg', 'mp3', 'mod', 'mid'}

# user options (from --use-port)
opts: dict[str, set] = {
  'formats': set(),
}


def needed(settings):
  return settings.USE_SDL_MIXER == 2


def get_formats(settings):
  return opts['formats'].union(settings.SDL2_MIXER_FORMATS)


def get_lib_name(settings):
  formats = '-'.join(sorted(get_formats(settings)))

  libname = 'libSDL2_mixer'
  if formats != '':
    libname += '-' + formats
  if settings.PTHREADS:
    libname += '-mt'
  libname += '.a'

  return libname


def get(ports, settings, shared):
  ports.fetch_project('sdl2_mixer', f'https://github.com/libsdl-org/SDL_mixer/archive/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    source_path = ports.get_dir('sdl2_mixer', 'SDL_mixer-' + TAG)

    formats = get_formats(settings)

    flags = [
      '-sUSE_SDL=2',
      '-DMUSIC_WAV',
    ]

    if "ogg" in formats:
      flags += [
        '-sUSE_VORBIS',
        '-DMUSIC_OGG',
      ]

    if "mp3" in formats:
      flags += [
        '-sUSE_MPG123',
        '-DMUSIC_MP3_MPG123',
      ]

    if "mod" in formats:
      flags += [
        '-sUSE_MODPLUG',
        '-DMUSIC_MOD_MODPLUG',
      ]

    if "mid" in formats:
      flags += [
        '-DMUSIC_MID_TIMIDITY',
      ]

    if settings.PTHREADS:
      flags.append('-pthread')

    include_path = os.path.join(source_path, 'include')
    includes = [
      include_path,
      os.path.join(source_path, 'src'),
      os.path.join(source_path, 'src', 'codecs'),
    ]
    ports.build_port(
      source_path,
      final,
      'sdl2_mixer',
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
  formats = get_formats(settings)
  if "ogg" in formats:
    deps.append('vorbis')
    settings.USE_VORBIS = 1
  if "mp3" in formats:
    deps.append('mpg123')
    settings.USE_MPG123 = 1
  if "mod" in formats:
    deps.append('libmodplug')
    settings.USE_MODPLUG = 1


def handle_options(options, error_handler):
  formats = options['formats'].split(',')
  for format in formats:
    format = format.lower().strip()
    if format not in SUPPORTED_FORMATS:
      error_handler(f'{format} is not a supported format')
    else:
      opts['formats'].add(format)


def show():
  return 'sdl2_mixer (-sUSE_SDL_MIXER=2 or --use-port=sdl2_mixer; zlib license)'
