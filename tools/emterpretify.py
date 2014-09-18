#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=0
'''

import os, sys, re, json
import asm_module, shared

# params

EMT_STACK_MAX = 1024*1024

# consts

BLACKLIST = set(['_memcpy', '_memset', 'copyTempDouble', 'copyTempFloat', '_strlen', 'stackAlloc', 'setThrew', 'stackRestore', 'setTempRet0', 'getTempRet0', 'stackSave', 'runPostSets'])

OPCODES = { # l, lx, ly etc - one of 256 locals
  '0':   'SET',   # [lx, ly, 0]          lx = ly (int or float, not double)
  '1':   'GETST', # [l, 0, 0]            l = STACKTOP
  '2':   'SETST', # [l, 0, 0]            STACKTOP = l
  '3':   'SETI',  # [l, vl, vh]          l = v (16-bit int)
  '253': 'CALL',  # [target, sig, params..]   target(params..)
  '254': 'RET',   # [l, 0, 0]            return l (depending on which emterpreter_x we are in, has the right type)
  '255': 'FUNC',  # [n, 0, 0]            function with n locals (each taking 64 bits)
}

ROPCODES = {}
for o in OPCODES:
  ROPCODES[OPCODES[o]] = int(o);

CASES = {}
CASES[ROPCODES['SET']] = 'HEAP32[sp + (lx << 3) >> 2] = HEAP32[sp + (ly << 3) >> 2]|0;'
CASES[ROPCODES['GETST']] = 'HEAP32[sp + (lx << 3) >> 2] = STACKTOP;'
CASES[ROPCODES['SETST']] = 'STACKTOP = HEAP32[sp + (lx << 3) >> 2]|0;'
CASES[ROPCODES['SETI']] = 'HEAP32[sp + (lx << 3) >> 2] = inst >>> 16;'
CASES[ROPCODES['CALL']] = 'assert(1111);'

# utils

settings = { 'PRECISE_F32': 0 } # TODO

def make_emterpreter(t):
  # return is specialized per interpreter
  if t == 'void':
    CASES[ROPCODES['RET']] = 'return;'
  elif t == 'int':
    CASES[ROPCODES['RET']] = 'return HEAP32[sp + (lx << 3) >> 2]|0;'
  elif t == 'double':
    CASES[ROPCODES['RET']] = 'return +HEAPF64[sp + (lx << 3) >> 3];'

  return r'''
function emterpret%s%s(pc) {
 pc = pc | 0;
 var sp = 0, inst = 0, op = 0, lx = 0, ly = 0, lz = 0;
 sp = EMTSTACKTOP;
 assert(((HEAPU8[pc>>0]>>>0) == %d)|0);
 EMTSTACKTOP = EMTSTACKTOP + (HEAP8[pc + 1 >> 0] << 3) | 0;
 pc = pc + 4 | 0;
 assert(((EMTSTACKTOP|0) <= (EMT_STACK_MAX|0))|0);
 while (1) {
  inst = HEAP32[pc>>2]|0;
  op = inst & 255;
  lx = (inst >> 8) & 255;
  ly = (inst >> 16) & 255;
  lz = inst >>> 24;
  switch (op|0) {
%s
   default: assert(0);
  }
 }
 %s
}''' % (
  '_' if t != 'void' else '',
  '' if t == 'void' else t[0],
  ROPCODES['FUNC'],
  '\n'.join(['   case %d: %s break;' % (k, v) for k, v in CASES.iteritems()]),
  '' if t == 'void' else 'return %s;' % shared.JS.make_initializer(t[0], settings)
)

# main

infile = sys.argv[1]
outfile = sys.argv[2]

print 'emterpretifying %s to %s' % (infile, outfile)

# final global functions

asm = asm_module.AsmModule(infile)
global_funcs = {}
rglobal_funcs = {}
global_id = 0
for k, v in asm.imports.iteritems():
  if '|' not in v and '+' not in v and 'new ' not in v and '.0' not in v and v != '0':
    global_funcs[k] = global_id
    rglobal_funcs[global_id] = k
    global_id += 1

lines = asm.funcs_js.split('\n')
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function ') and '}' not in line:
    func = line.split(' ')[1].split('(')[0]
    if func not in BLACKLIST:
      global_funcs[func] = global_id
      global_id += 1
assert global_id < 256

# process functions, generating bytecode
temp = infile + '.tmp.js'
shared.Building.js_optimizer(infile, ['emterpretify'], extra_info={ 'blacklist': list(BLACKLIST), 'globalFuncs': global_funcs, 'opcodes': OPCODES, 'ropcodes': ROPCODES }, output_filename=temp)

# load the module and modify it
asm = asm_module.AsmModule(temp)

in_mem_file = infile + '.mem'
out_mem_file = outfile + '.mem'
assert in_mem_file in asm.pre_js, 'we assume a mem init file for now'
asm.pre_js = asm.pre_js.replace(in_mem_file, out_mem_file)
assert os.path.exists(in_mem_file), 'need to find mem file at %s' % mem_file
mem_init = map(ord, open(in_mem_file, 'rb').read())
zero_space = asm.staticbump - len(mem_init)
assert zero_space >= 0 # can be positive, if we add a bump of zeros

assert 'GLOBAL_BASE: 8,' in asm.pre_js

# parse out bytecode and add to mem init file
all_code = []
funcs = {}
lines = asm.funcs_js.split('\n')
asm.funcs_js = None
func = None

# first pass, collect and process bytecode

call_sigs = {} # signatures appearing for each call target
def process_code(code):
  # find CALL instructions and fix their targets and signatures
  for i in range(len(code)/4):
    j = i*4
    if code[j] == ROPCODES['CALL']:
      target = code[j+1]
      sig = code[j+2]
      if target not in call_sigs: call_sigs[target] = []
      sigs = call_sigs[target]
      if sig not in sigs: sigs.append(sig)
      code[j+1] = global_funcs[target]
      code[j+2] = sigs.index(sig)

for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function ') and '}' not in line:
    assert not func
    func = line.split(' ')[1].split('(')[0]
  elif line.startswith('}'):
    assert func
    try:
      curr = json.loads(line[4:])
    except:
      if '[' in line: print >> sys.stderr, 'failed to parse code from', line
      curr = None
    if curr is not None:
      assert len(curr) % 4 == 0, curr
      funcs[func] = len(all_code) # no operation here should change the length
      process_code(curr)
      print >> sys.stderr, 'bytecode for %s:' % func, curr
      all_code += curr
    func = None
    lines[i] = '}'

# create new mem init, and calculate where code will start
while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
code_start = len(mem_init) + 8 # 8 is GLOBAL_BASE
mem_init = mem_init + all_code
asm.staticbump += len(all_code)

while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
stack_start = len(mem_init)
asm.staticbump += EMT_STACK_MAX

open(out_mem_file, 'wb').write(''.join(map(chr, mem_init)))

# second pass, finalize trampolines
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function ') and '}' not in line:
    assert not func
    func = line.split(' ')[1].split('(')[0]
  elif line.startswith('}'):
    assert func
    func = None
  elif func and func in funcs:
    call = '(EMTERPRETER_' + func + ')'
    if call in line:
      lines[i] = lines[i].replace(call, '(%s)' % (funcs[func] + code_start))

# finalize funcs JS
asm.funcs_js = '\n'.join(['\n'.join(lines), make_emterpreter('void'), make_emterpreter('int'), make_emterpreter('double')])
lines = None

# set up emterpreter stack top
asm.set_pre_js(js='var EMTSTACKTOP = STATIC_BASE + %s, EMT_STACK_MAX = EMTSTACKTOP + %d;' % (stack_start, EMT_STACK_MAX))

# send EMT vars into asm
brace = asm.post_js.find(', {') + 3
asm.post_js = asm.post_js[:brace] + ' "EMTSTACKTOP": EMTSTACKTOP, "EMT_STACK_MAX": EMT_STACK_MAX, ' + asm.post_js[brace:]
asm.imports_js += 'var EMTSTACKTOP = env.EMTSTACKTOP|0;\nvar EMT_STACK_MAX = env.EMT_STACK_MAX|0;\n'

asm.write(outfile)

