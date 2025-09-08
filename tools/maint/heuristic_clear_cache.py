#!/usr/bin/env python3
# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# This script can be used to clear the cache based on a heuristic detection of
# whether any source files generating the cache have changed.
# This can help avoid redundant cache clears in a workflow, but note that this
# script is only a heuristic, and not necessarily perfect.

import os
import sys

__scriptdir__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import utils
from tools import cache

# Modifying the following files/paths can often disturb Emscripten generated cache.
# If any of those are modified, then the Emscripten cache should be cleared.
FILES_THAT_CAN_DISTURB_CACHE = [
  utils.path_from_root('system'),
  utils.path_from_root('tools', 'system_libs.py'),
]


def recursive_mtime(path):
  mtime = os.path.getmtime(path)
  if os.path.isfile(path):
    return mtime

  with os.scandir(path) as it:
    for entry in it:
      mtime = max(mtime, recursive_mtime(entry.path) if entry.is_dir() else os.path.getmtime(entry.path))
  return mtime


def newest_mtime(paths):
  return max((recursive_mtime(path) for path in paths), default=0)


def heuristic_clear_cache():
  mtime_file = cache.get_path('system_libs_mtime.txt')
  try:
    system_libs_mtime = open(mtime_file).read()
  except Exception:
    system_libs_mtime = 0

  newest_system_libs_mtime = str(newest_mtime(FILES_THAT_CAN_DISTURB_CACHE))

  if newest_system_libs_mtime != system_libs_mtime:
    print(f'Cache timestamp {system_libs_mtime} does not match with current timestamp {newest_system_libs_mtime}. Clearing cache...')
    cache.erase()
    open(mtime_file, 'w').write(str(newest_system_libs_mtime))
  else:
    print('Cache timestamp is up to date, no clear needed.')


if __name__ == '__main__':
    sys.exit(heuristic_clear_cache())
