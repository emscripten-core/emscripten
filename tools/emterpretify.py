#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=0
'''

import os, sys
import asm_module, shared

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

# parse out bytecode and add to mem init file
code = [] # XXX

while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
code_start = len(mem_init)
mem_init = mem_init + code
asm.staticbump += len(code)

while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
stack_start = len(mem_init)
asm.staticbump += 1024*1024 # 1MB default emterpreter stack TODO: customize

open(out_mem_file, 'wb').write(''.join(map(chr, mem_init)))

asm.set_pre_js(js='var EMTCODE = STATIC_BASE + %d, EMTSTACKTOP = STATIC_BASE + %s;' % (code_start, stack_start)) # apply staticbump and do allocations

asm.write(outfile)

