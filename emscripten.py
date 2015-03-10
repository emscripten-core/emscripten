#!/usr/bin/env python2

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import os, sys, json, optparse, subprocess, re, time, multiprocessing, string, logging

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

def scan(ll, settings):
  # blockaddress(@main, %23)
  blockaddrs = []
  for blockaddr in re.findall('blockaddress\([^)]*\)', ll):
    b = blockaddr.split('(')[1][:-1].split(', ')
    blockaddrs.append(b)
  if len(blockaddrs) > 0:
    settings['NECESSARY_BLOCKADDRS'] = blockaddrs

NUM_CHUNKS_PER_CORE = 1.0
MIN_CHUNK_SIZE = 1024*1024
MAX_CHUNK_SIZE = float(os.environ.get('EMSCRIPT_MAX_CHUNK_SIZE') or 'inf') # configuring this is just for debugging purposes

STDERR_FILE = os.environ.get('EMCC_STDERR_FILE')
if STDERR_FILE:
  STDERR_FILE = os.path.abspath(STDERR_FILE)
  logging.info('logging stderr in js compiler phase into %s' % STDERR_FILE)
  STDERR_FILE = open(STDERR_FILE, 'w')

def process_funcs((i, funcs_file, meta, settings_file, compiler, forwarded_file, libraries, compiler_engine, DEBUG)):
  try:
    #print >> sys.stderr, 'running', str([settings_file, funcs_file, 'funcs', forwarded_file] + libraries).replace("'/", "'") # can use this in src/compiler_funcs.html arguments,
    #                                                                                                                         # just copy temp dir to under this one
    out = jsrun.run_js(
      compiler,
      engine=compiler_engine,
      args=[settings_file, funcs_file, 'funcs', forwarded_file] + libraries,
      stdout=subprocess.PIPE,
      stderr=STDERR_FILE,
      cwd=path_from_root('src'))
  except KeyboardInterrupt:
    # Python 2.7 seems to lock up when a child process throws KeyboardInterrupt
    raise Exception()
  if DEBUG: logging.debug('.')
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

  if DEBUG: logging.debug('emscript: ll=>js')

  if jcache: jcache.ensure()

  # Pre-scan ll and alter settings as necessary
  if DEBUG: t = time.time()
  ll = open(infile).read()
  scan(ll, settings)
  total_ll_size = len(ll)
  if DEBUG: logging.debug('  emscript: scan took %s seconds' % (time.time() - t))

  # Split input into the relevant parts for each phase

  if DEBUG: t = time.time()

  pre = []
  funcs = [] # split up functions here, for parallelism later

  meta_start = ll.find('\n!')
  if meta_start > 0:
    meta = ll[meta_start:]
  else:
    meta = ''
    meta_start = -1

  start = ll.find('\n') if ll[0] == ';' else 0 # ignore first line, which contains ; ModuleID = '/dir name'

  func_start = start
  last = func_start
  while 1:
    last = func_start
    func_start = ll.find('\ndefine ', func_start)
    if func_start > last:
      pre.append(ll[last:min(func_start+1, meta_start) if meta_start > 0 else func_start+1] + '\n')
    if func_start < 0:
      pre.append(ll[last:meta_start] + '\n')
      break
    header = ll[func_start+1:ll.find('\n', func_start+1)+1]
    end = ll.find('\n}', func_start)
    last = end+3
    funcs.append((header, ll[func_start+1:last]))
    pre.append(header + '}\n')
    func_start = last
  ll = None

  if DEBUG and len(meta) > 1024*1024: logging.debug('emscript warning: large amounts of metadata, will slow things down')
  if DEBUG: logging.debug('  emscript: split took %s seconds' % (time.time() - t))

  if len(funcs) == 0:
    logging.error('No functions to process. Make sure you prevented LLVM from eliminating them as dead (use EXPORTED_FUNCTIONS if necessary, see the FAQ)')

  #if DEBUG:
  #  logging.debug('========= pre ================\n')
  #  logging.debug(''.join(pre))
  #  logging.debug('========== funcs ===============\n')
  #  for func in funcs:
  #    logging.debug('\n// ===\n\n', ''.join(func))
  #  logging.debug('=========================\n')

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
    if DEBUG_CACHE: logging.debug('shortkey', shortkey)

    out = jcache.get(shortkey, keys)

    if DEBUG_CACHE and not out:
      dfpath = os.path.join(get_configuration().TEMP_DIR, "ems_" + shortkey)
      dfp = open(dfpath, 'w')
      dfp.write(pre_input)
      dfp.write("\n\n========================== settings_text\n\n")
      dfp.write(settings_text)
      dfp.write("\n\n========================== libraries\n\n")
      dfp.write("\n".join(libraries))
      dfp.close()
      logging.debug('  cache miss, key data dumped to %s' % dfpath)

    if out and DEBUG: logging.debug('  loading pre from jcache')
  if not out:
    open(pre_file, 'w').write(pre_input)
    #print >> sys.stderr, 'running', str([settings_file, pre_file, 'pre'] + libraries).replace("'/", "'") # see funcs
    out = jsrun.run_js(compiler, compiler_engine, [settings_file, pre_file, 'pre'] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                       cwd=path_from_root('src'))
    assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
    if jcache:
      if DEBUG: logging.debug('  saving pre to jcache')
      jcache.set(shortkey, keys, out)
  pre, forwarded_data = out.split('//FORWARDED_DATA:')
  forwarded_file = temp_files.get('.json').name
  pre_input = None
  open(forwarded_file, 'w').write(forwarded_data)
  if DEBUG: logging.debug('  emscript: phase 1 took %s seconds' % (time.time() - t))

  indexed_functions = set()
  forwarded_json = json.loads(forwarded_data)
  for key in forwarded_json['Functions']['indexedFunctions'].iterkeys():
    indexed_functions.add(key)

  # Phase 2 - func

  cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())
  assert cores >= 1
  if cores > 1:
    intended_num_chunks = int(round(cores * NUM_CHUNKS_PER_CORE))
    chunk_size = max(MIN_CHUNK_SIZE, total_ll_size / intended_num_chunks)
    chunk_size += 3*len(meta) # keep ratio of lots of function code to meta (expensive to process, and done in each parallel task)
    chunk_size = min(MAX_CHUNK_SIZE, chunk_size)
  else:
    chunk_size = MAX_CHUNK_SIZE # if 1 core, just use the max chunk size

  if DEBUG: t = time.time()
  if settings.get('ASM_JS'):
    settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
    save_settings()

  chunks = cache_module.chunkify(
    funcs, chunk_size,
    jcache.get_cachename('emscript_files') if jcache else None)

  #sys.exit(1)
  #chunks = [chunks[0]] # pick specific chunks for debugging/profiling

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
      if out and DEBUG: logging.debug('  loading %d funcchunks from jcache' % len(cached_outputs))
    else:
      cached_outputs = []

  # TODO: minimize size of forwarded data from funcs to what we actually need

  if len(chunks) > 0:
    if cores == 1 and total_ll_size < MAX_CHUNK_SIZE:
      assert len(chunks) == 1, 'no point in splitting up without multiple cores'

    if DEBUG: logging.debug('  emscript: phase 2 working on %d chunks %s (intended chunk size: %.2f MB, meta: %.2f MB, forwarded: %.2f MB, total: %.2f MB)' % (len(chunks), ('using %d cores' % cores) if len(chunks) > 1 else '', chunk_size/(1024*1024.), len(meta)/(1024*1024.), len(forwarded_data)/(1024*1024.), total_ll_size/(1024*1024.)))

    commands = []
    for i in range(len(chunks)):
      funcs_file = temp_files.get('.func_%d.ll' % i).name
      f = open(funcs_file, 'w')
      f.write(chunks[i])
      if not jcache:
        chunks[i] = None # leave chunks array alive (need its length later)
      f.write('\n')
      f.write(meta)
      f.close()
      commands.append(
        (i, funcs_file, meta, settings_file, compiler, forwarded_file, libraries, compiler_engine,# + ['--prof'],
         DEBUG)
      )

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
    if out and DEBUG and len(chunks) > 0: logging.debug('  saving %d funcchunks to jcache' % len(chunks))

  chunks = None

  if jcache: outputs += cached_outputs # TODO: preserve order

  outputs = [output.split('//FORWARDED_DATA:') for output in outputs]
  for output in outputs:
    assert len(output) == 2, 'Did not receive forwarded data in an output - process failed? We only got: ' + output[0][-3000:]

  if DEBUG: logging.debug('  emscript: phase 2 took %s seconds' % (time.time() - t))
  if DEBUG: t = time.time()

  # merge forwarded data
  if settings.get('ASM_JS'):
    all_exported_functions = set(settings['EXPORTED_FUNCTIONS']) # both asm.js and otherwise
    for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
      all_exported_functions.add('_' + additional_export)
    exported_implemented_functions = set()
  for func_js, curr_forwarded_data in outputs:
    curr_forwarded_json = json.loads(curr_forwarded_data)
    forwarded_json['Types']['hasInlineJS'] = forwarded_json['Types']['hasInlineJS'] or curr_forwarded_json['Types']['hasInlineJS']
    forwarded_json['Types']['usesSIMD'] = forwarded_json['Types']['usesSIMD'] or curr_forwarded_json['Types']['usesSIMD']
    forwarded_json['Types']['preciseI64MathUsed'] = forwarded_json['Types']['preciseI64MathUsed'] or curr_forwarded_json['Types']['preciseI64MathUsed']
    for key, value in curr_forwarded_json['Functions']['blockAddresses'].iteritems():
      forwarded_json['Functions']['blockAddresses'][key] = value
    for key in curr_forwarded_json['Functions']['indexedFunctions'].iterkeys():
      indexed_functions.add(key)
    if settings.get('ASM_JS'):
      export_bindings = settings['EXPORT_BINDINGS']
      export_all = settings['EXPORT_ALL']
      for key in curr_forwarded_json['Functions']['implementedFunctions'].iterkeys():
        if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
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
  if DEBUG: logging.debug('  emscript: phase 2b took %s seconds' % (time.time() - t))
  if DEBUG: t = time.time()

  # calculations on merged forwarded data
  forwarded_json['Functions']['indexedFunctions'] = {}
  i = settings['FUNCTION_POINTER_ALIGNMENT'] # universal counter
  if settings['ASM_JS']: i += settings['RESERVED_FUNCTION_POINTERS']*settings['FUNCTION_POINTER_ALIGNMENT']
  base_fp = i
  table_counters = {} # table-specific counters
  alias = settings['ASM_JS'] and settings['ALIASING_FUNCTION_POINTERS']
  sig = None
  for indexed in indexed_functions:
    if alias:
      sig = forwarded_json['Functions']['implementedFunctions'].get(indexed) or forwarded_json['Functions']['unimplementedFunctions'].get(indexed)
      assert sig, indexed
      if sig not in table_counters:
        table_counters[sig] = base_fp
      curr = table_counters[sig]
      table_counters[sig] += settings['FUNCTION_POINTER_ALIGNMENT']
    else:
      curr = i
      i += settings['FUNCTION_POINTER_ALIGNMENT']
    #logging.debug('function indexing ' + str([indexed, curr, sig]))
    forwarded_json['Functions']['indexedFunctions'][indexed] = curr # make sure not to modify this python object later - we use it in indexize

  def split_32(x):
    x = int(x)
    return '%d,%d,%d,%d' % (x&255, (x >> 8)&255, (x >> 16)&255, (x >> 24)&255)

  indexing = forwarded_json['Functions']['indexedFunctions']
  def indexize_mem(js):
    return re.sub(r"\"?'?{{ FI_([\w\d_$]+) }}'?\"?,0,0,0", lambda m: split_32(indexing.get(m.groups(0)[0]) or 0), js)
  def indexize(js):
    return re.sub(r"'{{ FI_([\w\d_$]+) }}'", lambda m: str(indexing.get(m.groups(0)[0]) or 0), js)

  blockaddrs = forwarded_json['Functions']['blockAddresses']
  def blockaddrsize_mem(js):
    return re.sub(r'"?{{{ BA_([\w\d_$]+)\|([\w\d_$]+) }}}"?,0,0,0', lambda m: split_32(blockaddrs[m.groups(0)[0]][m.groups(0)[1]]), js)
  def blockaddrsize(js):
    return re.sub(r'"?{{{ BA_([\w\d_$]+)\|([\w\d_$]+) }}}"?', lambda m: str(blockaddrs[m.groups(0)[0]][m.groups(0)[1]]), js)

  pre = blockaddrsize(blockaddrsize_mem(indexize(indexize_mem(pre))))

  if settings.get('ASM_JS'):
    # move postsets into the asm module
    class PostSets: js = ''
    def handle_post_sets(m):
      PostSets.js = m.group(0)
      return '\n'
    pre = re.sub(r'function runPostSets[^}]+}', handle_post_sets, pre)

  #if DEBUG: outfile.write('// pre\n')
  outfile.write(pre)
  pre = None

  #if DEBUG: outfile.write('// funcs\n')

  # forward
  forwarded_data = json.dumps(forwarded_json)
  forwarded_file = temp_files.get('.2.json').name
  open(forwarded_file, 'w').write(indexize(forwarded_data))
  if DEBUG: logging.debug('  emscript: phase 2c took %s seconds' % (time.time() - t))

  # Phase 3 - post
  if DEBUG: t = time.time()
  post_file = temp_files.get('.post.ll').name
  open(post_file, 'w').write('\n') # no input, just processing of forwarded data
  out = jsrun.run_js(compiler, compiler_engine, [settings_file, post_file, 'post', forwarded_file] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
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
      j = 0
    pre_tables = last_forwarded_json['Functions']['tables']['pre']
    del last_forwarded_json['Functions']['tables']['pre']

    def make_table(sig, raw):
      i = Counter.i
      Counter.i += 1
      bad = 'b' + str(i)
      params = ','.join(['p%d' % p for p in range(len(sig)-1)])
      coercions = ';'.join(['p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, sig[p+1], settings)) for p in range(len(sig)-1)]) + ';'
      ret = '' if sig[0] == 'v' else ('return %s' % shared.JS.make_initializer(sig[0], settings))
      start = raw.index('[')
      end = raw.rindex(']')
      body = raw[start+1:end].split(',')
      for j in range(settings['RESERVED_FUNCTION_POINTERS']):
        body[settings['FUNCTION_POINTER_ALIGNMENT'] * (1 + j)] = 'jsCall_%s_%s' % (sig, j)
      Counter.j = 0
      def fix_item(item):
        Counter.j += 1
        newline = Counter.j % 30 == 29
        if item == '0': return bad if not newline else (bad + '\n')
        return item if not newline else (item + '\n')
      body = ','.join(map(fix_item, body))
      return ('function %s(%s) { %s %s(%d); %s }' % (bad, params, coercions, 'abort' if not settings['ASSERTIONS'] else 'nullFunc', i, ret), ''.join([raw[:start+1], body, raw[end:]]))

    infos = [make_table(sig, raw) for sig, raw in last_forwarded_json['Functions']['tables'].iteritems()]

    function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n// EMSCRIPTEN_END_FUNCS\n' + '\n'.join([info[1] for info in infos])

    asm_setup = ''
    maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul']]
    fundamentals = ['Math', 'Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array']
    if settings['ALLOW_MEMORY_GROWTH']: fundamentals.append('byteLength')
    math_envs = ['Math.min'] # TODO: move min to maths
    asm_setup += '\n'.join(['var %s = %s;' % (f.replace('.', '_'), f) for f in math_envs])

    if settings['PRECISE_F32']: maths += ['Math.fround']

    basic_funcs = ['abort', 'assert', 'asmPrintInt', 'asmPrintFloat'] + [m.replace('.', '_') for m in math_envs]
    if settings['RESERVED_FUNCTION_POINTERS'] > 0: basic_funcs.append('jsCall')
    if settings['SAFE_HEAP']: basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_STORE', 'SAFE_FT_MASK']
    if settings['CHECK_HEAP_ALIGN']: basic_funcs += ['CHECK_ALIGN_2', 'CHECK_ALIGN_4', 'CHECK_ALIGN_8']
    if settings['ASSERTIONS']:
      basic_funcs += ['nullFunc']
      asm_setup += 'function nullFunc(x) { Module["printErr"]("Invalid function pointer called. Perhaps a miscast function pointer (check compilation warnings) or bad vtable lookup (maybe due to derefing a bad pointer, like NULL)?"); abort(x) }\n'

    basic_vars = ['STACKTOP', 'STACK_MAX', 'tempDoublePtr', 'ABORT']
    basic_float_vars = ['NaN', 'Infinity']

    if forwarded_json['Types']['preciseI64MathUsed'] or \
       forwarded_json['Functions']['libraryFunctions'].get('_llvm_cttz_i32') or \
       forwarded_json['Functions']['libraryFunctions'].get('_llvm_ctlz_i32'):
      basic_vars += ['cttz_i8', 'ctlz_i8']

    if settings.get('DLOPEN_SUPPORT'):
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_vars.append('F_BASE_%s' % sig)
        asm_setup += '  var F_BASE_%s = %s;\n' % (sig, 'FUNCTION_TABLE_OFFSET' if settings.get('SIDE_MODULE') else '0') + '\n'

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'setThrew'] + ['setTempRet%d' % i for i in range(10)] + ['getTempRet%d' % i for i in range(10)]
    # function tables
    function_tables = ['dynCall_' + table for table in last_forwarded_json['Functions']['tables']]
    function_tables_impls = []

    for sig in last_forwarded_json['Functions']['tables'].iterkeys():
      args = ','.join(['a' + str(i) for i in range(1, len(sig))])
      arg_coercions = ' '.join(['a' + str(i) + '=' + shared.JS.make_coercion('a' + str(i), sig[i], settings) + ';' for i in range(1, len(sig))])
      coerced_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings) for i in range(1, len(sig))])
      ret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('FUNCTION_TABLE_%s[index&{{{ FTM_%s }}}](%s)' % (sig, sig, coerced_args), sig[0], settings)
      function_tables_impls.append('''
  function dynCall_%s(index%s%s) {
    index = index|0;
    %s
    %s;
  }
''' % (sig, ',' if len(sig) > 1 else '', args, arg_coercions, ret))

      for i in range(settings['RESERVED_FUNCTION_POINTERS']):
        jsret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('jsCall(%d%s%s)' % (i, ',' if coerced_args else '', coerced_args), sig[0], settings)
        function_tables_impls.append('''
  function jsCall_%s_%s(%s) {
    %s
    %s;
  }

''' % (sig, i, args, arg_coercions, jsret))
      shared.Settings.copy(settings)
      asm_setup += '\n' + shared.JS.make_invoke(sig) + '\n'
      basic_funcs.append('invoke_%s' % sig)
      if settings.get('DLOPEN_SUPPORT'):
        asm_setup += '\n' + shared.JS.make_extcall(sig) + '\n'
        basic_funcs.append('extCall_%s' % sig)

    # calculate exports
    exported_implemented_functions = list(exported_implemented_functions)
    exported_implemented_functions.append('runPostSets')
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
    global_funcs = [key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1]
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global.' + g + ';\n' for g in maths]) + \
                       ''.join(['  var ' + g + '=env.' + math_fix(g) + ';\n' for g in basic_funcs + global_funcs])
    asm_global_vars = ''.join(['  var ' + g + '=env.' + g + '|0;\n' for g in basic_vars + global_vars]) + \
                      ''.join(['  var ' + g + '=+env.' + g + ';\n' for g in basic_float_vars])
    # In linkable modules, we need to add some explicit globals for global variables that can be linked and used across modules
    if settings.get('MAIN_MODULE') or settings.get('SIDE_MODULE'):
      assert settings.get('TARGET_ASMJS_UNKNOWN_EMSCRIPTEN'), 'TODO: support x86 target when linking modules (needs offset of 4 and not 8 here)'
      for key, value in forwarded_json['Variables']['globals'].iteritems():
        if value.get('linkable'):
          init = forwarded_json['Variables']['indexedGlobals'][key] + 8 # 8 is Runtime.GLOBAL_BASE / STATIC_BASE
          if settings.get('SIDE_MODULE'): init = '(H_BASE+' + str(init) + ')|0'
          asm_global_vars += '  var %s=%s;\n' % (key, str(init))

    # sent data
    the_global = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'
    sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in basic_funcs + global_funcs + basic_vars + basic_float_vars + global_vars]) + ' }'
    # received
    if not simple:
      receiving = ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables])
    else:
      receiving = 'var _main = Module["_main"] = asm;'

    # finalize

    if DEBUG: logging.debug('asm text sizes' + str([map(len, funcs_js), len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables), len('\n'.join(function_tables_impls)), len(function_tables_defs.replace('\n', '\n  ')), len(exports), len(the_global), len(sending), len(receiving)]))

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
  %s
%s
''' % (asm_setup, "'use asm';" if not forwarded_json['Types']['hasInlineJS'] and not settings['SIDE_MODULE'] and settings['ASM_JS'] == 1 else "'almost asm';",
'''
  var HEAP8 = new global.Int8Array(buffer);
  var HEAP16 = new global.Int16Array(buffer);
  var HEAP32 = new global.Int32Array(buffer);
  var HEAPU8 = new global.Uint8Array(buffer);
  var HEAPU16 = new global.Uint16Array(buffer);
  var HEAPU32 = new global.Uint32Array(buffer);
  var HEAPF32 = new global.Float32Array(buffer);
  var HEAPF64 = new global.Float64Array(buffer);
''' if not settings['ALLOW_MEMORY_GROWTH'] else '''
  var Int8View = global.Int8Array;
  var Int16View = global.Int16Array;
  var Int32View = global.Int32Array;
  var Uint8View = global.Uint8Array;
  var Uint16View = global.Uint16Array;
  var Uint32View = global.Uint32Array;
  var Float32View = global.Float32Array;
  var Float64View = global.Float64Array;
  var HEAP8 = new Int8View(buffer);
  var HEAP16 = new Int16View(buffer);
  var HEAP32 = new Int32View(buffer);
  var HEAPU8 = new Uint8View(buffer);
  var HEAPU16 = new Uint16View(buffer);
  var HEAPU32 = new Uint32View(buffer);
  var HEAPF32 = new Float32View(buffer);
  var HEAPF64 = new Float64View(buffer);
  var byteLength = global.byteLength;
''') + '\n' + asm_global_vars + '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
''' + ''.join(['''
  var tempRet%d = 0;''' % i for i in range(10)]) + '\n' + asm_global_funcs] + ['  var tempFloat = %s;\n' % ('Math_fround(0)' if settings.get('PRECISE_F32') else '0.0')] + (['  const f0 = Math_fround(0);\n'] if settings.get('PRECISE_F32') else []) + ['''
// EMSCRIPTEN_START_FUNCS
function stackAlloc(size) {
  size = size|0;
  var ret = 0;
  ret = STACKTOP;
  STACKTOP = (STACKTOP + size)|0;
''' + ('STACKTOP = (STACKTOP + 3)&-4;' if settings['TARGET_X86'] else 'STACKTOP = (STACKTOP + 7)&-8;') + '''
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
function copyTempFloat(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr>>0] = HEAP8[ptr>>0];
  HEAP8[tempDoublePtr+1>>0] = HEAP8[ptr+1>>0];
  HEAP8[tempDoublePtr+2>>0] = HEAP8[ptr+2>>0];
  HEAP8[tempDoublePtr+3>>0] = HEAP8[ptr+3>>0];
}
function copyTempDouble(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr>>0] = HEAP8[ptr>>0];
  HEAP8[tempDoublePtr+1>>0] = HEAP8[ptr+1>>0];
  HEAP8[tempDoublePtr+2>>0] = HEAP8[ptr+2>>0];
  HEAP8[tempDoublePtr+3>>0] = HEAP8[ptr+3>>0];
  HEAP8[tempDoublePtr+4>>0] = HEAP8[ptr+4>>0];
  HEAP8[tempDoublePtr+5>>0] = HEAP8[ptr+5>>0];
  HEAP8[tempDoublePtr+6>>0] = HEAP8[ptr+6>>0];
  HEAP8[tempDoublePtr+7>>0] = HEAP8[ptr+7>>0];
}
''' + ''.join(['''
function setTempRet%d(value) {
  value = value|0;
  tempRet%d = value;
}
''' % (i, i) for i in range(10)]) + ''.join(['''
function getTempRet%d() {
  return tempRet%d|0;
}
''' % (i, i) for i in range(10)])] + [PostSets.js + '\n'] + funcs_js + ['''
  %s

  return %s;
})
// EMSCRIPTEN_END_ASM
(%s, %s, buffer);
%s;
''' % (pre_tables + '\n'.join(function_tables_impls) + '\n' + function_tables_defs.replace('\n', '\n  '), exports, the_global, sending, receiving)]

    if not settings.get('SIDE_MODULE'):
      funcs_js.append('''
Runtime.stackAlloc = asm['stackAlloc'];
Runtime.stackSave = asm['stackSave'];
Runtime.stackRestore = asm['stackRestore'];
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
      return re.sub(r'{{{ FTM_([\w\d_$]+) }}}', lambda m: fix(m), js) # masks[m.groups(0)[0]]
    funcs_js = map(lambda js: function_table_maskize(js, masks), funcs_js)

    if settings.get('DLOPEN_SUPPORT'):
      funcs_js.append('''
  asm.maxFunctionIndex = %(max_mask)d;
  DLFCN.registerFunctions(asm, %(max_mask)d+1, %(sigs)s, Module);
  Module.SYMBOL_TABLE = SYMBOL_TABLE;
''' % { 'max_mask': max_mask, 'sigs': str(map(str, last_forwarded_json['Functions']['tables'].keys())) })

  else:
    function_tables_defs = '\n'.join([table for table in last_forwarded_json['Functions']['tables'].itervalues()])
    outfile.write(function_tables_defs)
    funcs_js = ['''
// EMSCRIPTEN_START_FUNCS
'''] + funcs_js + ['''
// EMSCRIPTEN_END_FUNCS
''']

  # Create symbol table for self-dlopen
  if settings.get('DLOPEN_SUPPORT'):
    symbol_table = {}
    for k, v in forwarded_json['Variables']['indexedGlobals'].iteritems():
       if forwarded_json['Variables']['globals'][k]['named']:
         symbol_table[k] = str(v + forwarded_json['Runtime']['GLOBAL_BASE'])
    for raw in last_forwarded_json['Functions']['tables'].itervalues():
      if raw == '': continue
      table = map(string.strip, raw[raw.find('[')+1:raw.find(']')].split(","))
      for i in range(len(table)):
        value = table[i]
        if value != '0':
          if settings.get('SIDE_MODULE'):
            symbol_table[value] = 'FUNCTION_TABLE_OFFSET+' + str(i)
          else:
            symbol_table[value] = str(i)
    outfile.write("var SYMBOL_TABLE = %s;" % json.dumps(symbol_table).replace('"', ''))

  for i in range(len(funcs_js)): # do this loop carefully to save memory
    funcs_js_item = funcs_js[i]
    funcs_js[i] = None
    funcs_js_item = indexize(funcs_js_item)
    funcs_js_item = blockaddrsize(funcs_js_item)
    if WINDOWS: funcs_js_item = funcs_js_item.replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
    outfile.write(funcs_js_item)
  funcs_js = None

  indexized = indexize(post)
  if WINDOWS: indexized = indexized.replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
  outfile.write(indexized)
  outfile.close()
  if DEBUG: logging.debug('  emscript: phase 3 took %s seconds' % (time.time() - t))

# emscript_fast: emscript'en code using the 'fast' compilation path, using
#                an LLVM backend
# FIXME: this is just a copy-paste of normal emscript(), and we trample it
#        if the proper env var is set (see below). we should refactor to
#        share code between the two, once emscript_fast stabilizes (or,
#        leaving it separate like it is will make it trivial to rip out
#        if the experiment fails)

def emscript_fast(infile, settings, outfile, libraries=[], compiler_engine=None,
             jcache=None, temp_files=None, DEBUG=None, DEBUG_CACHE=None):
  """Runs the emscripten LLVM-to-JS compiler.

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted settings that override the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """

  assert settings['ASM_JS'], 'fastcomp is asm.js-only (mode 1 or 2)'

  success = False

  try:

    # Overview:
    #   * Run LLVM backend to emit JS. JS includes function bodies, memory initializer,
    #     and various metadata
    #   * Run compiler.js on the metadata to emit the shell js code, pre/post-ambles,
    #     JS library dependencies, etc.

    temp_js = temp_files.get('.4.js').name
    backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
    backend_args = [backend_compiler, infile, '-march=js', '-filetype=asm', '-o', temp_js]
    if settings['PRECISE_F32']:
      backend_args += ['-emscripten-precise-f32']
    if settings['WARN_UNALIGNED']:
      backend_args += ['-emscripten-warn-unaligned']
    if settings['RESERVED_FUNCTION_POINTERS'] > 0:
      backend_args += ['-emscripten-reserved-function-pointers=%d' % settings['RESERVED_FUNCTION_POINTERS']]
    if settings['ASSERTIONS'] > 0:
      backend_args += ['-emscripten-assertions=%d' % settings['ASSERTIONS']]
    if settings['ALIASING_FUNCTION_POINTERS'] == 0:
      backend_args += ['-emscripten-no-aliasing-function-pointers']
    if settings['GLOBAL_BASE'] >= 0:
      backend_args += ['-emscripten-global-base=%d' % settings['GLOBAL_BASE']]
    backend_args += ['-O' + str(settings['OPT_LEVEL'])]
    if DEBUG:
      logging.debug('emscript: llvm backend: ' + ' '.join(backend_args))
      t = time.time()
    shared.jsrun.timeout_run(subprocess.Popen(backend_args, stdout=subprocess.PIPE))
    if DEBUG:
      logging.debug('  emscript: llvm backend took %s seconds' % (time.time() - t))
      t = time.time()

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
    metadata = json.loads(metadata_raw)
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
          e = num.find('e');
          if e < 0:
            num += '.0'
          else:
            num = num[:e] + '.0' + num[e:]
        return m.group(1) + m.group(2) + num
      funcs = re.sub(r'([(=,+\-*/%<>:?] *)\+(-?)((0x)?[0-9a-f]*\.?[0-9]+([eE][-+]?[0-9]+)?)', lambda m: fix_dot_zero(m), funcs)

    # js compiler

    if DEBUG: logging.debug('emscript: js compiler glue')

    # Settings changes
    assert settings['TARGET_ASMJS_UNKNOWN_EMSCRIPTEN'] == 1
    settings['TARGET_ASMJS_UNKNOWN_EMSCRIPTEN'] = 2
    i64_funcs = ['i64Add', 'i64Subtract', '__muldi3', '__divdi3', '__udivdi3', '__remdi3', '__uremdi3']
    for i64_func in i64_funcs:
      if i64_func in metadata['declares']:
        settings['PRECISE_I64_MATH'] = 2
        break

    metadata['declares'] = filter(lambda i64_func: i64_func not in ['getHigh32', 'setHigh32', '__muldi3', '__divdi3', '__remdi3', '__udivdi3', '__uremdi3'], metadata['declares']) # FIXME: do these one by one as normal js lib funcs

    # Integrate info from backend
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
                       [settings_file, ';', 'glue'] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                       cwd=path_from_root('src'), error_limit=300)
    assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
    glue, forwarded_data = out.split('//FORWARDED_DATA:')

    if DEBUG:
      logging.debug('  emscript: glue took %s seconds' % (time.time() - t))
      t = time.time()

    last_forwarded_json = forwarded_json = json.loads(forwarded_data)

    # merge in information from llvm backend

    last_forwarded_json['Functions']['tables'] = metadata['tables']

    '''indexed_functions = set()
    for key in forwarded_json['Functions']['indexedFunctions'].iterkeys():
      indexed_functions.add(key)'''

    pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

    #print >> sys.stderr, 'glue:', pre, '\n\n||||||||||||||||\n\n', post, '...............'

    # memory and global initializers

    global_initializers = ', '.join(map(lambda i: '{ func: function() { %s() } }' % i, metadata['initializers']))

    if settings['SIMD'] == 1:
      pre = open(path_from_root(os.path.join('src', 'ecmascript_simd.js'))).read() + '\n\n' + pre

    staticbump = mem_init.count(',')+1
    while staticbump % 16 != 0: staticbump += 1
    pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + %d;
  /* global initializers */ __ATINIT__.push(%s);
  %s''' % (staticbump, global_initializers, mem_init)) # XXX wrong size calculation!

    funcs_js = [funcs]
    parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
    if len(parts) > 1:
      pre = parts[0]
      funcs_js.append(parts[1])

    # merge forwarded data
    settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
    all_exported_functions = set(settings['EXPORTED_FUNCTIONS']) # both asm.js and otherwise
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
        if requested not in all_implemented and \
           requested != '_malloc': # special-case malloc, EXPORTED by default for internal use, but we bake in a trivial allocator and warn at runtime if used in ASSERTIONS
          logging.warning('function requested to be exported, but not implemented: "%s"', requested)

    # Add named globals
    named_globals = '\n'.join(['var %s = %s;' % (k, v) for k, v in metadata['namedGlobals'].iteritems()])
    pre = pre.replace('// === Body ===', '// === Body ===\n' + named_globals + '\n')

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
    if not settings['BOOTSTRAPPING_STRUCT_INFO']:
      funcs_js[1] = re.sub(r'/\* PRE_ASM \*/(.*)\n', lambda m: move_preasm(m), funcs_js[1])

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
    def make_func(name, code, params, coercions): return 'function %s(%s) { %s %s }' % (name, params, coercions, code)

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
            def make_emulated_param(i):
              if i >= len(sig): return shared.JS.make_initializer(proper_sig[i], settings) # extra param, just send a zero
              return shared.JS.make_coercion('p%d' % (i-1), proper_sig[i], settings, convert_from=sig[i])
            proper_code = proper_target + '(' + ','.join(map(lambda i: make_emulated_param(i+1), range(len(proper_sig)-1))) + ')'
            if proper_sig[0] != 'v':
              # proper sig has a return, which the wrapper may or may not use
              proper_code = shared.JS.make_coercion(proper_code, proper_sig[0], settings)
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
        if item not in implemented_functions:
          # this is imported into asm, we must wrap it
          call_ident = item
          if call_ident in metadata['redirects']: call_ident = metadata['redirects'][call_ident]
          if not call_ident.startswith('_') and not call_ident.startswith('Math_'): call_ident = '_' + call_ident
          code = call_ident + '(' + coerced_params + ')'
          if sig[0] != 'v':
            # ffis cannot return float
            if sig[0] == 'f': code = '+' + code
            code = 'return ' + shared.JS.make_coercion(code, sig[0], settings)
          code += ';'
          Counter.pre.append(make_func(item + '__wrapper', code, params, coercions))
          return item + '__wrapper'
        return item if not newline else (item + '\n')
      if settings['ASSERTIONS'] >= 2:
        debug_tables[sig] = body
      body = ','.join(map(fix_item, body))
      return ('\n'.join(Counter.pre), ''.join([raw[:start+1], body, raw[end:]]))

    infos = [make_table(sig, raw) for sig, raw in last_forwarded_json['Functions']['tables'].iteritems()]
    Counter.pre = []

    function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n\n// EMSCRIPTEN_END_FUNCS\n' + '\n'.join([info[1] for info in infos])

    asm_setup = ''
    maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul', 'min', 'clz32']]
    simdfloattypes = ['float32x4']
    simdinttypes = ['int32x4']
    simdtypes = simdfloattypes + simdinttypes
    simdfuncs = ['check', 'add', 'sub', 'neg', 'mul',
                 'equal', 'lessThan', 'greaterThan',
                 'notEqual', 'lessThanOrEqual', 'greaterThanOrEqual',
                 'select', 'and', 'or', 'xor', 'not',
                 'splat', 'swizzle', 'shuffle',
                 'withX', 'withY', 'withZ', 'withW',
                 'load', 'store', 'loadX', 'storeX', 'loadXY', 'storeXY', 'loadXYZ', 'storeXYZ']
    simdfloatfuncs = simdfuncs + ['div', 'min', 'max', 'minNum', 'maxNum', 'sqrt',
                                  'abs', 'fromInt32x4', 'fromInt32x4Bits',
                                  'reciprocalApproximation', 'reciprocalSqrtApproximation'];
    simdintfuncs = simdfuncs + ['fromFloat32x4', 'fromFloat32x4Bits',
                                'shiftRightArithmeticByScalar',
                                'shiftRightLogicalByScalar',
                                'shiftLeftByScalar'];
    fundamentals = ['Math', 'Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array', 'NaN', 'Infinity']
    if metadata['simd']:
        fundamentals += ['SIMD']
    if settings['ALLOW_MEMORY_GROWTH']: fundamentals.append('byteLength')
    math_envs = []

    provide_fround = settings['PRECISE_F32'] or settings['SIMD']

    if provide_fround: maths += ['Math.fround']

    basic_funcs = ['abort', 'assert'] + [m.replace('.', '_') for m in math_envs]
    if settings['RESERVED_FUNCTION_POINTERS'] > 0: basic_funcs.append('jsCall')
    if settings['SAFE_HEAP']: basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_STORE', 'SAFE_FT_MASK']
    if settings['CHECK_HEAP_ALIGN']: basic_funcs += ['CHECK_ALIGN_2', 'CHECK_ALIGN_4', 'CHECK_ALIGN_8']
    if settings['ASSERTIONS']:
      if settings['ASSERTIONS'] >= 2: import difflib
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_funcs += ['nullFunc_' + sig]
        if settings['ASSERTIONS'] <= 1:
          extra = ' Module["printErr"]("Build with ASSERTIONS=2 for more info.");'
          pointer = ' '
        else:
          pointer = ' \'" + x + "\' '
          asm_setup += '\nvar debug_table_' + sig + ' = ' + json.dumps(debug_tables[sig]) + ';'
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
        asm_setup += '\nfunction nullFunc_' + sig + '(x) { Module["printErr"]("Invalid function pointer' + pointer + 'called with signature \'' + sig + '\'. ' + \
                     'Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? ' + \
                     'Or calling a function with an incorrect type, which will fail? ' + \
                     '(it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)' + \
                     '"); ' + extra + ' abort(x) }\n'

    basic_vars = ['STACKTOP', 'STACK_MAX', 'tempDoublePtr', 'ABORT']
    basic_float_vars = []

    if metadata.get('preciseI64MathUsed'):
      basic_vars += ['cttz_i8']
    else:
      if forwarded_json['Functions']['libraryFunctions'].get('_llvm_cttz_i32'):
        basic_vars += ['cttz_i8']

    if settings.get('DLOPEN_SUPPORT'):
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_vars.append('F_BASE_%s' % sig)
        asm_setup += '  var F_BASE_%s = %s;\n' % (sig, 'FUNCTION_TABLE_OFFSET' if settings.get('SIDE_MODULE') else '0') + '\n'

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'setThrew', 'setTempRet0', 'getTempRet0']

    # See if we need ASYNCIFY functions
    # We might not need them even if ASYNCIFY is enabled
    need_asyncify = '_emscripten_alloc_async_context' in exported_implemented_functions
    if need_asyncify:
      basic_vars += ['___async', '___async_unwind', '___async_retval', '___async_cur_frame']
      asm_runtime_funcs += ['setAsync']

    if settings.get('EMTERPRETIFY'):
      asm_runtime_funcs += ['emterpret']
      if settings.get('EMTERPRETIFY_ASYNC'):
        asm_runtime_funcs += ['setAsyncState', 'emtStackSave']

    # function tables
    function_tables = ['dynCall_' + table for table in last_forwarded_json['Functions']['tables']]
    function_tables_impls = []

    for sig in last_forwarded_json['Functions']['tables'].iterkeys():
      args = ','.join(['a' + str(i) for i in range(1, len(sig))])
      arg_coercions = ' '.join(['a' + str(i) + '=' + shared.JS.make_coercion('a' + str(i), sig[i], settings) + ';' for i in range(1, len(sig))])
      coerced_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings) for i in range(1, len(sig))])
      ret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('FUNCTION_TABLE_%s[index&{{{ FTM_%s }}}](%s)' % (sig, sig, coerced_args), sig[0], settings)
      function_tables_impls.append('''
function dynCall_%s(index%s%s) {
  index = index|0;
  %s
  %s;
}
''' % (sig, ',' if len(sig) > 1 else '', args, arg_coercions, ret))

      ffi_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings, ffi_arg=True) for i in range(1, len(sig))])
      for i in range(settings['RESERVED_FUNCTION_POINTERS']):
        jsret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('jsCall(%d%s%s)' % (i, ',' if ffi_args else '', ffi_args), sig[0], settings, ffi_result=True)
        function_tables_impls.append('''
function jsCall_%s_%s(%s) {
  %s
  %s;
}

''' % (sig, i, args, arg_coercions, jsret))
      shared.Settings.copy(settings)
      asm_setup += '\n' + shared.JS.make_invoke(sig) + '\n'
      basic_funcs.append('invoke_%s' % sig)
      if settings.get('DLOPEN_SUPPORT'):
        asm_setup += '\n' + shared.JS.make_extcall(sig) + '\n'
        basic_funcs.append('extCall_%s' % sig)

    def quote(prop):
      if settings['CLOSURE_COMPILER'] == 2:
        return "'" + prop + "'"
      else:
        return prop

    def access_quote(prop):
      if settings['CLOSURE_COMPILER'] == 2:
        return "['" + prop + "']"
      else:
        return '.' + prop

    # calculate exports
    exported_implemented_functions = list(exported_implemented_functions) + metadata['initializers']
    exported_implemented_functions.append('runPostSets')
    if settings['ALLOW_MEMORY_GROWTH']:
      exported_implemented_functions.append('_emscripten_replace_memory')
    exports = []
    for export in exported_implemented_functions + asm_runtime_funcs + function_tables:
      exports.append(quote(export) + ": " + export)
    exports = '{ ' + ', '.join(exports) + ' }'
    # calculate globals
    try:
      del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
    except:
      pass
    # If no named globals, only need externals
    global_vars = metadata['externs'] #+ forwarded_json['Variables']['globals']
    global_funcs = list(set([key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]).difference(set(global_vars)).difference(implemented_functions))
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1]
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global' + access_quote(g) + ';\n' for g in maths]);
    asm_global_funcs += ''.join(['  var ' + g + '=env' + access_quote(math_fix(g)) + ';\n' for g in basic_funcs + global_funcs])
    if metadata['simd']:
      asm_global_funcs += ''.join(['  var SIMD_' + ty + '=global' + access_quote('SIMD') + access_quote(ty) + ';\n' for ty in simdtypes])
      asm_global_funcs += ''.join(['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in simdinttypes for g in simdintfuncs])
      asm_global_funcs += ''.join(['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in simdfloattypes for g in simdfloatfuncs])
    asm_global_vars = ''.join(['  var ' + g + '=env' + access_quote(g) + '|0;\n' for g in basic_vars + global_vars])
    # In linkable modules, we need to add some explicit globals for global variables that can be linked and used across modules
    if settings.get('MAIN_MODULE') or settings.get('SIDE_MODULE'):
      assert settings.get('TARGET_ASMJS_UNKNOWN_EMSCRIPTEN'), 'TODO: support x86 target when linking modules (needs offset of 4 and not 8 here)'
      for key, value in forwarded_json['Variables']['globals'].iteritems():
        if value.get('linkable'):
          init = forwarded_json['Variables']['indexedGlobals'][key] + 8 # 8 is Runtime.GLOBAL_BASE / STATIC_BASE
          if settings.get('SIDE_MODULE'): init = '(H_BASE+' + str(init) + ')|0'
          asm_global_vars += '  var %s=%s;\n' % (key, str(init))

    if settings['POINTER_MASKING']:
      for i in [0, 1, 2, 3]:
        if settings['POINTER_MASKING_DYNAMIC']:
          asm_global_vars += '  const MASK%d=env' % i + access_quote('MASK%d' % i) + '|0;\n';
          basic_vars += ['MASK%d' %i]
        else:
          asm_global_vars += '  const MASK%d=%d;\n' % (i, (settings['TOTAL_MEMORY']-1) & (~((2**i)-1)));

    # sent data
    the_global = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'
    sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in basic_funcs + global_funcs + basic_vars + basic_float_vars + global_vars]) + ' }'
    # received
    receiving = ''
    if settings['ASSERTIONS']:
      # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
      # some support code like malloc TODO: verify that malloc is actually safe to use that way
      receiving = '\n'.join(['var real_' + s + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_''' + s + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_malloc', '_free', '_memcpy', '_memset']])

    if not settings['SWAPPABLE_ASM_MODULE']:
      receiving += ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables])
    else:
      receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + s + ' = Module["' + s + '"] = function() { return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions + function_tables])

    if settings['EXPORT_FUNCTION_TABLES']:
      receiving += '\n'
      for table in last_forwarded_json['Functions']['tables'].values():
        tableName = table.split()[1]
        table = table.replace('var ' + tableName, 'var ' + tableName + ' = Module["' + tableName + '"]')
        receiving += table + '\n'

    # finalize

    if DEBUG: logging.debug('asm text sizes' + str([map(len, funcs_js), len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables), len('\n'.join(function_tables_impls)), len(function_tables_defs.replace('\n', '\n  ')), len(exports), len(the_global), len(sending), len(receiving)]))

    funcs_js = ['''
%s
Module%s = %s;
Module%s = %s;
// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
  %s
  %s
''' % (asm_setup,
       access_quote('asmGlobalArg'), the_global,
       access_quote('asmLibraryArg'), sending,
       "'use asm';" if not metadata.get('hasInlineJS') and not settings['SIDE_MODULE'] and settings['ASM_JS'] == 1 else "'almost asm';", '''
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
     access_quote('Float64Array')) if not settings['ALLOW_MEMORY_GROWTH'] else '''
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
''' % (access_quote('NaN'), access_quote('Infinity'))) + ''.join(['''
  var tempRet%d = 0;''' % i for i in range(10)]) + '\n' + asm_global_funcs] + \
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
'''] + \
  ['' if not settings['POINTER_MASKING'] or settings['POINTER_MASKING_DYNAMIC'] else '''
function _declare_heap_length() {
  return HEAP8[%s] | 0;
}
  ''' % (settings['TOTAL_MEMORY'] + settings['POINTER_MASKING_OVERFLOW'] - 1)] + ['''
// EMSCRIPTEN_START_FUNCS
function stackAlloc(size) {
  size = size|0;
  var ret = 0;
  ret = STACKTOP;
  STACKTOP = (STACKTOP + size)|0;
''' + ('STACKTOP = (STACKTOP + 3)&-4;' if settings['TARGET_X86'] else 'STACKTOP = (STACKTOP + 15)&-16;\n') +
      ('if ((STACKTOP|0) >= (STACK_MAX|0)) abort();\n' if settings['ASSERTIONS'] else '') + '''
  return ret|0;
}
function stackSave() {
  return STACKTOP|0;
}
function stackRestore(top) {
  top = top|0;
  STACKTOP = top;
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
''' if settings['EMTERPRETIFY_ASYNC'] else '') + '''
function setThrew(threw, value) {
  threw = threw|0;
  value = value|0;
  if ((__THREW__|0) == 0) {
    __THREW__ = threw;
    threwValue = value;
  }
}
function copyTempFloat(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr>>0] = HEAP8[ptr>>0];
  HEAP8[tempDoublePtr+1>>0] = HEAP8[ptr+1>>0];
  HEAP8[tempDoublePtr+2>>0] = HEAP8[ptr+2>>0];
  HEAP8[tempDoublePtr+3>>0] = HEAP8[ptr+3>>0];
}
function copyTempDouble(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr>>0] = HEAP8[ptr>>0];
  HEAP8[tempDoublePtr+1>>0] = HEAP8[ptr+1>>0];
  HEAP8[tempDoublePtr+2>>0] = HEAP8[ptr+2>>0];
  HEAP8[tempDoublePtr+3>>0] = HEAP8[ptr+3>>0];
  HEAP8[tempDoublePtr+4>>0] = HEAP8[ptr+4>>0];
  HEAP8[tempDoublePtr+5>>0] = HEAP8[ptr+5>>0];
  HEAP8[tempDoublePtr+6>>0] = HEAP8[ptr+6>>0];
  HEAP8[tempDoublePtr+7>>0] = HEAP8[ptr+7>>0];
}
function setTempRet0(value) {
  value = value|0;
  tempRet0 = value;
}
function getTempRet0() {
  return tempRet0|0;
}
'''] + funcs_js + ['''
  %s

  return %s;
})
// EMSCRIPTEN_END_ASM
(%s, %s, buffer);
%s;
''' % (pre_tables + '\n'.join(function_tables_impls) + '\n' + function_tables_defs, exports,
       'Module' + access_quote('asmGlobalArg'),
       'Module' + access_quote('asmLibraryArg'),
       receiving)]

    if not settings.get('SIDE_MODULE'):
      funcs_js.append('''
Runtime.stackAlloc = asm['stackAlloc'];
Runtime.stackSave = asm['stackSave'];
Runtime.stackRestore = asm['stackRestore'];
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
      return re.sub(r'{{{ FTM_([\w\d_$]+) }}}', lambda m: fix(m), js) # masks[m.groups(0)[0]]
    funcs_js = map(lambda js: function_table_maskize(js, masks), funcs_js)

    if settings.get('DLOPEN_SUPPORT'):
      funcs_js.append('''
  asm.maxFunctionIndex = %(max_mask)d;
  DLFCN.registerFunctions(asm, %(max_mask)d+1, %(sigs)s, Module);
  Module.SYMBOL_TABLE = SYMBOL_TABLE;
''' % { 'max_mask': max_mask, 'sigs': str(map(str, last_forwarded_json['Functions']['tables'].keys())) })

    # Create symbol table for self-dlopen
    if settings.get('DLOPEN_SUPPORT'):
      symbol_table = {}
      for k, v in forwarded_json['Variables']['indexedGlobals'].iteritems():
         if forwarded_json['Variables']['globals'][k]['named']:
           symbol_table[k] = str(v + forwarded_json['Runtime']['GLOBAL_BASE'])
      for raw in last_forwarded_json['Functions']['tables'].itervalues():
        if raw == '': continue
        table = map(string.strip, raw[raw.find('[')+1:raw.find(']')].split(","))
        for i in range(len(table)):
          value = table[i]
          if value != '0':
            if settings.get('SIDE_MODULE'):
              symbol_table[value] = 'FUNCTION_TABLE_OFFSET+' + str(i)
            else:
              symbol_table[value] = str(i)
      outfile.write("var SYMBOL_TABLE = %s;" % json.dumps(symbol_table).replace('"', ''))

    for i in range(len(funcs_js)): # do this loop carefully to save memory
      if WINDOWS: funcs_js[i] = funcs_js[i].replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
      outfile.write(funcs_js[i])
    funcs_js = None

    if WINDOWS: post = post.replace('\r\n', '\n') # Normalize to UNIX line endings, otherwise writing to text file will duplicate \r\n to \r\r\n!
    outfile.write(post)

    outfile.close()

    if DEBUG: logging.debug('  emscript: final python processing took %s seconds' % (time.time() - t))

    success = True

  finally:
    if not success:
      outfile.close()
      shared.try_delete(outfile.name) # remove partial output

if os.environ.get('EMCC_FAST_COMPILER') != '0':
  emscript = emscript_fast
else:
  logging.critical('Non-fastcomp compiler is no longer available, please use fastcomp or an older version of emscripten')
  sys.exit(1)

def main(args, compiler_engine, cache, jcache, relooper, temp_files, DEBUG, DEBUG_CACHE):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    settings[name] = json.loads(value)

  # libraries
  libraries = args.libraries[0].split(',') if len(args.libraries) > 0 else []

  # Compile the assembly to Javascript.
  if settings.get('RELOOP'):
    if not relooper:
      relooper = settings.get('RELOOPER')
      if not relooper:
        relooper = cache.get_path('relooper.js')
    settings.setdefault('RELOOPER', relooper)
    if not os.path.exists(relooper):
      shared.Building.ensure_relooper(relooper)

  settings.setdefault('STRUCT_INFO', cache.get_path('struct_info.compiled.json'))
  struct_info = settings.get('STRUCT_INFO')

  if not os.path.exists(struct_info) and not settings.get('BOOTSTRAPPING_STRUCT_INFO'):
    if DEBUG: logging.debug('  emscript: bootstrapping struct info...')
    shared.Building.ensure_struct_info(struct_info)
    if DEBUG: logging.debug('  emscript: bootstrapping struct info complete')

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
