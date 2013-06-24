#!/usr/bin/env python2

'''
Fast static linker for emscripten outputs. Specifically this links asm.js modules.

Usage: emlink.py [main module] [side module] [output name]

  Main module should be built with -s MAIN_MODULE=1 (and possibly -s DISABLE_GL_EMULATION=1, if you do not need GL emulation)
  Side module should be built with -s SIDE_MODULE=1

Note that the output file can be used as a main module, so you can link multiple
side modules into a main module that way.
'''

import os, subprocess, sys
from tools import shared

try:
  me, main, side, out = sys.argv[:4]
except:
  print >> sys.stderr, 'usage: emlink.py [main module] [side module] [output name]'
  sys.exit(1)

print 'Main module:', main
print 'Side module:', side
print 'Output:', out

