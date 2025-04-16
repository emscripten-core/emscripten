#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is a helper script. It runs ./configure (or cmake,
etc.) for you, setting the environment variables to use
emcc and so forth. Usage:

  emconfigure ./configure [FLAGS]

You can also use this for cmake and other configure-like
stages. What happens is that all compilations done during
this command are to native code, not JS, so that configure
tests will work properly.
"""

import sys
from tools import building
from tools import shared
from subprocess import CalledProcessError


#
# Main run() function
#
def run():
  if len(sys.argv) < 2 or sys.argv[1] in ('--version', '--help'):
    print('''\
emconfigure is a helper for configure, setting various environment
variables so that emcc etc. are used. Typical usage:

  emconfigure ./configure [FLAGS]

(but you can run any command instead of configure)''', file=sys.stderr)
    return 1

  args = sys.argv[1:]

  if 'cmake' in args:
    print('error: use `emcmake` rather then `emconfigure` for cmake projects', file=sys.stderr)
    return 1

  env = building.get_building_env()
  # When we configure via a ./configure script, don't do config-time
  # compilation with emcc, but instead do builds natively with Clang. This
  # is a heuristic emulation that may or may not work.
  env['EMMAKEN_JUST_CONFIGURE'] = '1'
  print('configure: ' + shared.shlex_join(args), file=sys.stderr)
  try:
    shared.check_call(args, env=env)
    return 0
  except CalledProcessError as e:
    return e.returncode


if __name__ == '__main__':
  sys.exit(run())
