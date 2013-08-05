#!/usr/bin/env python2

'''
Splits a compiler outputted program into the asm module and the surrounding shell. This
can be useful if you want to process the shell in some manner (e.g. minifiy it) in ways
that would be harmful to asm.js code.
'''

import sys
import shared

try:
  me, in_shell, in_asm, outfile = sys.argv[:4]
except:
  print >> sys.stderr, 'usage: emlink.py [input file] [shell output file] [asm output file]'
  sys.exit(1)

print 'Shell input:', in_shell
print 'Asm input:', in_asm
print 'Input file:', outfile

shared.try_delete(outfile)

pre, post = open(in_shell).read().split('// ASM_CODE\n')
open(outfile, 'w').write(pre + '\n' + open(in_asm).read() + '\n' + post)

