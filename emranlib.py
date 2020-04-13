#!/usr/bin/env python
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.


"""emranlib - ranlib helper script

This script acts as a frontend replacement for ranlib, and simply invokes
llvm-ranlib internally.
"""

from __future__ import print_function
import sys
from tools import shared


def run():
  newargs = [shared.LLVM_RANLIB] + sys.argv[1:]
  return shared.run_process(newargs, stdin=sys.stdin, check=False).returncode


if __name__ == '__main__':
  sys.exit(run())
