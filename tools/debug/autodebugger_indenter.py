# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Autodebugger output contains -1 for function entry and -2 for function exit.
This script will indent the output nicely
'''

from __future__ import print_function
import os, sys

lines = sys.stdin.read().split('\n')

depth = 0
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('AD:-2,'):
    depth -= 1
  print(str(depth) + '|' + line)
  if line.startswith('AD:-1,'):
    depth += 1

