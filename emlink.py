#!/usr/bin/env python2

'''
Fast static linker for emscripten outputs. Specifically this links asm.js modules.

Usage: emlink.py [main module] [side module] [output name]

  Main module should be built with -s MAIN_MODULE=1 (and possibly -s DISABLE_GL_EMULATION=1, if you do not need GL emulation)
  Side module should be built with -s SIDE_MODULE=1

Note that the output file can be used as a main module, so you can link multiple
side modules into a main module that way.
'''

import os, subprocess, sys, re
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

    # pre
    self.pre_js = self.js[:self.start_asm]

    # heap initializer
    mem_init = re.search(shared.JS.memory_initializer_pattern, self.pre_js)
    if mem_init:
      self.mem_init_full_js = mem_init.group(0)
      self.mem_init_js = mem_init.groups(0)[0][:-2]
      self.mem_init_size = self.mem_init_js.count(',') + self.mem_init_js.count('concat') + 1 # XXX add testing for large and small ones
      pad = 8 - (self.mem_init_size % 8)
      #print >> sys.stderr, 'pad', self.mem_init_size, pad
      if pad < 8:
        self.mem_init_js += '.concat([' + ','.join(['0']*pad) + '])'
        self.mem_init_size += pad
    else:
      self.mem_init_js = ''
      self.mem_init_size = 0
    #print >> sys.stderr, self.mem_init_js

    # global initializers
    global_inits = re.search(shared.JS.global_initializers_pattern, self.pre_js)
    if global_inits:
      self.global_inits_js = global_inits.group(0)
      self.global_inits = map(lambda init: init.split('{')[2][1:].split('(')[0], global_inits.groups(0)[0].split(','))
    else:
      self.global_inits_js = ''
      self.global_inits = []

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
    self.exports = set([export.strip() for export in self.exports_js[self.exports_js.find('{')+1:self.exports_js.find('}')].split(',')])
    #print >> sys.stderr, self.exports

    # post
    self.post_js = self.js[self.end_asm:]
    self.module_defs = set(re.findall('var [\w\d_$]+ = Module\["[\w\d_$]+"\] = asm\["[\w\d_$]+"\];\n', self.post_js))

  def relocate_into(self, main):
    # heap initializer
    concat = '.concat(' if main.mem_init_js and self.mem_init_js else ''
    end = ')' if main.mem_init_js and self.mem_init_js else ''
    allocation = main.mem_init_js + concat + self.mem_init_js + end
    if allocation:
      full_allocation = '/* memory initializer */ allocate(' + allocation + ', "i8", ALLOC_NONE, Runtime.GLOBAL_BASE)'
      main.pre_js = re.sub(shared.JS.memory_initializer_pattern if main.mem_init_js else shared.JS.no_memory_initializer_pattern, full_allocation, main.pre_js, count=1)
      main.pre_js = re.sub('STATICTOP = STATIC_BASE \+ (\d+);', 'STATICTOP = STATIC_BASE + %d' % (main.mem_init_size + side.mem_init_size), main.pre_js, count=1)

    # imports
    main_imports = set(main.imports)
    new_imports = [imp for imp in self.imports if imp not in main_imports]
    main.imports_js += '\n'.join(new_imports)

    # Find function name replacements TODO: do not rename duplicate names with duplicate contents, just merge them
    main_funcs = set(main.funcs)
    replacements = {}
    for i in range(len(self.funcs)):
      rep = func = self.funcs[i]
      while rep in main_funcs:
        rep += '_'
        replacements[func] = rep

    temp = shared.Building.js_optimizer(self.filename, ['asm', 'relocate'], extra_info={
      'replacements': replacements,
      'fBase': 0,
      'hBase': main.mem_init_size
    })
    #print >> sys.stderr, 'relocated side into', temp
    relocated_funcs = AsmModule(temp)
    shared.try_delete(temp)
    main.extra_funcs_js = relocated_funcs.funcs_js.replace(js_optimizer.start_funcs_marker, '\n')

    # global initializers
    if self.global_inits:
      my_global_inits = map(lambda init: replacements[init] if init in replacements else init, self.global_inits)
      all_global_inits = map(lambda init: '{ func: function() { %s() } }' % init, main.global_inits + my_global_inits)
      all_global_inits_js = '/* global initializers */ __ATINIT__.push(' + ','.join(all_global_inits) + ');'
      if main.global_inits:
        target = main.global_inits_js
      else:
        target = '// === Body ===\n'
        all_global_inits_js = target + all_global_inits_js
      main.pre_js = main.pre_js.replace(target, all_global_inits_js)

    # tables TODO

    # exports
    exports = main.exports.union(self.exports)
    main.exports_js = 'return {' + ','.join(list(exports)) + '};\n})\n'

    # post
    new_module_defs = self.module_defs.difference(main.module_defs)
    if len(new_module_defs) > 0:
      position = main.post_js.find('Runtime.') # Runtime is the start of the hardcoded ones
      main.post_js = main.post_js[:position] + ''.join(list(new_module_defs)) + '\n' + main.post_js[position:]

  def write(self, out):
    f = open(out, 'w')
    f.write(self.pre_js)
    f.write(self.imports_js)
    f.write(self.funcs_js)
    f.write(self.extra_funcs_js)
    f.write(self.tables_js)
    f.write(self.exports_js)
    f.write(self.post_js)
    f.close()

main = AsmModule(main)
side = AsmModule(side)

side.relocate_into(main)
main.write(out)

