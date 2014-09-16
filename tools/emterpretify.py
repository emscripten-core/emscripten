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

assert '.mem' in asm.pre_js, 'we assume a mem init file for now'
mem_file = infile + '.mem'
assert os.path.exists(mem_file), 'need to find mem file at %s' % mem_file
mem_init = open(mem_file, 'rb').read()
zero_space = asm.staticbump - len(mem_init)
assert zero_space >= 0 # can be positive, if we add a bump of zeros

# parse out bytecode and add to mem init file
code = [] # XXX

while asm.staticbump % 8 != 0: asm.staticbump += 1
code_start = asm.staticbump
asm.staticbump += len(code)
while asm.staticbump % 8 != 0: asm.staticbump += 1
stack_start = asm.staticbump
asm.staticbump += 1024*1024 # 1MB default emterpreter stack TODO: customize
asm.set_pre_js(js='var EMTCODE = %d, EMTSTACKTOP = %s;' % (code_start, stack_start)) # apply staticbump and do allocations

asm.write(outfile)

