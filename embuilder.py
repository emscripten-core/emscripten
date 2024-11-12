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
import fnmatch
import logging
import sys
import time
from contextlib import contextmanager

from tools import cache
from tools import shared
from tools import system_libs
from tools import ports
from tools import utils
from tools.settings import settings
from tools.system_libs import USE_NINJA


# Minimal subset of targets used by CI systems to build enough to be useful
MINIMAL_TASKS = [
    'libbulkmemory',
    'libcompiler_rt',
    'libcompiler_rt-wasm-sjlj',
    'libcompiler_rt-ww',
    'libc',
    'libc-debug',
    'libc-ww-debug',
    'libc_optz',
    'libc_optz-debug',
    'libc++abi',
    'libc++abi-except',
    'libc++abi-noexcept',
    'libc++abi-debug',
    'libc++abi-debug-except',
    'libc++abi-debug-noexcept',
    'libc++abi-debug-ww-noexcept',
    'libc++',
    'libc++-except',
    'libc++-noexcept',
    'libc++-ww-noexcept',
    'libal',
    'libdlmalloc',
    'libdlmalloc-tracing',
    'libdlmalloc-debug',
    'libdlmalloc-ww',
    'libembind',
    'libembind-rtti',
    'libemmalloc',
    'libemmalloc-debug',
    'libemmalloc-memvalidate',
    'libemmalloc-verbose',
    'libemmalloc-memvalidate-verbose',
    'libmimalloc',
    'libmimalloc-mt',
    'libGL',
    'libGL-getprocaddr',
    'libGL-emu-getprocaddr',
    'libGL-emu-webgl2-ofb-getprocaddr',
    'libGL-webgl2-ofb-getprocaddr',
    'libGL-ww-getprocaddr',
    'libhtml5',
    'libsockets',
    'libsockets-ww',
    'libstubs',
    'libstubs-debug',
    'libstandalonewasm-nocatch',
    'crt1',
    'crt1_proxy_main',
    'crtbegin',
    'libunwind-except',
    'libnoexit',
    'sqlite3',
    'sqlite3-mt',
    'libwebgpu',
    'libwebgpu_cpp',
]

# Additional tasks on top of MINIMAL_TASKS that are necessary for PIC testing on
# CI (which has slightly more tests than other modes that want to use MINIMAL)
MINIMAL_PIC_TASKS = MINIMAL_TASKS + [
    'libcompiler_rt-mt',
    'libc-mt',
    'libc-mt-debug',
    'libc_optz-mt',
    'libc_optz-mt-debug',
    'libc++abi-mt',
    'libc++abi-mt-noexcept',
    'libc++abi-debug-mt',
    'libc++abi-debug-mt-noexcept',
    'libc++-mt',
    'libc++-mt-noexcept',
    'libdlmalloc-mt',
    'libGL-emu',
    'libGL-emu-webgl2-getprocaddr',
    'libGL-mt-getprocaddr',
    'libGL-mt-emu',
    'libGL-mt-emu-webgl2-getprocaddr',
    'libGL-mt-emu-webgl2-ofb-getprocaddr',
    'libsockets_proxy',
    'libsockets-mt',
    'crtbegin',
    'libsanitizer_common_rt',
    'libubsan_rt',
    'libwasm_workers-debug-stub',
    'libfetch',
    'libfetch-mt',
    'libwasmfs',
    'libwasmfs-debug',
    'libwasmfs_no_fs',
    'giflib',
]

PORTS = sorted(list(ports.ports_by_name.keys()) + list(ports.port_variants.keys()))

temp_files = shared.get_temp_files()
logger = logging.getLogger('embuilder')
legacy_prefixes = {
  'libgl': 'libGL',
}


def get_help():
  all_tasks = get_all_tasks()
  all_tasks.sort()
  return '''
Available targets:

  build / clear
        %s

Issuing 'embuilder build ALL' causes each task to be built.
''' % '\n        '.join(all_tasks)


@contextmanager
def get_port_variant(name):
  if name in ports.port_variants:
    name, extra_settings = ports.port_variants[name]
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
  return system_libraries, system_tasks


def get_all_tasks():
  return get_system_tasks()[1] + PORTS


def handle_port_error(target, message):
  utils.exit_with_error(f'error building port `{target}` | {message}')


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
  parser.add_argument('operation', choices=['build', 'clear', 'rebuild'])
  parser.add_argument('targets', nargs='*', help='see below')
  args = parser.parse_args()

  if args.operation != 'rebuild' and len(args.targets) == 0:
    shared.exit_with_error('no build targets specified')

  if args.operation == 'rebuild' and not USE_NINJA:
    shared.exit_with_error('"rebuild" operation is only valid when using Ninja')

  # process flags

  # Check sanity so that if settings file has changed, the cache is cleared here.
  # Otherwise, the cache will clear in an emcc process, which is invoked while building
  # a system library into the cache, causing trouble.
  cache.setup()
  shared.check_sanity()

  if args.lto:
    settings.LTO = args.lto

  if args.verbose:
    shared.PRINT_SUBPROCS = True

  if args.pic:
    settings.RELOCATABLE = 1

  if args.wasm64:
    settings.MEMORY64 = 2
    MINIMAL_TASKS[:] = [t for t in MINIMAL_TASKS if 'emmalloc' not in t]

  do_build = args.operation == 'build'
  do_clear = args.operation == 'clear'
  if args.force:
    do_clear = True

  system_libraries, system_tasks = get_system_tasks()

  # process tasks
  auto_tasks = False
  task_targets = dict.fromkeys(args.targets) # use dict to keep targets order

  # substitute
  predefined_tasks = {
    'SYSTEM': system_tasks,
    'USER': PORTS,
    'MINIMAL': MINIMAL_TASKS,
    'MINIMAL_PIC': MINIMAL_PIC_TASKS,
    'ALL': system_tasks + PORTS,
  }
  for name, tasks in predefined_tasks.items():
    if name in task_targets:
      task_targets[name] = tasks
      auto_tasks = True

  # flatten tasks
  tasks = []
  for name, targets in task_targets.items():
    if targets is None:
      # Use target name as task
      if '*' in name:
        tasks.extend(fnmatch.filter(get_all_tasks(), name))
      else:
        tasks.append(name)
    else:
      # There are some ports that we don't want to build as part
      # of ALL since the are not well tested or widely used:
      if 'cocos2d' in targets:
        targets.remove('cocos2d')

      # Use targets from predefined_tasks
      tasks.extend(targets)

  if auto_tasks:
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
        if USE_NINJA:
          library.generate()
        else:
          library.build(deterministic_paths=True)
    elif what == 'sysroot':
      if do_clear:
        cache.erase_file('sysroot_install.stamp')
      if do_build:
        system_libs.ensure_sysroot()
    elif what in PORTS:
      if do_clear:
        clear_port(what)
      if do_build:
        build_port(what)
    elif ':' in what or what.endswith('.py'):
      name = ports.handle_use_port_arg(settings, what, lambda message: handle_port_error(what, message))
      if do_clear:
        clear_port(name)
      if do_build:
        build_port(name)
    else:
      logger.error('unfamiliar build target: ' + what)
      return 1

    time_taken = time.time() - start_time
    logger.info('...success. Took %s(%.2fs)' % (('%02d:%02d mins ' % (time_taken // 60, time_taken % 60) if time_taken >= 60 else ''), time_taken))

  if USE_NINJA and args.operation != 'clear':
    system_libs.build_deferred()

  if len(tasks) > 1 or USE_NINJA:
    all_build_time_taken = time.time() - all_build_start_time
    logger.info('Built %d targets in %s(%.2fs)' % (len(tasks), ('%02d:%02d mins ' % (all_build_time_taken // 60, all_build_time_taken % 60) if all_build_time_taken >= 60 else ''), all_build_time_taken))

  return 0


if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
