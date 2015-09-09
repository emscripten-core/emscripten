'''
Separates out the core asm module out of an emscripten output file.

This is useful because it lets you load the asm module first, then the main script, which on some browsers uses less memory
'''

import os, sys
import asm_module

infile = sys.argv[1]
asmfile = sys.argv[2]
otherfile = sys.argv[3]

everything = open(infile).read()
module = asm_module.AsmModule(infile).asm_js

module = module[module.find('=')+1:] # strip the initial "var asm =" bit, leave just the raw module as a function
everything = everything.replace(module, 'Module["asm"]')

o = open(asmfile, 'w')
o.write('Module["asm"] = ')
o.write(module)
o.write(';')
o.close()

o = open(otherfile, 'w')
o.write(everything)
o.close()

