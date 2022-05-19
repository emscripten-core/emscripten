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
import time
from contextlib import contextmanager

from tools import shared
from tools import system_libs
from tools import ports
from tools.settings import settings
import emscripten


# Minimal subset of targets used by CI systems to build enough to useful
MINIMAL_TASKS = [
    'libcompiler_rt',
    'libc',
    'libc-debug',
    'libc-optz',
    'libc-optz-debug',
    'libc++abi',
    'libc++abi-except',
    'libc++abi-noexcept',
    'libc++',
    'libc++-except',
    'libc++-noexcept',
    'libal',
    'libdlmalloc',
    'libdlmalloc-noerrno',
    'libdlmalloc-tracing',
    'libdlmalloc-debug',
    'libemmalloc',
    'libemmalloc-debug',
    'libemmalloc-memvalidate',
    'libemmalloc-verbose',
    'libemmalloc-memvalidate-verbose',
    'libGL',
    'libhtml5',
    'libsockets',
    'libstubs',
    'libstubs-debug',
    'struct_info',
    'libstandalonewasm',
    'crt1',
    'libunwind-except',
    'libnoexit',
]

# Variant builds that we want to support for certain ports
# TODO: It would be nice if the ports themselves could specify the variants that they
# support.
PORT_VARIANTS = {
    'regal-mt': ('regal', {'USE_PTHREADS': 1}),
    'harfbuzz-mt': ('harfbuzz', {'USE_PTHREADS': 1}),
    'sdl2-mt': ('sdl2', {'USE_PTHREADS': 1}),
    'icu-mt': ('icu', {'USE_PTHREADS': 1}),
    'sdl2_mixer_mp3': ('sdl2_mixer', {'SDL2_MIXER_FORMATS': ["mp3"]}),
    'sdl2_mixer_none': ('sdl2_mixer', {'SDL2_MIXER_FORMATS': []}),
    'sdl2_image_png': ('sdl2_image', {'SDL2_IMAGE_FORMATS': ["png"]}),
    'sdl2_image_jpg': ('sdl2_image', {'SDL2_IMAGE_FORMATS': ["jpg"]}),
    'libpng-mt': ('libpng', {'USE_PTHREADS': 1}),
}

PORTS = sorted(list(ports.ports_by_name.keys()) + list(PORT_VARIANTS.keys()))

temp_files = shared.get_temp_files()
logger = logging.getLogger('embuilder')
legacy_prefixes = {
  'libgl': 'libGL',
}


def get_help():
  all_tasks = get_system_tasks()[1] + PORTS
  all_tasks.sort()
  return '''
Available targets:

  build / clear %s

Issuing 'embuilder build ALL' causes each task to be built.
''' % '\n        '.join(all_tasks)


@contextmanager
def get_port_variant(name):
  if name in PORT_VARIANTS:
    name, extra_settings = PORT_VARIANTS[name]
    old_settings = settings.dict().copy()
    for key, value in extra_settings.items():
      setattr(settings, key, value)
  else:
    old_settings = None

  yield name

  if old_settings:
    settings.dict().update(old_settings)


def clear_port(port_name):
  with get_port_variant(port_name) as port_name:
    ports.clear_port(port_name, settings)


def build_port(port_name):
  with get_port_variant(port_name) as port_name:
    ports.build_port(port_name, settings)


def get_system_tasks():
  system_libraries = system_libs.Library.get_all_variations()
  system_tasks = list(system_libraries.keys())
  # This is needed to build the generated_struct_info.json file.
  # It is not a system library, but it needs to be built before
  # running with FROZEN_CACHE.
  system_tasks += ['struct_info']

  return system_libraries, system_tasks


def main():
  all_build_start_time = time.time()

  parser = argparse.ArgumentParser(description=__doc__,
                                   formatter_class=argparse.RawDescriptionHelpFormatter,
                                   epilog=get_help())
  parser.add_argument('--lto', action='store_const', const='full', help='build bitcode object for LTO')
  parser.add_argument('--lto=thin', dest='lto', action='store_const', const='thin', help='build bitcode object for ThinLTO')
  parser.add_argument('--pic', action='store_true',
                      help='build relocatable objects for suitable for dynamic linking')
  parser.add_argument('--force', action='store_true',
                      help='force rebuild of target (by removing it first)')
  parser.add_argument('--verbose', action='store_true',
                      help='show build commands')
  parser.add_argument('--wasm64', action='store_true',
                      help='use wasm64 architecture')
  parser.add_argument('operation', help='currently only "build" and "clear" are supported')
  parser.add_argument('targets', nargs='+', help='see below')
  args = parser.parse_args()

  if args.operation not in ('build', 'clear'):
    shared.exit_with_error('unfamiliar operation: ' + args.operation)

  # process flags

  # Check sanity so that if settings file has changed, the cache is cleared here.
  # Otherwise, the cache will clear in an emcc process, which is invoked while building
  # a system library into the cache, causing trouble.
  shared.check_sanity()

  if args.lto:
    settings.LTO = args.lto

  if args.verbose:
    shared.PRINT_STAGES = True

  if args.pic:
    settings.RELOCATABLE = 1

  if args.wasm64:
    settings.MEMORY64 = 2
    MINIMAL_TASKS[:] = [t for t in MINIMAL_TASKS if 'emmalloc' not in t]

  do_build = args.operation == 'build'
  do_clear = args.operation == 'clear'
  if args.force:
    do_clear = True

  # process tasks
  auto_tasks = False
  tasks = args.targets
  system_libraries, system_tasks = get_system_tasks()
  if 'SYSTEM' in tasks:
    tasks = system_tasks
    auto_tasks = True
  elif 'USER' in tasks:
    tasks = PORTS
    auto_tasks = True
  elif 'MINIMAL' in tasks:
    tasks = MINIMAL_TASKS
    auto_tasks = True
  elif 'ALL' in tasks:
    tasks = system_tasks + PORTS
    auto_tasks = True
  if auto_tasks:
    # There are some ports that we don't want to build as part
    # of ALL since the are not well tested or widely used:
    skip_tasks = ['cocos2d']
    tasks = [x for x in tasks if x not in skip_tasks]
    print('Building targets: %s' % ' '.join(tasks))
  for what in tasks:
    for old, new in legacy_prefixes.items():
      if what.startswith(old):
        what = what.replace(old, new)
    if do_build:
      logger.info('building ' + what)
    else:
      logger.info('clearing ' + what)
    start_time = time.time()
    if what in system_libraries:
      library = system_libraries[what]
      if do_clear:
        library.erase()
      if do_build:
        library.get_path()
    elif what == 'sysroot':
      if do_clear:
        shared.Cache.erase_file('sysroot_install.stamp')
      if do_build:
        system_libs.ensure_sysroot()
    elif what == 'struct_info':
      if do_clear:
        emscripten.clear_struct_info()
      if do_build:
        emscripten.generate_struct_info()
    elif what in PORTS:
      if do_clear:
        clear_port(what)
      if do_build:
        build_port(what)
    else:
      logger.error('unfamiliar build target: ' + what)
      return 1

    time_taken = time.time() - start_time
    logger.info('...success. Took %s(%.2fs)' % (('%02d:%02d mins ' % (time_taken // 60, time_taken % 60) if time_taken >= 60 else ''), time_taken))

  if len(tasks) > 1:
    all_build_time_taken = time.time() - all_build_start_time
    logger.info('Built %d targets in %s(%.2fs)' % (len(tasks), ('%02d:%02d mins ' % (all_build_time_taken // 60, all_build_time_taken % 60) if all_build_time_taken >= 60 else ''), all_build_time_taken))

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
