#!/usr/bin/env python2

'''
You should normally never use this! Use emcc instead.

This is a small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
'''

from tools.toolchain_profiler import ToolchainProfiler
if __name__ == '__main__':
  ToolchainProfiler.record_process_start()

import difflib
import os, sys, json, optparse, subprocess, re, time, logging
import shutil

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

    with ToolchainProfiler.profile_block('get_and_parse_backend'):
      backend_output = compile_js(infile, settings, temp_files, DEBUG)
      funcs, metadata, mem_init = parse_backend_output(backend_output, DEBUG)
      fixup_metadata_tables(metadata, settings)
      funcs = fixup_functions(funcs, metadata, settings)
    with ToolchainProfiler.profile_block('compiler_glue'):
      glue, forwarded_data = compiler_glue(metadata, settings, libraries, compiler_engine, temp_files, DEBUG)

    with ToolchainProfiler.profile_block('function_tables_and_exports'):
      (post, function_table_data, bundled_args) = (
          function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, settings, outfile, DEBUG))
    with ToolchainProfiler.profile_block('write_output_file'):
      finalize_output(outfile, post, function_table_data, bundled_args, metadata, settings, DEBUG)
    success = True

  finally:
    outfile.close()
    if not success:
      shared.try_delete(outfile.name) # remove partial output


def compile_js(infile, settings, temp_files, DEBUG):
  """Compile infile with asm.js backend, return the contents of the compiled js"""
  with temp_files.get_file('.4.js') as temp_js:
    backend_args = create_backend_args(infile, temp_js, settings)

    if DEBUG:
      logging.debug('emscript: llvm backend: ' + ' '.join(backend_args))
      t = time.time()
    with ToolchainProfiler.profile_block('emscript_llvm_backend'):
      shared.jsrun.timeout_run(subprocess.Popen(backend_args, stdout=subprocess.PIPE), note_args=backend_args)
    if DEBUG:
      logging.debug('  emscript: llvm backend took %s seconds' % (time.time() - t))

    # Split up output
    backend_output = open(temp_js).read()
    #if DEBUG: print >> sys.stderr, backend_output
  return backend_output


def parse_backend_output(backend_output, DEBUG):
  start_funcs_marker = '// EMSCRIPTEN_START_FUNCTIONS'
  end_funcs_marker = '// EMSCRIPTEN_END_FUNCTIONS'
  metadata_split_marker = '// EMSCRIPTEN_METADATA'

  start_funcs = backend_output.index(start_funcs_marker)
  end_funcs = backend_output.rindex(end_funcs_marker)
  metadata_split = backend_output.rindex(metadata_split_marker)

  funcs = backend_output[start_funcs+len(start_funcs_marker):end_funcs]
  metadata_raw = backend_output[metadata_split+len(metadata_split_marker):]
  mem_init = backend_output[end_funcs+len(end_funcs_marker):metadata_split]

  try:
    #if DEBUG: print >> sys.stderr, "METAraw", metadata_raw
    metadata = json.loads(metadata_raw)
  except Exception, e:
    logging.error('emscript: failure to parse metadata output from compiler backend. raw output is: \n' + metadata_raw)
    raise e

  #if DEBUG: print >> sys.stderr, "FUNCS", funcs
  #if DEBUG: print >> sys.stderr, "META", metadata
  #if DEBUG: print >> sys.stderr, "meminit", mem_init
  return funcs, metadata, mem_init


def fixup_metadata_tables(metadata, settings):
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


def fixup_functions(funcs, metadata, settings):
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

  return funcs


def compiler_glue(metadata, settings, libraries, compiler_engine, temp_files, DEBUG):
  if DEBUG:
    logging.debug('emscript: js compiler glue')
    t = time.time()

  # Settings changes
  i64_funcs = ['i64Add', 'i64Subtract', '__muldi3', '__divdi3', '__udivdi3', '__remdi3', '__uremdi3']
  for i64_func in i64_funcs:
    if i64_func in metadata['declares']:
      settings['PRECISE_I64_MATH'] = 2
      break

  # FIXME: do these one by one as normal js lib funcs
  metadata['declares'] = filter(lambda i64_func: i64_func not in ['getHigh32', 'setHigh32'], metadata['declares'])

  optimize_syscalls(metadata['declares'], settings, DEBUG)
  update_settings_glue(settings, metadata)
  assert not (metadata['simd'] and settings['SPLIT_MEMORY']), 'SIMD is used, but not supported in SPLIT_MEMORY'

  assert not (metadata['simd'] and settings['WASM']), 'SIMD is used, but not supported in WASM mode yet'
  assert not (settings['SIMD'] and settings['WASM']), 'SIMD is requested, but not supported in WASM mode yet'

  glue, forwarded_data = compile_settings(compiler_engine, settings, libraries, temp_files)

  if DEBUG:
    logging.debug('  emscript: glue took %s seconds' % (time.time() - t))

  return glue, forwarded_data


def function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, settings, outfile, DEBUG):
  if DEBUG:
    logging.debug('emscript: python processing: function tables and exports')
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  # merge in information from llvm backend

  function_table_data = metadata['tables']

  # merge forwarded data
  settings['EXPORTED_FUNCTIONS'] = forwarded_json['EXPORTED_FUNCTIONS']

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  #print >> sys.stderr, 'glue:', pre, '\n\n||||||||||||||||\n\n', post, '...............'

  pre = memory_and_global_initializers(pre, metadata, mem_init, settings)
  pre, funcs_js = get_js_funcs(pre, funcs)
  all_exported_functions = get_all_exported_functions(function_table_data, settings)
  all_implemented = get_all_implemented(forwarded_json, metadata)
  check_all_implemented(all_implemented, pre, settings)
  implemented_functions = get_implemented_functions(metadata)
  pre = include_asm_consts(pre, forwarded_json, metadata, settings)
  #if DEBUG: outfile.write('// pre\n')
  outfile.write(pre)
  pre = None

  #if DEBUG: outfile.write('// funcs\n')

  # Move preAsms to their right place
  def move_preasm(m):
    contents = m.groups(0)[0]
    outfile.write(contents + '\n')
    return ''
  if not settings['BOOTSTRAPPING_STRUCT_INFO'] and len(funcs_js) > 1:
    funcs_js[1] = re.sub(r'/\* PRE_ASM \*/(.*)\n', move_preasm, funcs_js[1])

  if 'pre' in function_table_data:
    pre_tables = function_table_data['pre']
    del function_table_data['pre']
  else:
    pre_tables = ''

  function_table_sigs = function_table_data.keys()

  in_table, debug_tables, function_tables_defs = make_function_tables_defs(
    implemented_functions, all_implemented, function_table_data, settings, metadata)

  exported_implemented_functions = get_exported_implemented_functions(
    all_exported_functions, all_implemented, metadata, settings)

  asm_setup = create_asm_setup(debug_tables, function_table_data, metadata, settings)
  basic_funcs = create_basic_funcs(function_table_sigs, settings)
  basic_vars = create_basic_vars(exported_implemented_functions, forwarded_json, metadata, settings)

  shared.Settings.copy(settings)

  funcs_js += create_mftCall_funcs(function_table_data, settings)

  exports = create_exports(exported_implemented_functions, in_table, function_table_data, metadata, settings)

  # calculate globals
  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except:
    pass
  if not settings['RELOCATABLE']:
    global_vars = metadata['externs']
  else:
    global_vars = [] # linkable code accesses globals through function calls
  global_funcs = list(set([key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2])
                      .difference(set(global_vars)).difference(implemented_functions))
  if settings['RELOCATABLE']:
    global_funcs += ['g$' + extern for extern in metadata['externs']]

  bg_funcs = basic_funcs + global_funcs
  bg_vars = basic_vars + global_vars
  asm_global_funcs= create_asm_global_funcs(bg_funcs, metadata, settings)
  asm_global_vars = create_asm_global_vars(bg_vars, settings)

  the_global = create_the_global(metadata, settings)
  sending_vars = basic_funcs + global_funcs + basic_vars + global_vars
  sending = '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in sending_vars]) + ' }'

  receiving = create_receiving(function_table_data, function_tables_defs,
                               exported_implemented_functions, settings)

  function_tables_impls = make_function_tables_impls(function_table_data, settings)
  final_function_tables = '\n'.join(function_tables_impls) + '\n' + function_tables_defs
  if settings.get('EMULATED_FUNCTION_POINTERS'):
    final_function_tables = (
      final_function_tables
      .replace("asm['", '')
      .replace("']", '')
      .replace('var SIDE_FUNCTION_TABLE_', 'var FUNCTION_TABLE_')
      .replace('var dynCall_', '//')
    )

  if DEBUG:
    logging.debug('asm text sizes' + str([
      map(len, funcs_js), len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables),
      len('\n'.join(function_tables_impls)), len(function_tables_defs) + (function_tables_defs.count('\n') * len('  ')),
      len(exports), len(the_global), len(sending), len(receiving)]))
    logging.debug('  emscript: python processing: function tables and exports took %s seconds' % (time.time() - t))

  bundled_args = (funcs_js, asm_setup, the_global, sending, receiving, asm_global_vars,
                  asm_global_funcs, pre_tables, final_function_tables, exports)
  return (post, function_table_data, bundled_args)


def finalize_output(outfile, post, function_table_data, bundled_args, metadata, settings, DEBUG):
  function_table_sigs = function_table_data.keys()
  module = create_module(function_table_sigs, metadata, settings, *bundled_args)

  if DEBUG:
    logging.debug('emscript: python processing: finalize')
    t = time.time()

  write_output_file(outfile, post, module)
  module = None

  if DEBUG:
    logging.debug('  emscript: python processing: finalize took %s seconds' % (time.time() - t))

  write_cyberdwarf_data(outfile, metadata, settings)


def create_module(function_table_sigs, metadata, settings,
                  funcs_js, asm_setup, the_global, sending, receiving, asm_global_vars,
                  asm_global_funcs, pre_tables, final_function_tables, exports):
  receiving += create_named_globals(metadata, settings)
  runtime_funcs = create_runtime_funcs(exports, settings)

  asm_start_pre = create_asm_start_pre(asm_setup, the_global, sending, metadata, settings)
  asm_temp_vars = create_asm_temp_vars(settings)
  asm_runtime_thread_local_vars = create_asm_runtime_thread_local_vars(settings)
  asm_start = asm_start_pre + '\n' + asm_global_vars + asm_temp_vars + asm_runtime_thread_local_vars + '\n' + asm_global_funcs

  temp_float = '  var tempFloat = %s;\n' % ('Math_fround(0)' if provide_fround(settings) else '0.0')
  async_state = '  var asyncState = 0;\n' if settings.get('EMTERPRETIFY_ASYNC') else ''
  f0_fround = '  const f0 = Math_fround(0);\n' if provide_fround(settings) else ''

  replace_memory = create_replace_memory(settings)

  start_funcs_marker = '\n// EMSCRIPTEN_START_FUNCS\n'

  asm_end = create_asm_end(exports, settings)

  runtime_library_overrides = create_runtime_library_overrides(settings)

  module = [
    asm_start,
    temp_float,
    async_state,
    f0_fround,
    replace_memory,
    start_funcs_marker
  ] + runtime_funcs + funcs_js + ['\n  ',
    pre_tables, final_function_tables, asm_end,
    '\n', receiving, ';\n', runtime_library_overrides]

  if settings['SIDE_MODULE']:
    module.append('''
Runtime.registerFunctions(%(sigs)s, Module);
''' % { 'sigs': str(map(str, function_table_sigs)) })

  return module


def write_output_file(outfile, post, module):
  for i in range(len(module)): # do this loop carefully to save memory
    module[i] = normalize_line_endings(module[i])
    outfile.write(module[i])

  post = normalize_line_endings(post)
  outfile.write(post)


def write_cyberdwarf_data(outfile, metadata, settings):
  if settings['CYBERDWARF']:
    assert('cyberdwarf_data' in metadata)
    cd_file_name = outfile.name + ".cd"
    with open(cd_file_name, "w") as cd_file:
      json.dump({ 'cyberdwarf': metadata['cyberdwarf_data'] }, cd_file)


def create_backend_args(infile, temp_js, settings):
  """Create args for asm.js backend from settings dict"""
  backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
  args = [
    backend_compiler, infile, '-march=js', '-filetype=asm', '-o', temp_js,
    '-emscripten-stack-size=%d' % settings['TOTAL_STACK'],
    '-O' + str(settings['OPT_LEVEL']),
  ]
  if settings['PRECISE_F32']:
    args += ['-emscripten-precise-f32']
  if settings['USE_PTHREADS']:
    args += ['-emscripten-enable-pthreads']
  if settings['WARN_UNALIGNED']:
    args += ['-emscripten-warn-unaligned']
  if settings['RESERVED_FUNCTION_POINTERS'] > 0:
    args += ['-emscripten-reserved-function-pointers=%d' % settings['RESERVED_FUNCTION_POINTERS']]
  if settings['ASSERTIONS'] > 0:
    args += ['-emscripten-assertions=%d' % settings['ASSERTIONS']]
  if settings['ALIASING_FUNCTION_POINTERS'] == 0:
    args += ['-emscripten-no-aliasing-function-pointers']
  if settings['EMULATED_FUNCTION_POINTERS']:
    args += ['-emscripten-emulated-function-pointers']
  if settings['RELOCATABLE']:
    args += ['-emscripten-relocatable']
    args += ['-emscripten-global-base=0']
  elif settings['GLOBAL_BASE'] >= 0:
    args += ['-emscripten-global-base=%d' % settings['GLOBAL_BASE']]
  if settings['SIDE_MODULE']:
    args += ['-emscripten-side-module']
  if settings['LEGALIZE_JS_FFI'] != 1:
    args += ['-emscripten-legalize-javascript-ffi=0']
  if settings['DISABLE_EXCEPTION_CATCHING'] != 1:
    args += ['-enable-emscripten-cpp-exceptions']
    if settings['DISABLE_EXCEPTION_CATCHING'] == 2:
      args += ['-emscripten-cpp-exceptions-whitelist=' + ','.join(settings['EXCEPTION_CATCHING_WHITELIST'] or ['fake'])]
  if settings['ASYNCIFY']:
    args += ['-emscripten-asyncify']
    args += ['-emscripten-asyncify-functions=' + ','.join(settings['ASYNCIFY_FUNCTIONS'])]
    args += ['-emscripten-asyncify-whitelist=' + ','.join(settings['ASYNCIFY_WHITELIST'])]
  if settings['NO_EXIT_RUNTIME']:
    args += ['-emscripten-no-exit-runtime']
  if settings['BINARYEN']:
    args += ['-emscripten-wasm']
    if shared.Building.is_wasm_only():
      args += ['-emscripten-only-wasm']
  if settings['CYBERDWARF']:
    args += ['-enable-cyberdwarf']
  return args


def optimize_syscalls(declares, settings, DEBUG):
  """Disables filesystem if only a limited subset of syscalls is used.

  Our syscalls are static, and so if we see a very limited set of them - in particular,
  no open() syscall and just simple writing - then we don't need full filesystem support.
  If FORCE_FILESYSTEM is set, we can't do this. We also don't do it if INCLUDE_FULL_LIBRARY, since
  not including the filesystem would mean not including the full JS libraries, and the same for
  MAIN_MODULE since a side module might need the filesystem.
  """
  relevant_settings = ['NO_FILESYSTEM', 'FORCE_FILESYSTEM', 'INCLUDE_FULL_LIBRARY', 'MAIN_MODULE']
  if all([not settings[s] for s in relevant_settings]):
    syscall_prefix = '__syscall'
    syscall_numbers = [d[len(syscall_prefix):] for d in declares if d.startswith(syscall_prefix)]
    syscalls = [int(s) for s in syscall_numbers if is_int(s)]
    if set(syscalls).issubset(set([6, 54, 140, 146])): # close, ioctl, llseek, writev
      if DEBUG: logging.debug('very limited syscalls (%s) so disabling full filesystem support' % ', '.join(map(str, syscalls)))
      settings['NO_FILESYSTEM'] = 1


def is_int(x):
  try:
    int(x)
    return True
  except:
    return False


def update_settings_glue(settings, metadata):
  if settings['CYBERDWARF']:
    settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'].append("cyberdwarf_Debugger")
    settings['EXPORTED_FUNCTIONS'].append("cyberdwarf_Debugger")

  # Integrate info from backend
  if settings['SIDE_MODULE']:
    settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = [] # we don't need any JS library contents in side modules

  if metadata.get('cantValidate') and settings['ASM_JS'] != 2:
    logging.warning('disabling asm.js validation due to use of non-supported features: ' + metadata['cantValidate'])
    settings['ASM_JS'] = 2

  settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] = list(
    set(settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE'] + map(shared.JS.to_nice_ident, metadata['declares'])).difference(
      map(lambda x: x[1:], metadata['implementedFunctions'])
    )
  ) + map(lambda x: x[1:], metadata['externs'])

  if metadata['simd']:
    settings['SIMD'] = 1

  settings['MAX_GLOBAL_ALIGN'] = metadata['maxGlobalAlign']
  settings['IMPLEMENTED_FUNCTIONS'] = metadata['implementedFunctions']


def compile_settings(compiler_engine, settings, libraries, temp_files):
  # Save settings to a file to work around v8 issue 1579
  with temp_files.get_file('.txt') as settings_file:
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
  return glue, forwarded_data


def memory_and_global_initializers(pre, metadata, mem_init, settings):
  global_initializers = str(', '.join(map(lambda i: '{ func: function() { %s() } }' % i, metadata['initializers'])))

  if settings['SIMD'] == 1:
    pre = open(path_from_root(os.path.join('src', 'ecmascript_simd.js'))).read() + '\n\n' + pre

  staticbump = metadata['staticBump']
  while staticbump % 16 != 0: staticbump += 1
  split_memory = ''
  if settings['SPLIT_MEMORY']:
    split_memory = ('assert(STATICTOP < SPLIT_MEMORY, "SPLIT_MEMORY size must be big enough so the '
                    'entire static memory, need " + STATICTOP);')
  pthread = ''
  if settings['USE_PTHREADS']:
    pthread = 'if (!ENVIRONMENT_IS_PTHREAD)'
  pre = pre.replace('STATICTOP = STATIC_BASE + 0;',
    '''STATICTOP = STATIC_BASE + {staticbump};{split_memory}
/* global initializers */ {pthread} __ATINIT__.push({global_initializers});
{mem_init}'''.format(staticbump=staticbump,
                     split_memory=split_memory,
                     pthread=pthread,
                     global_initializers=global_initializers,
                     mem_init=mem_init))

  if settings['SIDE_MODULE']:
    pre = pre.replace('Runtime.GLOBAL_BASE', 'gb')
  if settings['SIDE_MODULE'] or settings['BINARYEN']:
    pre = pre.replace('{{{ STATIC_BUMP }}}', str(staticbump))

  return pre


def get_js_funcs(pre, funcs):
  funcs_js = [funcs]
  parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
  if len(parts) > 1:
    pre = parts[0]
    funcs_js.append(parts[1])
  return pre, funcs_js


def get_all_exported_functions(function_table_data, settings):
  all_exported_functions = set(shared.expand_response(settings['EXPORTED_FUNCTIONS'])) # both asm.js and otherwise

  for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
    all_exported_functions.add('_' + additional_export)
  if settings['EXPORT_FUNCTION_TABLES']:
    for table in function_table_data.values():
      for func in table.split('[')[1].split(']')[0].split(','):
        if func[0] == '_':
          all_exported_functions.add(func)
  return all_exported_functions


def get_all_implemented(forwarded_json, metadata):
  return metadata['implementedFunctions'] + forwarded_json['Functions']['implementedFunctions'].keys() # XXX perf?


def check_all_implemented(all_implemented, pre, settings):
  if settings['ASSERTIONS'] and settings.get('ORIGINAL_EXPORTED_FUNCTIONS'):
    original_exports = settings['ORIGINAL_EXPORTED_FUNCTIONS']
    if original_exports[0] == '@':
      original_exports = json.loads(open(original_exports[1:]).read())
    for requested in original_exports:
      if not is_already_implemented(requested, pre, all_implemented):
        # could be a js library func
        logging.warning('function requested to be exported, but not implemented: "%s"', requested)


def is_already_implemented(requested, pre, all_implemented):
  is_implemented = requested in all_implemented
  # special-case malloc, EXPORTED by default for internal use, but we bake in a trivial allocator and warn at runtime if used in ASSERTIONS
  is_exception = requested == '_malloc'
  in_pre = ('function ' + requested.encode('utf-8')) in pre
  return is_implemented or is_exception or in_pre

def get_exported_implemented_functions(all_exported_functions, all_implemented, metadata, settings):
  funcs = set(metadata['exports'])
  export_bindings = settings['EXPORT_BINDINGS']
  export_all = settings['EXPORT_ALL']
  for key in all_implemented:
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      funcs.add(key)

  funcs = list(funcs) + metadata['initializers']
  if not settings['ONLY_MY_CODE']:
    funcs.append('runPostSets')
    if settings['ALLOW_MEMORY_GROWTH']:
      funcs.append('_emscripten_replace_memory')
    if not settings['SIDE_MODULE']:
      funcs += ['stackAlloc', 'stackSave', 'stackRestore', 'establishStackSpace']
    if settings['SAFE_HEAP']:
      funcs += ['setDynamicTop']
    if not settings['RELOCATABLE']:
      funcs += ['setTempRet0', 'getTempRet0']
    if not (settings['BINARYEN'] and settings['SIDE_MODULE']):
      funcs += ['setThrew']
  return list(set(funcs))


def get_implemented_functions(metadata):
  return set(metadata['implementedFunctions'])


def include_asm_consts(pre, forwarded_json, metadata, settings):
  if settings['BINARYEN'] and settings['SIDE_MODULE']:
    assert len(metadata['asmConsts']) == 0, 'EM_ASM is not yet supported in shared wasm module (it cannot be stored in the wasm itself, need some solution)'

  asm_consts, all_sigs = all_asm_consts(metadata)
  asm_const_funcs = []
  for sig in set(all_sigs):
    forwarded_json['Functions']['libraryFunctions']['_emscripten_asm_const_' + sig] = 1
    args = ['a%d' % i for i in range(len(sig)-1)]
    all_args = ['code'] + args
    asm_const_funcs.append(r'''
function _emscripten_asm_const_%s(%s) {
  return ASM_CONSTS[code](%s);
}''' % (sig.encode('utf-8'), ', '.join(all_args), ', '.join(args)))

  asm_consts_text = '\nvar ASM_CONSTS = [' + ',\n '.join(asm_consts) + '];\n'
  asm_funcs_text = '\n'.join(asm_const_funcs) + '\n'

  body_marker = '// === Body ==='
  return pre.replace(body_marker, body_marker + '\n' + asm_consts_text + asm_funcs_text)


def all_asm_consts(metadata):
  asm_consts = [0]*len(metadata['asmConsts'])
  all_sigs = []
  for k, v in metadata['asmConsts'].iteritems():
    const = v[0].encode('utf-8')
    sigs = v[1]
    if len(const) > 1 and const[0] == '"' and const[-1] == '"':
      const = const[1:-1]
    const = '{ ' + const + ' }'
    args = []
    arity = max(map(len, sigs)) - 1
    for i in range(arity):
      args.append('$' + str(i))
    const = 'function(' + ', '.join(args) + ') ' + const
    asm_consts[int(k)] = const
    all_sigs += sigs
  return asm_consts, all_sigs


def unfloat(s):
  """lower float to double for ffis"""
  return 'd' if s == 'f' else s


def make_function_tables_defs(implemented_functions, all_implemented, function_table_data, settings, metadata):
  class Counter(object):
    next_bad_item = 0
    next_item = 0
    pre = []

  in_table = set()
  debug_tables = {}

  def make_params(sig): return ','.join(['p%d' % p for p in range(len(sig)-1)])
  def make_coerced_params(sig): return ','.join([shared.JS.make_coercion('p%d', unfloat(sig[p+1]), settings) % p for p in range(len(sig)-1)])
  def make_coercions(sig): return ';'.join(['p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, sig[p+1], settings)) for p in range(len(sig)-1)]) + ';'

  # when emulating function pointer casts, we need to know what is the target of each pointer
  if settings['EMULATE_FUNCTION_POINTER_CASTS']:
    function_pointer_targets = {}
    for sig, table in function_table_data.iteritems():
      start = table.index('[')
      end = table.rindex(']')
      body = table[start+1:end].split(',')
      parsed = map(lambda x: x.strip(), body)
      for i in range(len(parsed)):
        if parsed[i] != '0':
          assert i not in function_pointer_targets
          function_pointer_targets[i] = [sig, str(parsed[i])]

  def make_table(sig, raw):
    if '[]' in raw: return ('', '') # empty table
    params = make_params(sig)
    coerced_params = make_coerced_params(sig)
    coercions = make_coercions(sig)
    def make_bad(target=None):
      i = Counter.next_bad_item
      Counter.next_bad_item += 1
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
        if item not in all_implemented:
          # this is not implemented; it would normally be wrapped, but with emulation, we just use it directly outside
          return item
        in_table.add(item)
        return "asm['" + item + "']"
      body = map(receive, body)
    for j in range(settings['RESERVED_FUNCTION_POINTERS']):
      curr = 'jsCall_%s_%s' % (sig, j)
      body[settings['FUNCTION_POINTER_ALIGNMENT'] * (1 + j)] = curr
      implemented_functions.add(curr)
    Counter.next_item = 0
    def fix_item(item):
      j = Counter.next_item
      Counter.next_item += 1
      newline = Counter.next_item % 30 == 29
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
      # in wasm we never need wrappers though
      if clean_item not in implemented_functions and not (settings['EMULATED_FUNCTION_POINTERS'] and not settings['RELOCATABLE']) and not settings['BINARYEN']:
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
        assert not sig == 'X', 'must know the signature in order to create a wrapper for "%s" (TODO for shared wasm modules)' % item
        return clean_item + '__wrapper'
      return item if not newline else (item + '\n')
    if settings['ASSERTIONS'] >= 2:
      debug_tables[sig] = body
    body = ','.join(map(fix_item, body))
    return ('\n'.join(Counter.pre), ''.join([raw[:start+1], body, raw[end:]]))

  infos = [make_table(sig, raw) for sig, raw in function_table_data.iteritems()]
  Counter.pre = []

  function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n'
  function_tables_defs += '\n// EMSCRIPTEN_END_FUNCS\n'
  function_tables_defs += '\n'.join([info[1] for info in infos])
  return in_table, debug_tables, function_tables_defs


def make_func(name, code, params, coercions):
  return 'function %s(%s) {\n %s %s\n}' % (name, params, coercions, code)


def math_fix(g):
  return g if not g.startswith('Math_') else g.split('_')[1]


def make_function_tables_impls(function_table_data, settings):
  function_tables_impls = []
  for sig, table in function_table_data.iteritems():
    args = ','.join(['a' + str(i) for i in range(1, len(sig))])
    arg_coercions = ' '.join(['a' + str(i) + '=' + shared.JS.make_coercion('a' + str(i), sig[i], settings) + ';' for i in range(1, len(sig))])
    coerced_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], settings) for i in range(1, len(sig))])
    sig_mask = str(table.count(','))
    ret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('FUNCTION_TABLE_%s[index&%s](%s)' % (sig, sig_mask, coerced_args), sig[0], settings)
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
  return function_tables_impls


def create_mftCall_funcs(function_table_data, settings):
  mftCall_funcs = []
  if settings.get('EMULATED_FUNCTION_POINTERS'):
    if settings.get('RELOCATABLE') and not settings['BINARYEN']: # in wasm, emulated function pointers are just simple table calls
      for sig, table in function_table_data.iteritems():
        params = ','.join(['ptr'] + ['p%d' % p for p in range(len(sig)-1)])
        coerced_params = ','.join([shared.JS.make_coercion('ptr', 'i', settings)] + [shared.JS.make_coercion('p%d', unfloat(sig[p+1]), settings) % p for p in range(len(sig)-1)])
        coercions = ';'.join(['ptr = ptr | 0'] + ['p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, unfloat(sig[p+1]), settings)) for p in range(len(sig)-1)]) + ';'
        mini_coerced_params = ','.join([shared.JS.make_coercion('p%d', sig[p+1], settings) % p for p in range(len(sig)-1)])
        maybe_return = '' if sig[0] == 'v' else 'return'
        final_return = maybe_return + ' ' + shared.JS.make_coercion('ftCall_' + sig + '(' + coerced_params + ')', unfloat(sig[0]), settings) + ';'
        if settings['EMULATED_FUNCTION_POINTERS'] == 1:
          body = final_return
        else:
          sig_mask = str(table.count(','))
          body = ('if (((ptr|0) >= (fb|0)) & ((ptr|0) < (fb + ' + sig_mask + ' | 0))) { ' + maybe_return + ' ' +
                  shared.JS.make_coercion(
                    'FUNCTION_TABLE_' + sig + '[(ptr-fb)&' + sig_mask + '](' +
                    mini_coerced_params + ')', sig[0], settings, ffi_arg=True
                  ) + '; ' + ('return;' if sig[0] == 'v' else '') + ' }' + final_return)
        mftCall_funcs.append(make_func('mftCall_' + sig, body, params, coercions) + '\n')
  return mftCall_funcs


def get_function_pointer_error(sig, function_table_sigs, settings):
  if settings['ASSERTIONS'] <= 1:
    extra = ' Module["printErr"]("Build with ASSERTIONS=2 for more info.");'
    pointer = ' '
  else:
    pointer = ' \'" + x + "\' '
    extra = ' Module["printErr"]("This pointer might make sense in another type signature: '
    # sort signatures, attempting to show most likely related ones first
    sigs = list(function_table_sigs)
    sigs.sort(key=signature_sort_key(sig))
    for other in sigs:
      if other != sig:
        extra += other + ': " + debug_table_' + other + '[x] + "  '
    extra += '"); '
  return 'Module["printErr"]("Invalid function pointer' + pointer + 'called with signature \'' + sig + '\'. ' + \
         'Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? ' + \
         'Or calling a function with an incorrect type, which will fail? ' + \
         '(it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)' + \
         '"); ' + extra


def signature_sort_key(sig):
  def closure(other):
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
  return closure


def create_asm_global_funcs(bg_funcs, metadata, settings):
  access_quote = access_quoter(settings)
  maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul', 'min', 'max', 'clz32']]
  if provide_fround(settings):
    maths += ['Math.fround']

  asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global' + access_quote(g) + ';\n' for g in maths])
  asm_global_funcs += ''.join(['  var ' + g + '=env' + access_quote(math_fix(g)) + ';\n' for g in bg_funcs])
  asm_global_funcs += global_simd_funcs(access_quote, metadata, settings)
  if settings['USE_PTHREADS']:
    asm_global_funcs += ''.join(['  var Atomics_' + ty + '=global' + access_quote('Atomics') + access_quote(ty) + ';\n' for ty in ['load', 'store', 'exchange', 'compareExchange', 'add', 'sub', 'and', 'or', 'xor']])
  return asm_global_funcs


def create_asm_global_vars(bg_vars, settings):
  access_quote = access_quoter(settings)
  asm_global_vars = ''.join(['  var ' + g + '=env' + access_quote(g) + '|0;\n' for g in bg_vars])
  if settings['BINARYEN'] and settings['SIDE_MODULE']:
    # wasm side modules internally define their stack, these are set at module startup time
    asm_global_vars += '\n  var STACKTOP = 0, STACK_MAX = 0;\n'

  return asm_global_vars


def global_simd_funcs(access_quote, metadata, settings):
  # Always import SIMD when building with -s SIMD=1, since in that mode memcpy is SIMD optimized.
  if not (metadata['simd'] or settings['SIMD']):
    return ''

  def string_contains_any(s, str_list):
    for sub in str_list:
      if sub in s:
        return True
    return False

  nonexisting_simd_symbols = ['Int8x16_fromInt8x16', 'Uint8x16_fromUint8x16', 'Int16x8_fromInt16x8', 'Uint16x8_fromUint16x8', 'Int32x4_fromInt32x4', 'Uint32x4_fromUint32x4', 'Float32x4_fromFloat32x4', 'Float64x2_fromFloat64x2']
  nonexisting_simd_symbols += ['Int32x4_addSaturate', 'Int32x4_subSaturate', 'Uint32x4_addSaturate', 'Uint32x4_subSaturate']
  nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8', 'Float64x2'] for y in ['load2', 'store2']]
  nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8'] for y in ['load1', 'store1']]

  simd = make_simd_types(metadata, settings)

  simd_func_text = ''
  simd_func_text += ''.join(['  var SIMD_' + ty + '=global' + access_quote('SIMD') + access_quote(ty) + ';\n' for ty in simd['types']])

  def generate_symbols(types, funcs):
    symbols = ['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in types for g in funcs]
    symbols = filter(lambda x: not string_contains_any(x, nonexisting_simd_symbols), symbols)
    return ''.join(symbols)

  simd_func_text += generate_symbols(simd['int_types'], simd['int_funcs'])
  simd_func_text += generate_symbols(simd['float_types'], simd['float_funcs'])
  simd_func_text += generate_symbols(simd['bool_types'], simd['bool_funcs'])

  # SIMD conversions (not bitcasts) between same lane sizes:
  def add_simd_cast(dst, src):
    return '  var SIMD_' + dst + '_from' + src + '=SIMD_' + dst + '.from' + src + ';\n'
  def add_simd_casts(t1, t2):
    return add_simd_cast(t1, t2) + add_simd_cast(t2, t1)

  # Bug: Skip importing conversions for int<->uint for now, they don't validate as asm.js. https://bugzilla.mozilla.org/show_bug.cgi?id=1313512
  # This is not an issue when building SSEx code, because it doesn't use these. (but it will be an issue if using SIMD.js intrinsics from vector.h to explicitly call these)
#      if metadata['simdInt8x16'] and metadata['simdUint8x16']: simd_func_text += add_simd_casts('Int8x16', 'Uint8x16')
#      if metadata['simdInt16x8'] and metadata['simdUint16x8']: simd_func_text += add_simd_casts('Int16x8', 'Uint16x8')
#      if metadata['simdInt32x4'] and metadata['simdUint32x4']: simd_func_text += add_simd_casts('Int32x4', 'Uint32x4')

  if metadata['simdInt32x4'] and metadata['simdFloat32x4']: simd_func_text += add_simd_casts('Int32x4', 'Float32x4')
  if metadata['simdUint32x4'] and metadata['simdFloat32x4']: simd_func_text += add_simd_casts('Uint32x4', 'Float32x4')
  if metadata['simdInt32x4'] and metadata['simdFloat64x2']: simd_func_text += add_simd_cast('Int32x4', 'Float64x2') # Unofficial, needed for emscripten_int32x4_fromFloat64x2
  if metadata['simdUint32x4'] and metadata['simdFloat64x2']: simd_func_text += add_simd_cast('Uint32x4', 'Float64x2') # Unofficial, needed for emscripten_uint32x4_fromFloat64x2

  # Unofficial, Bool64x2 does not yet exist, but needed for Float64x2 comparisons.
  if metadata['simdFloat64x2']:
    simd_func_text += '  var SIMD_Int32x4_fromBool64x2Bits = global.SIMD.Int32x4.fromBool64x2Bits;\n'
  return simd_func_text


def make_simd_types(metadata, settings):
  simd_float_types = []
  simd_int_types = []
  simd_bool_types = []
  simd_funcs = ['splat', 'check', 'extractLane', 'replaceLane']
  simd_intfloat_funcs = ['add', 'sub', 'neg', 'mul',
                         'equal', 'lessThan', 'greaterThan',
                         'notEqual', 'lessThanOrEqual', 'greaterThanOrEqual',
                         'select', 'swizzle', 'shuffle',
                         'load', 'store', 'load1', 'store1', 'load2', 'store2']
  simd_intbool_funcs = ['and', 'xor', 'or', 'not']
  if metadata['simdUint8x16']:
    simd_int_types += ['Uint8x16']
    simd_intfloat_funcs += ['fromUint8x16Bits']
  if metadata['simdInt8x16']:
    simd_int_types += ['Int8x16']
    simd_intfloat_funcs += ['fromInt8x16Bits']
  if metadata['simdUint16x8']:
    simd_int_types += ['Uint16x8']
    simd_intfloat_funcs += ['fromUint16x8Bits']
  if metadata['simdInt16x8']:
    simd_int_types += ['Int16x8']
    simd_intfloat_funcs += ['fromInt16x8Bits']
  if metadata['simdUint32x4']:
    simd_int_types += ['Uint32x4']
    simd_intfloat_funcs += ['fromUint32x4Bits']
  if metadata['simdInt32x4'] or settings['SIMD']:
    # Always import Int32x4 when building with -s SIMD=1, since memcpy is SIMD optimized.
    simd_int_types += ['Int32x4']
    simd_intfloat_funcs += ['fromInt32x4Bits']
  if metadata['simdFloat32x4']:
    simd_float_types += ['Float32x4']
    simd_intfloat_funcs += ['fromFloat32x4Bits']
  if metadata['simdFloat64x2']:
    simd_float_types += ['Float64x2']
    simd_intfloat_funcs += ['fromFloat64x2Bits']
  if metadata['simdBool8x16']:
    simd_bool_types += ['Bool8x16']
  if metadata['simdBool16x8']:
    simd_bool_types += ['Bool16x8']
  if metadata['simdBool32x4']:
    simd_bool_types += ['Bool32x4']
  if metadata['simdBool64x2']:
    simd_bool_types += ['Bool64x2']

  simd_float_funcs = simd_funcs + simd_intfloat_funcs + ['div', 'min', 'max', 'minNum', 'maxNum', 'sqrt',
                                'abs', 'reciprocalApproximation', 'reciprocalSqrtApproximation']
  simd_int_funcs = simd_funcs + simd_intfloat_funcs + simd_intbool_funcs + ['shiftLeftByScalar', 'shiftRightByScalar', 'addSaturate', 'subSaturate']
  simd_bool_funcs = simd_funcs + simd_intbool_funcs + ['anyTrue', 'allTrue']
  simd_types = simd_float_types + simd_int_types + simd_bool_types
  return {
    'types': simd_types,
    'float_types': simd_float_types,
    'int_types': simd_int_types,
    'bool_types': simd_bool_types,
    'funcs': simd_funcs,
    'float_funcs': simd_float_funcs,
    'int_funcs': simd_int_funcs,
    'bool_funcs': simd_bool_funcs,
    'intfloat_funcs': simd_intfloat_funcs,
    'intbool_funcs': simd_intbool_funcs,
  }


def need_asyncify(exported_implemented_functions):
  return '_emscripten_alloc_async_context' in exported_implemented_functions


def asm_safe_heap(settings):
  """optimized safe heap in asm, when we can"""
  return settings['SAFE_HEAP'] and not settings['SAFE_HEAP_LOG'] and not settings['RELOCATABLE']


def provide_fround(settings):
  return settings['PRECISE_F32'] or settings['SIMD']


def create_asm_setup(debug_tables, function_table_data, metadata, settings):
  function_table_sigs = function_table_data.keys()

  asm_setup = ''
  if settings['ASSERTIONS'] >= 2:
    for sig in function_table_data:
      asm_setup += '\nvar debug_table_' + sig + ' = ' + json.dumps(debug_tables[sig]) + ';'
  if settings['ASSERTIONS']:
    for sig in function_table_sigs:
      asm_setup += '\nfunction nullFunc_' + sig + '(x) { ' + get_function_pointer_error(sig, function_table_sigs, settings) + 'abort(x) }\n'
  if settings['BINARYEN']:
    def table_size(table):
      table_contents = table[table.index('[') + 1: table.index(']')]
      if len(table_contents) == 0: # empty table
        return 0
      return table_contents.count(',') + 1

    table_total_size = sum(map(table_size, function_table_data.values()))
    asm_setup += "\nModule['wasmTableSize'] = %d;\n" % table_total_size
    if not settings['EMULATED_FUNCTION_POINTERS']:
      asm_setup += "\nModule['wasmMaxTableSize'] = %d;\n" % table_total_size
  if settings['RELOCATABLE']:
    asm_setup += 'var setTempRet0 = Runtime.setTempRet0, getTempRet0 = Runtime.getTempRet0;\n'
    if not settings['SIDE_MODULE']:
      asm_setup += 'var gb = Runtime.GLOBAL_BASE, fb = 0;\n'
    side = 'parent' if settings['SIDE_MODULE'] else ''
    def check(extern):
      if settings['ASSERTIONS']:
        return ('assert(' + side + 'Module["' + extern + '"], "external function \'' + extern +
                '\' is missing. perhaps a side module was not linked in? if this symbol was expected to arrive '
                'from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");')
      return ''
    for extern in metadata['externs']:
      asm_setup += 'var g$' + extern + ' = function() { ' + check(extern) + ' return ' + side + 'Module["' + extern + '"] };\n'

  asm_setup += setup_function_pointers(function_table_sigs, settings)

  if settings.get('EMULATED_FUNCTION_POINTERS'):
    function_tables_impls = make_function_tables_impls(function_table_data, settings)
    asm_setup += '\n' + '\n'.join(function_tables_impls) + '\n'

  return asm_setup


def setup_function_pointers(function_table_sigs, settings):
  asm_setup = ''
  for sig in function_table_sigs:
    asm_setup += '\n' + shared.JS.make_invoke(sig) + '\n'
    if settings.get('RESERVED_FUNCTION_POINTERS'):
      asm_setup += '\n' + shared.JS.make_jscall(sig) + '\n'
    if settings.get('EMULATED_FUNCTION_POINTERS'):
      args = ['a%d' % i for i in range(len(sig)-1)]
      full_args = ['x'] + args
      table_access = 'FUNCTION_TABLE_' + sig
      if settings['SIDE_MODULE']:
        table_access = 'parentModule["' + table_access + '"]' # side module tables were merged into the parent, we need to access the global one
      if settings['BINARYEN']:
        # wasm uses a Table, which means we have function pointer emulation capabilities all the time, at no cost. just call the table
        table_access = "Module['wasmTable']"
        table_read = table_access + '.get(x)'
      else:
        table_read = table_access + '[x]'
      prelude = '''
  if (x < 0 || x >= %s.length) { Module.printErr("Function table mask error (out of range)"); %s ; abort(x) }''' % (table_access, get_function_pointer_error(sig, function_table_sigs, settings))
      asm_setup += '''
function ftCall_%s(%s) {%s
  return %s(%s);
}
''' % (sig, ', '.join(full_args), prelude, table_read, ', '.join(args))
  return asm_setup


def create_basic_funcs(function_table_sigs, settings):
  basic_funcs = ['abort', 'assert', 'enlargeMemory', 'getTotalMemory']
  if settings['ABORTING_MALLOC']:
    basic_funcs += ['abortOnCannotGrowMemory']
  if settings['STACK_OVERFLOW_CHECK']:
    basic_funcs += ['abortStackOverflow']
  if settings['SAFE_HEAP']:
    if asm_safe_heap(settings):
      basic_funcs += ['segfault', 'alignfault', 'ftfault']
    else:
      basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_LOAD_D', 'SAFE_HEAP_STORE', 'SAFE_HEAP_STORE_D', 'SAFE_FT_MASK']
  if settings['ASSERTIONS']:
    for sig in function_table_sigs:
      basic_funcs += ['nullFunc_' + sig]
  if settings['RELOCATABLE']:
    basic_funcs += ['setTempRet0', 'getTempRet0']

  for sig in function_table_sigs:
    basic_funcs.append('invoke_%s' % sig)
    if settings.get('RESERVED_FUNCTION_POINTERS'):
      basic_funcs.append('jsCall_%s' % sig)
    if settings.get('EMULATED_FUNCTION_POINTERS'):
      if not settings['BINARYEN']: # in wasm, emulated function pointers are just simple table calls
        basic_funcs.append('ftCall_%s' % sig)
  return basic_funcs


def create_basic_vars(exported_implemented_functions, forwarded_json, metadata, settings):
  basic_vars = ['DYNAMICTOP_PTR', 'tempDoublePtr', 'ABORT']
  if not (settings['BINARYEN'] and settings['SIDE_MODULE']):
    basic_vars += ['STACKTOP', 'STACK_MAX']
  if metadata.get('preciseI64MathUsed'):
    basic_vars += ['cttz_i8']
  else:
    if forwarded_json['Functions']['libraryFunctions'].get('_llvm_cttz_i32'):
      basic_vars += ['cttz_i8']
  if settings['RELOCATABLE']:
    if not (settings['BINARYEN'] and settings['SIDE_MODULE']):
      basic_vars += ['gb', 'fb']
    else:
      basic_vars += ['memoryBase', 'tableBase'] # wasm side modules have a specific convention for these

  # See if we need ASYNCIFY functions
  # We might not need them even if ASYNCIFY is enabled
  if need_asyncify(exported_implemented_functions):
    basic_vars += ['___async', '___async_unwind', '___async_retval', '___async_cur_frame']
  return basic_vars


def create_exports(exported_implemented_functions, in_table, function_table_data, metadata, settings):
  quote = quoter(settings)
  asm_runtime_funcs = create_asm_runtime_funcs(settings)
  if need_asyncify(exported_implemented_functions):
    asm_runtime_funcs.append('setAsync')
  all_exported = exported_implemented_functions + asm_runtime_funcs + function_tables(function_table_data, settings)
  if settings['EMULATED_FUNCTION_POINTERS']:
    all_exported = list(set(all_exported).union(in_table))
  exports = []
  for export in set(all_exported):
    exports.append(quote(export) + ": " + export)
  if settings['BINARYEN'] and settings['SIDE_MODULE']:
    # named globals in side wasm modules are exported globals from asm/wasm
    for k, v in metadata['namedGlobals'].iteritems():
      exports.append(quote('_' + str(k)) + ': ' + str(v))
    # aliases become additional exports
    for k, v in metadata['aliases'].iteritems():
      exports.append(quote(str(k)) + ': ' + str(v))
  return '{ ' + ', '.join(exports) + ' }'


def create_asm_runtime_funcs(settings):
  funcs = []
  if not (settings['BINARYEN'] and settings['SIDE_MODULE']):
    funcs += ['stackAlloc', 'stackSave', 'stackRestore', 'establishStackSpace', 'setThrew']
  if not settings['RELOCATABLE']:
    funcs += ['setTempRet0', 'getTempRet0']
  if settings['SAFE_HEAP']:
    funcs += ['setDynamicTop']
  if settings['ONLY_MY_CODE']:
    funcs = []
  if settings.get('EMTERPRETIFY'):
    funcs += ['emterpret']
    if settings.get('EMTERPRETIFY_ASYNC'):
      funcs += ['setAsyncState', 'emtStackSave', 'emtStackRestore']
  return funcs


def function_tables(function_table_data, settings):
  if not settings['EMULATED_FUNCTION_POINTERS']:
    return ['dynCall_' + table for table in function_table_data]
  else:
    return []


def create_the_global(metadata, settings):
  fundamentals = ['Math']
  fundamentals += ['Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array']
  fundamentals += ['NaN', 'Infinity']
  if metadata['simd'] or settings['SIMD']:
    # Always import SIMD when building with -s SIMD=1, since in that mode memcpy is SIMD optimized.
    fundamentals += ['SIMD']
  if settings['ALLOW_MEMORY_GROWTH']:
    fundamentals.append('byteLength')
  return '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'


def create_receiving(function_table_data, function_tables_defs, exported_implemented_functions, settings):
  receiving = ''
  if not settings['ASSERTIONS']:
    runtime_assertions = ''
  else:
    # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
    # some support code
    runtime_assertions = '''
  assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
  assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
'''
    receiving = '\n'.join(['var real_' + s + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {''' + runtime_assertions + '''  return real_''' + s + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_memcpy', '_memset', 'runPostSets', '_emscripten_replace_memory', '__start_module']])
  if not settings['SWAPPABLE_ASM_MODULE']:
    receiving += ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables(function_table_data, settings)])
  else:
    receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + s + ' = Module["' + s + '"] = function() {' + runtime_assertions + '  return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions + function_tables(function_table_data, settings)])
  receiving += ';\n'

  if settings['EXPORT_FUNCTION_TABLES'] and not settings['BINARYEN']:
    for table in function_table_data.values():
      tableName = table.split()[1]
      table = table.replace('var ' + tableName, 'var ' + tableName + ' = Module["' + tableName + '"]')
      receiving += table + '\n'

  if settings.get('EMULATED_FUNCTION_POINTERS'):
    receiving += '\n' + function_tables_defs.replace('// EMSCRIPTEN_END_FUNCS\n', '') + '\n' + ''.join(['Module["dynCall_%s"] = dynCall_%s\n' % (sig, sig) for sig in function_table_data])
    if not settings['BINARYEN']:
      for sig in function_table_data.keys():
        name = 'FUNCTION_TABLE_' + sig
        fullname = name if not settings['SIDE_MODULE'] else ('SIDE_' + name)
        receiving += 'Module["' + name + '"] = ' + fullname + ';\n'

  return receiving


def create_named_globals(metadata, settings):
  named_globals = ''
  if settings['RELOCATABLE']:
    named_globals += '''
var NAMED_GLOBALS = { %s };
for (var named in NAMED_GLOBALS) {
  Module['_' + named] = gb + NAMED_GLOBALS[named];
}
Module['NAMED_GLOBALS'] = NAMED_GLOBALS;
''' % ', '.join('"' + k + '": ' + str(v) for k, v in metadata['namedGlobals'].iteritems())
    if settings['BINARYEN']:
      # wasm side modules are pure wasm, and cannot create their g$..() methods, so we help them out
      # TODO: this works if we are the main module, but if the supplying module is later, it won't, so
      #       we'll need another solution for that. one option is to scan the module imports, if/when
      #       wasm supports that, then the loader can do this.
      named_globals += '''
for (var named in NAMED_GLOBALS) {
  (function(named) {
    Module['g$_' + named] = function() { return Module['_' + named] };
  })(named);
}
'''
    named_globals += ''.join(["Module['%s'] = Module['%s']\n" % (k, v) for k, v in metadata['aliases'].iteritems()])
  return named_globals


def create_runtime_funcs(exports, settings):
  if settings['ONLY_MY_CODE']:
    return []
  return ['''
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
}''' if need_asyncify(exports) else '') + ('''
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
  HEAP32[DYNAMICTOP_PTR>>2] = value;
}
'''] + ['' if not asm_safe_heap(settings) else '''
function SAFE_HEAP_STORE(dest, value, bytes) {
  dest = dest | 0;
  value = value | 0;
  bytes = bytes | 0;
  if ((dest|0) <= 0) segfault();
  if (((dest + bytes)|0) > (HEAP32[DYNAMICTOP_PTR>>2]|0)) segfault();
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
  if (((dest + bytes)|0) > (HEAP32[DYNAMICTOP_PTR>>2]|0)) segfault();
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
  if ((dest + bytes|0) > (HEAP32[DYNAMICTOP_PTR>>2]|0)) segfault();
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
  if ((dest + bytes|0) > (HEAP32[DYNAMICTOP_PTR>>2]|0)) segfault();
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


def create_asm_start_pre(asm_setup, the_global, sending, metadata, settings):
  access_quote = access_quoter(settings)

  shared_array_buffer = ''
  if settings['USE_PTHREADS']:
    shared_array_buffer = "Module.asmGlobalArg['Atomics'] = Atomics;"

  module_get = 'Module{access} = {val};'
  module_global = module_get.format(access=access_quote('asmGlobalArg'), val=the_global)
  module_library = module_get.format(access=access_quote('asmLibraryArg'), val=sending)

  asm_function_top = ('// EMSCRIPTEN_START_ASM\n'
                      'var asm = (function(global, env, buffer) {')

  use_asm = "'almost asm';"
  if not metadata.get('hasInlineJS') and settings['ASM_JS'] == 1:
    use_asm = "'use asm';"

  lines = [
    asm_setup,
    module_global,
    shared_array_buffer,
    module_library,
    asm_function_top,
    use_asm,
    create_first_in_asm(settings),
    create_memory_views(settings),
  ]
  return '\n'.join(lines)


def create_asm_temp_vars(settings):
  access_quote = access_quoter(settings)
  return '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var nan = global%s, inf = global%s;
  var tempInt = 0, tempBigInt = 0, tempBigIntS = 0, tempValue = 0, tempDouble = 0.0;
  var tempRet0 = 0;
''' % (access_quote('NaN'), access_quote('Infinity'))

def create_asm_runtime_thread_local_vars(settings):
  if settings['USE_PTHREADS']:
    return '''
  var __pthread_ptr = 0;
  var __pthread_is_main_runtime_thread = 0;
  var __pthread_is_main_browser_thread = 0;
'''
  else:
    return ''

def create_replace_memory(settings):
  if not settings['ALLOW_MEMORY_GROWTH']:
    return ''
  return '''
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
'''


def create_asm_end(exports, settings):
  access_quote = access_quoter(settings)
  return '''

  return %s;
})
// EMSCRIPTEN_END_ASM
(%s, %s, buffer);
''' % (exports,
       'Module' + access_quote('asmGlobalArg'),
       'Module' + access_quote('asmLibraryArg'))


def create_runtime_library_overrides(settings):
  overrides = []
  if not settings.get('SIDE_MODULE'):
    overrides += [
      'stackAlloc',
      'stackSave',
      'stackRestore',
      'establishStackSpace',
    ]
    if settings['SAFE_HEAP']:
      overrides.append('setDynamicTop')

  if not settings['RELOCATABLE']:
    overrides += ['setTempRet0', 'getTempRet0']

  lines = ["Runtime.{0} = Module['{0}'];".format(func) for func in overrides]
  return '\n'.join(lines)


def create_first_in_asm(settings):
  first_in_asm = ''
  if settings['SPLIT_MEMORY']:
    if not settings['SAFE_SPLIT_MEMORY']:
      first_in_asm += ''.join([make_get_set(info) for info in HEAP_TYPE_INFOS]) + '\n'
    first_in_asm += 'buffer = new ArrayBuffer(32); // fake\n'
  return first_in_asm


def make_get_set(info):
  """Generates get*/set* functions for the different heap types.

  Generated symbols:
    get8 get16 get32 getU8 getU16 getU32 getF32 getF64
    set8 set16 set32 setU8 setU16 setU32 setF32 setF64
  """
  access = ('{name}s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> {shift}]'
            .format(name=info.heap_name, shift=info.shift_amount))
  # TODO: fround when present for Float32
  return '''
function get{short}(ptr) {{
  ptr = ptr | 0;
  return {coerced_access};
}}
function set{short}(ptr, value) {{
  ptr = ptr | 0;
  value = {coerced_value};
  {access} = value;
}}'''.format(
    short=info.short_name(),
    coerced_value=info.coerce('value'),
    access=access,
    coerced_access=info.coerce(access))


def create_memory_views(settings):
  """Generates memory views for the different heap types.

  Generated symbols:
    Int8View    Int16View   Int32View
    Uint8View   Uint16View  Uint32View
    Float32View Float64View
  """
  access_quote = access_quoter(settings)
  ret = '\n'
  grow_memory = settings['ALLOW_MEMORY_GROWTH']
  for info in HEAP_TYPE_INFOS:
    access = access_quote('{}Array'.format(info.long_name))
    format_args = {
      'heap': info.heap_name,
      'long': info.long_name,
      'access': access,
    }
    if grow_memory:
      ret += ('  var {long}View = global{access};\n'
              '  var {heap} = new {long}View(buffer);\n').format(**format_args)
    else:
      ret += '  var {heap} = new global{access}(buffer);\n'.format(**format_args)
  if grow_memory:
    ret += '  var byteLength = global.byteLength;\n'
  return ret


class HeapTypeInfo(object):
  """Struct that holds data for a type of HEAP* views."""
  def __init__(self, heap_name, long_name, shift_amount):
    assert heap_name.startswith('HEAP')
    self.heap_name = heap_name
    self.long_name = long_name
    self.shift_amount = shift_amount

  def short_name(self):
    """The unique part of the heap name for this type.

    Derive this from heap_name instead of the other way around so that searching,
    e.g. for HEAP8, from the generated JS code leads back here.
    """
    return self.heap_name[len('HEAP'):]

  def is_int(self):
    """Whether this heap type is an integer type or not."""
    return self.short_name()[0] != 'F'

  def coerce(self, expression):
    """Adds asm.js type coercion to a string expression."""
    if self.is_int():
      return expression + '| 0'
    else:
      return '+' + expression


HEAP_TYPE_INFOS = [
  HeapTypeInfo(heap_name='HEAP8',   long_name='Int8',    shift_amount=0),
  HeapTypeInfo(heap_name='HEAP16',  long_name='Int16',   shift_amount=1),
  HeapTypeInfo(heap_name='HEAP32',  long_name='Int32',   shift_amount=2),
  HeapTypeInfo(heap_name='HEAPU8',  long_name='Uint8',   shift_amount=0),
  HeapTypeInfo(heap_name='HEAPU16', long_name='Uint16',  shift_amount=1),
  HeapTypeInfo(heap_name='HEAPU32', long_name='Uint32',  shift_amount=2),
  HeapTypeInfo(heap_name='HEAPF32', long_name='Float32', shift_amount=2),
  HeapTypeInfo(heap_name='HEAPF64', long_name='Float64', shift_amount=3),
]


def emscript_wasm_backend(infile, settings, outfile, libraries=None, compiler_engine=None,
                          temp_files=None, DEBUG=None):
  # Overview:
  #   * Run LLVM backend to emit .s
  #   * Run Binaryen's s2wasm to generate WebAssembly.
  #   * We may also run some Binaryen passes here.

  if libraries is None: libraries = []

  wast = build_wasm(temp_files, infile, outfile, settings, DEBUG)

  # js compiler

  if DEBUG: logging.debug('emscript: js compiler glue')

  # Integrate info from backend

  output = open(wast).read()
  parts = output.split('\n;; METADATA:')
  assert len(parts) == 2
  metadata_raw = parts[1]
  parts = output = None

  if DEBUG: logging.debug("METAraw %s", metadata_raw)
  metadata = create_metadata_wasm(metadata_raw, wast)
  if DEBUG: logging.debug(repr(metadata))

  update_settings_glue(settings, metadata)

  if DEBUG: t = time.time()
  glue, forwarded_data = compile_settings(compiler_engine, settings, libraries, temp_files)
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

  exported_implemented_functions = create_exported_implemented_functions_wasm(pre, forwarded_json, metadata, settings)

  asm_consts, asm_const_funcs = create_asm_consts_wasm(forwarded_json, metadata)
  pre = pre.replace('// === Body ===', '// === Body ===\n' + '\nvar ASM_CONSTS = [' + ',\n '.join(asm_consts) + '];\n' + '\n'.join(asm_const_funcs) + '\n')

  outfile.write(pre)
  pre = None

  invoke_funcs = read_wast_invoke_imports(wast)

  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except:
    pass

  # sent data
  sending = create_sending_wasm(invoke_funcs, forwarded_json, metadata, settings)
  receiving = create_receiving_wasm(exported_implemented_functions, settings)

  # finalize
  module = create_module_wasm(sending, receiving, invoke_funcs, settings)

  write_output_file(outfile, post, module)
  module = None

  outfile.close()


def build_wasm(temp_files, infile, outfile, settings, DEBUG):
  with temp_files.get_file('.wb.s') as temp_s:
    backend_args = create_backend_args_wasm(infile, temp_s, settings)
    if DEBUG:
      logging.debug('emscript: llvm wasm backend: ' + ' '.join(backend_args))
      t = time.time()
    shared.check_call(backend_args)
    if DEBUG:
      logging.debug('  emscript: llvm wasm backend took %s seconds' % (time.time() - t))
      t = time.time()
      shutil.copyfile(temp_s, os.path.join(shared.CANONICAL_TEMP_DIR, 'emcc-llvm-backend-output.s'))

    assert shared.Settings.BINARYEN_ROOT, 'need BINARYEN_ROOT config set so we can use Binaryen s2wasm on the backend output'
    basename = shared.unsuffixed(outfile.name)
    wast = basename + '.wast'
    s2wasm_args = create_s2wasm_args(temp_s)
    if DEBUG:
      logging.debug('emscript: binaryen s2wasm: ' + ' '.join(s2wasm_args))
      t = time.time()
      #s2wasm_args += ['--debug']
    shared.check_call(s2wasm_args, stdout=open(wast, 'w'))
    # Also convert wasm text to binary
    wasm_as_args = [os.path.join(shared.Settings.BINARYEN_ROOT, 'bin', 'wasm-as'),
                    wast, '-o', basename + '.wasm']
    if settings['DEBUG_LEVEL'] >= 2 or settings['PROFILING_FUNCS']:
      wasm_as_args += ['-g']
    logging.debug('  emscript: binaryen wasm-as: ' + ' '.join(wasm_as_args))
    shared.check_call(wasm_as_args)

  if DEBUG:
    logging.debug('  emscript: binaryen s2wasm took %s seconds' % (time.time() - t))
    t = time.time()
    shutil.copyfile(wast, os.path.join(shared.CANONICAL_TEMP_DIR, 'emcc-s2wasm-output.wast'))
  return wast


def create_metadata_wasm(metadata_raw, wast):
  metadata = load_metadata(metadata_raw)
  add_metadata_from_wast(metadata, wast)
  return metadata


def create_exported_implemented_functions_wasm(pre, forwarded_json, metadata, settings):
  exported_implemented_functions = set(metadata['exports'])

  all_exported_functions = set(shared.expand_response(settings['EXPORTED_FUNCTIONS'])) # both asm.js and otherwise
  for additional_export in settings['DEFAULT_LIBRARY_FUNCS_TO_INCLUDE']: # additional functions to export from asm, if they are implemented
    all_exported_functions.add('_' + additional_export)
  all_implemented = metadata['implementedFunctions'] + forwarded_json['Functions']['implementedFunctions'].keys() # XXX perf?

  export_bindings = settings['EXPORT_BINDINGS']
  export_all = settings['EXPORT_ALL']
  for key in all_implemented:
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      exported_implemented_functions.add(key)

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

  return exported_implemented_functions

def create_asm_consts_wasm(forwarded_json, metadata):
  asm_consts = [0]*len(metadata['asmConsts'])
  all_sigs = []
  for k, v in metadata['asmConsts'].iteritems():
    const = v[0].encode('utf-8')
    sigs = v[1]
    if len(const) > 1 and const[0] == '"' and const[-1] == '"':
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

  return asm_consts, asm_const_funcs


def read_wast_invoke_imports(wast):
  invoke_funcs = []
  for line in open(wast).readlines():
    if line.strip().startswith('(import '):
      parts = line.split()
      func_name = parts[2][1:-1]
      if func_name.startswith('invoke_'):
        invoke_funcs.append(func_name)
  return invoke_funcs


def create_sending_wasm(invoke_funcs, forwarded_json, metadata, settings):
  basic_funcs = ['abort', 'assert', 'enlargeMemory', 'getTotalMemory']
  if settings['ABORTING_MALLOC']:
    basic_funcs += ['abortOnCannotGrowMemory']

  basic_vars = ['STACKTOP', 'STACK_MAX', 'DYNAMICTOP_PTR', 'ABORT']

  if not settings['RELOCATABLE']:
    global_vars = metadata['externs']
  else:
    global_vars = [] # linkable code accesses globals through function calls

  implemented_functions = set(metadata['implementedFunctions'])
  global_funcs = list(set([key for key, value in forwarded_json['Functions']['libraryFunctions'].iteritems() if value != 2]).difference(set(global_vars)).difference(implemented_functions))

  send_items = basic_funcs + invoke_funcs + global_funcs + basic_vars + global_vars
  def math_fix(g):
    return g if not g.startswith('Math_') else g.split('_')[1]
  return '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in send_items]) + ' }'


def create_receiving_wasm(exported_implemented_functions, settings):
  receiving = ''
  if settings['ASSERTIONS']:
    # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
    # some support code
    receiving = '\n'.join(['var real_' + asmjs_mangle(s) + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_''' + asmjs_mangle(s) + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_memcpy', '_memset', 'runPostSets', '_emscripten_replace_memory', '__start_module']])

  if not settings['SWAPPABLE_ASM_MODULE']:
    receiving += ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = asm["' + s + '"]' for s in exported_implemented_functions])
  else:
    receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = function() { return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions])
  receiving += ';\n'
  return receiving


def create_module_wasm(sending, receiving, invoke_funcs, settings):
  access_quote = access_quoter(settings)
  invoke_wrappers = create_invoke_wrappers(invoke_funcs)

  the_global = '{}'

  if settings['USE_PTHREADS']:
    shared_array_buffer = "if (typeof SharedArrayBuffer !== 'undefined') Module.asmGlobalArg['Atomics'] = Atomics;"
  else:
    shared_array_buffer = ''

  module = ['''
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
  module.append('''
STACKTOP = STACK_BASE + TOTAL_STACK;
STACK_MAX = STACK_BASE;
HEAP32[%d >> 2] = STACKTOP;
Runtime.stackAlloc = Module['_stackAlloc'];
Runtime.stackSave = Module['_stackSave'];
Runtime.stackRestore = Module['_stackRestore'];
Runtime.establishStackSpace = Module['establishStackSpace'];
''' % shared.Settings.GLOBAL_BASE)

  module.append('''
Runtime.setTempRet0 = Module['setTempRet0'];
Runtime.getTempRet0 = Module['getTempRet0'];
''')

  module.append(invoke_wrappers)
  return module

def create_backend_args_wasm(infile, temp_s, settings):
  backend_compiler = os.path.join(shared.LLVM_ROOT, 'llc')
  args = [backend_compiler, infile, '-march=wasm32', '-filetype=asm',
                  '-asm-verbose=false',
                  '-o', temp_s]
  args += ['-thread-model=single'] # no threads support in backend, tell llc to not emit atomics
  # disable slow and relatively unimportant optimization passes
  args += ['-combiner-global-alias-analysis=false']

  # asm.js-style exception handling
  if settings['DISABLE_EXCEPTION_CATCHING'] != 1:
    args += ['-enable-emscripten-cxx-exceptions']
  if settings['DISABLE_EXCEPTION_CATCHING'] == 2:
    whitelist = ','.join(settings['EXCEPTION_CATCHING_WHITELIST'] or ['__fake'])
    args += ['-emscripten-cxx-exceptions-whitelist=' + whitelist]

  # asm.js-style setjmp/longjmp handling
  args += ['-enable-emscripten-sjlj']
  return args


def create_s2wasm_args(temp_s):
  def wasm_rt_fail(archive_file):
    def wrapped():
      raise Exception('Expected {} to already be built'.format(archive_file))
    return wrapped
  compiler_rt_lib = shared.Cache.get('wasm_compiler_rt.a', wasm_rt_fail('wasm_compiler_rt.a'), 'a')
  libc_rt_lib = shared.Cache.get('wasm_libc_rt.a', wasm_rt_fail('wasm_libc_rt.a'), 'a')

  s2wasm_path = os.path.join(shared.Settings.BINARYEN_ROOT, 'bin', 's2wasm')

  args = [s2wasm_path, temp_s, '--emscripten-glue']
  args += ['--global-base=%d' % shared.Settings.GLOBAL_BASE]
  args += ['--initial-memory=%d' % shared.Settings.TOTAL_MEMORY]
  args += ['--allow-memory-growth'] if shared.Settings.ALLOW_MEMORY_GROWTH else []
  args += ['-l', libc_rt_lib]
  args += ['-l', compiler_rt_lib]
  return args


def load_metadata(metadata_raw):
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

  # Initializers call the global var version of the export, so they get the mangled name.
  metadata['initializers'] = map(asmjs_mangle, metadata['initializers'])

  return metadata


def add_metadata_from_wast(metadata, wast):
  """Reads .wast file and adds metadata we can read from the code.

  TODO: emit this metadata directly from s2wasm.
  """
  for line in open(wast).readlines():
    line = line.strip()
    if line.startswith('(import '):
      parts = line.split()
      # Don't include Invoke wrapper names (for asm.js-style exception handling)
      # in metadata[declares], the invoke wrappers will be generated in
      # this script later.
      import_type = parts[3][1:]
      import_name = parts[2][1:-1]
      if import_type == 'memory':
        continue
      elif import_type == 'func':
        if not import_name.startswith('invoke_'):
          metadata['declares'].append(import_name)
      elif import_type == 'global':
        metadata['externs'].append('_' + import_name)
      else:
        assert False, 'Unhandled import type "%s"' % import_type
    elif line.startswith('(func '):
      parts = line.split()
      func_name = parts[1][1:]
      metadata['implementedFunctions'].append('_' + func_name)
    elif line.startswith('(export '):
      parts = line.split()
      export_name = parts[1][1:-1]
      export_type = parts[2][1:]
      if export_type == 'func':
        assert asmjs_mangle(export_name) not in metadata['exports']
        metadata['exports'].append(export_name)
      else:
        assert False, 'Unhandled export type "%s"' % export_type

  # we emit those ourselves
  metadata['declares'] = filter(lambda x: not x.startswith('emscripten_asm_const'), metadata['declares'])


def create_invoke_wrappers(invoke_funcs):
  """Asm.js-style exception handling: invoke wrapper generation."""
  invoke_wrappers = ''
  for invoke in invoke_funcs:
    sig = invoke[len('invoke_'):]
    invoke_wrappers += '\n' + shared.JS.make_invoke(sig) + '\n'
  return invoke_wrappers


def asmjs_mangle(name):
  """Mangle a name the way asm.js/JSBackend globals are mangled.

  Prepends '_' and replaces non-alphanumerics with '_'.
  Used by wasm backend for JS library consistency with asm.js.
  """
  library_functions_in_module = ('setThrew', 'setTempRet0', 'getTempRet0')
  if name.startswith('dynCall_'): return name
  if name in library_functions_in_module: return name
  return '_' + ''.join(['_' if not c.isalnum() else c for c in name])


if os.environ.get('EMCC_FAST_COMPILER') == '0':
  logging.critical('Non-fastcomp compiler is no longer available, please use fastcomp or an older version of emscripten')
  sys.exit(1)


def normalize_line_endings(text):
  """Normalize to UNIX line endings.

  On Windows, writing to text file will duplicate \r\n to \r\r\n otherwise.
  """
  if WINDOWS:
    return text.replace('\r\n', '\n')
  return text


def main(args, compiler_engine, cache, temp_files, DEBUG):
  # Prepare settings for serialization to JSON.
  settings = {}
  for setting in args.settings:
    name, value = setting.strip().split('=', 1)
    value = json.loads(value)
    if isinstance(value, unicode):
      value = value.encode('utf8')
    settings[name] = value

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
  sys.exit(0)
