#!/usr/bin/env python2

'''
Fast static linker for emscripten outputs. Specifically this links asm.js modules.

See https://github.com/kripken/emscripten/wiki/Linking
'''

import sys
from tools import shared
from tools.asm_module import AsmModule

def run():
  try:
    me, main, side, out = sys.argv[:4]
  except:
    print >> sys.stderr, 'usage: emlink.py [main module] [side module] [output name]'
    sys.exit(1)

  print 'Main module:', main
  print 'Side module:', side
  print 'Output:', out

  shared.try_delete(out)

  main = AsmModule(main)
  side = AsmModule(side)

  side.relocate_into(main)
  main.write(out)

if __name__ == '__main__':
  run()
