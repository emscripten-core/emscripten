'''
Gets the core asm module out of an emscripten output file
'''

import os, sys
import asm_module

infile = sys.argv[1]
outfile = sys.argv[2]

asm = asm_module.AsmModule(infile)
open(outfile, 'w').write(asm.asm_js + ';')

