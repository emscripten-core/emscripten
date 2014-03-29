#!/usr/bin/env python2
# -*- Mode: python -*-

'''
C compiler backend functionality for LLVM: uses the Emscripten infrastructure
to compile LLVM IR into C.

Run with no arguments to see instructions.

Limitations:

  * No varargs calls to library functions (no printf. use -O1 or above so
    trivial printfs turn into puts which are ok).
  * Links with your system libc. There are probably inconsistencies with
    the libc used by emscripten, so things may not work.
  * argc/argv do not work.

'''

import os, sys
import shared
from shared import execute, unsuffixed
from asm_module import AsmModule

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
c = open(output).read()

print '[em-c-backend] finalize C'
asm = AsmModule(temp_name)
data = asm.mem_init_js.split('[')[1].split(']')[0]
c = c.replace('""', "{ " + ('0,'*8) + data + " }")
# add runtime and libc support code
open(output, 'w').write(c + r'''
int32_t em__emscripten_memcpy_big(int32_t dest, int32_t src, int32_t num) {
  int32_t i;
  for (i = 0; i < num; i++) {
    MEM[8+dest+i] = MEM[8+src+i];
  }
  return dest;
}

int32_t em__sbrk(int32_t bytes) {
  static int32_t DYNAMICTOP = 5246976;
  int32_t ret = DYNAMICTOP;
  if (bytes != 0) DYNAMICTOP += (bytes+7)&-8;
  return ret;
}

'''
)

