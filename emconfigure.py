#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
This is a helper script. It runs ./configure (or cmake,
etc.) for you, setting the environment variables to use
emcc and so forth. Usage:

  emconfigure ./configure [FLAGS]

You can also use this for cmake and other configure-like
stages. What happens is that all compilations done during
this command are to native code, not JS, so that configure
tests will work properly.
'''

from __future__ import print_function
import sys
from tools import building
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

  if 'cmake' in sys.argv[1]:
    print('error: use `emcmake` rather then `emconfigure` for cmake projects', file=sys.stderr)
    return 1

  try:
    building.configure(sys.argv[1:])
    return 0
  except CalledProcessError as e:
    return e.returncode


if __name__ == '__main__':
  sys.exit(run())
