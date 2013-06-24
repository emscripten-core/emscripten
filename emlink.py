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
from tools import js_optimizer

try:
  me, main, side, out = sys.argv[:4]
except:
  print >> sys.stderr, 'usage: emlink.py [main module] [side module] [output name]'
  sys.exit(1)

print 'Main module:', main
print 'Side module:', side
print 'Output:', out

class AsmModule():
  def __init__(self, filename):
    self.js = open(filename).read()
    # imports
    imports_js = self.js[self.js.find(js_optimizer.start_asm_marker):self.js.rfind(js_optimizer.start_funcs_marker)]
    self.imports = [m.group(0) for m in js_optimizer.import_sig.finditer(imports_js)]
    #print 'imports', self.imports

    # funcs
    funcs_js = self.js[self.js.find(js_optimizer.start_funcs_marker):self.js.rfind(js_optimizer.end_funcs_marker)]
    self.funcs = [m.group(2) for m in js_optimizer.func_sig.finditer(funcs_js)]
    #print 'funcs', self.funcs

    # exports

  def relocate(self, main):
    # Find function name replacements TODO: do not rename duplicate names with duplicate contents, just merge them
    main_funcs = set(main.funcs)
    replacements = {}
    for i in range(len(self.funcs)):
      rep = func = self.funcs[i]
      while rep in main_funcs:
        rep += '_'
        replacements[func] = rep
    print replacements

  def write(self, out):
    open(out, 'w').write(self.js)

main = AsmModule(main)
side = AsmModule(side)

side.relocate(main)
#main.write(out)

