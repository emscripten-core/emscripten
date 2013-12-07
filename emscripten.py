#!/usr/bin/env python2

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

import os, sys, json, optparse, subprocess, re, time, multiprocessing, string, logging, shutil

from tools import shared
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
    math_envs = ['Math.min'] # TODO: move min to maths
    asm_setup += '\n'.join(['var %s = %s;' % (f.replace('.', '_'), f) for f in math_envs])

    if settings['PRECISE_F32']: maths += ['Math.fround']

    basic_funcs = ['abort', 'assert', 'asmPrintInt', 'asmPrintFloat'] + [m.replace('.', '_') for m in math_envs]
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

    if settings.get('DLOPEN_SUPPORT'):
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_vars.append('F_BASE_%s' % sig)
        asm_setup += '  var F_BASE_%s = %s;\n' % (sig, 'FUNCTION_TABLE_OFFSET' if settings.get('SIDE_MODULE') else '0') + '\n'

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'setThrew'] + ['setTempRet%d' % i for i in range(10)]
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
    global_funcs = ['_' + key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1]
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global.' + g + ';\n' for g in maths]) + \
                       ''.join(['  var ' + g + '=env.' + math_fix(g) + ';\n' for g in basic_funcs + global_funcs])
    asm_global_vars = ''.join(['  var ' + g + '=env.' + g + '|0;\n' for g in basic_vars + global_vars]) + \
                      ''.join(['  var ' + g + '=+env.' + g + ';\n' for g in basic_float_vars])
    # In linkable modules, we need to add some explicit globals for global variables that can be linked and used across modules
    if settings.get('MAIN_MODULE') or settings.get('SIDE_MODULE'):
      assert settings.get('TARGET_LE32'), 'TODO: support x86 target when linking modules (needs offset of 4 and not 8 here)'
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
  var HEAP8 = new global.Int8Array(buffer);
  var HEAP16 = new global.Int16Array(buffer);
  var HEAP32 = new global.Int32Array(buffer);
  var HEAPU8 = new global.Uint8Array(buffer);
  var HEAPU16 = new global.Uint16Array(buffer);
  var HEAPU32 = new global.Uint32Array(buffer);
  var HEAPF32 = new global.Float32Array(buffer);
  var HEAPF64 = new global.Float64Array(buffer);
''' % (asm_setup, "'use asm';" if not forwarded_json['Types']['hasInlineJS'] and not settings['SIDE_MODULE'] and settings['ASM_JS'] == 1 else "'almost asm';") + '\n' + asm_global_vars + '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
''' + ''.join(['''
  var tempRet%d = 0;''' % i for i in range(10)]) + '\n' + asm_global_funcs] + ['  var tempFloat = %s;\n' % ('Math_fround(0)' if settings.get('PRECISE_F32') else '0.0')] + ['''
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
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1|0] = HEAP8[ptr+1|0];
  HEAP8[tempDoublePtr+2|0] = HEAP8[ptr+2|0];
  HEAP8[tempDoublePtr+3|0] = HEAP8[ptr+3|0];
}
function copyTempDouble(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1|0] = HEAP8[ptr+1|0];
  HEAP8[tempDoublePtr+2|0] = HEAP8[ptr+2|0];
  HEAP8[tempDoublePtr+3|0] = HEAP8[ptr+3|0];
  HEAP8[tempDoublePtr+4|0] = HEAP8[ptr+4|0];
  HEAP8[tempDoublePtr+5|0] = HEAP8[ptr+5|0];
  HEAP8[tempDoublePtr+6|0] = HEAP8[ptr+6|0];
  HEAP8[tempDoublePtr+7|0] = HEAP8[ptr+7|0];
}
''' + ''.join(['''
function setTempRet%d(value) {
  value = value|0;
  tempRet%d = value;
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
Runtime.stackAlloc = function(size) { return asm['stackAlloc'](size) };
Runtime.stackSave = function() { return asm['stackSave']() };
Runtime.stackRestore = function(top) { asm['stackRestore'](top) };
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
    outfile.write(funcs_js_item)
  funcs_js = None

  outfile.write(indexize(post))
  if DEBUG: logging.debug('  emscript: phase 3 took %s seconds' % (time.time() - t))

  outfile.close()

# emscript_fast: emscript'en code using the 'fast' compilation path, using
#                an LLVM backend
# FIXME: this is just a copy-paste of normal emscript(), and we trample it
#        if the proper env var is set (see below). we should refactor to
#        share code between the two, once emscript_fast stabilizes (or,
#        leaving it separate like it is will make it trivial to rip out
#        if the experiment fails)

def emscript_fast(infile, settings, outfile, libraries=[], compiler_engine=None,
             jcache=None, temp_files=None, DEBUG=None, DEBUG_CACHE=None):
  """Runs the emscripten LLVM-to-JS compiler. We parallelize as much as possible

  Args:
    infile: The path to the input LLVM assembly file.
    settings: JSON-formatted settings that override the values
      defined in src/settings.js.
    outfile: The file where the output is written.
  """

  assert(settings['ASM_JS']) # TODO: apply ASM_JS even in -O0 for fastcomp
  assert(settings['RUNNING_JS_OPTS'])

  # Overview:
  #   * Run LLVM backend to emit JS. JS includes function bodies, memory initializer,
  #     and various metadata
  #   * Run compiler.js on the metadata to emit the shell js code, pre/post-ambles,
  #     JS library dependencies, etc.

  if DEBUG: logging.debug('emscript: llvm backend')

  # TODO: proper temp files
  # TODO: use a single LLVM toolchain instead of normal for source, pnacl for simplification, custom for js backend

  if DEBUG: shutil.copyfile(infile, os.path.join(shared.CANONICAL_TEMP_DIR, 'temp0.ll'))

  if DEBUG: logging.debug('  ..1..')
  temp1 = temp_files.get('.1.bc').name
  shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'opt'), infile, '-pnacl-abi-simplify-preopt', '-o', temp1]))
  assert os.path.exists(temp1)
  if DEBUG:
    shutil.copyfile(temp1, os.path.join(shared.CANONICAL_TEMP_DIR, 'temp1.bc'))
    shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'llvm-dis'), 'temp1.bc', '-o', 'temp1.ll']))

  #if DEBUG: logging.debug('  ..2..')
  #temp2 = temp_files.get('.2.bc').name
  #shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'opt'), temp1, '-O3', '-o', temp2]))
  #assert os.path.exists(temp2)
  #if DEBUG:
  #  shutil.copyfile(temp2, os.path.join(shared.CANONICAL_TEMP_DIR, 'temp2.bc'))
  #  shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'llvm-dis'), 'temp2.bc', '-o', 'temp2.ll']))
  temp2 = temp1 # XXX if we optimize the bc, we remove some pnacl clutter, but it also makes varargs stores be 8-byte aligned

  if DEBUG: logging.debug('  ..3..')
  temp3 = temp_files.get('.3.bc').name
  shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'opt'), temp2, '-pnacl-abi-simplify-postopt', '-o', temp3]))
  assert os.path.exists(temp3)
  if DEBUG:
    shutil.copyfile(temp3, os.path.join(shared.CANONICAL_TEMP_DIR, 'temp3.bc'))
    shared.jsrun.timeout_run(subprocess.Popen([os.path.join(shared.LLVM_ROOT, 'llvm-dis'), 'temp3.bc', '-o', 'temp3.ll']))

  if DEBUG: logging.debug('  ..4..')
  temp4 = temp_files.get('.4.js').name
  backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
  shared.jsrun.timeout_run(subprocess.Popen([backend_compiler, temp3, '-march=js', '-filetype=asm', '-o', temp4], stdout=subprocess.PIPE))
  if DEBUG: shutil.copyfile(temp4, os.path.join(shared.CANONICAL_TEMP_DIR, 'temp4.js'))

  # Split up output
  backend_output = open(temp4).read()
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

  # function table masks

  table_sizes = {}
  for k, v in metadata['tables'].iteritems():
    table_sizes[k] = str(v.count(',')) # undercounts by one, but that is what we want
  funcs = re.sub(r"#FM_(\w+)#", lambda m: table_sizes[m.groups(0)[0]], funcs)

  # js compiler

  if DEBUG: logging.debug('emscript: js compiler glue')

  # Settings changes
  assert settings['TARGET_LE32'] == 1
  settings['TARGET_LE32'] = 2
  if 'i64Add' in metadata['declares']: # TODO: others, once we split them up
    settings['PRECISE_I64_MATH'] = 2
    metadata['declares'] = filter(lambda i64_func: i64_func not in ['getHigh32', 'setHigh32', '__muldi3', '__divdi3', '__remdi3', '__udivdi3', '__uremdi3'], metadata['declares']) # FIXME: do these one by one as normal js lib funcs

  # Integrate info from backend
  settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = list(
    set(settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] + map(shared.JS.to_nice_ident, metadata['declares'])).difference(
      map(lambda x: x[1:], metadata['implementedFunctions'])
    )
  ) + map(lambda x: x[1:], metadata['externs'])

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
  out = jsrun.run_js(path_from_root('src', 'compiler.js'), compiler_engine, [settings_file, ';', 'glue'] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                     cwd=path_from_root('src'))
  assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
  glue, forwarded_data = out.split('//FORWARDED_DATA:')

  #print >> sys.stderr, out

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

  pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + Runtime.alignMemory(%d);
/* global initializers */ __ATINIT__.push(%s);
%s''' % (mem_init.count(',')+1, global_initializers, mem_init)) # XXX wrong size calculation!

  funcs_js = [funcs]
  if settings.get('ASM_JS'):
    parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
    if len(parts) > 1:
      pre = parts[0]
      funcs_js.append(parts[1])

  # calculations on merged forwarded data TODO

  # merge forwarded data
  assert settings.get('ASM_JS'), 'fastcomp is asm.js only'
  settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']
  all_exported_functions = set(settings['EXPORTED_FUNCTIONS']) # both asm.js and otherwise
  for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
    all_exported_functions.add('_' + additional_export)
  exported_implemented_functions = set()
  export_bindings = settings['EXPORT_BINDINGS']
  export_all = settings['EXPORT_ALL']
  for key in metadata['implementedFunctions'] + forwarded_json['Functions']['implementedFunctions'].keys(): # XXX perf
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      exported_implemented_functions.add(key)

  #if DEBUG: outfile.write('// pre\n')
  outfile.write(pre)
  pre = None

  #if DEBUG: outfile.write('// funcs\n')

  if settings.get('ASM_JS'):
    # Move preAsms to their right place
    def move_preasm(m):
      contents = m.groups(0)[0]
      outfile.write(contents + '\n')
      return ''
    funcs_js[1] = re.sub(r'/\* PRE_ASM \*/(.*)\n', lambda m: move_preasm(m), funcs_js[1])

    funcs_js += ['\n// EMSCRIPTEN_END_FUNCS\n']

    simple = os.environ.get('EMCC_SIMPLE_ASM')
    class Counter:
      i = 0
      j = 0
    if 'pre' in last_forwarded_json['Functions']['tables']:
      pre_tables = last_forwarded_json['Functions']['tables']['pre']
      del last_forwarded_json['Functions']['tables']['pre']
    else:
      pre_tables = ''

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
    math_envs = ['Math.min'] # TODO: move min to maths
    asm_setup += '\n'.join(['var %s = %s;' % (f.replace('.', '_'), f) for f in math_envs])

    if settings['PRECISE_F32']: maths += ['Math.fround']

    basic_funcs = ['abort', 'assert', 'asmPrintInt', 'asmPrintFloat'] + [m.replace('.', '_') for m in math_envs]
    if settings['RESERVED_FUNCTION_POINTERS'] > 0: basic_funcs.append('jsCall')
    if settings['SAFE_HEAP']: basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_STORE', 'SAFE_HEAP_CLEAR']
    if settings['CHECK_HEAP_ALIGN']: basic_funcs += ['CHECK_ALIGN_2', 'CHECK_ALIGN_4', 'CHECK_ALIGN_8']
    if settings['ASSERTIONS']:
      basic_funcs += ['nullFunc']
      asm_setup += 'function nullFunc(x) { Module["printErr"]("Invalid function pointer called. Perhaps a miscast function pointer (check compilation warnings) or bad vtable lookup (maybe due to derefing a bad pointer, like NULL)?"); abort(x) }\n'

    basic_vars = ['STACKTOP', 'STACK_MAX', 'tempDoublePtr', 'ABORT']
    basic_float_vars = ['NaN', 'Infinity']

    if metadata.get('preciseI64MathUsed') or \
       forwarded_json['Functions']['libraryFunctions'].get('llvm_cttz_i32') or \
       forwarded_json['Functions']['libraryFunctions'].get('llvm_ctlz_i32'):
      basic_vars += ['cttz_i8', 'ctlz_i8']

    if settings.get('DLOPEN_SUPPORT'):
      for sig in last_forwarded_json['Functions']['tables'].iterkeys():
        basic_vars.append('F_BASE_%s' % sig)
        asm_setup += '  var F_BASE_%s = %s;\n' % (sig, 'FUNCTION_TABLE_OFFSET' if settings.get('SIDE_MODULE') else '0') + '\n'

    asm_runtime_funcs = ['stackAlloc', 'stackSave', 'stackRestore', 'setThrew'] + ['setTempRet%d' % i for i in range(10)]
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
    exported_implemented_functions = list(exported_implemented_functions) + metadata['initializers']
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
    global_vars = metadata['externs'] #+ forwarded_json['Variables']['globals']
    global_funcs = list(set(['_' + key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]).difference(set(global_vars))) #  + metadata['externFuncs']/'declares'
    def math_fix(g):
      return g if not g.startswith('Math_') else g.split('_')[1]
    asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global.' + g + ';\n' for g in maths]) + \
                       ''.join(['  var ' + g + '=env.' + math_fix(g) + ';\n' for g in basic_funcs + global_funcs])
    asm_global_vars = ''.join(['  var ' + g + '=env.' + g + '|0;\n' for g in basic_vars + global_vars])
    # In linkable modules, we need to add some explicit globals for global variables that can be linked and used across modules
    if settings.get('MAIN_MODULE') or settings.get('SIDE_MODULE'):
      assert settings.get('TARGET_LE32'), 'TODO: support x86 target when linking modules (needs offset of 4 and not 8 here)'
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
  var HEAP8 = new global.Int8Array(buffer);
  var HEAP16 = new global.Int16Array(buffer);
  var HEAP32 = new global.Int32Array(buffer);
  var HEAPU8 = new global.Uint8Array(buffer);
  var HEAPU16 = new global.Uint16Array(buffer);
  var HEAPU32 = new global.Uint32Array(buffer);
  var HEAPF32 = new global.Float32Array(buffer);
  var HEAPF64 = new global.Float64Array(buffer);
''' % (asm_setup, "'use asm';" if not metadata.get('hasInlineJS') and not settings['SIDE_MODULE'] and settings['ASM_JS'] == 1 else "'almost asm';") + '\n' + asm_global_vars + '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var nan = +env.NaN, inf = +env.Infinity;
  var tempInt = 0, tempBigInt = 0, tempBigIntP = 0, tempBigIntS = 0, tempBigIntR = 0.0, tempBigIntI = 0, tempBigIntD = 0, tempValue = 0, tempDouble = 0.0;
''' + ''.join(['''
  var tempRet%d = 0;''' % i for i in range(10)]) + '\n' + asm_global_funcs] + ['  var tempFloat = %s;\n' % ('Math_fround(0)' if settings.get('PRECISE_F32') else '0.0')] + ['''
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
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1|0] = HEAP8[ptr+1|0];
  HEAP8[tempDoublePtr+2|0] = HEAP8[ptr+2|0];
  HEAP8[tempDoublePtr+3|0] = HEAP8[ptr+3|0];
}
function copyTempDouble(ptr) {
  ptr = ptr|0;
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1|0] = HEAP8[ptr+1|0];
  HEAP8[tempDoublePtr+2|0] = HEAP8[ptr+2|0];
  HEAP8[tempDoublePtr+3|0] = HEAP8[ptr+3|0];
  HEAP8[tempDoublePtr+4|0] = HEAP8[ptr+4|0];
  HEAP8[tempDoublePtr+5|0] = HEAP8[ptr+5|0];
  HEAP8[tempDoublePtr+6|0] = HEAP8[ptr+6|0];
  HEAP8[tempDoublePtr+7|0] = HEAP8[ptr+7|0];
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
''' % (pre_tables + '\n'.join(function_tables_impls) + '\n' + function_tables_defs.replace('\n', '\n  '), exports, the_global, sending, receiving)]

    if not settings.get('SIDE_MODULE'):
      funcs_js.append('''
Runtime.stackAlloc = function(size) { return asm['stackAlloc'](size) };
Runtime.stackSave = function() { return asm['stackSave']() };
Runtime.stackRestore = function(top) { asm['stackRestore'](top) };
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
    outfile.write(funcs_js[i])
  funcs_js = None

  outfile.write(post)

  outfile.close()

if os.environ.get('EMCC_FAST_COMPILER'):
  emscript = emscript_fast

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
      relooper = cache.get_path('relooper.js')
    settings.setdefault('RELOOPER', relooper)
    if not os.path.exists(relooper):
      shared.Building.ensure_relooper(relooper)
  
  settings.setdefault('STRUCT_INFO', cache.get_path('struct_info.compiled.json'))
  struct_info = settings.get('STRUCT_INFO')
  
  if not os.path.exists(struct_info):
    shared.Building.ensure_struct_info(struct_info)
  
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
