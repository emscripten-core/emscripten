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

TODO:

  * global constructors

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

which will emit a.out.js, a.out.c and a.out.exe. Or,

  python c_backend.py source.cpp other.cpp -O2 -o project.c

This will emit project.* files (all 3, as before) which will
contain the output from two source files, optimized. You can
also do

  python c_backend.py source.cpp other.cpp -O2 -o project

(without a suffix).

Additional arguments:

  --c-compiler C_COMPILER    - the C compiler to use
'''
  sys.exit(1)

output = 'a.out.c'
c_compiler = shared.CLANG_CC
opts = '-O0'

for i in range(len(sys.argv)):
  if sys.argv[i] is None: continue
  if sys.argv[i] == '-o':
    output = sys.argv[i+1]
    sys.argv[i] = None
    sys.argv[i+1] = None
  elif sys.argv[i] == '--c-compiler':
    c_compiler = sys.argv[i+1]
    sys.argv[i] = None
    sys.argv[i+1] = None
  elif sys.argv[i].startswith('-O'):
    opts = sys.argv[i]
    if opts == '-O3': opts = '-O2' # map js to c opts
    # leave this one, for JS opts
sys.argv = filter(lambda x: x is not None, sys.argv)

if '.' in output: output = unsuffixed(output)
js_name = output + '.js'
c_name = output + '.c'
exe_name = output + '.exe'

print '[em-c-backend] emitting asm.js, to', js_name, c_name, exe_name
execute([shared.PYTHON, shared.EMCC, '-g2'] + sys.argv[1:] + ['-s', 'FINALIZE_JS=0', '-s', 'SIMPLE_MEM_INIT=1', '-o', js_name])

print '[em-c-backend] converting to C'
out = open(c_name, 'w')
execute([shared.PYTHON, shared.path_from_root('tools', 'js_optimizer.py'), js_name, 'cIfy'], stderr=out, env={ 'EMCC_JSOPT_MIN_CHUNK_SIZE': str(2**31-1) })
out.close()
c = open(c_name).read()
c, includes = c.split('INCLUDES: ')
pre_c, post_c = c.split('\nSPLIT\n')

print '[em-c-backend] finalize C'
asm = AsmModule(js_name)
data = asm.mem_init_js.split('[')[1].split(']')[0]
# add runtime and libc support code
o = open(c_name, 'w')
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
static int8_t STATIC_INIT[%d] = { %s };
static int8_t MEM[64*1024*1024];

static int32_t STACKTOP;
static int32_t DYNAMICTOP;
static int32_t tempDoublePtr = 0;

int32_t em__emscripten_memcpy_big(int32_t dest, int32_t src, int32_t num);
int32_t em__sbrk(int32_t bytes);
int32_t __errno_location();
int32_t em____cxa_allocate_exception(int32_t x);
int32_t em____cxa_throw(int32_t x, int32_t y, int32_t z);
void em___ZNSt9exceptionD2Ev();

int32_t em__llvm_bswap_i32(int32_t);
void em____assert_fail(int32_t, int32_t, int32_t, int32_t);

int32_t relocate(int32_t ptr);

void em__emscripten_run_script(int32_t code);

void em____cxa_begin_catch(int32_t);
void em___ZSt9terminatev();
void em____cxa_pure_virtual();

void em__emscripten_set_main_loop(int32_t func, int32_t fps, int32_t simulate);
void em__emscripten_cancel_main_loop();

''' % (data.count(',') + 9, ('0,'*8) + data) + pre_c)

def get_c_type(s):
  if s == 'i':
    return 'int32_t';
  elif s == 'd':
    return 'double'
  elif s == 'f':
   return 'float'
  elif s == 'v':
    return 'void'
  else:
    assert 0

for name, table in asm.tables.iteritems():
  sig = name.split('_')[-1]
  o.write('typedef ' + get_c_type(sig[0]) + ' (*fp_' + sig + ')(' + ','.join(map(get_c_type, sig[1:])) + ');\n')
  table = '{ ' + ','.join(map(lambda item: ('em_' + item) if item[0] != 'b' else 'NULL', table[1:-1].split(','))) + ' }'
  o.write('fp_' + sig + ' ' + name + '[] = ' + table + ';\n\n')

o.write('//===\n' + post_c + r'''//===

int32_t em__emscripten_memcpy_big(int32_t dest, int32_t src, int32_t num) {
  int32_t i;
  for (i = 0; i < num; i++) {
    MEM[dest+i] = MEM[src+i];
  }
  return dest;
}

int32_t em__sbrk(int32_t bytes) {
  static int first = 1;
  if (first) {
    DYNAMICTOP = (DYNAMICTOP+4095)&(-4096);
    first = 0;
  }
  int32_t ret = DYNAMICTOP;
  if (bytes != 0) DYNAMICTOP += (bytes+7)&-8;
  assert(DYNAMICTOP < sizeof(MEM));
  return ret;
}

int32_t __errno = 0;
int32_t __errno_location() {
  return ((int32_t)&__errno) - (int32_t)MEM; // out of bounds, but so what...
}

#define DIE(msg) \
 printf(msg "\n"); \
 exit(-1);

int32_t em____cxa_allocate_exception(int32_t x) {
  DIE("exceptions not supported");
  return 0;
}

int32_t em____cxa_throw(int32_t x, int32_t y, int32_t z) {
  DIE("exceptions not supported");
  return 0;
}

void em___ZNSt9exceptionD2Ev() {
  DIE("exceptions not supported");
}

int32_t em__llvm_bswap_i32(int32_t x) {
  return ((x&0xff)<<24) | (((x>>8)&0xff)<<16) | (((x>>16)&0xff)<<8) | ((x>>24)&0xff);
}

void em____assert_fail(int32_t condition, int32_t filename, int32_t line, int32_t func) {
  printf("assert failed at line %d\n", line);
  exit(3);
}

int32_t relocate(int32_t ptr) {
  if (ptr == 0) return 0; // NULL is NULL
  return ptr + (int32_t)MEM;
}

void em__emscripten_run_script(int32_t code) {
  printf("[not running script code]\n");
}

void em____cxa_begin_catch(int32_t ex) {
  DIE("exceptions not supported");
}

void em___ZSt9terminatev() {
  DIE("terminate");
}

void em____cxa_pure_virtual() {
  DIE("pure virtual");
}

void em__emscripten_set_main_loop(int32_t func, int32_t fps, int32_t simulate) {
  DIE("no main loop");
}

void em__emscripten_cancel_main_loop() {
  DIE("no main loop");
}

// main

int32_t alignMemory(int32_t p) {
  return (p+7)&-8;
}

int32_t dynamicAlloc(int32_t bytes) {
  int32_t ret = DYNAMICTOP;
  DYNAMICTOP += alignMemory(bytes);
  return ret;
}

int main(int argc, char **argv) {
  memcpy(MEM, STATIC_INIT, sizeof(STATIC_INIT));
  STACKTOP = alignMemory(sizeof(STATIC_INIT));
  DYNAMICTOP = alignMemory(STACKTOP + 5242880);
''')

if 'em__main(void)' in c:
  o.write(r'''
  return em__main();
''')
else:
  # translate args
  o.write(r'''
  assert(sizeof(void*) == 4 && "must build this code on a 32-bit arch (use -m32 if necessary)");
  char **em_argv = (char**)(MEM + DYNAMICTOP);
  dynamicAlloc(sizeof(char*)*(argc+1));
  int i;
  for (i = 0; i < argc; i++) {
    char *arg = argv[i];
    int len = strlen(arg);
    char *em_arg = (char*)(MEM + DYNAMICTOP);
    dynamicAlloc(len+1);
    strcpy(em_arg, arg);
    em_argv[i] = (char*)(((int32_t)em_arg) - (int32_t)MEM);
  }
  return em__main(argc, ((int32_t)em_argv) - (int32_t)MEM);  
''')

o.write(r'''}

''')
o.close()

# generate executable

print '[em-c-backend] build executable (%s %s)' % (c_compiler, opts)
execute([c_compiler, opts, '-m32', '-fno-inline', '-lm', c_name, '-o', exe_name])

