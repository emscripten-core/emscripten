#!/usr/bin/env python2
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Archive helper script

This script is a simple wrapper around llvm-ar.  It used to have special
handling for duplicate basenames in order to allow bitcode linking process to
read such files.  This is now handled by using tools/arfile.py to read archives.
"""

import sys

from tools import shared


def run():
  newargs = [shared.LLVM_AR] + sys.argv[1:]
  return shared.run_process(newargs, stdin=sys.stdin, check=False).returncode


if __name__ == '__main__':
  sys.exit(run())
