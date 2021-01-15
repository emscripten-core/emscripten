#!/usr/bin/env python3
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool to manage building of system libraries and ports.

In general emcc will build them automatically on demand, so you do not
strictly need to use this tool, but it gives you more control over the
process (in particular, if emcc does this automatically, and you are
running multiple build commands in parallel, confusion can occur).
"""

import argparse
import logging
import sys

from tools import shared
from tools import system_libs
import emscripten


SYSTEM_LIBRARIES = system_libs.Library.get_all_variations()
SYSTEM_TASKS = list(SYSTEM_LIBRARIES.keys())

# This is needed to build the generated_struct_info.json file.
# It is not a system library, but it needs to be built before running with FROZEN_CACHE.
SYSTEM_TASKS += ['struct_info']

# Minimal subset of SYSTEM_TASKS used by CI systems to build enough to useful
MINIMAL_TASKS = [
    'libcompiler_rt',
    'libc',
    'libc++abi',
    'libc++abi-except',
    'libc++abi-noexcept',
    'libc++',
    'libc++-except',
    'libc++-noexcept',
    'libal',
    'libdlmalloc',
    'libdlmalloc-debug',
    'libemmalloc',
    'libemmalloc-64bit',
    'libsockets',
    'libc_rt_wasm',
    'struct_info',
    'libstandalonewasm',
    'crt1',
    'libunwind-except'
]

USER_TASKS = [
    'boost_headers',
    'bullet',
    'bzip2',
    'cocos2d',
    'freetype',
    'giflib',
    'harfbuzz',
    'icu',
    'libjpeg',
    'libpng',
    'ogg',
    'regal',
    'regal-mt',
    'sdl2',
    'sdl2-mt',
    'sdl2-gfx',
    'sdl2-image',
    'sdl2-image-png',
    'sdl2-image-jpg',
    'sdl2-mixer',
    'sdl2-mixer-ogg',
    'sdl2-mixer-mp3',
    'sdl2-net',
    'sdl2-ttf',
    'vorbis',
    'zlib',
]

temp_files = shared.configuration.get_temp_files()
logger = logging.getLogger('embuilder')
force = False


def get_help():
  all_tasks = SYSTEM_TASKS + USER_TASKS
  all_tasks.sort()
  return '''
Available targets:

  build %s

Issuing 'embuilder.py build ALL' causes each task to be built.
''' % '\n        '.join(all_tasks)


def build_port(port_name, lib_name):
  if force:
    shared.Cache.erase_file(lib_name)

  system_libs.build_port(port_name, shared.Settings)


def main():
  global force
  parser = argparse.ArgumentParser(description=__doc__,
                                   formatter_class=argparse.RawDescriptionHelpFormatter,
                                   epilog=get_help())
  parser.add_argument('--lto', action='store_true', help='build bitcode object for LTO')
  parser.add_argument('--pic', action='store_true',
                      help='build relocatable objects for suitable for dynamic linking')
  parser.add_argument('--force', action='store_true',
                      help='force rebuild of target (by removing it first)')
  parser.add_argument('operation', help='currently only "build" is supported')
  parser.add_argument('targets', nargs='+', help='see below')
  args = parser.parse_args()

  if args.operation != 'build':
    shared.exit_with_error('unfamiliar operation: ' + args.operation)

  # process flags

  # Check sanity so that if settings file has changed, the cache is cleared here.
  # Otherwise, the cache will clear in an emcc process, which is invoked while building
  # a system library into the cache, causing trouble.
  shared.check_sanity()

  if args.lto:
    shared.Settings.LTO = "full"

  if args.pic:
    shared.Settings.RELOCATABLE = 1

  if args.force:
    force = True

  # process tasks
  auto_tasks = False
  tasks = args.targets
  if 'SYSTEM' in tasks:
    tasks = SYSTEM_TASKS
    auto_tasks = True
  elif 'USER' in tasks:
    tasks = USER_TASKS
    auto_tasks = True
  elif 'MINIMAL' in tasks:
    tasks = MINIMAL_TASKS
    auto_tasks = True
  elif 'ALL' in tasks:
    tasks = SYSTEM_TASKS + USER_TASKS
    auto_tasks = True
  if auto_tasks:
    # cocos2d: must be ported, errors on
    # "Cannot recognize the target platform; are you targeting an unsupported platform?"
    skip_tasks = ['cocos2d']
    tasks = [x for x in tasks if x not in skip_tasks]
    print('Building targets: %s' % ' '.join(tasks))
  for what in tasks:
    logger.info('building and verifying ' + what)
    if what in SYSTEM_LIBRARIES:
      library = SYSTEM_LIBRARIES[what]
      if force:
        library.erase()
      library.get_path()
    elif what == 'sysroot':
      if force:
        shared.Cache.erase_file('sysroot_install.stamp')
      system_libs.ensure_sysroot()
    elif what == 'struct_info':
      if force:
        shared.Cache.erase_file('generated_struct_info.json')
      emscripten.generate_struct_info()
    elif what == 'icu':
      build_port('icu', 'libicuuc.a')
    elif what == 'zlib':
      shared.Settings.USE_ZLIB = 1
      build_port('zlib', 'libz.a')
      shared.Settings.USE_ZLIB = 0
    elif what == 'bzip2':
      build_port('bzip2', 'libbz2.a')
    elif what == 'bullet':
      build_port('bullet', 'libbullet.a')
    elif what == 'vorbis':
      build_port('vorbis', 'libvorbis.a')
    elif what == 'ogg':
      build_port('ogg', 'libogg.a')
    elif what == 'giflib':
      build_port('giflib', 'libgif.a')
    elif what == 'libjpeg':
      build_port('libjpeg', 'libjpeg.a')
    elif what == 'libpng':
      build_port('libpng', 'libpng.a')
    elif what == 'sdl2':
      build_port('sdl2', 'libSDL2.a')
    elif what == 'sdl2-mt':
      shared.Settings.USE_PTHREADS = 1
      build_port('sdl2', 'libSDL2-mt.a')
      shared.Settings.USE_PTHREADS = 0
    elif what == 'sdl2-gfx':
      build_port('sdl2_gfx', 'libSDL2_gfx.a')
    elif what == 'sdl2-image':
      build_port('sdl2_image', 'libSDL2_image.a')
    elif what == 'sdl2-image-png':
      shared.Settings.SDL2_IMAGE_FORMATS = ["png"]
      build_port('sdl2_image', 'libSDL2_image_png.a')
      shared.Settings.SDL2_IMAGE_FORMATS = []
    elif what == 'sdl2-image-jpg':
      shared.Settings.SDL2_IMAGE_FORMATS = ["jpg"]
      build_port('sdl2_image', 'libSDL2_image_jpg.a')
      shared.Settings.SDL2_IMAGE_FORMATS = []
    elif what == 'sdl2-net':
      build_port('sdl2_net', 'libSDL2_net.a')
    elif what == 'sdl2-mixer':
      old_formats = shared.Settings.SDL2_MIXER_FORMATS
      shared.Settings.SDL2_MIXER_FORMATS = []
      build_port('sdl2_mixer', 'libSDL2_mixer.a')
      shared.Settings.SDL2_MIXER_FORMATS = old_formats
    elif what == 'sdl2-mixer-ogg':
      old_formats = shared.Settings.SDL2_MIXER_FORMATS
      shared.Settings.SDL2_MIXER_FORMATS = ["ogg"]
      build_port('sdl2_mixer', 'libSDL2_mixer_ogg.a')
      shared.Settings.SDL2_MIXER_FORMATS = old_formats
    elif what == 'sdl2-mixer-mp3':
      old_formats = shared.Settings.SDL2_MIXER_FORMATS
      shared.Settings.SDL2_MIXER_FORMATS = ["mp3"]
      build_port('sdl2_mixer', 'libSDL2_mixer_mp3.a')
      shared.Settings.SDL2_MIXER_FORMATS = old_formats
    elif what == 'freetype':
      build_port('freetype', 'libfreetype.a')
    elif what == 'harfbuzz':
      build_port('harfbuzz', 'libharfbuzz.a')
    elif what == 'harfbuzz-mt':
      shared.Settings.USE_PTHREADS = 1
      build_port('harfbuzz', 'libharfbuzz-mt.a')
      shared.Settings.USE_PTHREADS = 0
    elif what == 'sdl2-ttf':
      build_port('sdl2_ttf', 'libSDL2_ttf.a')
    elif what == 'cocos2d':
      build_port('cocos2d', 'libcocos2d.a')
    elif what == 'regal':
      build_port('regal', 'libregal.a')
    elif what == 'regal-mt':
      shared.Settings.USE_PTHREADS = 1
      build_port('regal', 'libregal-mt.a')
      shared.Settings.USE_PTHREADS = 0
    elif what == 'boost_headers':
      build_port('boost_headers', 'libboost_headers.a')
    else:
      logger.error('unfamiliar build target: ' + what)
      return 1

    logger.info('...success')
  return 0


if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
