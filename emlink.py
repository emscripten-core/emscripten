#!/usr/bin/env python2
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Fast static linker for emscripten outputs. Specifically this links asm.js modules.

See https://github.com/kripken/emscripten/wiki/Linking
'''

from __future__ import print_function
import sys
from tools import shared
from tools.asm_module import AsmModule

def run():
  try:
    me, main, side, out = sys.argv[:4]
  except:
    print('usage: emlink.py [main module] [side module] [output name]', file=sys.stderr)
    sys.exit(1)

  print('Main module:', main)
  print('Side module:', side)
  print('Output:', out)

  shared.try_delete(out)

  main = AsmModule(main)
  side = AsmModule(side)

  side.relocate_into(main)
  main.write(out)

if __name__ == '__main__':
  run()
