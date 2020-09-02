#!/usr/bin/python
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Usage: creduce ./creduce_tester.py newfail1.c
"""

import os
import sys
from subprocess import Popen, PIPE

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'tools')]
import shared

# creduce will only pass the filename of the C file as the first arg, so other
# configuration options will have to be hardcoded.
CSMITH_CFLAGS = ['-I', os.path.join(os.environ['CSMITH_PATH'], 'runtime')]
EMCC_ARGS = ['-O2']

filename = sys.argv[1]
obj_filename = shared.unsuffixed(filename)
js_filename = obj_filename + '.js'
print('testing file', filename)

try:
  print('2) Compile natively')
  shared.run_process([shared.CLANG_CC, '-O2', filename, '-o', obj_filename] + CSMITH_CFLAGS)
  print('3) Run natively')
  correct = shared.timeout_run(Popen([obj_filename], stdout=PIPE, stderr=PIPE), 3)
except Exception as e:
  print('Failed or infinite looping in native, skipping', e)
  sys.exit(1) # boring

print('4) Compile JS-ly and compare')


def try_js(args):
  shared.run_process([shared.EMCC] + EMCC_ARGS + CSMITH_CFLAGS + args +
                     [filename, '-o', js_filename])
  js = shared.run_js_tool(js_filename, stderr=PIPE)
  assert correct == js


# Try normally, then try unaligned because csmith does generate nonportable code that requires x86 alignment
# If you are sure that alignment is not the cause, disable it for a faster reduction
for args in [[]]:
  try:
    try_js(args)
    break
  except Exception:
    pass
else:
  sys.exit(0)

sys.exit(1) # boring
