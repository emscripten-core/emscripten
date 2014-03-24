#!/usr/bin/env python2
# -*- Mode: python -*-

'''
C compiler backend functionality for LLVM: uses the Emscripten infrastructure
to compile LLVM IR into C.

Run with no arguments to see instructions
'''

import os, sys
import shared
from shared import execute, unsuffixed

if 'python' in sys.argv[0]:
  sys.argv = sys.argv[1:]

if len(sys.argv) <= 1:
  print '''
C compiler backend functionality for LLVM: uses the Emscripten infrastructure
to compile LLVM IR into C.

Usage:

  python c_backend.py [args to emcc]

The args to emcc can be anything emcc accepts, for example

  python c_backend.py source.cpp

which will emit a.out.c, or

  python c_backend.py source.cpp other.cpp -O2 -o project.c

This will emit project.c which will contain the output from
two source files, optimized.
'''
  sys.exit(1)

output = 'a.out.c'
for i in range(len(sys.argv)):
  if sys.argv[i] == '-o':
    output = sys.argv[i+1]
    break

temp_name = unsuffixed(output) + '.js'

print '[em-c-backend] emitting asm.js'
execute([shared.PYTHON, shared.EMCC, '-g2'] + sys.argv[1:] + ['-s', 'FINALIZE_JS=0', '-o', temp_name])

print '[em-c-backend] converting to C'
out = open(output, 'w')
execute([shared.PYTHON, shared.path_from_root('tools', 'js_optimizer.py'), temp_name, 'cIfy'], stderr=out, env={ 'EMCC_CORES': '1' })
out.close()

# use js_optimizer, but one core

