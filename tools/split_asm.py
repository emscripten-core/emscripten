#!/usr/bin/env python2

'''
Splits a compiler outputted program into the asm module and the surrounding shell. This
can be useful if you want to process the shell in some manner (e.g. minifiy it) in ways
that would be harmful to asm.js code.
'''

import sys
import shared
from asm_module import AsmModule

try:
  me, infile, out_shell, out_asm = sys.argv[:4]
except:
  print >> sys.stderr, 'usage: emlink.py [input file] [shell output file] [asm output file]'
  sys.exit(1)

print 'Input file:', infile
print 'Shell output:', out_shell
print 'Asm output:', out_asm

shared.try_delete(out_shell)
shared.try_delete(out_asm)

module = AsmModule(infile)

open(out_shell, 'w').write(module.pre_js + '\n// ASM_CODE\n' + module.post_js)
open(out_asm, 'w').write(module.asm_js)

