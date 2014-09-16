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

temp = infile + '.tmp.js'
shared.Building.js_optimizer(infile, ['emterpretify'], extra_info=None, output_filename=temp)

asm = asm_module.AsmModule(temp)

code_size = 1024 # XXX

while asm.staticbump % 8 != 0: asm.staticbump += 1
code_start = asm.staticbump
asm.staticbump += code_size
while asm.staticbump % 8 != 0: asm.staticbump += 1
stack_start = asm.staticbump
asm.staticbump += 1024*1024 # 1MB default emterpreter stack TODO: customize
asm.set_pre_js() # apply staticbump

asm.write(outfile)

