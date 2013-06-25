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
    self.filename = filename
    self.js = open(filename).read()

    self.start_asm = self.js.find(js_optimizer.start_asm_marker)
    self.start_funcs = self.js.find(js_optimizer.start_funcs_marker)
    self.end_funcs = self.js.rfind(js_optimizer.end_funcs_marker)
    self.end_asm = self.js.rfind(js_optimizer.end_asm_marker)

    # imports
    self.imports_js = self.js[self.start_asm:self.start_funcs]
    self.imports = [m.group(0) for m in js_optimizer.import_sig.finditer(self.imports_js)]
    #print 'imports', self.imports

    # funcs
    self.funcs_js = self.js[self.start_funcs:self.end_funcs]
    self.funcs = [m.group(2) for m in js_optimizer.func_sig.finditer(self.funcs_js)]
    #print 'funcs', self.funcs

    # tables and exports
    post_js = self.js[self.end_funcs:self.end_asm]
    ret = post_js.find('return')
    self.tables_js = post_js[:ret]
    self.exports_js = post_js[ret:]

  def relocate_into(self, main):
    # imports
    main_imports = set(main.imports)
    new_imports = [imp for imp in self.imports if imp not in main_imports]
    print 'new imports', new_imports
    #main.imports_js += '\n'.join(new_imports)

    # Find function name replacements TODO: do not rename duplicate names with duplicate contents, just merge them
    main_funcs = set(main.funcs)
    replacements = {}
    for i in range(len(self.funcs)):
      rep = func = self.funcs[i]
      while rep in main_funcs:
        rep += '_'
        replacements[func] = rep
    #print replacements

    temp = shared.Building.js_optimizer(self.filename, ['asm', 'relocate'])
    relocated_funcs = AsmModule(temp)
    shared.try_delete(temp)
    main.extra_funcs_js = relocated_funcs.funcs_js

    # tables

    # exports

  def write(self, out):
    f = open(out, 'w')
    f.write(self.js[:self.start_asm])
    f.write(self.imports_js)
    f.write(self.funcs_js)
    f.write(self.extra_funcs_js)
    f.write(self.tables_js)
    f.write(self.exports_js)
    f.write(self.js[self.end_asm:])
    f.close()

main = AsmModule(main)
side = AsmModule(side)

side.relocate_into(main)
main.write(out)

