#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=1
'''

import os, sys
import asm_module, shared

infile = sys.argv[1]
outfile = sys.argv[2]

print 'emterpretifying %s to %s' % (infile, outfile)

shared.Building.js_optimizer(infile, ['emterpretify'], extra_info=None, output_filename=outfile)

