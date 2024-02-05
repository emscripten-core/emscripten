#!/usr/bin/env python3
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is a helper tool which is designed to make it possible
for other apps to read emscripten's configuration variables
in a unified way.  Usage:

  em-config VAR_NAME

This tool prints the value of the variable to stdout if one
is found, or exits with 1 if the variable does not exist.
"""

import sys
import re
from tools import config


def main():
  if len(sys.argv) != 2 or \
    not re.match(r"^[\w\W_][\w\W_\d]*$", sys.argv[1]) or \
    not hasattr(config, sys.argv[1]):
    print('Usage: em-config VAR_NAME', file=sys.stderr)
    exit(1)

  print(getattr(config, sys.argv[1]))
  return 0


if __name__ == '__main__':
  sys.exit(main())
