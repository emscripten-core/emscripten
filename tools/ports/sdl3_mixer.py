# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

from typing import Dict, Set

VERSION = '3.2.0'
TAG = f'release-{VERSION}'
HASH = '96f374b3ca96202973fca84228e7775db3d6e38888888573d0ba0d045bc1d3cc6f876984e50dcce1b65875c80f8e263b5ff687570f4b4c720f48ca3cfaff0648'
SUBDIR = f'SDL3_mixer-{TAG}'

deps = ['sdl3']

variants = {'sdl3_mixer-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SDL_MIXER == 3


def get_lib_name(settings):
  return 'libSDL3_mixer' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('sdl3_mixer', f'https://github.com/libsdl-org/SDL_mixer/archive/refs/tags/{TAG}.zip', sha512hash=HASH)

  def create(final):
    src_root = ports.get_dir('sdl3_mixer', 'SDL_mixer-' + TAG)
    ports.install_header_dir(os.path.join(src_root, 'include'), target='.')
    srcs = [
	    "src/SDL_mixer.c",
	    "src/SDL_mixer_metadata_tags.c",
	    "src/SDL_mixer_spatialization.c",
	    "src/decoder_aiff.c",
	    "src/decoder_au.c",
	    "src/decoder_drflac.c",
	    "src/decoder_drmp3.c",
	    "src/decoder_flac.c",
	    "src/decoder_fluidsynth.c",
	    "src/decoder_gme.c",
	    "src/decoder_mpg123.c",
	    "src/decoder_opus.c",
	    "src/decoder_raw.c",
	    "src/decoder_sinewave.c",
	    "src/decoder_stb_vorbis.c",
	    "src/decoder_timidity.c",
	    "src/decoder_voc.c",
	    "src/decoder_vorbis.c",
	    "src/decoder_wav.c",
	    "src/decoder_wavpack.c",
	    "src/decoder_xmp.c",
      ]

    flags = ['-sUSE_SDL=3', '-DDECODER_WAV','-Wno-format-security', '-Wno-experimental']

    if settings.PTHREADS:
      flags += ['-pthread']

    ports.build_port(src_root, final, 'sdl3_mixer', flags=flags, srcs=srcs)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 3


def show():
  return 'sdl3_mixer (-sUSE_SDL_MIXER=3 or --use-port=sdl3_mixer; zlib license)'