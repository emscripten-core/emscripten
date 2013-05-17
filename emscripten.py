#!/usr/bin/env python2

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import os, sys, json, optparse, subprocess, re, time, multiprocessing, functools

from tools import jsrun, cache as cache_module, tempfiles
from tools.response_file import read_response_file

__rootpath__ = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  """Returns the absolute path for which the given path elements are
  relative to the emscripten root.
  """
  return os.path.join(__rootpath__, *pathelems)

def get_configuration():
  if hasattr(get_configuration, 'configuration'):
    return get_configuration.configuration

  from tools import shared
  configuration = shared.Configuration(environ=os.environ)
  get_configuration.configuration = configuration
  return configuration

def scan(ll, settings):
  # blockaddress(@main, %23)
  blockaddrs = []
  for blockaddr in re.findall('blockaddress\([^)]*\)', ll):
    b = blockaddr.split('(')[1][:-1].split(', ')
    blockaddrs.append(b)
  if len(blockaddrs) > 0:
    settings['NECESSARY_BLOCKADDRS'] = blockaddrs

NUM_CHUNKS_PER_CORE = 1.25
MIN_CHUNK_SIZE = 1024*1024
MAX_CHUNK_SIZE = float(os.environ.get('EMSCRIPT_MAX_CHUNK_SIZE') or 'inf') # configuring this is just for debugging purposes

def process_funcs((i, funcs, meta, settings_file, compiler, forwarded_file, libraries, compiler_engine, temp_files, DEBUG)):
  funcs_file = temp_files.get('.func_%d.ll' % i).name
  f = open(funcs_file, 'w')
  f.write(funcs)
  funcs = None
  f.write('\n')
  f.write(meta)
  f.close()
  out = jsrun.run_js(
    compiler,
    engine=compiler_engine,
    args=[settings_file, funcs_file, 'funcs', forwarded_file] + libraries,
    stdout=subprocess.PIPE,
    cwd=path_from_root('src'))
  tempfiles.try_delete(funcs_file)
  if DEBUG: print >> sys.stderr, '.'
  return out

def emscript(infile, settings, outfile, libraries=[], compiler_engine=None,
             jcache=None, temp_files=None, DEBUG=None, DEBUG_CACHE=None):
  """Runs the emscripten LLVM-to-JS compiler. We parallelize as much as possible

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted settings that override the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """

  compiler = path_from_root('src', 'compiler.js')

  # Parallelization: We run 3 phases:
  #   1 aka 'pre'  : Process types and metadata and so forth, and generate the preamble.
  #   2 aka 'funcs': Process functions. We can parallelize this, working on each function independently.
  #   3 aka 'post' : Process globals, generate postamble and finishing touches.

  if DEBUG: print >> sys.stderr, 'emscript: ll=>js'

  if jcache: jcache.ensure()

  # Pre-scan ll and alter settings as necessary
  if DEBUG: t = time.time()
  ll = open(infile).read()
  scan(ll, settings)
  total_ll_size = len(ll)
  ll = None # allow collection
  if DEBUG: print >> sys.stderr, '  emscript: scan took %s seconds' % (time.time() - t)

  # Split input into the relevant parts for each phase
  pre = []
  funcs = [] # split up functions here, for parallelism later
  meta = [] # needed by each function XXX

  if DEBUG: t = time.time()
  in_func = False
  ll_lines = open(infile).readlines()
  curr_func = None
  for line in ll_lines:
    if in_func:
      curr_func.append(line)
      if line.startswith('}'):
        in_func = False
        funcs.append((curr_func[0], ''.join(curr_func))) # use the entire line as the identifier
        # pre needs to know about all implemented functions, even for non-pre func
        pre.append(curr_func[0])
        pre.append(line)
        curr_func = None
    else:
      if line.startswith(';'): continue
      if line.startswith('define '):
        in_func = True
        curr_func = [line]
      elif line.find(' = type { ') > 0:
        pre.append(line) # type
      elif line.startswith('!'):
        if line.startswith('!llvm.module'): continue # we can ignore that
        meta.append(line) # metadata
      else:
        pre.append(line) # pre needs it so we know about globals in pre and funcs. So emit globals there
  ll_lines = None
  meta = ''.join(meta)
  if DEBUG and len(meta) > 1024*1024: print >> sys.stderr, 'emscript warning: large amounts of metadata, will slow things down'
  if DEBUG: print >> sys.stderr, '  emscript: split took %s seconds' % (time.time() - t)

  #if DEBUG:
  #  print >> sys.stderr, '========= pre ================\n'
  #  print >> sys.stderr, ''.join(pre)
  #  print >> sys.stderr, '========== funcs ===============\n'
  #  for func in funcs:
  #    print >> sys.stderr, '\n// ===\n\n', ''.join(func)
  #  print >> sys.stderr, '=========================\n'

  # Save settings to a file to work around v8 issue 1579
  settings_file = temp_files.get('.txt').name
  def save_settings():
    global settings_text
    settings_text = json.dumps(settings, sort_keys=True)
    s = open(settings_file, 'w')
    s.write(settings_text)
    s.close()
  save_settings()

  # Phase 1 - pre
  if DEBUG: t = time.time()
  pre_file = temp_files.get('.pre.ll').name
  pre_input = ''.join(pre) + '\n' + meta
  out = None
  if jcache:
    keys = [pre_input, settings_text, ','.join(libraries)]
    shortkey = jcache.get_shortkey(keys)
    if DEBUG_CACHE: print >>sys.stderr, 'shortkey', shortkey

    out = jcache.get(shortkey, keys)

    if DEBUG_CACHE and not out:
      dfpath = os.path.join(get_configuration().TEMP_DIR, "ems_" + shortkey)
      dfp = open(dfpath, 'w')
      dfp.write(pre_input);
      dfp.write("\n\n========================== settings_text\n\n");
      dfp.write(settings_text);
      dfp.write("\n\n========================== libraries\n\n");
      dfp.write("\n".join(libraries))
      dfp.close()
      print >>sys.stderr, '  cache miss, key data dumped to %s' % dfpath

    if out and DEBUG: print >> sys.stderr, '  loading pre from jcache'
  if not out:
    open(pre_file, 'w').write(pre_input)
    out = jsrun.run_js(compiler, compiler_engine, [settings_file, pre_file, 'pre'] + libraries, stdout=subprocess.PIPE,
                       cwd=path_from_root('src'))
    assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
    if jcache:
      if DEBUG: print >> sys.stderr, '  saving pre to jcache'
      jcache.set(shortkey, keys, out)
  pre, forwarded_data = out.split('//FORWARDED_DATA:')
  forwarded_file = temp_files.get('.json').name
  open(forwarded_file, 'w').write(forwarded_data)
  if DEBUG: print >> sys.stderr, '  emscript: phase 1 took %s seconds' % (time.time() - t)

  # Phase 2 - func

  cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
  assert cores >= 1
  if cores > 1:
    intended_num_chunks = int(round(cores * NUM_CHUNKS_PER_CORE))
    chunk_size = max(MIN_CHUNK_SIZE, total_ll_size / intended_num_chunks)
    chunk_size += 3*len(meta) + len(forwarded_data)/3 # keep ratio of lots of function code to meta (expensive to process, and done in each parallel task) and forwarded data (less expensive but potentially significant)
    chunk_size = min(MAX_CHUNK_SIZE, chunk_size)
  else:
    chunk_size = MAX_CHUNK_SIZE # if 1 core, just use the max chunk size

  if DEBUG: t = time.time()
  forwarded_json = json.loads(forwarded_data)
  indexed_functions = set()
  if settings.get('ASM_JS'):
    settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
    save_settings()

  chunks = cache_module.chunkify(
    funcs, chunk_size,
    jcache.get_cachename('emscript_files') if jcache else None)

  funcs = None

  if jcache:
    # load chunks from cache where we can # TODO: ignore small chunks
    cached_outputs = []
    def load_from_cache(chunk):
      keys = [settings_text, forwarded_data, chunk]
      shortkey = jcache.get_shortkey(keys) # TODO: share shortkeys with later code
      out = jcache.get(shortkey, keys) # this is relatively expensive (pickling?)
      if out:
        cached_outputs.append(out)
        return False
      return True
    chunks = filter(load_from_cache, chunks)
    if len(cached_outputs) > 0:
      if out and DEBUG: print >> sys.stderr, '  loading %d funcchunks from jcache' % len(cached_outputs)
    else:
      cached_outputs = []

  # TODO: minimize size of forwarded data from funcs to what we actually need

  if len(chunks) > 0:
    if cores == 1 and total_ll_size < MAX_CHUNK_SIZE:
      assert len(chunks) == 1, 'no point in splitting up without multiple cores'

    if DEBUG: print >> sys.stderr, '  emscript: phase 2 working on %d chunks %s (intended chunk size: %.2f MB, meta: %.2f MB, forwarded: %.2f MB, total: %.2f MB)' % (len(chunks), ('using %d cores' % cores) if len(chunks) > 1 else '', chunk_size/(1024*1024.), len(meta)/(1024*1024.), len(forwarded_data)/(1024*1024.), total_ll_size/(1024*1024.))

    commands = [
      (i, chunk, meta, settings_file, compiler, forwarded_file, libraries, compiler_engine, temp_files, DEBUG)
      for i, chunk in enumerate(chunks)
    ]

    if len(chunks) > 1:
      pool = multiprocessing.Pool(processes=cores)
      outputs = pool.map(process_funcs, commands, chunksize=1)
    elif len(chunks) == 1:
      outputs = [process_funcs(commands[0])]

    commands = None

  else:
    outputs = []

  if jcache:
    # save chunks to cache
    for i in range(len(chunks)):
      chunk = chunks[i]
      keys = [settings_text, forwarded_data, chunk]
      shortkey = jcache.get_shortkey(keys)
      jcache.set(shortkey, keys, outputs[i])
    if out and DEBUG and len(chunks) > 0: print >> sys.stderr, '  saving %d funcchunks to jcache' % len(chunks)

  chunks = None

  if jcache: outputs += cached_outputs # TODO: preserve order

  outputs = [output.split('//FORWARDED_DATA:') for output in outputs]
  for output in outputs:
    assert len(output) == 2, 'Did not receive forwarded data in an output - process failed? We only got: ' + output[0][-3000:]

  if DEBUG: print >> sys.stderr, '  emscript: phase 2 took %s seconds' % (time.time() - t)
  if DEBUG: t = time.time()

  # merge forwarded data
  if settings.get('ASM_JS'):
    all_exported_functions = set(settings['EXPORTED_FUNCTIONS']) # both asm.js and otherwise
    for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
      all_exported_functions.add('_' + additional_export)
    exported_implemented_functions = set()
  for func_js, curr_forwarded_data in outputs:
    curr_forwarded_json = json.loads(curr_forwarded_data)
    forwarded_json['Types']['preciseI64MathUsed'] = forwarded_json['Types']['preciseI64MathUsed'] or curr_forwarded_json['Types']['preciseI64MathUsed']
    for key, value in curr_forwarded_json['Functions']['blockAddresses'].iteritems():
      forwarded_json['Functions']['blockAddresses'][key] = value
    for key in curr_forwarded_json['Functions']['indexedFunctions'].iterkeys():
      indexed_functions.add(key)
    if settings.get('ASM_JS'):
      export_bindings = settings['EXPORT_BINDINGS']
      for key in curr_forwarded_json['Functions']['implementedFunctions'].iterkeys():
        if key in all_exported_functions or (export_bindings and key.startswith('_emscripten_bind')):
          exported_implemented_functions.add(key)
    for key, value in curr_forwarded_json['Functions']['unimplementedFunctions'].iteritems():
      forwarded_json['Functions']['unimplementedFunctions'][key] = value
    for key, value in curr_forwarded_json['Functions']['neededTables'].iteritems():
      forwarded_json['Functions']['neededTables'][key] = value

  if settings.get('ASM_JS'):
    parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
    if len(parts) > 1:
      pre = parts[0]
      outputs.append([parts[1]])
  funcs_js = [output[0] for output in outputs]

  outputs = None
  if DEBUG: print >> sys.stderr, '  emscript: phase 2b took %s seconds' % (time.time() - t)
  if DEBUG: t = time.time()

  # calculations on merged forwarded data
  forwarded_json['Functions']['indexedFunctions'] = {}
  i = 2 # universal counter
  if settings['ASM_JS']: i += 2*settings['RESERVED_FUNCTION_POINTERS']
  table_counters = {} # table-specific counters
  alias = settings['ASM_JS'] and settings['ALIASING_FUNCTION_POINTERS']
  sig = None
  for indexed in indexed_functions:
    if alias:
      sig = forwarded_json['Functions']['implementedFunctions'].get(indexed) or forwarded_json['Functions']['unimplementedFunctions'].get(indexed)
      assert sig, indexed
      if sig not in table_counters:
        table_counters[sig] = 2 + 2*settings['RESERVED_FUNCTION_POINTERS']
      curr = table_counters[sig]
      table_counters[sig] += 2
    else:
      curr = i
      i += 2
    #print >> sys.stderr, 'function indexing', indexed, curr, sig
    forwarded_json['Functions']['indexedFunctions'][indexed] = curr # make sure not to modify this python object later - we use it in indexize

  def split_32(x):
    x = int(x)
    return '%d,%d,%d,%d' % (x&255, (x >> 8)&255, (x >> 16)&255, (x >> 24)&255)

  indexing = forwarded_json['Functions']['indexedFunctions']
  def indexize(js):
    # In the global initial allocation, we need to split up into Uint8 format
    ret = re.sub(r"\"?'?{{ FI_([\w\d_$]+) }}'?\"?,0,0,0", lambda m: split_32(indexing.get(m.groups(0)[0]) or 0), js)
    return re.sub(r"'{{ FI_([\w\d_$]+) }}'", lambda m: str(indexing.get(m.groups(0)[0]) or 0), ret)

  blockaddrs = forwarded_json['Functions']['blockAddresses']
  def blockaddrsize(js):
    ret = re.sub(r'"?{{{ BA_([\w\d_$]+)\|([\w\d_$]+) }}}"?,0,0,0', lambda m: split_32(blockaddrs[m.groups(0)[0]][m.groups(0)[1]]), js)
    return re.sub(r'"?{{{ BA_([\w\d_$]+)\|([\w\d_$]+) }}}"?', lambda m: str(blockaddrs[m.groups(0)[0]][m.groups(0)[1]]), ret)

  #if DEBUG: outfile.write('// pre\n')
  outfile.write(blockaddrsize(indexize(pre)))
  pre = None

  #if DEBUG: outfile.write('// funcs\n')

  # forward
  forwarded_data = json.dumps(forwarded_json)
  forwarded_file = temp_files.get('.2.json').name
  open(forwarded_file, 'w').write(indexize(forwarded_data))
  if DEBUG: print >> sys.stderr, '  emscript: phase 2c took %s seconds' % (time.time() - t)

  # Phase 3 - post
  if DEBUG: t = time.time()
  post_file = temp_files.get('.post.ll').name
  open(post_file, 'w').write('\n') # no input, just processing of forwarded data
  out = jsrun.run_js(compiler, compiler_engine, [settings_file, post_file, 'post', forwarded_file] + libraries, stdout=subprocess.PIPE,
                     cwd=path_from_root('src'))
  post, last_forwarded_data = out.split('//FORWARDED_DATA:') # if this fails, perhaps the process failed prior to printing forwarded data?
  last_forwarded_json = json.loads(last_forwarded_data)

  if settings.get('ASM_JS'):
    post_funcs, post_rest = post.split('// EMSCRIPTEN_END_FUNCS\n')
    post = post_rest

    # Move preAsms to their right place
    def move_preasm(m):
      contents = m.groups(0)[0]
      outfile.write(contents + '\n')
      return ''
    post_funcs = re.sub(r'/\* PRE_ASM \*/(.*)\n', lambda m: move_preasm(m), post_funcs)

    funcs_js += ['\n' + post_funcs + '// EMSCRIPTEN_END_FUNCS\n']

    simple = os.environ.get('EMCC_SIMPLE_ASM')
    class Counter:
      i = 0
    pre_tables = last_forwarded_json['Functions']['tables']['pre']
    del last_forwarded_json['Functions']['tables']['pre']

    def make_table(sig, raw):
      i = Counter.i
      Counter.i += 1
      bad = 'b' + str(i)
      params = ','.join(['p%d' % p for p in range(len(sig)-1)])
      coercions = ';'.join(['p%d = %sp%d%s' % (p, '+' if sig[p+1] != 'i' else '', p, '' if sig[p+1] != 'i' else '|0') for p in range(len(sig)-1)]) + ';'
      ret = '' if sig[0] == 'v' else ('return %s0' % ('+' if sig[0] != 'i' else ''))
      start = raw.index('[')
      end = raw.rindex(']')
      body = raw[start+1:end].split(',')
      for j in range(settings['RESERVED_FUNCTION_POINTERS']):
        body[2 + 2*j] = 'jsCall_%s_%s' % (sig, j)
      def fix_item(item):
        newline = '\n' in item
        return (bad if item.replace('\n', '') == '0' else item) + ('\n' if newline else '')
      body = ','.join(map(fix_item, body))
      return ('function %s(%s) { %s %s(%d); %s }' % (bad, params, coercions, 'abort' if not settings['ASSERTIONS'] else 'nullFunc', i, ret), raw[:start+1] + body + raw[end:])
    infos = [make_table(sig, raw) for sig, raw in last_forwarded_json['Functions']['tables'].iteritems()]
    function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n// EMSCRIPTEN_END_FUNCS\n' + '\n'.join([info[1] for info in infos])

    asm_setup = ''
    maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul']]
    fundamentals = ['Math', 'Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array']
    math_envs = ['Math.min'] # TODO: move min to maths
    asm_setup += '\n'.join(['var %s = %s;' % (f.replace('.', '_'), f) for f in math_envs])

    basic_funcs = ['abort', 'assert', 'asmPrintInt', 'asmPrintFloat', 'copyTempDouble', 'copyTempFloat'] + [m.replace('.', '_') for m in math_envs]
    if settings['RESERVED_FUNCTION_POINTERS'] > 0: basic_funcs.append('jsCall')
    if settings['SAFE_HEAP']: basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_STORE', 'SAFE_HEAP_CLEAR']
    if settings['CHECK_HEAP_ALIGN']: basic_funcs += ['CHECK_ALIGN_2', 'CHECK_ALIGN_4', 'CHECK_ALIGN_8']
    if settings['ASSERTIONS']:
      basic_funcs += ['nullFunc']
      asm_setup += 'function nullFunc(x) { Module["printErr"]("Invalid function pointer called. Perhaps a miscast function pointer (check compilation warnings) or bad vtable lookup (maybe due to derefing a bad pointer, like NULL)?"); abort(x) }\n'

    basic_vars = ['STACKTOP', 'STACK_MAX', 'tempDoublePtr', 'ABORT']
    basic_float_vars = ['NaN', 'Infinity']

    if forwarded_json['Types']['preciseI64MathUsed'] or \
       forwarded_json['Functions']['libraryFunctions'].get('llvm_cttz_i32') or \
       forwarded_json['Functions']['libraryFunctions'].get('llvm_ctlz_i32'):
      basic_vars += ['cttz_i8', 'ctlz_i8']

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'setThrew'] + ['setTempRet%d' % i for i in range(10)]
    # function tables
    def asm_coerce(value, sig):
      if sig == 'v': return value
      return ('+' if sig != 'i' else '') + value + ('|0' if sig == 'i' else '')

    function_tables = ['dynCall_' + table for table in last_forwarded_json['Functions']['tables']]
    function_tables_impls = []
    for sig in last_forwarded_json['Functions']['tables'].iterkeys():
      args = ','.join(['a' + str(i) for i in range(1, len(sig))])
      arg_coercions = ' '.join(['a' + str(i) + '=' + asm_coerce('a' + str(i), sig[i]) + ';' for i in range(1, len(sig))])
      coerced_args = ','.join([asm_coerce('a' + str(i), sig[i]) for i in range(1, len(sig))])
      ret = ('return ' if sig[0] != 'v' else '') + asm_coerce('FUNCTION_TABLE_%s[index&{{{ FTM_%s }}}](%s)' % (sig, sig, coerced_args), sig[0])
      function_tables_impls.append('''
  function dynCall_%s(index%s%s) {
    index = index|0;
    %s
    %s;
  }
''' % (sig, ',' if len(sig) > 1 else '', args, arg_coercions, ret))

      for i in range(settings['RESERVED_FUNCTION_POINTERS']):
        jsret = ('return ' if sig[0] != 'v' else '') + asm_coerce('jsCall(%d%s%s)' % (i, ',' if coerced_args else '', coerced_args), sig[0])
        function_tables_impls.append('''
  function jsCall_%s_%s(%s) {
    %s
    %s;
  }

''' % (sig, i, args, arg_coercions, jsret))
      args = ','.join(['a' + str(i) for i in range(1, len(sig))])
      args = 'index' + (',' if args else '') + args
      # C++ exceptions are numbers, and longjmp is a string 'longjmp'
      asm_setup += '''
function invoke_%s(%s) {
  try {
    %sModule["dynCall_%s"](%s);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
''' % (sig, args, 'return ' if sig[0] != 'v' else '', sig, args)
      basic_funcs.append('invoke_%s' % sig)

    # calculate exports
    exported_implemented_functions = list(exported_implemented_functions)
    exports = []
    if not simple:
      for export in exported_implemented_functions + asm_runtime_funcs + function_tables:
        exports.append("%s: %s" % (export, export))
      exports = '{ ' + ', '.join(exports) + ' }'
    else:
      exports = '_main'
    # calculate globals
    try:
      del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
    except:
      pass
    # If no named globals, only need externals
    global_vars = map(lambda g: g['name'], filter(lambda g: settings['NAMED_GLOBALS'] or g.get('external') or g.get('unIndexable'), forwarded_json['Variables']['globals'].values()))
    global_funcs = ['_' + key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1];
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global.' + g + ';\n' for g in maths]) + \
                       ''.join(['  var ' + g + '=env.' + math_fix(g) + ';\n' for g in basic_funcs + global_funcs])
    asm_global_vars = ''.join(['  var ' + g + '=env.' + g + '|0;\n' for g in basic_vars + global_vars]) + \
                      ''.join(['  var ' + g + '=+env.' + g + ';\n' for g in basic_float_vars])
    # sent data
    the_global = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'
    sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in basic_funcs + global_funcs + basic_vars + basic_float_vars + global_vars]) + ' }'
    # received
    if not simple:
      receiving = ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables])
    else:
      receiving = 'var _main = Module["_main"] = asm;'

    # finalize

    if DEBUG: print >> sys.stderr, 'asm text sizes', map(len, funcs_js), len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables), len('\n'.join(function_tables_impls)), len(function_tables_defs.replace('\n', '\n  ')), len(exports), len(the_global), len(sending), len(receiving)

    funcs_js = ['''
%s
function asmPrintInt(x, y) {
  Module.print('int ' + x + ',' + y);// + ' ' + new Error().stack);
}
function asmPrintFloat(x, y) {
  Module.print('float ' + x + ',' + y);// + ' ' + new Error().stack);
}
// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
  'use asm';
  var HEAP8 = new global.Int8Array(buffer);
  var HEAP16 = new global.Int16Array(buffer);
  var HEAP32 = new global.Int32Array(buffer);
  var HEAPU8 = new global.Uint8Array(buffer);
  var HEAPU16 = new global.Uint16Array(buffer);
  var HEAPU32 = new global.Uint32Array(buffer);
  var HEAPF32 = new global.Float32Array(buffer);
  var HEAPF64 = new global.Float64Array(buffer);
''' % (asm_setup,) + '\n' + asm_global_vars + '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
''' + ''.join(['''
  var tempRet%d = 0;''' % i for i in range(10)]) + '\n' + asm_global_funcs + '''
// EMSCRIPTEN_START_FUNCS
  function stackAlloc(size) {
    size = size|0;
    var ret = 0;
    ret = STACKTOP;
    STACKTOP = (STACKTOP + size)|0;
''' + ('STACKTOP = ((STACKTOP + 3)>>2)<<2;' if settings['TARGET_X86'] else 'STACKTOP = ((STACKTOP + 7)>>3)<<3;') + '''
    return ret|0;
  }
  function stackSave() {
    return STACKTOP|0;
  }
  function stackRestore(top) {
    top = top|0;
    STACKTOP = top;
  }
  function setThrew(threw, value) {
    threw = threw|0;
    value = value|0;
    if ((__THREW__|0) == 0) {
      __THREW__ = threw;
      threwValue = value;
    }
  }
''' + ''.join(['''
  function setTempRet%d(value) {
    value = value|0;
    tempRet%d = value;
  }
''' % (i, i) for i in range(10)])] + funcs_js + ['''
  %s

  return %s;
})
// EMSCRIPTEN_END_ASM
(%s, %s, buffer);
%s;
Runtime.stackAlloc = function(size) { return asm['stackAlloc'](size) };
Runtime.stackSave = function() { return asm['stackSave']() };
Runtime.stackRestore = function(top) { asm['stackRestore'](top) };
''' % (pre_tables + '\n'.join(function_tables_impls) + '\n' + function_tables_defs.replace('\n', '\n  '), exports, the_global, sending, receiving)]

    # Set function table masks
    def function_table_maskize(js):
      masks = {}
      default = None
      for sig, table in last_forwarded_json['Functions']['tables'].iteritems():
        masks[sig] = str(table.count(','))
        default = sig
      def fix(m):
        sig = m.groups(0)[0]
        return masks[sig]
      return re.sub(r'{{{ FTM_([\w\d_$]+) }}}', lambda m: fix(m), js) # masks[m.groups(0)[0]]
    funcs_js = map(function_table_maskize, funcs_js)
  else:
    function_tables_defs = '\n'.join([table for table in last_forwarded_json['Functions']['tables'].itervalues()])
    outfile.write(function_tables_defs)
    funcs_js = ['''
// EMSCRIPTEN_START_FUNCS
'''] + funcs_js + ['''
// EMSCRIPTEN_END_FUNCS
''']

  for funcs_js_item in funcs_js: # do this loop carefully to save memory
    funcs_js_item = indexize(funcs_js_item)
    funcs_js_item = blockaddrsize(funcs_js_item)
    outfile.write(funcs_js_item)
  funcs_js = None

  outfile.write(indexize(post))
  if DEBUG: print >> sys.stderr, '  emscript: phase 3 took %s seconds' % (time.time() - t)

  outfile.close()

def main(args, compiler_engine, cache, jcache, relooper, temp_files, DEBUG, DEBUG_CACHE):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # Add header defines to settings
  defines = {}
  include_root = path_from_root('system', 'include')
  headers = args.headers[0].split(',') if len(args.headers) > 0 else []
  seen_headers = set()
  while len(headers) > 0:
    header = headers.pop(0)
    if not os.path.isabs(header):
      header = os.path.join(include_root, header)
    seen_headers.add(header)
    for line in open(header, 'r'):
      line = line.replace('\t', ' ')
      m = re.match('^ *# *define +(?P<name>[-\w_.]+) +\(?(?P<value>[-\w_.|]+)\)?.*', line)
      if not m:
        # Catch enum defines of a very limited sort
        m = re.match('^ +(?P<name>[A-Z_\d]+) += +(?P<value>\d+).*', line)
      if m:
        if m.group('name') != m.group('value'):
          defines[m.group('name')] = m.group('value')
        #else:
        #  print 'Warning: %s #defined to itself' % m.group('name') # XXX this can happen if we are set to be equal to an enum (with the same name)
      m = re.match('^ *# *include *["<](?P<name>[\w_.-/]+)[">].*', line)
      if m:
        # Find this file
        found = False
        for w in [w for w in os.walk(include_root)]:
          for f in w[2]:
            curr = os.path.join(w[0], f)
            if curr.endswith(m.group('name')) and curr not in seen_headers:
              headers.append(curr)
              found = True
              break
          if found: break
        #assert found, 'Could not find header: ' + m.group('name')
  if len(defines) > 0:
    def lookup(value):
      try:
        while not unicode(value).isnumeric():
          value = defines[value]
        return value
      except:
        pass
      try: # 0x300 etc.
        value = eval(value)
        return value
      except:
        pass
      try: # CONST1|CONST2
        parts = map(lookup, value.split('|'))
        value = reduce(lambda a, b: a|b, map(eval, parts))
        return value
      except:
        pass
      return None
    for key, value in defines.items():
      value = lookup(value)
      if value is not None:
        defines[key] = str(value)
      else:
        del defines[key]
    #print >> sys.stderr, 'new defs:', str(defines).replace(',', ',\n  '), '\n\n'
    settings.setdefault('C_DEFINES', {}).update(defines)

  # libraries
  libraries = args.libraries[0].split(',') if len(args.libraries) > 0 else []

  # Compile the assembly to Javascript.
  if settings.get('RELOOP'):
    if not relooper:
      relooper = cache.get_path('relooper.js')
    settings.setdefault('RELOOPER', relooper)
    if not os.path.exists(relooper):
      from tools import shared
      shared.Building.ensure_relooper(relooper)

  emscript(args.infile, settings, args.outfile, libraries, compiler_engine=compiler_engine,
           jcache=jcache, temp_files=temp_files, DEBUG=DEBUG, DEBUG_CACHE=DEBUG_CACHE)

def _main(environ):
  response_file = True
  while response_file:
    response_file = None
    for index in range(1, len(sys.argv)):
      if sys.argv[index][0] == '@':
        # found one, loop again next time
        response_file = True
        response_file_args = read_response_file(sys.argv[index])
        # slice in extra_args in place of the response file arg
        sys.argv[index:index+1] = response_file_args
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
  parser.add_option('--relooper',
                    default=None,
                    help='Which relooper file to use if RELOOP is enabled.')
  parser.add_option('-s', '--setting',
                    dest='settings',
                    default=[],
                    action='append',
                    metavar='FOO=BAR',
                    help=('Overrides for settings defined in settings.js. '
                          'May occur multiple times.'))
  parser.add_option('-j', '--jcache',
                    action='store_true',
                    default=False,
                    help=('Enable jcache (ccache-like caching of compilation results, for faster incremental builds).'))
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
                    default=environ.get('EMSCRIPTEN_SUPPRESS_USAGE_WARNING'),
                    help=('Suppress usage warning'))

  # Convert to the same format that argparse would have produced.
  keywords, positional = parser.parse_args()

  if not keywords.suppressUsageWarning:
    print >> sys.stderr, '''
==============================================================
WARNING: You should normally never use this! Use emcc instead.
==============================================================
  '''

  if len(positional) != 1:
    raise RuntimeError('Must provide exactly one positional argument.')
  keywords.infile = os.path.abspath(positional[0])
  if isinstance(keywords.outfile, basestring):
    keywords.outfile = open(keywords.outfile, 'w')

  if keywords.relooper:
    relooper = os.path.abspath(keywords.relooper)
  else:
    relooper = None # use the cache

  if keywords.temp_dir is None:
    temp_files = get_configuration().get_temp_files()
    temp_dir = get_configuration().TEMP_DIR
  else:
    temp_dir = os.path.abspath(keywords.temp_dir)
    if not os.path.exists(temp_dir):
      os.makedirs(temp_dir)
    temp_files = tempfiles.TempFiles(temp_dir)

  if keywords.compiler is None:
    from tools import shared
    keywords.compiler = shared.COMPILER_ENGINE

  if keywords.verbose is None:
    DEBUG = get_configuration().DEBUG
    DEBUG_CACHE = get_configuration().DEBUG_CACHE
  else:
    DEBUG = keywords.verbose
    DEBUG_CACHE = keywords.verbose

  cache = cache_module.Cache()
  temp_files.run_and_clean(lambda: main(
    keywords,
    compiler_engine=keywords.compiler,
    cache=cache,
    jcache=cache_module.JCache(cache) if keywords.jcache else None,
    relooper=relooper,
    temp_files=temp_files,
    DEBUG=DEBUG,
    DEBUG_CACHE=DEBUG_CACHE,
  ))

if __name__ == '__main__':
  _main(environ=os.environ)
