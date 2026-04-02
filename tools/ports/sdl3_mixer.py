# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

VERSION = '3.2.0'
TAG = f'release-{VERSION}'
HASH = '96f374b3ca96202973fca84228e7775db3d6e38888888573d0ba0d045bc1d3cc6f876984e50dcce1b65875c80f8e263b5ff687570f4b4c720f48ca3cfaff0648'
SUBDIR = f'SDL3_mixer-{TAG}'

deps = ['sdl3']

variants = {
  'sdl3_mixer-ogg': {'SDL3_MIXER_FORMATS': ['ogg']},
  'sdl3_mixer-none': {'SDL3_MIXER_FORMATS': []},
  'sdl3_mixer-ogg-mt': {'SDL3_MIXER_FORMATS': ['ogg'], 'PTHREADS': 1},
  'sdl3_mixer-none-mt': {'SDL3_MIXER_FORMATS': [], 'PTHREADS': 1},
}

OPTIONS = {
  'formats': 'A comma separated list of formats (ex: --use-port=sdl3_mixer:formats=ogg,mp3)',
}

SUPPORTED_FORMATS = {'ogg', 'mp3'}

# user options (from --use-port)
opts: dict[str, set] = {
  'formats': set(),
}


def needed(settings):
  return settings.USE_SDL_MIXER == 3


def get_formats(settings):
  return opts['formats'].union(settings.SDL3_MIXER_FORMATS)


def get_lib_name(settings):
  formats = '-'.join(sorted(get_formats(settings)))

  libname = 'libSDL3_mixer'
  if formats != '':
    libname += '-' + formats
  if settings.PTHREADS:
    libname += '-mt'
  libname += '.a'

  return libname


def get(ports, settings, shared):
  ports.fetch_project('sdl3_mixer', f'https://github.com/libsdl-org/SDL_mixer/archive/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    src_root = ports.get_dir('sdl3_mixer', 'SDL_mixer-' + TAG)
    ports.install_header_dir(os.path.join(src_root, 'include'), target='.')
    srcs = [
	    'src/SDL_mixer.c',
	    'src/SDL_mixer_metadata_tags.c',
	    'src/SDL_mixer_spatialization.c',
	    'src/decoder_raw.c',
	    'src/decoder_sinewave.c',
	    'src/decoder_wav.c',
      ]

    flags = ['-sUSE_SDL=3', '-DDECODER_WAV','-Wno-format-security', '-Wno-experimental']

    if settings.PTHREADS:
      flags += ['-pthread']

    formats = get_formats(settings)

    if 'ogg' in formats:
      flags += [
        '-sUSE_VORBIS',
        '-DDECODER_OGGVORBIS_VORBISFILE',
      ]
      srcs += ['src/decoder_vorbis.c',]

    if 'mp3' in formats:
      flags += [
        '-sUSE_MPG123',
        '-DDECODER_MP3_MPG123',
      ]
      srcs += ['src/decoder_mpg123.c',]

    ports.build_port(src_root, final, 'sdl3_mixer', flags=flags, srcs=srcs)
  return [shared.cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 3
  formats = get_formats(settings)
  if 'ogg' in formats:
    deps.append('vorbis')
    settings.USE_VORBIS = 1
  if 'mp3' in formats:
    deps.append('mpg123')
    settings.USE_MPG123 = 1


def handle_options(options, error_handler):
  formats = options['formats'].split(',')
  for format in formats:
    format = format.lower().strip()
    if format not in SUPPORTED_FORMATS:
      error_handler(f'{format} is not a supported format')
    else:
      opts['formats'].add(format)


def show():
  return 'sdl3_mixer (-sUSE_SDL_MIXER=3 or --use-port=sdl3_mixer; zlib license)'
