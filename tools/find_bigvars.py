# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Simple tool to find functions with lots of vars.
"""

from __future__ import print_function
import sys

filename = sys.argv[1]
i = 0
curr = None
data = []
size = 0
for line in open(filename):
  i += 1
  if line.startswith('function '):
    size = len(line.split(','))  # params
    curr = line
  elif line.strip().startswith('var '):
    size += len(line.split(',')) + 1  # vars
  elif line.startswith('}') and curr:
    data.append([curr, size])
    curr = None
data.sort(key=lambda x: x[1])
print(''.join('%6d : %s' % (x[1], x[0]) for x in data))
