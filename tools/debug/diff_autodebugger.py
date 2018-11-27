# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Very simple line-by line diff of autodebugger outputs. useful when there are no added or removed lines,
and there are float differences
'''
from __future__ import print_function
import os, sys

f1 = open(sys.argv[1], 'r').readlines()
f2 = open(sys.argv[2], 'r').readlines()

for i in range(len(f1)):
  if f1[i] == f2[i]: continue
  v1 = float(f1[i].split(',')[1])
  v2 = float(f2[i].split(',')[1])
  print('%5d %10s  %f           ' % (i+1, f1[i].split(',')[0], v1-v2), '         ', v1-v2, v1, v2)

