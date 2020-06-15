#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

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
emcmake is a helper for cmake, setting various environment
variables so that emcc etc. are used. Typical usage:

  emcmake cmake [FLAGS]
''', file=sys.stderr)
    return 1

  try:
    building.configure(sys.argv[1:])
    return 0
  except CalledProcessError as e:
    return e.returncode


if __name__ == '__main__':
  sys.exit(run())
