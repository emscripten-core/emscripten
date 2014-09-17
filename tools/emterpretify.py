#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=0
'''

import os, sys, re, json
import asm_module, shared

# utils

settings = { 'PRECISE_F32': 0 } # TODO

def make_emterpreter(t):
  return r'''
function emterpret%s%s(pc) {
 pc = pc | 0;
 // TODO: bump and check emtstack
 %s
}''' % (
  '_' if t != 'void' else '',
  '' if t == 'void' else t[0],
  '' if t == 'void' else 'return %s;' % shared.JS.make_initializer(t[0], settings)
)

# main

infile = sys.argv[1]
outfile = sys.argv[2]

print 'emterpretifying %s to %s' % (infile, outfile)

# process functions, generating bytecode
temp = infile + '.tmp.js'
shared.Building.js_optimizer(infile, ['emterpretify'], extra_info=None, output_filename=temp)

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
code = []
funcs = {}
lines = asm.funcs_js.split('\n')
asm.funcs_js = None
func = None

# first pass, collect bytecode
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function '):
    assert not func
    func = line.split(' ')[1].split('(')[0]
  elif line.startswith('}'):
    assert func
    curr = json.loads(line[4:])
    assert len(curr) % 4 == 0
    funcs[func] = len(code)
    code += curr
    func = None
    lines[i] = '}'

# create new mem init, and calculate where code will start
while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
code_start = len(mem_init) + 8 # 8 is GLOBAL_BASE
mem_init = mem_init + code
asm.staticbump += len(code)

while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
stack_start = len(mem_init)
asm.staticbump += 1024*1024 # 1MB default emterpreter stack TODO: customize

open(out_mem_file, 'wb').write(''.join(map(chr, mem_init)))

# second pass, finalize trampolines
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function '):
    assert not func
    func = line.split(' ')[1].split('(')[0]
  elif line.startswith('}'):
    assert func
    func = None
  elif func:
    call = '(EMTERPRETER_' + func + ')'
    if call in line:
      lines[i] = lines[i].replace(call, '(%s)' % (funcs[func] + code_start))

# finalize funcs JS
asm.funcs_js = '\n'.join(['\n'.join(lines), make_emterpreter('void'), make_emterpreter('int'), make_emterpreter('double'), make_emterpreter('float')])
lines = None

# set up emterpreter stack top
asm.set_pre_js(js='var EMTSTACKTOP = STATIC_BASE + %s;' % (stack_start))

# send EMT vars into asm
brace = asm.post_js.find(', {') + 3
asm.post_js = asm.post_js[:brace] + ' "EMTSTACKTOP": EMTSTACKTOP, ' + asm.post_js[brace:]
asm.imports_js += 'var EMTSTACKTOP = env.EMTSTACKTOP|0;\n'

asm.write(outfile)

