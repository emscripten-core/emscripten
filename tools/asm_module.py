
import sys, re, itertools

import shared, js_optimizer


class AsmModule():
  def __init__(self, filename):
    self.filename = filename
    self.js = open(filename).read()

    self.start_asm = self.js.find(js_optimizer.start_asm_marker)
    self.start_funcs = self.js.find(js_optimizer.start_funcs_marker)
    self.end_funcs = self.js.rfind(js_optimizer.end_funcs_marker)
    self.end_asm = self.js.rfind(js_optimizer.end_asm_marker)

    # pre and asm
    self.pre_js = self.js[:self.start_asm]
    self.asm_js = self.js[self.start_asm:self.end_asm]

    # heap initializer
    try:
      self.staticbump = int(re.search(shared.JS.memory_staticbump_pattern, self.pre_js).group(1))
    except:
      self.staticbump = 0
    if self.staticbump:
      try:
        self.mem_init_js = re.search(shared.JS.memory_initializer_pattern, self.pre_js).group(0)
      except:
        self.mem_init_js = ''

    # global initializers
    global_inits = re.search(shared.JS.global_initializers_pattern, self.pre_js)
    if global_inits:
      self.global_inits_js = global_inits.group(0)
      self.global_inits = map(lambda init: init.split('{')[2][1:].split('(')[0], global_inits.groups(0)[0].split(','))
    else:
      self.global_inits_js = ''
      self.global_inits = []

    # imports (and global variables)
    first_var = self.js.find('var ', self.js.find('var ', self.start_asm)+4)
    self.pre_imports_js = self.js[self.start_asm:first_var]
    self.imports_js = self.js[first_var:self.start_funcs]
    self.imports = {}
    for i in js_optimizer.import_sig.finditer(self.imports_js):
      imp = i.group(0).split('var ')[1][:-1]
      if ',' not in imp:
        key, value = imp.split('=', 1)
        self.imports[key.strip()] = value.strip()
      else:
        for part in imp.split(','):
          assert part.count('(') == part.count(')') # we must not break ',' in func(x, y)!
          assert part.count('=') == 1
          key, value = part.split('=')
          self.imports[key.strip()] = value.strip()

    #print >> sys.stderr, 'imports', self.imports

    # funcs
    self.funcs_js = self.js[self.start_funcs:self.end_funcs]
    self.funcs = set([m.group(1) for m in js_optimizer.func_sig.finditer(self.funcs_js)])
    #print 'funcs', self.funcs

    # tables and exports
    post_js = self.js[self.end_funcs:self.end_asm]
    ret = post_js.find('return ')
    self.tables_js = post_js[:ret]
    self.exports_js = post_js[ret:]
    self.tables = self.parse_tables(self.tables_js)
    self.exports = set([export.strip() for export in self.exports_js[self.exports_js.find('{')+1:self.exports_js.find('}')].split(',')])

    # post
    self.post_js = self.js[self.end_asm:]
    self.sendings = {}
    for sending in [sending.strip() for sending in self.post_js[self.post_js.find('}, { ')+5:self.post_js.find(' }, buffer);')].split(',')]:
      colon = sending.find(':')
      self.sendings[sending[:colon].replace('"', '')] = sending[colon+1:].strip()
    self.module_defs = set(re.findall('var [\w\d_$]+ = Module\["[\w\d_$]+"\] = asm\["[\w\d_$]+"\];\n', self.post_js))

    self.extra_funcs_js = ''

  def set_pre_js(self, staticbump=None, js=None):
    if staticbump is None: staticbump = self.staticbump
    if js is None: js = self.mem_init_js
    self.pre_js = re.sub(shared.JS.memory_staticbump_pattern, 'STATICTOP = STATIC_BASE + %d;\n' % (staticbump,) + js, self.pre_js, count=1)

  def relocate_into(self, main):
    # heap initializer
    if self.staticbump > 0:
      new_mem_init = self.mem_init_js[:self.mem_init_js.rfind(', ')] + ', Runtime.GLOBAL_BASE+%d)' % main.staticbump
      main.set_pre_js(main.staticbump + self.staticbump, new_mem_init)

    # Find function name replacements TODO: do not rename duplicate names with duplicate contents, just merge them
    replacements = {}
    for func in self.funcs:
      rep = func
      while rep in main.funcs:
        rep += '_'
        replacements[func] = rep
    #print >> sys.stderr, 'replacements:', replacements

    # sendings: add invokes for new tables
    all_sendings = main.sendings
    added_sending = False
    for table in self.tables:
      if table not in main.tables:
        sig = table[table.rfind('_')+1:]
        func = 'invoke_%s' % sig
        all_sendings[func] = func
        main.pre_js += 'var %s = %s;\n' % (func, shared.JS.make_invoke(sig, named=False))
        added_sending = True

    # imports
    all_imports = main.imports
    for key, value in self.imports.iteritems():
      if key in self.funcs or key in main.funcs: continue # external function in one module, implemented in the other
      value_concrete = '.' not in value # env.key means it is an import, an external value, and not a concrete one
      main_value = main.imports.get(key)
      main_value_concrete = main_value and '.' not in main_value
      if value_concrete and main_value_concrete: continue # standard global var
      if not main_value or value_concrete:
        if '+' in value:
          # relocate
          value = value.replace('(', '').replace(')', '').replace('| 0', '').replace('|0', '').replace(' ', '')
          left, right = value.split('+')
          assert left == 'H_BASE'
          value = str(main.staticbump + int(right))
        all_imports[key] = value
      if (value_concrete or main_value_concrete) and key in all_sendings:
        del all_sendings[key] # import of external value no longer needed
    for key in all_imports.keys():
      if key in self.funcs:
        del all_imports[key] # import in main, provided in side
    main.imports_js = '\n'.join(['var %s = %s;' % (key, value) for key, value in all_imports.iteritems()]) + '\n'

    # check for undefined references to global variables
    def check_import(key, value):
      if value.startswith('+') or value.endswith('|0'): # ignore functions
        if key not in all_sendings:
          print >> sys.stderr, 'warning: external variable %s is still not defined after linking' % key
          all_sendings[key] = '0'
    for key, value in all_imports.iteritems(): check_import(key, value)

    if added_sending:
      sendings_js = ', '.join(['%s: %s' % (key, value) for key, value in all_sendings.iteritems()])
      sendings_start = main.post_js.find('}, { ')+5
      sendings_end = main.post_js.find(' }, buffer);')
      main.post_js = main.post_js[:sendings_start] + sendings_js + main.post_js[sendings_end:]

    # tables
    f_bases = {}
    f_sizes = {}
    for table, data in self.tables.iteritems():
      main.tables[table] = self.merge_tables(table, main.tables.get(table), data, replacements, f_bases, f_sizes)
    main.combine_tables()
    #print >> sys.stderr, 'f bases', f_bases

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
    f.write(self.pre_imports_js)
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
      name, data = part.split('=')
      tables[name.strip()] = data.strip()
    return tables

  def merge_tables(self, table, main, side, replacements, f_bases, f_sizes):
    sig = table.split('_')[-1]
    side = side[1:-1].split(',')
    side = map(lambda s: s.strip(), side)
    side = map(lambda f: replacements[f] if f in replacements else f, side)
    if not main:
      f_bases[sig] = 0
      f_sizes[table] = len(side)
      return '[' + ','.join(side) + ']'
    main = main[1:-1].split(',')
    main = map(lambda m: m.strip(), main)
    # TODO: handle non-aliasing case too
    assert len(main) % 2 == 0
    f_bases[sig] = len(main)
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

  def get_table_funcs(self):
    return set(itertools.chain.from_iterable(map(lambda x: map(lambda y: y.strip(), x[1:-1].split(',')), self.tables.values())))

  def get_funcs_map(self):
    funcs = js_optimizer.split_funcs(self.funcs_js)
    ret = {}
    for name, content in funcs:
      ret[name] = content
    return ret

  def apply_funcs_map(self, funcs_map): # assumes self.funcs is the set of funcs, in the right order
    jses = []
    for f in self.funcs:
      if f in funcs_map: # TODO: fix
        jses.append(funcs_map[f])
    self.funcs_js = '\n'.join(jses)

  def get_import_type(self, imp):
    def is_int(x):
      try:
        int(x)
        return True
      except:
        return False

    def is_float(x):
      try:
        float(x)
        return True
      except:
        return False

    if '|0' in imp or '| 0' in imp or (is_int(imp) and not '.0' in imp or '+' in imp):
      return 'i'
    elif '.0' in imp or '+' in imp or is_float(imp):
      return 'd'
    else:
      return '?'

