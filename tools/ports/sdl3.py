# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import glob
import os
import shutil

from tools import diagnostics

VERSION = '3.2.30'
TAG = f'release-{VERSION}'
HASH = '80ef7b2f257f43fe47c7ea8aa0a64f1c6f23720d91065d5e9b42f0205c62fc98bcf8dd1f1834fe09c66bea2598a18a658b82212cb29810be2d2175dece0aadce'
SUBDIR = f'SDL-{TAG}'

variants = {'sdl3-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SDL == 3


def get_lib_name(settings):
  return 'libSDL3' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  # get the port
  ports.fetch_project('sdl3', f'https://github.com/libsdl-org/SDL/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    diagnostics.warning('experimental', 'sdl3 port is still experimental')
    root_dir = ports.get_dir('sdl3', SUBDIR)

    # In addition copy our pre-generated SDL_build_config.h file.
    sdl_build_config_h = os.path.join(os.path.dirname(__file__), 'sdl3/SDL_build_config.h')
    shutil.copyfile(sdl_build_config_h, os.path.join(root_dir, 'include/SDL3/SDL_build_config.h'))

    # copy includes to a location so they can be used as 'SDL3/'
    source_include_path = os.path.join(root_dir, 'include', 'SDL3')
    ports.install_headers(source_include_path, target='SDL3')
    ports.make_pkg_config('sdl3', VERSION, '-sUSE_SDL=3')

    # copy sdl3-config.cmake
    cmake_file = os.path.join(os.path.dirname(__file__), 'sdl3/sdl3-config.cmake')
    ports.install_file(cmake_file, 'lib/cmake/SDL3/sdl3-config.cmake')

    glob_patterns = [
      # Generic sources (from SDL3's CMakeLists.txt)
      '*.c',
      'atomic/*.c',
      'audio/*.c',
      'camera/*.c',
      'core/*.c',
      'cpuinfo/*.c',
      'dynapi/*.c',
      'events/*.c',
      'io/*.c',
      'io/generic/*.c',
      'filesystem/*.c',
      'gpu/*.c',
      'joystick/*.c',
      'haptic/*.c',
      'hidapi/*.c',
      'locale/*.c',
      'main/*.c',
      'misc/*.c',
      'power/*.c',
      'render/*.c',
      'render/*/*.c',
      'sensor/*.c',
      'stdlib/*.c',
      'storage/*.c',
      'thread/*.c',
      'time/*.c',
      'timer/*.c',
      'video/*.c',
      'video/yuv2rgb/*.c',
      'tray/*.c',
      # Platform specific sources
      'storage/generic/*.c',
      'tray/unix/*.c',
      'time/unix/*.c',
      'timer/unix/*.c',
      'main/emscripten/*.c',
      'filesystem/posix/*.c',
      'filesystem/emscripten/*.c',
      'locale/emscripten/*.c',
      'camera/emscripten/*.c',
      'joystick/emscripten/*.c',
      'joystick/virtual/*.c',
      'power/emscripten/*.c',
      'misc/emscripten/*.c',
      'audio/emscripten/*.c',
      'video/emscripten/*.c',
      'video/offscreen/*.c',
      'audio/disk/*.c',
      'loadso/dlopen/*.c',
      # Dummy backends, do we need to support these?
      'camera/dummy/*.c',
      'audio/dummy/*.c',
      'video/dummy/*.c',
      'haptic/dummy/*.c',
      'sensor/dummy/*.c',
    ]

    flags = ['-sUSE_SDL=0']
    if settings.PTHREADS:
      glob_patterns.append('thread/pthread/*.c')
      flags += ['-pthread']
    else:
      glob_patterns.append('thread/generic/*.c')

    srcs = []
    for pattern in glob_patterns:
      matches = glob.glob(os.path.join(root_dir, 'src', pattern))
      assert matches
      srcs += matches

    srcs = [os.path.join(root_dir, 'src', s) for s in srcs]
    includes = [ports.get_include_dir('SDL3'), os.path.join(root_dir, 'src')]
    ports.build_port(root_dir, final, 'sdl3', srcs=srcs, includes=includes, flags=flags)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def show():
  return 'sdl3 (-sUSE_SDL=3 or --use-port=sdl3; zlib license)'
