#!/usr/bin/env python2
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Small utility to build some llvm bitcode into native code. Useful when lli
(called from exec_llvm) fails for some reason.

 * Use llc to generate x86 asm
 * Use as to generate an object file
 * Use g++ to link it to an executable
"""

from __future__ import print_function
import os
import sys

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))

sys.path.insert(0, __rootpath__)
from tools.shared import LLVM_OPT, LLVM_COMPILER, path_from_root, run_process, PIPE

filename = sys.argv[1]
libs = sys.argv[2:] # e.g.: dl for dlopen/dlclose, util for openpty/forkpty

print('bc => clean bc')
run_process([LLVM_OPT, filename, '-strip-debug', '-o', filename + '.clean.bc'])
print('bc => s')
for params in [['-march=x86'], ['-march=x86-64']]: # try x86, then x86-64 FIXME
  print('params', params)
  for triple in [['-mtriple=i386-pc-linux-gnu'], []]:
    run_process([LLVM_COMPILER] + params + triple + [filename + '.clean.bc', '-o', filename + '.s'], check=False)
    print('s => o')
    run_process(['as', filename + '.s', '-o', filename + '.o'], check=False)
    if os.path.exists(filename + '.o'):
      break
  if os.path.exists(filename + '.o'):
    break

if not os.path.exists(filename + '.o'):
  print('tools/nativize_llvm.py: Failed to convert "' + filename + '" to "' + filename + '.o"!', file=sys.stderr)
  sys.exit(1)

print('o => runnable')
args = ['g++', path_from_root('system', 'lib', 'debugging.cpp'), filename + '.o', '-o', filename + '.run'] + ['-l' + lib for lib in libs]
supportes_no_pie = 'no-pie' in run_process(['g++', '-dumpspecs'], stdout=PIPE).stdout
if supportes_no_pie:
  args.append('-no-pie')
run_process(args)

sys.exit(0)
