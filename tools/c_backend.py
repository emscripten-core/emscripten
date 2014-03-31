#!/usr/bin/env python2
# -*- Mode: python -*-

'''
C compiler backend functionality for LLVM: uses the Emscripten infrastructure
to compile LLVM IR into C.

Run with no arguments to see instructions.

Limitations:

  * Links with your system libc. There are probably inconsistencies with
    the libc used by emscripten, so things may not work.
  * printf of %s (strings) does not work. Use puts.
  * varargs in general, including printf, might only work for a single arg.

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
    sys.argv[i] = None
    sys.argv[i+1] = None
    break
sys.argv = filter(lambda x: x is not None, sys.argv)

temp_name = unsuffixed(output) + '.js'

print '[em-c-backend] emitting asm.js, to', output, 'with temp', temp_name
execute([shared.PYTHON, shared.EMCC, '-g2'] + sys.argv[1:] + ['-s', 'FINALIZE_JS=0', '-o', temp_name])

print '[em-c-backend] converting to C'
out = open(output, 'w')
execute([shared.PYTHON, shared.path_from_root('tools', 'js_optimizer.py'), temp_name, 'cIfy'], stderr=out, env={ 'EMCC_CORES': '1' })
out.close()
c = open(output).read()
c, includes = c.split('INCLUDES: ')

print '[em-c-backend] finalize C'
asm = AsmModule(temp_name)
data = asm.mem_init_js.split('[')[1].split(']')[0]
# add runtime and libc support code
o = open(output, 'w')
o.write(r'''
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
''')

for i in includes.strip().split(','):
  if i: o.write('#include <' + i  + '>\n')

o.write(r'''
static int8_t MEM[64*1024*1024] = { %s };

static int32_t STACKTOP = sizeof(MEM) - 1024*1024;
static int32_t DYNAMICTOP = 5246976;
static int32_t tempDoublePtr = 0;

int32_t em__emscripten_memcpy_big(int32_t dest, int32_t src, int32_t num);
int32_t em__sbrk(int32_t bytes);

//===
''' % (('0,'*8) + data) + c + r'''//===

int32_t em__emscripten_memcpy_big(int32_t dest, int32_t src, int32_t num) {
  int32_t i;
  for (i = 0; i < num; i++) {
    MEM[8+dest+i] = MEM[8+src+i];
  }
  return dest;
}

int32_t em__sbrk(int32_t bytes) {
  int32_t ret = DYNAMICTOP;
  if (bytes != 0) DYNAMICTOP += (bytes+7)&-8;
  return ret;
}

int main(int argc, char **argv) {''')

if 'em__main(void)' in c:
  o.write(r'''
  return em__main();
''')
else:
  # translate args
  o.write(r'''
  assert(sizeof(void*) == 4 && "must build this code on a 32-bit arch (use -m32 if necessary)");
  char **em_argv = (char**)(MEM + DYNAMICTOP);
  em__sbrk(sizeof(char*)*(argc+1));
  int i;
  for (i = 0; i < argc; i++) {
    char *arg = argv[i];
    int len = strlen(arg);
    char *em_arg = (char*)(MEM + DYNAMICTOP);
    em__sbrk(len+1);
    strcpy(em_arg, arg);
    em_argv[i] = (char*)(((int32_t)em_arg) - (int32_t)MEM);
  }
  return em__main(argc, ((int32_t)em_argv) - (int32_t)MEM);  
''')

o.write(r'''}

''')
o.close()

