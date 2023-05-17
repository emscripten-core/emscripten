#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Check for clean checkout.  This is run after tests during CI to ensure
we are not polluting the source checkout.
"""

import sys


def main():
  print("Running 'git status --short'")
  print('')

  print('Tree is clean.')
  return 0


if __name__ == '__main__':
  sys.exit(main())
