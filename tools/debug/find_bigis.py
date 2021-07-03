# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Simple tool to find big i types in an .ll file. Anything over i64 is of interest.
'''

from __future__ import print_function
import os, sys, re
from pathlib import Path

filename = sys.argv[1]
data = Path(filename).read_text()
iss = re.findall(r' i\d+ [^=]', data)
set_iss = set(iss)
bigs = []
for iss in set_iss:
  size = int(iss[2:-2])
  if size > 64:
    bigs.append(size)
bigs.sort()
print(bigs)

