# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Finds why an .ll file is large by printing functions by size
'''

from __future__ import print_function
import os, sys

funcs = []
i = 0
for line in open(sys.argv[1]):
  i += 1
  if line.startswith('define '):
    inside = line.replace('define ', '').replace('\n', '')
    start = i
  elif line.startswith('}'):
    funcs.append((inside, i-start))

print('\n'.join([str(func[1]) + ':' + func[0] for func in sorted(funcs, key=lambda func: -func[1])]))

