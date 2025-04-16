#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is a helper script. It runs make for you, setting
the environment variables to use emcc and so forth. Usage:

  emmake make [FLAGS]

Note that if you ran configure with emconfigure, then
the environment variables have already been detected
and set. This script is useful if you have no configure
step, and your Makefile uses the environment vars
directly.

The difference between this and emconfigure is that
emconfigure runs compilation into native code, so
that configure tests pass. emmake uses Emscripten to
generate JavaScript.
"""

import shutil
import sys
from tools import building
from tools import shared
from tools import utils
from subprocess import CalledProcessError


#
# Main run() function
#
def run():
  if len(sys.argv) < 2 or sys.argv[1] in ('--version', '--help'):
    print('''\
emmake is a helper for make, setting various environment
variables so that emcc etc. are used. Typical usage:

  emmake make [FLAGS]

(but you can run any command instead of make)''', file=sys.stderr)
    return 1

  args = sys.argv[1:]
  env = building.get_building_env()

  # On Windows prefer building with mingw32-make instead of make, if it exists.
  if utils.WINDOWS:
    if args[0] == 'make':
      mingw32_make = shutil.which('mingw32-make')
      if mingw32_make:
        args[0] = mingw32_make

  # On Windows, run the execution through shell to get PATH expansion and
  # executable extension lookup, e.g. 'sdl2-config' will match with
  # 'sdl2-config.bat' in PATH.
  print('make: ' + ' '.join(args), file=sys.stderr)
  try:
    shared.check_call(args, shell=utils.WINDOWS, env=env)
    return 0
  except CalledProcessError as e:
    return e.returncode


if __name__ == '__main__':
  sys.exit(run())
