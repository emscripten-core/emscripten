#!/usr/bin/env python2

'''
Fast static linker for emscripten outputs. Specifically this links asm.js modules.

Usage: emlink.py [main module] [side module] [output name]

  Main module should be built with -s MAIN_MODULE=1 (and possibly -s DISABLE_GL_EMULATION=1, if you do not need GL emulation)
  Side module should be built with -s SIDE_MODULE=1

Note that the output file can be used as a main module, so you can link multiple
side modules into a main module that way.

Limitations:

 * Modules cannot be minified (but can be minified after linking)
 * We duplicate code in some cases, like overlapping names in different modules, and function aliases
 * We only share code between modules, not global variables. Global variables should be declared
   in the module that uses them, other modules can reach them through function calls. For example,
   a static class function implemented in one module can be called from another, and it can provide
   access to static variables on that class. (Note that implementing the static class function in a
   header will not work, then the code will be duplicated in each module, with only one of them
   able to correctly access the variable - the one in the same module as where the variable is
   declared.)
 * We do not link in compiled libraries (libc, libc++, etc.) in side modules. If the main module
   does not automatically link in the ones that side modules will need, you should compile the
   main module with

    EMCC_FORCE_STDLIBS=1 emcc ..

   which will link in all the libraries (you can also do EMCC_FORCE_STDLIBS=libc for example to
   include just libc and its dependencies; run with EMCC_DEBUG=1 to see which are necessary and
   include the first one).

Overall, this linking approach should be fast to perform, but generate less-optimal results than
to link all the bitcode together and build to JS as a single project. Final builds should be
done in that approach, but during development static linking can avoid recompiling your entire
project all the time.
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

shared.try_delete(out)

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
    self.staticbump = int(re.search(shared.JS.memory_staticbump_pattern, self.pre_js).group(1))
    if self.staticbump:
      self.mem_init_js = re.search(shared.JS.memory_initializer_pattern, self.pre_js).group(0)

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
    self.imports = {}
    for imp in js_optimizer.import_sig.finditer(self.imports_js):
      key, value = imp.group(0).split('var ')[1][:-1].replace(' ', '').split('=')
      self.imports[key] = value
    #print >> sys.stderr, 'imports', self.imports

    # funcs
    self.funcs_js = self.js[self.start_funcs:self.end_funcs]
    self.funcs = [m.group(2) for m in js_optimizer.func_sig.finditer(self.funcs_js)]
    #print 'funcs', self.funcs

    # tables and exports
    post_js = self.js[self.end_funcs:self.end_asm]
    ret = post_js.find('return')
    self.tables_js = post_js[:ret]
    self.exports_js = post_js[ret:]
    self.tables = self.parse_tables(self.tables_js)
    self.exports = set([export.strip() for export in self.exports_js[self.exports_js.find('{')+1:self.exports_js.find('}')].split(',')])

    # post
    self.post_js = self.js[self.end_asm:]
    self.sendings = set([sending.strip() for sending in self.post_js[self.post_js.find('}, { ')+5:self.post_js.find(' }, buffer);')].split(',')])
    self.module_defs = set(re.findall('var [\w\d_$]+ = Module\["[\w\d_$]+"\] = asm\["[\w\d_$]+"\];\n', self.post_js))

  def relocate_into(self, main):
    # heap initializer
    if self.staticbump > 0:
      new_mem_init = self.mem_init_js[:self.mem_init_js.rfind(', ')] + ', Runtime.GLOBAL_BASE+%d)' % main.staticbump
      main.pre_js = re.sub(shared.JS.memory_staticbump_pattern, 'STATICTOP = STATIC_BASE + %d;\n' % (main.staticbump + side.staticbump) + new_mem_init, main.pre_js, count=1)

    # Find function name replacements TODO: do not rename duplicate names with duplicate contents, just merge them
    main_funcs = set(main.funcs)
    replacements = {}
    for i in range(len(self.funcs)):
      rep = func = self.funcs[i]
      while rep in main_funcs:
        rep += '_'
        replacements[func] = rep
    #print >> sys.stderr, 'replacements:', replacements

    # imports
    new_imports = ['var %s = %s;' % (imp, self.imports[imp]) for imp in self.imports if imp not in main.imports and imp not in main_funcs]
    main.imports_js += '\n'.join(new_imports) + '\n'

    # sendings: add invokes for new tables
    new_sendings = []
    for table in self.tables:
      if table not in main.tables:
        sig = table[table.rfind('_')+1:]
        new_sendings.append('"invoke_%s": %s' % (sig, shared.JS.make_invoke(sig, named=False)))
    if new_sendings:
      sendings_js = ', '.join(main.sendings.union(new_sendings))
      sendings_start = main.post_js.find('}, { ')+5
      sendings_end = main.post_js.find(' }, buffer);')
      main.post_js = main.post_js[:sendings_start] + sendings_js + main.post_js[sendings_end:]

    # tables
    f_bases = {}
    f_sizes = {}
    for table, data in self.tables.iteritems():
      main.tables[table] = self.merge_tables(table, main.tables.get(table), data, replacements, f_bases, f_sizes)
    main.combine_tables()

    # relocate
    temp = shared.Building.js_optimizer(self.filename, ['asm', 'relocate', 'last'], extra_info={
      'replacements': replacements,
      'fBases': f_bases,
      'hBase': main.staticbump
    })
    #print >> sys.stderr, 'relocated side into', temp
    relocated_funcs = AsmModule(temp)
    shared.try_delete(temp)
    main.extra_funcs_js = relocated_funcs.funcs_js.replace(js_optimizer.start_funcs_marker, '\n')

    # update function table uses
    ft_marker = 'FUNCTION_TABLE_'

    def update_fts(what):
      updates = []
      i = 1 # avoid seeing marker in recursion
      while 1:
        i = what.find(ft_marker, i)
        if i < 0: break;
        start = i
        end = what.find('[', start)
        table = what[i:end]
        if table not in f_sizes:
          # table was not modified
          i += len(ft_marker)
          continue
        nesting = 1
        while nesting > 0:
          next = what.find(']', end+1)
          nesting -= 1
          nesting += what.count('[', end+1, next)
          end = next
        assert end > 0
        mask = what.rfind('&', start, end)
        assert mask > 0 and end - mask <= 13
        fixed = update_fts(what[start:mask+1] + str(f_sizes[table]-1) + ']')
        updates.append((start, end, fixed))
        i = end # additional function table uses were done by recursion
      # apply updates
      if len(updates) == 0: return what
      parts = []
      so_far = 0
      for i in range(len(updates)):
        start, end, fixed = updates[i]
        parts.append(what[so_far:start])
        parts.append(fixed)
        so_far = end+1
      parts.append(what[so_far:])
      return ''.join(parts)

    main.funcs_js = update_fts(main.funcs_js)
    main.extra_funcs_js = update_fts(main.extra_funcs_js)

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

    # exports
    def rep_exp(export):
      key, value = export.split(':')
      if key in replacements:
        repped = replacements[key]
        return repped + ': ' + repped
      return export
    my_exports = map(rep_exp, self.exports)
    exports = main.exports.union(my_exports)
    main.exports_js = 'return {' + ','.join(list(exports)) + '};\n})\n'

    # post
    def rep_def(deff):
      key = deff.split(' ')[1]
      if key in replacements:
        rep = replacements[key]
        return 'var %s = Module["%s"] = asm["%s"];\n' % (rep, rep, rep)
      return deff
    my_module_defs = map(rep_def, self.module_defs)
    new_module_defs = set(my_module_defs).difference(main.module_defs)
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

  # Utilities

  def parse_tables(self, js):
    tables = {}
    parts = js.split(';')
    for part in parts:
      if '=' not in part: continue
      part = part.split('var ')[1]
      name, data = part.split(' = ')
      tables[name] = data
    return tables

  def merge_tables(self, table, main, side, replacements, f_bases, f_sizes):
    side = side[1:-1].split(',')
    side = map(lambda f: replacements[f] if f in replacements else f, side)
    if not main:
      f_bases[table] = 0
      f_sizes[table] = len(side)
      return '[' + ','.join(side) + ']'
    main = main[1:-1].split(',')
    # TODO: handle non-aliasing case too
    assert len(main) % 2 == 0
    f_bases[table] = len(main)
    ret = main + side
    size = 2
    while size < len(ret): size *= 2
    aborter = ret[1] # we can assume odd indexes have an aborting function with the right signature
    ret = ret + [aborter]*(size - len(ret))
    assert len(ret) == size
    f_sizes[table] = size
    return '[' + ','.join(ret) + ']'

  def combine_tables(self):
    self.tables_js = '// EMSCRIPTEN_END_FUNCS\n'
    for table, data in self.tables.iteritems():
      self.tables_js += 'var %s = %s;\n' % (table, data)

main = AsmModule(main)
side = AsmModule(side)

side.relocate_into(main)
main.write(out)

