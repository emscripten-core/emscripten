#!/usr/bin/env python2

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import os, sys, json, optparse, subprocess, re, time, logging

from tools import shared
from tools import jsrun, cache as cache_module, tempfiles
from tools.response_file import read_response_file
from tools.shared import WINDOWS

__rootpath__ = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  """Returns the absolute path for which the given path elements are
  relative to the emscripten root.
  """
  return os.path.join(__rootpath__, *pathelems)

def get_configuration():
  if hasattr(get_configuration, 'configuration'):
    return get_configuration.configuration

  configuration = shared.Configuration(environ=os.environ)
  get_configuration.configuration = configuration
  return configuration

STDERR_FILE = os.environ.get('EMCC_STDERR_FILE')
if STDERR_FILE:
  STDERR_FILE = os.path.abspath(STDERR_FILE)
  logging.info('logging stderr in js compiler phase into %s' % STDERR_FILE)
  STDERR_FILE = open(STDERR_FILE, 'w')

def quoter(settings):
  def quote(prop):
    if settings['USE_CLOSURE_COMPILER'] == 2:
      return ''.join(map(lambda p: "'" + p + "'", prop.split('.')))
    else:
      return prop
  return quote

def access_quoter(settings):
  def access_quote(prop):
    if settings['USE_CLOSURE_COMPILER'] == 2:
      return ''.join(map(lambda p: "['" + p + "']", prop.split('.')))
    else:
      return '.' + prop
  return access_quote


def emscript(infile, settings, outfile, libraries=None, compiler_engine=None,
             temp_files=None, DEBUG=None):
  """Runs the emscripten LLVM-to-JS compiler.

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted settings that override the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """

  if libraries is None: libraries = []

  assert settings['ASM_JS'], 'fastcomp is asm.js-only (mode 1 or 2)'

  success = False

  try:

    # Overview:
    #   * Run LLVM backend to emit JS. JS includes function bodies, memory initializer,
    #     and various metadata
    #   * Run compiler.js on the metadata to emit the shell js code, pre/post-ambles,
    #     JS library dependencies, etc.

    # metadata and settings are modified by reference in some of the below
    # these functions are split up to force variables to go out of scope and allow
    # memory to be reclaimed

    funcs, metadata, mem_init = get_and_parse_backend(infile, settings, temp_files, DEBUG)
    glue, forwarded_data = compiler_glue(metadata, settings, libraries, compiler_engine, temp_files, DEBUG)

    post, funcs_js, need_asyncify, provide_fround, asm_safe_heap, sending, receiving, asm_setup, the_global, asm_global_vars, asm_global_funcs, pre_tables, final_function_tables, exports, last_forwarded_json = function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, settings, outfile, DEBUG)
    finalize_output(metadata, post, funcs_js, need_asyncify, provide_fround, asm_safe_heap, sending, receiving, asm_setup, the_global, asm_global_vars, asm_global_funcs, pre_tables, final_function_tables, exports, last_forwarded_json, settings, outfile, DEBUG)

    success = True

  finally:
    outfile.close()
    if not success:
      shared.try_delete(outfile.name) # remove partial output

def get_and_parse_backend(infile, settings, temp_files, DEBUG):
    temp_js = temp_files.get('.4.js').name
    backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
    backend_args = [backend_compiler, infile, '-march=js', '-filetype=asm', '-o', temp_js]
    if settings['PRECISE_F32']:
      backend_args += ['-emscripten-precise-f32']
    if settings['USE_PTHREADS']:
      backend_args += ['-emscripten-enable-pthreads']
    if settings['WARN_UNALIGNED']:
      backend_args += ['-emscripten-warn-unaligned']
    if settings['RESERVED_FUNCTION_POINTERS'] > 0:
      backend_args += ['-emscripten-reserved-function-pointers=%d' % settings['RESERVED_FUNCTION_POINTERS']]
    if settings['ASSERTIONS'] > 0:
      backend_args += ['-emscripten-assertions=%d' % settings['ASSERTIONS']]
    if settings['ALIASING_FUNCTION_POINTERS'] == 0:
      backend_args += ['-emscripten-no-aliasing-function-pointers']
    if settings['EMULATED_FUNCTION_POINTERS']:
      backend_args += ['-emscripten-emulated-function-pointers']
    if settings['RELOCATABLE']:
      backend_args += ['-emscripten-relocatable']
      backend_args += ['-emscripten-global-base=0']
    elif settings['GLOBAL_BASE'] >= 0:
      backend_args += ['-emscripten-global-base=%d' % settings['GLOBAL_BASE']]
    backend_args += ['-O' + str(settings['OPT_LEVEL'])]
    if settings['DISABLE_EXCEPTION_CATCHING'] != 1:
      backend_args += ['-enable-emscripten-cxx-exceptions']
      if settings['DISABLE_EXCEPTION_CATCHING'] == 2:
        backend_args += ['-emscripten-cxx-exceptions-whitelist=' + ','.join(settings['EXCEPTION_CATCHING_WHITELIST'] or ['fake'])]
    if settings['ASYNCIFY']:
      backend_args += ['-emscripten-asyncify']
      backend_args += ['-emscripten-asyncify-functions=' + ','.join(settings['ASYNCIFY_FUNCTIONS'])]
      backend_args += ['-emscripten-asyncify-whitelist=' + ','.join(settings['ASYNCIFY_WHITELIST'])]
    if settings['NO_EXIT_RUNTIME']:
      backend_args += ['-emscripten-no-exit-runtime']
    if settings['BINARYEN']:
      backend_args += ['-emscripten-wasm']
    if settings['CYBERDWARF']:
      backend_args += ['-enable-cyberdwarf']

    if DEBUG:
      logging.debug('emscript: llvm backend: ' + ' '.join(backend_args))
      t = time.time()
    shared.jsrun.timeout_run(subprocess.Popen(backend_args, stdout=subprocess.PIPE))
    if DEBUG:
      logging.debug('  emscript: llvm backend took %s seconds' % (time.time() - t))

    # Split up output
    backend_output = open(temp_js).read()
    #if DEBUG: print >> sys.stderr, backend_output

    start_funcs_marker = '// EMSCRIPTEN_START_FUNCTIONS'
    end_funcs_marker = '// EMSCRIPTEN_END_FUNCTIONS'
    metadata_split_marker = '// EMSCRIPTEN_METADATA'

    start_funcs = backend_output.index(start_funcs_marker)
    end_funcs = backend_output.rindex(end_funcs_marker)
    metadata_split = backend_output.rindex(metadata_split_marker)

    funcs = backend_output[start_funcs+len(start_funcs_marker):end_funcs]
    metadata_raw = backend_output[metadata_split+len(metadata_split_marker):]
    #if DEBUG: print >> sys.stderr, "METAraw", metadata_raw
    try:
      metadata = json.loads(metadata_raw)
    except Exception, e:
      logging.error('emscript: failure to parse metadata output from compiler backend. raw output is: \n' + metadata_raw)
      raise e
    mem_init = backend_output[end_funcs+len(end_funcs_marker):metadata_split]
    #if DEBUG: print >> sys.stderr, "FUNCS", funcs
    #if DEBUG: print >> sys.stderr, "META", metadata
    #if DEBUG: print >> sys.stderr, "meminit", mem_init

    # if emulating pointer casts, force all tables to the size of the largest
    if settings['EMULATE_FUNCTION_POINTER_CASTS']:
      max_size = 0
      for k, v in metadata['tables'].iteritems():
        max_size = max(max_size, v.count(',')+1)
      for k, v in metadata['tables'].iteritems():
        curr = v.count(',')+1
        if curr < max_size:
          metadata['tables'][k] = v.replace(']', (',0'*(max_size - curr)) + ']')

    if settings['SIDE_MODULE']:
      for k in metadata['tables'].keys():
        metadata['tables'][k] = metadata['tables'][k].replace('var FUNCTION_TABLE_', 'var SIDE_FUNCTION_TABLE_')

    # function table masks

    table_sizes = {}
    for k, v in metadata['tables'].iteritems():
      table_sizes[k] = str(v.count(',')) # undercounts by one, but that is what we want
      #if settings['ASSERTIONS'] >= 2 and table_sizes[k] == 0:
      #  print >> sys.stderr, 'warning: no function pointers with signature ' + k + ', but there is a call, which will abort if it occurs (this can result from undefined behavior, check for compiler warnings on your source files and consider -Werror)'
    funcs = re.sub(r"#FM_(\w+)#", lambda m: table_sizes[m.groups(0)[0]], funcs)

    # fix +float into float.0, if not running js opts
    if not settings['RUNNING_JS_OPTS']:
      def fix_dot_zero(m):
        num = m.group(3)
        # TODO: handle 0x floats?
        if num.find('.') < 0:
          e = num.find('e')
          if e < 0:
            num += '.0'
          else:
            num = num[:e] + '.0' + num[e:]
        return m.group(1) + m.group(2) + num
      funcs = re.sub(r'([(=,+\-*/%<>:?] *)\+(-?)((0x)?[0-9a-f]*\.?[0-9]+([eE][-+]?[0-9]+)?)', fix_dot_zero, funcs)

    return funcs, metadata, mem_init

def compiler_glue(metadata, settings, libraries, compiler_engine, temp_files, DEBUG):
    # js compiler

    if DEBUG:
      logging.debug('emscript: js compiler glue')
      t = time.time()

    # Settings changes
    i64_funcs = ['i64Add', 'i64Subtract', '__muldi3', '__divdi3', '__udivdi3', '__remdi3', '__uremdi3']
    for i64_func in i64_funcs:
      if i64_func in metadata['declares']:
        settings['PRECISE_I64_MATH'] = 2
        break

    metadata['declares'] = filter(lambda i64_func: i64_func not in ['getHigh32', 'setHigh32'], metadata['declares']) # FIXME: do these one by one as normal js lib funcs

    # Syscalls optimization. Our syscalls are static, and so if we see a very limited set of them - in particular,
    # no open() syscall and just simple writing - then we don't need full filesystem support.
    # If FORCE_FILESYSTEM is set, we can't do this. We also don't do it if INCLUDE_FULL_LIBRARY, since
    # not including the filesystem would mean not including the full JS libraries, and the same for
    # MAIN_MODULE since a side module might need the filesystem.
    if not settings['NO_FILESYSTEM'] and not settings['FORCE_FILESYSTEM'] and not settings['INCLUDE_FULL_LIBRARY'] and not settings['MAIN_MODULE']:
      syscall_prefix = '__syscall'
      syscalls = filter(lambda declare: declare.startswith(syscall_prefix), metadata['declares'])
      def is_int(x):
        try:
          int(x)
          return True
        except:
          return False
      syscalls = filter(lambda declare: is_int(declare[len(syscall_prefix):]), syscalls)
      syscalls = map(lambda declare: int(declare[len(syscall_prefix):]), syscalls)
      if set(syscalls).issubset(set([6, 54, 140, 146])): # close, ioctl, llseek, writev
        if DEBUG: logging.debug('very limited syscalls (%s) so disabling full filesystem support' % ', '.join(map(str, syscalls)))
        settings['NO_FILESYSTEM'] = 1

    if settings['CYBERDWARF']:
      settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'].append("cyberdwarf_Debugger")
      settings['EXPORTED_FUNCTIONS'].append("cyberdwarf_Debugger")

    # Integrate info from backend
    if settings['SIDE_MODULE']:
      settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = [] # we don't need any JS library contents in side modules
    settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = list(
      set(settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] + map(shared.JS.to_nice_ident, metadata['declares'])).difference(
        map(lambda x: x[1:], metadata['implementedFunctions'])
      )
    ) + map(lambda x: x[1:], metadata['externs'])
    if metadata['simd']:
      settings['SIMD'] = 1
    if metadata['cantValidate'] and settings['ASM_JS'] != 2:
      logging.warning('disabling asm.js validation due to use of non-supported features: ' + metadata['cantValidate'])
      settings['ASM_JS'] = 2

    settings['MAX_GLOBAL_ALIGN'] = metadata['maxGlobalAlign']

    settings['IMPLEMENTED_FUNCTIONS'] = metadata['implementedFunctions']

    assert not (metadata['simd'] and settings['SPLIT_MEMORY']), 'SIMD is used, but not supported in SPLIT_MEMORY'

    # Save settings to a file to work around v8 issue 1579
    settings_file = temp_files.get('.txt').name
    def save_settings():
      global settings_text
      settings_text = json.dumps(settings, sort_keys=True)
      s = open(settings_file, 'w')
      s.write(settings_text)
      s.close()
    save_settings()

    # Call js compiler
    out = jsrun.run_js(path_from_root('src', 'compiler.js'), compiler_engine,
                       [settings_file] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                       cwd=path_from_root('src'), error_limit=300)
    assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
    glue, forwarded_data = out.split('//FORWARDED_DATA:')

    if DEBUG:
      logging.debug('  emscript: glue took %s seconds' % (time.time() - t))

    return glue, forwarded_data

def function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, settings, outfile, DEBUG):

    if DEBUG:
      logging.debug('emscript: python processing: function tables and exports')
      t = time.time()

    access_quote = access_quoter(settings)
    quote = quoter(settings)

    last_forwarded_json = forwarded_json = json.loads(forwarded_data)

    # merge in information from llvm backend

    last_forwarded_json['Functions']['tables'] = metadata['tables']

    pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

    #print >> sys.stderr, 'glue:', pre, '\n\n||||||||||||||||\n\n', post, '...............'

    # memory and global initializers

    global_initializers = str(', '.join(map(lambda i: '{ func: function() { %s() } }' % i, metadata['initializers'])))

    if settings['SIMD'] == 1:
      pre = open(path_from_root(os.path.join('src', 'ecmascript_simd.js'))).read() + '\n\n' + pre

    staticbump = metadata['staticBump']
    while staticbump % 16 != 0: staticbump += 1
    pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + %d;%s
  /* global initializers */ %s __ATINIT__.push(%s);
  %s''' % (staticbump,
           'assert(STATICTOP < SPLIT_MEMORY, "SPLIT_MEMORY size must be big enough so the entire static memory, need " + STATICTOP);' if settings['SPLIT_MEMORY'] else '',
           'if (!ENVIRONMENT_IS_PTHREAD)' if settings['USE_PTHREADS'] else '',
           global_initializers,
           mem_init))

    if settings['SIDE_MODULE']:
      pre = pre.replace('Runtime.GLOBAL_BASE', 'gb')
    if settings['SIDE_MODULE'] or settings['BINARYEN']:
      pre = pre.replace('{{{ STATIC_BUMP }}}', str(staticbump))

    funcs_js = [funcs]
    parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
    if len(parts) > 1:
      pre = parts[0]
      funcs_js.append(parts[1])

    # merge forwarded data
    settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
    all_exported_functions = set(shared.expand_response(settings['EXPORTED_FUNCTIONS'])) # both asm.js and otherwise

    for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
      all_exported_functions.add('_' + additional_export)
    if settings['EXPORT_FUNCTION_TABLES']:
      for table in last_forwarded_json['Functions']['tables'].values():
        for func in table.split('[')[1].split(']')[0].split(','):
          if func[0] == '_':
            all_exported_functions.add(func)
    exported_implemented_functions = set(metadata['exports'])
    export_bindings = settings['EXPORT_BINDINGS']
    export_all = settings['EXPORT_ALL']
    all_implemented = metadata['implementedFunctions'] + forwarded_json['Functions']['implementedFunctions'].keys() # XXX perf?
    for key in all_implemented:
      if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
        exported_implemented_functions.add(key)
    implemented_functions = set(metadata['implementedFunctions'])
    if settings['ASSERTIONS'] and settings.get('ORIGINAL_EXPORTED_FUNCTIONS'):
      original_exports = settings['ORIGINAL_EXPORTED_FUNCTIONS']
      if original_exports[0] == '@': original_exports = json.loads(open(original_exports[1:]).read())
      for requested in original_exports:
        # check if already implemented
        # special-case malloc, EXPORTED by default for internal use, but we bake in a trivial allocator and warn at runtime if used in ASSERTIONS \
        if requested not in all_implemented and \
           requested != '_malloc' and \
           (('function ' + requested.encode('utf-8')) not in pre): # could be a js library func
          logging.warning('function requested to be exported, but not implemented: "%s"', requested)

    asm_consts = [0]*len(metadata['asmConsts'])
    all_sigs = []
    for k, v in metadata['asmConsts'].iteritems():
      const = v[0].encode('utf-8')
      sigs = v[1]
      if const[0] == '"' and const[-1] == '"':
        const = const[1:-1]
      const = '{ ' + const + ' }'
      args = []
      arity = max(map(len, sigs)) - 1
      for i in range(arity):
        args.append('$' + str(i))
      const = 'function(' + ', '.join(args) + ') ' + const
      asm_consts[int(k)] = const
      all_sigs += sigs

    asm_const_funcs = []
    for sig in set(all_sigs):
      forwarded_json['Functions']['libraryFunctions']['_emscripten_asm_const_' + sig] = 1
      args = ['a%d' % i for i in range(len(sig)-1)]
      all_args = ['code'] + args
      asm_const_funcs.append(r'''
function _emscripten_asm_const_%s(%s) {
 return ASM_CONSTS[code](%s);
}''' % (sig.encode('utf-8'), ', '.join(all_args), ', '.join(args)))

    pre = pre.replace('// === Body ===', '// === Body ===\n' + '\nvar ASM_CONSTS = [' + ',\n '.join(asm_consts) + '];\n' + '\n'.join(asm_const_funcs) + '\n')

    #if DEBUG: outfile.write('// pre\n')
    outfile.write(pre)
    pre = None

    #if DEBUG: outfile.write('// funcs\n')

    # when emulating function pointer casts, we need to know what is the target of each pointer
    if settings['EMULATE_FUNCTION_POINTER_CASTS']:
      function_pointer_targets = {}
      for sig, table in last_forwarded_json['Functions']['tables'].iteritems():
        start = table.index('[')
        end = table.rindex(']')
        body = table[start+1:end].split(',')
        parsed = map(lambda x: x.strip(), body)
        for i in range(len(parsed)):
          if parsed[i] != '0':
            assert i not in function_pointer_targets
            function_pointer_targets[i] = [sig, str(parsed[i])]

    # Move preAsms to their right place
    def move_preasm(m):
      contents = m.groups(0)[0]
      outfile.write(contents + '\n')
      return ''
    if not settings['BOOTSTRAPPING_STRUCT_INFO'] and len(funcs_js) > 1:
      funcs_js[1] = re.sub(r'/\* PRE_ASM \*/(.*)\n', move_preasm, funcs_js[1])

    class Counter:
      i = 0
      j = 0
    if 'pre' in last_forwarded_json['Functions']['tables']:
      pre_tables = last_forwarded_json['Functions']['tables']['pre']
      del last_forwarded_json['Functions']['tables']['pre']
    else:
      pre_tables = ''

    def unfloat(s):
      return 'd' if s == 'f' else s # lower float to double for ffis

    if settings['ASSERTIONS'] >= 2:
      debug_tables = {}

    def make_params(sig): return ','.join(['p%d' % p for p in range(len(sig)-1)])
    def make_coerced_params(sig): return ','.join([shared.JS.make_coercion('p%d', unfloat(sig[p+1]), settings) % p for p in range(len(sig)-1)])
    def make_coercions(sig): return ';'.join(['p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, sig[p+1], settings)) for p in range(len(sig)-1)]) + ';'
    def make_func(name, code, params, coercions): return 'function %s(%s) {\n %s %s\n}' % (name, params, coercions, code)

    in_table = set()

    def make_table(sig, raw):
      params = make_params(sig)
      coerced_params = make_coerced_params(sig)
      coercions = make_coercions(sig)
      def make_bad(target=None):
        i = Counter.i
        Counter.i += 1
        if target is None: target = i
        name = 'b' + str(i)
        if not settings['ASSERTIONS']:
          code = 'abort(%s);' % target
        else:
          code = 'nullFunc_' + sig + '(%d);' % target
        if sig[0] != 'v':
          code += 'return %s' % shared.JS.make_initializer(sig[0], settings) + ';'
        return name, make_func(name, code, params, coercions)
      bad, bad_func = make_bad() # the default bad func
      if settings['ASSERTIONS'] <= 1:
        Counter.pre = [bad_func]
      else:
        Counter.pre = []
      start = raw.index('[')
      end = raw.rindex(']')
      body = raw[start+1:end].split(',')
      if settings['EMULATED_FUNCTION_POINTERS']:
        def receive(item):
          if item == '0':
            return item
          else:
            if item in all_implemented:
              in_table.add(item)
              return "asm['" + item + "']"
            else:
              return item # this is not implemented; it would normally be wrapped, but with emulation, we just use it directly outside
        body = map(receive, body)
      for j in range(settings['RESERVED_FUNCTION_POINTERS']):
        curr = 'jsCall_%s_%s' % (sig, j)
        body[settings['FUNCTION_POINTER_ALIGNMENT'] * (1 + j)] = curr
        implemented_functions.add(curr)
      Counter.j = 0
      def fix_item(item):
        j = Counter.j
        Counter.j += 1
        newline = Counter.j % 30 == 29
        if item == '0':
          if j > 0 and settings['EMULATE_FUNCTION_POINTER_CASTS'] and j in function_pointer_targets: # emulate all non-null pointer calls, if asked to
            proper_sig, proper_target = function_pointer_targets[j]
            if settings['EMULATED_FUNCTION_POINTERS']:
              if proper_target in all_implemented:
                proper_target = "asm['" + proper_target + "']"
            def make_emulated_param(i):
              if i >= len(sig): return shared.JS.make_initializer(proper_sig[i], settings) # extra param, just send a zero
              return shared.JS.make_coercion('p%d' % (i-1), proper_sig[i], settings, convert_from=sig[i])
            proper_code = proper_target + '(' + ','.join(map(lambda i: make_emulated_param(i+1), range(len(proper_sig)-1))) + ')'
            if proper_sig[0] != 'v':
              # proper sig has a return, which the wrapper may or may not use
              proper_code = shared.JS.make_coercion(proper_code, proper_sig[0], settings)
              if proper_sig[0] != sig[0]:
                # first coercion ensured we call the target ok; this one ensures we return the right type in the wrapper
                proper_code = shared.JS.make_coercion(proper_code, sig[0], settings, convert_from=proper_sig[0])
              if sig[0] != 'v':
                proper_code = 'return ' + proper_code
            else:
              # proper sig has no return, we may need a fake return
              if sig[0] != 'v':
                proper_code = 'return ' + shared.JS.make_initializer(sig[0], settings)
            name = 'fpemu_%s_%d' % (sig, j)
            wrapper = make_func(name, proper_code, params, coercions)
            Counter.pre.append(wrapper)
            return name if not newline else (name + '\n')
          if settings['ASSERTIONS'] <= 1:
            return bad if not newline else (bad + '\n')
          else:
            specific_bad, specific_bad_func = make_bad(j)
            Counter.pre.append(specific_bad_func)
            return specific_bad if not newline else (specific_bad + '\n')
        clean_item = item.replace("asm['", '').replace("']", '')
        # when emulating function pointers, we don't need wrappers
        # but if relocating, then we also have the copies in-module, and do
        # also if binaryen, as wasm requires wrappers for the function table
        if clean_item not in implemented_functions and not (settings['EMULATED_FUNCTION_POINTERS'] and not settings['RELOCATABLE'] and not settings['BINARYEN']):
          # this is imported into asm, we must wrap it
          call_ident = clean_item
          if call_ident in metadata['redirects']: call_ident = metadata['redirects'][call_ident]
          if not call_ident.startswith('_') and not call_ident.startswith('Math_'): call_ident = '_' + call_ident
          code = call_ident + '(' + coerced_params + ')'
          if sig[0] != 'v':
            # ffis cannot return float
            if sig[0] == 'f': code = '+' + code
            code = 'return ' + shared.JS.make_coercion(code, sig[0], settings)
          code += ';'
          Counter.pre.append(make_func(clean_item + '__wrapper', code, params, coercions))
          return clean_item + '__wrapper'
        return item if not newline else (item + '\n')
      if settings['ASSERTIONS'] >= 2:
        debug_tables[sig] = body
      body = ','.join(map(fix_item, body))
      return ('\n'.join(Counter.pre), ''.join([raw[:start+1], body, raw[end:]]))

    infos = [make_table(sig, raw) for sig, raw in last_forwarded_json['Functions']['tables'].iteritems()]
    Counter.pre = []

    function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n'
    function_tables_defs += '\n// EMSCRIPTEN_END_FUNCS\n'
    function_tables_defs += '\n'.join([info[1] for info in infos])

    asm_setup = ''

    if settings['ASSERTIONS'] >= 2:
      for sig in last_forwarded_json['Functions']['tables']:
        asm_setup += '\nvar debug_table_' + sig + ' = ' + json.dumps(debug_tables[sig]) + ';'

    maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul', 'min', 'clz32']]
    simdfloattypes = []
    simdinttypes = []
    simdbooltypes = []
    simdfuncs = ['splat', 'check', 'extractLane', 'replaceLane']
    simdintfloatfuncs = ['add', 'sub', 'neg', 'mul',
                         'equal', 'lessThan', 'greaterThan',
                         'notEqual', 'lessThanOrEqual', 'greaterThanOrEqual',
                         'select', 'swizzle', 'shuffle',
                         'load', 'store', 'load1', 'store1', 'load2', 'store2', 'load3', 'store3']
    simdintboolfuncs = ['and', 'xor', 'or', 'not']
    if metadata['simdUint8x16']:
      simdinttypes += ['Uint8x16']
      simdintfloatfuncs += ['fromUint8x16Bits']
    if metadata['simdInt8x16']:
      simdinttypes += ['Int8x16']
      simdintfloatfuncs += ['fromInt8x16Bits']
    if metadata['simdUint16x8']:
      simdinttypes += ['Uint16x8']
      simdintfloatfuncs += ['fromUint16x8Bits']
    if metadata['simdInt16x8']:
      simdinttypes += ['Int16x8']
      simdintfloatfuncs += ['fromInt16x8Bits']
    if metadata['simdUint32x4']:
      simdinttypes += ['Uint32x4']
      simdintfloatfuncs += ['fromUint32x4Bits']
    if metadata['simdInt32x4']:
      simdinttypes += ['Int32x4']
      simdintfloatfuncs += ['fromInt32x4Bits']
    if metadata['simdFloat32x4']:
      simdfloattypes += ['Float32x4']
      simdintfloatfuncs += ['fromFloat32x4Bits']
    if metadata['simdFloat64x2']:
      simdfloattypes += ['Float64x2']
      simdintfloatfuncs += ['fromFloat64x2Bits']
    if metadata['simdBool8x16']:
      simdbooltypes += ['Bool8x16']
    if metadata['simdBool16x8']:
      simdbooltypes += ['Bool16x8']
    if metadata['simdBool32x4']:
      simdbooltypes += ['Bool32x4']
    if metadata['simdBool64x2']:
      simdbooltypes += ['Bool64x2']

    simdfloatfuncs = simdfuncs + simdintfloatfuncs + ['div', 'min', 'max', 'minNum', 'maxNum', 'sqrt',
                                  'abs', 'reciprocalApproximation', 'reciprocalSqrtApproximation']
    simdintfuncs = simdfuncs + simdintfloatfuncs + simdintboolfuncs + ['shiftLeftByScalar', 'shiftRightByScalar', 'addSaturate', 'subSaturate']
    simdboolfuncs = simdfuncs + simdintboolfuncs + ['anyTrue', 'allTrue']
    simdtypes = simdfloattypes + simdinttypes + simdbooltypes

    fundamentals = ['Math']
    fundamentals += ['Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array']
    fundamentals += ['NaN', 'Infinity']
    if metadata['simd']:
        fundamentals += ['SIMD']
    if settings['ALLOW_MEMORY_GROWTH']: fundamentals.append('byteLength')
    math_envs = []

    provide_fround = settings['PRECISE_F32'] or settings['SIMD']

    if provide_fround: maths += ['Math.fround']

    def get_function_pointer_error(sig):
      if settings['ASSERTIONS'] <= 1:
        extra = ' Module["printErr"]("Build with ASSERTIONS=2 for more info.");'
        pointer = ' '
      else:
        pointer = ' \'" + x + "\' '
        extra = ' Module["printErr"]("This pointer might make sense in another type signature: '
        # sort signatures, attempting to show most likely related ones first
        sigs = last_forwarded_json['Functions']['tables'].keys()
        def keyfunc(other):
          ret = 0
          minlen = min(len(other), len(sig))
          maxlen = min(len(other), len(sig))
          if other.startswith(sig) or sig.startswith(other): ret -= 1000 # prioritize prefixes, could be dropped params
          ret -= 133*difflib.SequenceMatcher(a=other, b=sig).ratio() # prioritize on diff similarity
          ret += 15*abs(len(other) - len(sig))/float(maxlen) # deprioritize the bigger the length difference is
          for i in range(minlen):
            if other[i] == sig[i]: ret -= 5/float(maxlen) # prioritize on identically-placed params
          ret += 20*len(other) # deprioritize on length
          return ret
        sigs.sort(key=keyfunc)
        for other in sigs:
          if other != sig:
            extra += other + ': " + debug_table_' + other + '[x] + "  '
        extra += '"); '
      return 'Module["printErr"]("Invalid function pointer' + pointer + 'called with signature \'' + sig + '\'. ' + \
             'Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? ' + \
             'Or calling a function with an incorrect type, which will fail? ' + \
             '(it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)' + \
             '"); ' + extra

    basic_funcs = ['abort', 'assert'] + [m.replace('.', '_') for m in math_envs]
    if settings['STACK_OVERFLOW_CHECK']: basic_funcs += ['abortStackOverflow']

    asm_safe_heap = settings['SAFE_HEAP'] and not settings['SAFE_HEAP_LOG'] and not settings['RELOCATABLE'] # optimized safe heap in asm, when we can

    if settings['SAFE_HEAP']:
      if asm_safe_heap:
        basic_funcs += ['segfault', 'alignfault', 'ftfault']
      else:
        basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_LOAD_D', 'SAFE_HEAP_STORE', 'SAFE_HEAP_STORE_D', 'SAFE_FT_MASK']
    if settings['ASSERTIONS']:
      if settings['ASSERTIONS'] >= 2: import difflib
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_funcs += ['nullFunc_' + sig]
        asm_setup += '\nfunction nullFunc_' + sig + '(x) { ' + get_function_pointer_error(sig) + 'abort(x) }\n'

    basic_vars = ['STACKTOP', 'STACK_MAX', 'tempDoublePtr', 'ABORT']
    basic_float_vars = []

    if settings['SAFE_HEAP']: basic_vars += ['DYNAMICTOP']

    if metadata.get('preciseI64MathUsed'):
      basic_vars += ['cttz_i8']
    else:
      if forwarded_json['Functions']['libraryFunctions'].get('_llvm_cttz_i32'):
        basic_vars += ['cttz_i8']

    if settings['RELOCATABLE']:
      basic_vars += ['gb', 'fb']
      if not settings['SIDE_MODULE']:
        asm_setup += 'var gb = Runtime.GLOBAL_BASE, fb = 0;\n'

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'establishStackSpace', 'setThrew']
    if not settings['RELOCATABLE']:
      asm_runtime_funcs += ['setTempRet0', 'getTempRet0']
    else:
      basic_funcs += ['setTempRet0', 'getTempRet0']
      asm_setup += 'var setTempRet0 = Runtime.setTempRet0, getTempRet0 = Runtime.getTempRet0;\n'

    # See if we need ASYNCIFY functions
    # We might not need them even if ASYNCIFY is enabled
    need_asyncify = '_emscripten_alloc_async_context' in exported_implemented_functions
    if need_asyncify:
      basic_vars += ['___async', '___async_unwind', '___async_retval', '___async_cur_frame']
      asm_runtime_funcs += ['setAsync']

    if settings.get('EMTERPRETIFY'):
      asm_runtime_funcs += ['emterpret']
      if settings.get('EMTERPRETIFY_ASYNC'):
        asm_runtime_funcs += ['setAsyncState', 'emtStackSave', 'emtStackRestore']

    if settings['SAFE_HEAP']:
      asm_runtime_funcs += ['setDynamicTop']

    if settings['ONLY_MY_CODE']:
      asm_runtime_funcs = []

    # function tables
    if not settings['EMULATED_FUNCTION_POINTERS']:
      function_tables = ['dynCall_' + table for table in last_forwarded_json['Functions']['tables']]
    else:
      function_tables = []
    function_tables_impls = []

    for sig in last_forwarded_json['Functions']['tables'].iterkeys():
      args = ','.join(['a' + str(i) for i in range(1, len(sig))])
      arg_coercions = ' '.join(['a' + str(i) + '=' + shared.JS.make_coercion('a' + str(i), sig[i], settings) + ';' for i in range(1, len(sig))])
      coerced_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings) for i in range(1, len(sig))])
      ret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('FUNCTION_TABLE_%s[index&{{{ FTM_%s }}}](%s)' % (sig, sig, coerced_args), sig[0], settings)
      if not settings['EMULATED_FUNCTION_POINTERS']:
        function_tables_impls.append('''
function dynCall_%s(index%s%s) {
  index = index|0;
  %s
  %s;
}
''' % (sig, ',' if len(sig) > 1 else '', args, arg_coercions, ret))
      else:
        function_tables_impls.append('''
var dynCall_%s = ftCall_%s;
''' % (sig, sig))

      ffi_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings, ffi_arg=True) for i in range(1, len(sig))])
      for i in range(settings['RESERVED_FUNCTION_POINTERS']):
        jsret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('jsCall_%s(%d%s%s)' % (sig, i, ',' if ffi_args else '', ffi_args), sig[0], settings, ffi_result=True)
        function_tables_impls.append('''
function jsCall_%s_%s(%s) {
  %s
  %s;
}

''' % (sig, i, args, arg_coercions, jsret))
      shared.Settings.copy(settings)
      asm_setup += '\n' + shared.JS.make_invoke(sig) + '\n'
      basic_funcs.append('invoke_%s' % sig)
      if settings.get('RESERVED_FUNCTION_POINTERS'):
        asm_setup += '\n' + shared.JS.make_jscall(sig) + '\n'
        basic_funcs.append('jsCall_%s' % sig)
      if settings.get('EMULATED_FUNCTION_POINTERS'):
        args = ['a%d' % i for i in range(len(sig)-1)]
        full_args = ['x'] + args
        table_access = 'FUNCTION_TABLE_' + sig
        if settings['SIDE_MODULE']:
          table_access = 'parentModule["' + table_access + '"]' # side module tables were merged into the parent, we need to access the global one
        prelude = '''
  if (x < 0 || x >= %s.length) { Module.printErr("Function table mask error (out of range)"); %s ; abort(x) }''' % (table_access, get_function_pointer_error(sig))
        asm_setup += '''
function ftCall_%s(%s) {%s
  return %s[x](%s);
}
''' % (sig, ', '.join(full_args), prelude, table_access, ', '.join(args))
        basic_funcs.append('ftCall_%s' % sig)

        if settings.get('RELOCATABLE'):
          params = ','.join(['ptr'] + ['p%d' % p for p in range(len(sig)-1)])
          coerced_params = ','.join([shared.JS.make_coercion('ptr', 'i', settings)] + [shared.JS.make_coercion('p%d', unfloat(sig[p+1]), settings) % p for p in range(len(sig)-1)])
          coercions = ';'.join(['ptr = ptr | 0'] + ['p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, unfloat(sig[p+1]), settings)) for p in range(len(sig)-1)]) + ';'
          mini_coerced_params = ','.join([shared.JS.make_coercion('p%d', sig[p+1], settings) % p for p in range(len(sig)-1)])
          maybe_return = '' if sig[0] == 'v' else 'return'
          final_return = maybe_return + ' ' + shared.JS.make_coercion('ftCall_' + sig + '(' + coerced_params + ')', unfloat(sig[0]), settings) + ';'
          if settings['EMULATED_FUNCTION_POINTERS'] == 1:
            body = final_return
          else:
            body = 'if (((ptr|0) >= (fb|0)) & ((ptr|0) < (fb + {{{ FTM_' + sig + ' }}} | 0))) { ' + maybe_return + ' ' + shared.JS.make_coercion('FUNCTION_TABLE_' + sig + '[(ptr-fb)&{{{ FTM_' + sig + ' }}}](' + mini_coerced_params + ')', sig[0], settings, ffi_arg=True) + '; ' + ('return;' if sig[0] == 'v' else '') + ' }' + final_return
          funcs_js.append(make_func('mftCall_' + sig, body, params, coercions) + '\n')

    # calculate exports
    exported_implemented_functions = list(exported_implemented_functions) + metadata['initializers']
    if not settings['ONLY_MY_CODE']:
      exported_implemented_functions.append('runPostSets')
    if settings['ALLOW_MEMORY_GROWTH']:
      exported_implemented_functions.append('_emscripten_replace_memory')
    all_exported = exported_implemented_functions + asm_runtime_funcs + function_tables
    exported_implemented_functions = list(set(exported_implemented_functions))
    if settings['EMULATED_FUNCTION_POINTERS']:
      all_exported = list(set(all_exported).union(in_table))
    exports = []
    for export in all_exported:
      exports.append(quote(export) + ": " + export)
    exports = '{ ' + ', '.join(exports) + ' }'
    # calculate globals
    try:
      del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
    except:
      pass
    if not settings['RELOCATABLE']:
      global_vars = metadata['externs']
    else:
      global_vars = [] # linkable code accesses globals through function calls
    global_funcs = list(set([key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]).difference(set(global_vars)).difference(implemented_functions))
    if settings['RELOCATABLE']:
      global_funcs += ['g$' + extern for extern in metadata['externs']]
      side = 'parent' if settings['SIDE_MODULE'] else ''
      def check(extern):
        if settings['ASSERTIONS']: return 'assert(' + side + 'Module["' + extern + '"], "external function \'' + extern + '\' is missing. perhaps a side module was not linked in? if this symbol was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");'
        return ''
      for extern in metadata['externs']:
        asm_setup += 'var g$' + extern + ' = function() { ' + check(extern) + ' return ' + side + 'Module["' + extern + '"] };\n'
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1]
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global' + access_quote(g) + ';\n' for g in maths])
    asm_global_funcs += ''.join(['  var ' + g + '=env' + access_quote(math_fix(g)) + ';\n' for g in basic_funcs + global_funcs])
    if metadata['simd']:
      def string_contains_any(s, str_list):
        for sub in str_list:
          if sub in s:
            return True
        return False
      nonexisting_simd_symbols = ['Int8x16_fromInt8x16', 'Uint8x16_fromUint8x16', 'Int16x8_fromInt16x8', 'Uint16x8_fromUint16x8', 'Int32x4_fromInt32x4', 'Uint32x4_fromUint32x4', 'Float32x4_fromFloat32x4', 'Float64x2_fromFloat64x2']
      nonexisting_simd_symbols += ['Int32x4_addSaturate', 'Int32x4_subSaturate', 'Uint32x4_addSaturate', 'Uint32x4_subSaturate']
      nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8', 'Float64x2'] for y in ['load2', 'load3', 'store2', 'store3']]
      nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8'] for y in ['load1', 'store1']]

      asm_global_funcs += ''.join(['  var SIMD_' + ty + '=global' + access_quote('SIMD') + access_quote(ty) + ';\n' for ty in simdtypes])

      simd_int_symbols = ['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in simdinttypes for g in simdintfuncs]
      simd_int_symbols = filter(lambda x: not string_contains_any(x, nonexisting_simd_symbols), simd_int_symbols)
      asm_global_funcs += ''.join(simd_int_symbols)

      simd_float_symbols = ['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in simdfloattypes for g in simdfloatfuncs]
      simd_float_symbols = filter(lambda x: not string_contains_any(x, nonexisting_simd_symbols), simd_float_symbols)
      asm_global_funcs += ''.join(simd_float_symbols)

      simd_bool_symbols = ['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in simdbooltypes for g in simdboolfuncs]
      simd_bool_symbols = filter(lambda x: not string_contains_any(x, nonexisting_simd_symbols), simd_bool_symbols)
      asm_global_funcs += ''.join(simd_bool_symbols)

      # SIMD conversions (not bitcasts) between same lane sizes:
      def add_simd_cast(dst, src):
        return '  var SIMD_' + dst + '_from' + src + '=SIMD_' + dst + '.from' + src + ';\n'
      def add_simd_casts(t1, t2):
        return add_simd_cast(t1, t2) + add_simd_cast(t2, t1)
      if metadata['simdInt8x16'] and metadata['simdUint8x16']: asm_global_funcs += add_simd_casts('Int8x16', 'Uint8x16')
      if metadata['simdInt16x8'] and metadata['simdUint16x8']: asm_global_funcs += add_simd_casts('Int16x8', 'Uint16x8')
      if metadata['simdInt32x4'] and metadata['simdUint32x4']: asm_global_funcs += add_simd_casts('Int32x4', 'Uint32x4')
      if metadata['simdInt32x4'] and metadata['simdFloat32x4']: asm_global_funcs += add_simd_casts('Int32x4', 'Float32x4')
      if metadata['simdUint32x4'] and metadata['simdFloat32x4']: asm_global_funcs += add_simd_casts('Uint32x4', 'Float32x4')
      if metadata['simdInt32x4'] and metadata['simdFloat64x2']: asm_global_funcs += add_simd_cast('Int32x4', 'Float64x2') # Unofficial, needed for emscripten_int32x4_fromFloat64x2
      if metadata['simdUint32x4'] and metadata['simdFloat64x2']: asm_global_funcs += add_simd_cast('Uint32x4', 'Float64x2') # Unofficial, needed for emscripten_uint32x4_fromFloat64x2

      # Unofficial, Bool64x2 does not yet exist, but needed for Float64x2 comparisons.
      if metadata['simdFloat64x2']:
        asm_global_funcs += '  var SIMD_Int32x4_fromBool64x2Bits = global.SIMD.Int32x4.fromBool64x2Bits;\n'
    if settings['USE_PTHREADS']:
      asm_global_funcs += ''.join(['  var Atomics_' + ty + '=global' + access_quote('Atomics') + access_quote(ty) + ';\n' for ty in ['load', 'store', 'exchange', 'compareExchange', 'add', 'sub', 'and', 'or', 'xor']])
    asm_global_vars = ''.join(['  var ' + g + '=env' + access_quote(g) + '|0;\n' for g in basic_vars + global_vars])

    # sent data
    the_global = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'
    sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in basic_funcs + global_funcs + basic_vars + basic_float_vars + global_vars]) + ' }'
    # received
    receiving = ''
    if settings['ASSERTIONS']:
      # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
      # some support code
      receiving = '\n'.join(['var real_' + s + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_''' + s + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_memcpy', '_memset', 'runPostSets', '_emscripten_replace_memory']])

    if not settings['SWAPPABLE_ASM_MODULE']:
      receiving += ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables])
    else:
      receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + s + ' = Module["' + s + '"] = function() { return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions + function_tables])
    receiving += ';\n'

    if settings['EXPORT_FUNCTION_TABLES']:
      for table in last_forwarded_json['Functions']['tables'].values():
        tableName = table.split()[1]
        table = table.replace('var ' + tableName, 'var ' + tableName + ' = Module["' + tableName + '"]')
        receiving += table + '\n'

    if DEBUG: logging.debug('asm text sizes' + str([map(len, funcs_js), len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables), len('\n'.join(function_tables_impls)), len(function_tables_defs) + (function_tables_defs.count('\n') * len('  ')), len(exports), len(the_global), len(sending), len(receiving)]))

    final_function_tables = '\n'.join(function_tables_impls) + '\n' + function_tables_defs
    if settings.get('EMULATED_FUNCTION_POINTERS'):
      asm_setup += '\n' + '\n'.join(function_tables_impls) + '\n'
      receiving += '\n' + function_tables_defs.replace('// EMSCRIPTEN_END_FUNCS\n', '') + '\n' + ''.join(['Module["dynCall_%s"] = dynCall_%s\n' % (sig, sig) for sig in last_forwarded_json['Functions']['tables']])
      for sig in last_forwarded_json['Functions']['tables'].keys():
        name = 'FUNCTION_TABLE_' + sig
        fullname = name if not settings['SIDE_MODULE'] else ('SIDE_' + name)
        receiving += 'Module["' + name + '"] = ' + fullname + ';\n'

      final_function_tables = final_function_tables.replace("asm['", '').replace("']", '').replace('var SIDE_FUNCTION_TABLE_', 'var FUNCTION_TABLE_').replace('var dynCall_', '//')

    if DEBUG:
      logging.debug('  emscript: python processing: function tables and exports took %s seconds' % (time.time() - t))

    return post, funcs_js, need_asyncify, provide_fround, asm_safe_heap, sending, receiving, asm_setup, the_global, asm_global_vars, asm_global_funcs, pre_tables, final_function_tables, exports, last_forwarded_json

def finalize_output(metadata, post, funcs_js, need_asyncify, provide_fround, asm_safe_heap, sending, receiving, asm_setup, the_global, asm_global_vars, asm_global_funcs, pre_tables, final_function_tables, exports, last_forwarded_json, settings, outfile, DEBUG):

    if DEBUG:
      logging.debug('emscript: python processing: finalize')
      t = time.time()

    access_quote = access_quoter(settings)

    if settings['RELOCATABLE']:
      receiving += '''
var NAMED_GLOBALS = { %s };
for (var named in NAMED_GLOBALS) {
  Module['_' + named] = gb + NAMED_GLOBALS[named];
}
Module['NAMED_GLOBALS'] = NAMED_GLOBALS;
''' % ', '.join('"' + k + '": ' + str(v) for k, v in metadata['namedGlobals'].iteritems())

      receiving += ''.join(["Module['%s'] = Module['%s']\n" % (k, v) for k, v in metadata['aliases'].iteritems()])

    if settings['USE_PTHREADS']:
      shared_array_buffer = "if (typeof SharedArrayBuffer !== 'undefined') Module.asmGlobalArg['Atomics'] = Atomics;"
    else:
      shared_array_buffer = ''

    first_in_asm = ''
    if settings['SPLIT_MEMORY']:
      if not settings['SAFE_SPLIT_MEMORY']:
        first_in_asm += '''
function get8(ptr) {
  ptr = ptr | 0;
  return HEAP8s[ptr >> SPLIT_MEMORY_BITS][ptr & SPLIT_MEMORY_MASK] | 0;
}
function get16(ptr) {
  ptr = ptr | 0;
  return HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function get32(ptr) {
  ptr = ptr | 0;
  return HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] | 0;
}
function getU8(ptr) {
  ptr = ptr | 0;
  return HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] | 0;
}
function getU16(ptr) {
  ptr = ptr | 0;
  return HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function getU32(ptr) {
  ptr = ptr | 0;
  return HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] | 0;
}
function getF32(ptr) {
  ptr = ptr | 0;
  return +HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2]; // TODO: fround when present
}
function getF64(ptr) {
  ptr = ptr | 0;
  return +HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3];
}
function set8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAP8s[ptr >> SPLIT_MEMORY_BITS][ptr & SPLIT_MEMORY_MASK] = value;
}
function set16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function set32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setU8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] = value;
}
function setU16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function setU32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
  HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF32(ptr, value) {
  ptr = ptr | 0;
  value = +value;
  HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF64(ptr, value) {
  ptr = ptr | 0;
  value = +value;
  HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3] = value;
}
'''
      first_in_asm += 'buffer = new ArrayBuffer(32); // fake\n'

    runtime_funcs = []
    if not settings['ONLY_MY_CODE']:
      runtime_funcs = ['''
function stackAlloc(size) {
  size = size|0;
  var ret = 0;
  ret = STACKTOP;
  STACKTOP = (STACKTOP + size)|0;
  STACKTOP = (STACKTOP + 15)&-16;
''' + ('  if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(size|0);\n' if (settings['ASSERTIONS'] or settings['STACK_OVERFLOW_CHECK'] >= 2) else '') + '''
  return ret|0;
}
function stackSave() {
  return STACKTOP|0;
}
function stackRestore(top) {
  top = top|0;
  STACKTOP = top;
}
function establishStackSpace(stackBase, stackMax) {
  stackBase = stackBase|0;
  stackMax = stackMax|0;
  STACKTOP = stackBase;
  STACK_MAX = stackMax;
}
''' + ('''
function setAsync() {
  ___async = 1;
}''' if need_asyncify else '') + ('''
function emterpret(pc) { // this will be replaced when the emterpreter code is generated; adding it here allows validation until then
  pc = pc | 0;
  assert(0);
}
''' if settings['EMTERPRETIFY'] else '') + ('''
function setAsyncState(x) {
  x = x | 0;
  asyncState = x;
}
function emtStackSave() {
  return EMTSTACKTOP|0;
}
function emtStackRestore(x) {
  x = x | 0;
  EMTSTACKTOP = x;
}
''' if settings['EMTERPRETIFY_ASYNC'] else '') + '''
function setThrew(threw, value) {
  threw = threw|0;
  value = value|0;
  if ((__THREW__|0) == 0) {
    __THREW__ = threw;
    threwValue = value;
  }
}
'''] + ['' if not settings['SAFE_HEAP'] else '''
function setDynamicTop(value) {
  value = value | 0;
  DYNAMICTOP = value;
}
'''] + ['' if not asm_safe_heap else '''
function SAFE_HEAP_STORE(dest, value, bytes) {
  dest = dest | 0;
  value = value | 0;
  bytes = bytes | 0;
  if ((dest|0) <= 0) segfault();
  if (((dest + bytes)|0) > (DYNAMICTOP|0)) segfault();
  if ((bytes|0) == 4) {
    if ((dest&3)) alignfault();
    HEAP32[dest>>2] = value;
  } else if ((bytes|0) == 1) {
    HEAP8[dest>>0] = value;
  } else {
    if ((dest&1)) alignfault();
    HEAP16[dest>>1] = value;
  }
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  dest = dest | 0;
  value = +value;
  bytes = bytes | 0;
  if ((dest|0) <= 0) segfault();
  if (((dest + bytes)|0) > (DYNAMICTOP|0)) segfault();
  if ((bytes|0) == 8) {
    if ((dest&7)) alignfault();
    HEAPF64[dest>>3] = value;
  } else {
    if ((dest&3)) alignfault();
    HEAPF32[dest>>2] = value;
  }
}
function SAFE_HEAP_LOAD(dest, bytes, unsigned) {
  dest = dest | 0;
  bytes = bytes | 0;
  unsigned = unsigned | 0;
  if ((dest|0) <= 0) segfault();
  if ((dest + bytes|0) > (DYNAMICTOP|0)) segfault();
  if ((bytes|0) == 4) {
    if ((dest&3)) alignfault();
    return HEAP32[dest>>2] | 0;
  } else if ((bytes|0) == 1) {
    if (unsigned) {
      return HEAPU8[dest>>0] | 0;
    } else {
      return HEAP8[dest>>0] | 0;
    }
  }
  if ((dest&1)) alignfault();
  if (unsigned) return HEAPU16[dest>>1] | 0;
  return HEAP16[dest>>1] | 0;
}
function SAFE_HEAP_LOAD_D(dest, bytes) {
  dest = dest | 0;
  bytes = bytes | 0;
  if ((dest|0) <= 0) segfault();
  if ((dest + bytes|0) > (DYNAMICTOP|0)) segfault();
  if ((bytes|0) == 8) {
    if ((dest&7)) alignfault();
    return +HEAPF64[dest>>3];
  }
  if ((dest&3)) alignfault();
  return +HEAPF32[dest>>2];
}
function SAFE_FT_MASK(value, mask) {
  value = value | 0;
  mask = mask | 0;
  var ret = 0;
  ret = value & mask;
  if ((ret|0) != (value|0)) ftfault();
  return ret | 0;
}
'''] + ['''
function setTempRet0(value) {
  value = value|0;
  tempRet0 = value;
}
function getTempRet0() {
  return tempRet0|0;
}
''' if not settings['RELOCATABLE'] else '']

    funcs_js = ['''
%s
Module%s = %s;
%s
Module%s = %s;
// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
  %s
  %s
  %s
''' % (asm_setup,
       access_quote('asmGlobalArg'), the_global,
       shared_array_buffer,
       access_quote('asmLibraryArg'), sending,
       "'use asm';" if not metadata.get('hasInlineJS') and settings['ASM_JS'] == 1 else "'almost asm';",
       first_in_asm,
       '''
  var HEAP8 = new global%s(buffer);
  var HEAP16 = new global%s(buffer);
  var HEAP32 = new global%s(buffer);
  var HEAPU8 = new global%s(buffer);
  var HEAPU16 = new global%s(buffer);
  var HEAPU32 = new global%s(buffer);
  var HEAPF32 = new global%s(buffer);
  var HEAPF64 = new global%s(buffer);
''' % (access_quote('Int8Array'),
     access_quote('Int16Array'),
     access_quote('Int32Array'),
     access_quote('Uint8Array'),
     access_quote('Uint16Array'),
     access_quote('Uint32Array'),
     access_quote('Float32Array'),
     access_quote('Float64Array'))
     if not settings['ALLOW_MEMORY_GROWTH'] else '''
  var Int8View = global%s;
  var Int16View = global%s;
  var Int32View = global%s;
  var Uint8View = global%s;
  var Uint16View = global%s;
  var Uint32View = global%s;
  var Float32View = global%s;
  var Float64View = global%s;
  var HEAP8 = new Int8View(buffer);
  var HEAP16 = new Int16View(buffer);
  var HEAP32 = new Int32View(buffer);
  var HEAPU8 = new Uint8View(buffer);
  var HEAPU16 = new Uint16View(buffer);
  var HEAPU32 = new Uint32View(buffer);
  var HEAPF32 = new Float32View(buffer);
  var HEAPF64 = new Float64View(buffer);
  var byteLength = global.byteLength;
''' % (access_quote('Int8Array'),
     access_quote('Int16Array'),
     access_quote('Int32Array'),
     access_quote('Uint8Array'),
     access_quote('Uint16Array'),
     access_quote('Uint32Array'),
     access_quote('Float32Array'),
     access_quote('Float64Array'))) + '\n' + asm_global_vars + ('''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var nan = global%s, inf = global%s;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
  var tempRet0 = 0;
''' % (access_quote('NaN'), access_quote('Infinity'))) + '\n' + asm_global_funcs] + \
  ['  var tempFloat = %s;\n' % ('Math_fround(0)' if provide_fround else '0.0')] + \
  ['  var asyncState = 0;\n' if settings.get('EMTERPRETIFY_ASYNC') else ''] + \
  (['  const f0 = Math_fround(0);\n'] if provide_fround else []) + \
  ['' if not settings['ALLOW_MEMORY_GROWTH'] else '''
function _emscripten_replace_memory(newBuffer) {
  if ((byteLength(newBuffer) & 0xffffff || byteLength(newBuffer) <= 0xffffff) || byteLength(newBuffer) > 0x80000000) return false;
  HEAP8 = new Int8View(newBuffer);
  HEAP16 = new Int16View(newBuffer);
  HEAP32 = new Int32View(newBuffer);
  HEAPU8 = new Uint8View(newBuffer);
  HEAPU16 = new Uint16View(newBuffer);
  HEAPU32 = new Uint32View(newBuffer);
  HEAPF32 = new Float32View(newBuffer);
  HEAPF64 = new Float64View(newBuffer);
  buffer = newBuffer;
  return true;
}
'''] + ['''
// EMSCRIPTEN_START_FUNCS
'''] + runtime_funcs + funcs_js + ['''
  ''', pre_tables, final_function_tables, '''

  return %s;
})
// EMSCRIPTEN_END_ASM
(%s, %s, buffer);
''' % (exports,
       'Module' + access_quote('asmGlobalArg'),
       'Module' + access_quote('asmLibraryArg')), '''
''', receiving, ''';
''']

    if not settings.get('SIDE_MODULE'):
      funcs_js.append('''
Runtime.stackAlloc = asm['stackAlloc'];
Runtime.stackSave = asm['stackSave'];
Runtime.stackRestore = asm['stackRestore'];
Runtime.establishStackSpace = asm['establishStackSpace'];
''')
      if settings['SAFE_HEAP']:
        funcs_js.append('''
Runtime.setDynamicTop = asm['setDynamicTop'];
''')

    if not settings['RELOCATABLE']:
      funcs_js.append('''
Runtime.setTempRet0 = asm['setTempRet0'];
Runtime.getTempRet0 = asm['getTempRet0'];
''')

    # Set function table masks
    masks = {}
    max_mask = 0
    for sig, table in last_forwarded_json['Functions']['tables'].iteritems():
      mask = table.count(',')
      masks[sig] = str(mask)
      max_mask = max(mask, max_mask)
    def function_table_maskize(js, masks):
      def fix(m):
        sig = m.groups(0)[0]
        return masks[sig]
      return re.sub(r'{{{ FTM_([\w\d_$]+) }}}', fix, js) # masks[m.groups(0)[0]]
    for i in range(len(funcs_js)): # in-place as this can be large
      funcs_js[i] = function_table_maskize(funcs_js[i], masks)

    if settings['SIDE_MODULE']:
      funcs_js.append('''
Runtime.registerFunctions(%(sigs)s, Module);
''' % { 'sigs': str(map(str, last_forwarded_json['Functions']['tables'].keys())) })

    for i in range(len(funcs_js)): # do this loop carefully to save memory
      if WINDOWS: funcs_js[i] = funcs_js[i].replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
      outfile.write(funcs_js[i])
    funcs_js = None

    if WINDOWS: post = post.replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
    outfile.write(post)

    if DEBUG:
      logging.debug('  emscript: python processing: finalize took %s seconds' % (time.time() - t))

    if settings['CYBERDWARF']:
      assert('cyberdwarf_data' in metadata)
      cd_file_name = outfile.name + ".cd"
      with open(cd_file_name, "w") as cd_file:
        json.dump({ 'cyberdwarf': metadata['cyberdwarf_data'] }, cd_file)

def emscript_wasm_backend(infile, settings, outfile, libraries=None, compiler_engine=None,
                          temp_files=None, DEBUG=None):
  # Overview:
  #   * Run LLVM backend to emit .s
  #   * Run Binaryen's s2wasm to generate WebAssembly.
  #   * We may also run some Binaryen passes here.

  if libraries is None: libraries = []

  temp_s = temp_files.get('.wb.s').name
  backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
  backend_args = [backend_compiler, infile, '-march=wasm32', '-filetype=asm',
                  '-asm-verbose=false',
                  '-o', temp_s]
  backend_args += ['-thread-model=single'] # no threads support in backend, tell llc to not emit atomics
  # disable slow and relatively unimportant optimization passes
  backend_args += ['-combiner-alias-analysis=false', '-combiner-global-alias-analysis=false']
  if DEBUG:
    logging.debug('emscript: llvm wasm backend: ' + ' '.join(backend_args))
    t = time.time()
  shared.check_call(backend_args)
  if DEBUG:
    logging.debug('  emscript: llvm wasm backend took %s seconds' % (time.time() - t))
    t = time.time()
    import shutil
    shutil.copyfile(temp_s, os.path.join(shared.CANONICAL_TEMP_DIR, 'emcc-llvm-backend-output.s'))

  assert shared.Settings.BINARYEN_ROOT, 'need BINARYEN_ROOT config set so we can use Binaryen s2wasm on the backend output'
  wasm = outfile.name[:-3] + '.wast'
  s2wasm_args = [os.path.join(shared.Settings.BINARYEN_ROOT, 'bin', 's2wasm'), temp_s]
  s2wasm_args += ['--emscripten-glue']
  s2wasm_args += ['--global-base=%d' % shared.Settings.GLOBAL_BASE]
  s2wasm_args += ['--initial-memory=%d' % shared.Settings.TOTAL_MEMORY]
  def compiler_rt_fail(): raise Exception('Expected wasm_compiler_rt.a to already be built')
  compiler_rt_lib = shared.Cache.get('wasm_compiler_rt.a', lambda: compiler_rt_fail(), 'a')
  s2wasm_args += ['-l', compiler_rt_lib]
  if DEBUG:
    logging.debug('emscript: binaryen s2wasm: ' + ' '.join(s2wasm_args))
    t = time.time()
    #s2wasm_args += ['--debug']
  shared.check_call(s2wasm_args, stdout=open(wasm, 'w'))
  if DEBUG:
    logging.debug('  emscript: binaryen s2wasm took %s seconds' % (time.time() - t))
    t = time.time()
    import shutil
    shutil.copyfile(wasm, os.path.join(shared.CANONICAL_TEMP_DIR, 'emcc-s2wasm-output.wast'))

  # js compiler

  if DEBUG: logging.debug('emscript: js compiler glue')

  # Integrate info from backend

  output = open(wasm).read()
  parts = output.split('\n;; METADATA:')
  assert len(parts) == 2
  metadata_raw = parts[1]
  parts = output = None

  if DEBUG: logging.debug("METAraw %s", metadata_raw)
  try:
    metadata_json = json.loads(metadata_raw)
  except Exception, e:
    logging.error('emscript: failure to parse metadata output from s2wasm. raw output is: \n' + metadata_raw)
    raise e

  metadata = {
    'declares': [],
    'implementedFunctions': [],
    'externs': [],
    'simd': False,
    'maxGlobalAlign': 0,
    'initializers': [],
    'exports': [],
  }

  for k, v in metadata_json.iteritems():
    metadata[k] = v

  def asmjs_mangle(name):
    # Mangle a name the way asm.js/JSBackend globals are mangled (i.e. prepend
    # '_' and replace non-alphanumerics with '_')
    if name.startswith('dynCall_'): return name
    return '_' + ''.join(['_' if not c.isalnum() else c for c in name])

  # Initializers call the global var version of the export, so they get the mangled name.
  metadata['initializers'] = [asmjs_mangle(i) for i in metadata['initializers']]

  # TODO: emit it from s2wasm; for now, we parse it right here
  for line in open(wasm).readlines():
    if line.startswith('  (import '):
      parts = line.split(' ')
      metadata['declares'].append(parts[3][1:])
    elif line.startswith('  (func '):
      parts = line.split(' ')
      func = parts[3][1:]
      metadata['implementedFunctions'].append(func)
    elif line.startswith('  (export '):
      parts = line.split(' ')
      exportname = parts[3][1:-1]
      assert asmjs_mangle(exportname) not in metadata['exports']
      metadata['exports'].append(exportname)

  metadata['declares'] = filter(lambda x: not x.startswith('emscripten_asm_const'), metadata['declares']) # we emit those ourselves

  if DEBUG: logging.debug(repr(metadata))

  settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = list(
    set(settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] + map(shared.JS.to_nice_ident, metadata['declares'])).difference(
      map(lambda x: x[1:], metadata['implementedFunctions'])
    )
  ) + map(lambda x: x[1:], metadata['externs'])
  if metadata['simd']:
    settings['SIMD'] = 1

  settings['MAX_GLOBAL_ALIGN'] = metadata['maxGlobalAlign']

  settings['IMPLEMENTED_FUNCTIONS'] = metadata['implementedFunctions']

  # Save settings to a file to work around v8 issue 1579
  settings_file = temp_files.get('.txt').name
  def save_settings():
    global settings_text
    settings_text = json.dumps(settings, sort_keys=True)
    s = open(settings_file, 'w')
    s.write(settings_text)
    s.close()
  save_settings()

  # Call js compiler
  if DEBUG: t = time.time()
  out = jsrun.run_js(path_from_root('src', 'compiler.js'), compiler_engine,
                     [settings_file] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                     cwd=path_from_root('src'), error_limit=300)
  assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
  glue, forwarded_data = out.split('//FORWARDED_DATA:')

  if DEBUG:
    logging.debug('  emscript: glue took %s seconds' % (time.time() - t))
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  # memory and global initializers

  global_initializers = str(', '.join(map(lambda i: '{ func: function() { %s() } }' % i, metadata['initializers'])))

  staticbump = metadata['staticBump']
  while staticbump % 16 != 0: staticbump += 1
  pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + %d;%s
/* global initializers */ %s __ATINIT__.push(%s);
''' % (staticbump,
         'assert(STATICTOP < SPLIT_MEMORY, "SPLIT_MEMORY size must be big enough so the entire static memory, need " + STATICTOP);' if settings['SPLIT_MEMORY'] else '',
         'if (!ENVIRONMENT_IS_PTHREAD)' if settings['USE_PTHREADS'] else '',
         global_initializers))

  pre = pre.replace('{{{ STATIC_BUMP }}}', str(staticbump))

  # merge forwarded data
  settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
  all_exported_functions = set(shared.expand_response(settings['EXPORTED_FUNCTIONS'])) # both asm.js and otherwise

  for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
    all_exported_functions.add('_' + additional_export)
  exported_implemented_functions = set(metadata['exports'])
  export_bindings = settings['EXPORT_BINDINGS']
  export_all = settings['EXPORT_ALL']
  all_implemented = metadata['implementedFunctions'] + forwarded_json['Functions']['implementedFunctions'].keys() # XXX perf?
  for key in all_implemented:
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      exported_implemented_functions.add(key)
  implemented_functions = set(metadata['implementedFunctions'])
  if settings['ASSERTIONS'] and settings.get('ORIGINAL_EXPORTED_FUNCTIONS'):
    original_exports = settings['ORIGINAL_EXPORTED_FUNCTIONS']
    if original_exports[0] == '@': original_exports = json.loads(open(original_exports[1:]).read())
    for requested in original_exports:
      # check if already implemented
      # special-case malloc, EXPORTED by default for internal use, but we bake in a trivial allocator and warn at runtime if used in ASSERTIONS \
      if requested not in all_implemented and \
         requested != '_malloc' and \
         (('function ' + requested.encode('utf-8')) not in pre): # could be a js library func
        logging.warning('function requested to be exported, but not implemented: "%s"', requested)

  asm_consts = [0]*len(metadata['asmConsts'])
  all_sigs = []
  for k, v in metadata['asmConsts'].iteritems():
    const = v[0].encode('utf-8')
    sigs = v[1]
    if const[0] == '"' and const[-1] == '"':
      const = const[1:-1]
    const = '{ ' + const + ' }'
    args = []
    arity = max(map(len, sigs)) - 1
    for i in range(arity):
      args.append('$' + str(i))
    const = 'function(' + ', '.join(args) + ') ' + const
    asm_consts[int(k)] = const
    all_sigs += sigs

  asm_const_funcs = []
  for sig in set(all_sigs):
    forwarded_json['Functions']['libraryFunctions']['_emscripten_asm_const_' + sig] = 1
    args = ['a%d' % i for i in range(len(sig)-1)]
    all_args = ['code'] + args
    asm_const_funcs.append(r'''
function _emscripten_asm_const_%s(%s) {
return ASM_CONSTS[code](%s);
}''' % (sig.encode('utf-8'), ', '.join(all_args), ', '.join(args)))

  pre = pre.replace('// === Body ===', '// === Body ===\n' + '\nvar ASM_CONSTS = [' + ',\n '.join(asm_consts) + '];\n' + '\n'.join(asm_const_funcs) + '\n')

  outfile.write(pre)
  pre = None

  basic_funcs = ['abort', 'assert']

  access_quote = access_quoter(settings)

  # calculate globals
  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except:
    pass
  if not settings['RELOCATABLE']:
    global_vars = metadata['externs']
  else:
    global_vars = [] # linkable code accesses globals through function calls
  global_funcs = list(set([key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]).difference(set(global_vars)).difference(implemented_functions))
  def math_fix(g):
    return g if not g.startswith('Math_') else g.split('_')[1]

  basic_vars = ['STACKTOP', 'STACK_MAX', 'ABORT']
  basic_float_vars = []

  # sent data
  the_global = '{}'
  sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in basic_funcs + global_funcs + basic_vars + basic_float_vars + global_vars]) + ' }'
  # received
  receiving = ''
  if settings['ASSERTIONS']:
    # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
    # some support code
    receiving = '\n'.join(['var real_' + asmjs_mangle(s) + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_''' + asmjs_mangle(s) + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_memcpy', '_memset', 'runPostSets', '_emscripten_replace_memory']])

  if not settings['SWAPPABLE_ASM_MODULE']:
    receiving += ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = asm["' + s + '"]' for s in exported_implemented_functions])
  else:
    receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = function() { return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions])
  receiving += ';\n'

  # finalize

  if settings['USE_PTHREADS']:
    shared_array_buffer = "if (typeof SharedArrayBuffer !== 'undefined') Module.asmGlobalArg['Atomics'] = Atomics;"
  else:
    shared_array_buffer = ''

  funcs_js = ['''
Module%s = %s;
%s
Module%s = %s;
''' % (access_quote('asmGlobalArg'), the_global,
     shared_array_buffer,
     access_quote('asmLibraryArg'), sending) + '''
var asm = Module['asm'](%s, %s, buffer);
%s;
''' % ('Module' + access_quote('asmGlobalArg'),
     'Module' + access_quote('asmLibraryArg'),
     receiving)]

  # wasm backend stack goes down, and is stored in the first global var location
  funcs_js.append('''
STACKTOP = STACK_BASE + TOTAL_STACK;
STACK_MAX = STACK_BASE;
HEAP32[%d >> 2] = STACKTOP;
Runtime.stackAlloc = asm['stackAlloc'];
Runtime.stackSave = asm['stackSave'];
Runtime.stackRestore = asm['stackRestore'];
Runtime.establishStackSpace = asm['establishStackSpace'];
''' % shared.Settings.GLOBAL_BASE)

  funcs_js.append('''
Runtime.setTempRet0 = asm['setTempRet0'];
Runtime.getTempRet0 = asm['getTempRet0'];
''')

  for i in range(len(funcs_js)): # do this loop carefully to save memory
    if WINDOWS: funcs_js[i] = funcs_js[i].replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
    outfile.write(funcs_js[i])
  funcs_js = None

  if WINDOWS: post = post.replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
  outfile.write(post)

  outfile.close()

if os.environ.get('EMCC_FAST_COMPILER') == '0':
  logging.critical('Non-fastcomp compiler is no longer available, please use fastcomp or an older version of emscripten')
  sys.exit(1)

def main(args, compiler_engine, cache, temp_files, DEBUG):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # libraries
  libraries = args.libraries[0].split(',') if len(args.libraries) > 0 else []

  settings.setdefault('STRUCT_INFO', shared.path_from_root('src', 'struct_info.compiled.json'))
  struct_info = settings.get('STRUCT_INFO')

  if not os.path.exists(struct_info) and not settings.get('BOOTSTRAPPING_STRUCT_INFO') and not settings.get('ONLY_MY_CODE'):
    if DEBUG: logging.debug('  emscript: bootstrapping struct info...')
    shared.Building.ensure_struct_info(struct_info)
    if DEBUG: logging.debug('  emscript: bootstrapping struct info complete')

  emscripter = emscript_wasm_backend if settings['WASM_BACKEND'] else emscript

  emscripter(args.infile, settings, args.outfile, libraries, compiler_engine=compiler_engine,
             temp_files=temp_files, DEBUG=DEBUG)

def _main(args=None):
  if args is None:
    args = sys.argv[1:]

  response_file = True
  while response_file:
    response_file = None
    for index in range(len(args)):
      if args[index][0] == '@':
        # found one, loop again next time
        response_file = True
        response_file_args = read_response_file(args[index])
        # slice in extra_args in place of the response file arg
        args[index:index+1] = response_file_args
        break

  parser = optparse.OptionParser(
    usage='usage: %prog [-h] [-H HEADERS] [-o OUTFILE] [-c COMPILER_ENGINE] [-s FOO=BAR]* infile',
    description=('You should normally never use this! Use emcc instead. '
                 'This is a wrapper around the JS compiler, converting .ll to .js.'),
    epilog='')
  parser.add_option('-H', '--headers',
                    default=[],
                    action='append',
                    help='System headers (comma separated) whose #defines should be exposed to the compiled code.')
  parser.add_option('-L', '--libraries',
                    default=[],
                    action='append',
                    help='Library files (comma separated) to use in addition to those in emscripten src/library_*.')
  parser.add_option('-o', '--outfile',
                    default=sys.stdout,
                    help='Where to write the output; defaults to stdout.')
  parser.add_option('-c', '--compiler',
                    default=None,
                    help='Which JS engine to use to run the compiler; defaults to the one in ~/.emscripten.')
  parser.add_option('-s', '--setting',
                    dest='settings',
                    default=[],
                    action='append',
                    metavar='FOO=BAR',
                    help=('Overrides for settings defined in settings.js. '
                          'May occur multiple times.'))
  parser.add_option('-T', '--temp-dir',
                    default=None,
                    help=('Where to create temporary files.'))
  parser.add_option('-v', '--verbose',
                    action='store_true',
                    dest='verbose',
                    help='Displays debug output')
  parser.add_option('-q', '--quiet',
                    action='store_false',
                    dest='verbose',
                    help='Hides debug output')
  parser.add_option('--suppressUsageWarning',
                    action='store_true',
                    default=os.environ.get('EMSCRIPTEN_SUPPRESS_USAGE_WARNING'),
                    help=('Suppress usage warning'))

  # Convert to the same format that argparse would have produced.
  keywords, positional = parser.parse_args(args)

  if not keywords.suppressUsageWarning:
    logging.warning('''
==============================================================
WARNING: You should normally never use this! Use emcc instead.
==============================================================
  ''')

  if len(positional) != 1:
    raise RuntimeError('Must provide exactly one positional argument. Got ' + str(len(positional)) + ': "' + '", "'.join(positional) + '"')
  keywords.infile = os.path.abspath(positional[0])
  if isinstance(keywords.outfile, basestring):
    keywords.outfile = open(keywords.outfile, 'w')

  if keywords.temp_dir is None:
    temp_files = get_configuration().get_temp_files()
    temp_dir = get_configuration().TEMP_DIR
  else:
    temp_dir = os.path.abspath(keywords.temp_dir)
    if not os.path.exists(temp_dir):
      os.makedirs(temp_dir)
    temp_files = tempfiles.TempFiles(temp_dir)

  if keywords.compiler is None:
    keywords.compiler = shared.COMPILER_ENGINE

  if keywords.verbose is None:
    DEBUG = get_configuration().DEBUG
  else:
    DEBUG = keywords.verbose

  cache = cache_module.Cache()
  temp_files.run_and_clean(lambda: main(
    keywords,
    compiler_engine=keywords.compiler,
    cache=cache,
    temp_files=temp_files,
    DEBUG=DEBUG,
  ))

if __name__ == '__main__':
  _main()
