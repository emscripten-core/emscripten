#!/usr/bin/env python2
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, subprocess, sys
from tools import shared

#
# Main run() function
#
def run():
  configure_path = shared.path_from_root('emconfigure')

  exit(subprocess.call([shared.PYTHON, configure_path] + sys.argv[1:]))

if __name__ == '__main__':
  run()
