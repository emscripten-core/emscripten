# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""A small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
"""

import difflib
import os
import json
import subprocess
import re
import time
import logging
import shutil
import pprint
from collections import OrderedDict

from tools import shared
from tools import gen_struct_info
from tools import jsrun
from tools.response_file import substitute_response_files
from tools.shared import WINDOWS, asstr, path_from_root, exit_with_error
from tools.toolchain_profiler import ToolchainProfiler
from tools.minified_js_name_generator import MinifiedJsNameGenerator

logger = logging.getLogger('emscripten')

if __name__ == '__main__':
  ToolchainProfiler.record_process_start()

STDERR_FILE = os.environ.get('EMCC_STDERR_FILE')
if STDERR_FILE:
  STDERR_FILE = os.path.abspath(STDERR_FILE)
  logger.info('logging stderr in js compiler phase into %s' % STDERR_FILE)
  STDERR_FILE = open(STDERR_FILE, 'w')


def get_configuration():
  if hasattr(get_configuration, 'configuration'):
    return get_configuration.configuration

  configuration = shared.Configuration(environ=os.environ)
  get_configuration.configuration = configuration
  return configuration


def quote(prop):
  if shared.Settings.USE_CLOSURE_COMPILER == 2:
    return ''.join(["'" + p + "'" for p in prop.split('.')])
  else:
    return prop


def access_quote(prop):
  if shared.Settings.USE_CLOSURE_COMPILER == 2:
    return ''.join(["['" + p + "']" for p in prop.split('.')])
  else:
    return '.' + prop


def emscript(infile, outfile, memfile, libraries, compiler_engine, temp_files,
             DEBUG):
  """Runs the emscripten LLVM-to-JS compiler.

  Args:
    infile: The path to the input LLVM assembly file.
    outfile: An open file object where the output is written.
  """

  assert shared.Settings.ASM_JS, 'fastcomp is asm.js-only (mode 1 or 2)'

  success = False

  try:

    # Overview:
    #   * Run LLVM backend to emit JS. JS includes function bodies, memory initializer,
    #     and various metadata
    #   * Run compiler.js on the metadata to emit the shell js code, pre/post-ambles,
    #     JS library dependencies, etc.

    # metadata is modified by reference in some of the below
    # these functions are split up to force variables to go out of scope and allow
    # memory to be reclaimed

    with ToolchainProfiler.profile_block('get_and_parse_backend'):
      backend_output = compile_js(infile, temp_files, DEBUG)
      funcs, metadata, mem_init = parse_backend_output(backend_output, DEBUG)
      fixup_metadata_tables(metadata)
      funcs = fixup_functions(funcs, metadata)
    with ToolchainProfiler.profile_block('compiler_glue'):
      glue, forwarded_data = compiler_glue(metadata, libraries, compiler_engine, temp_files, DEBUG)

    with ToolchainProfiler.profile_block('function_tables_and_exports'):
      (post, function_table_data, bundled_args) = (
          function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, outfile, DEBUG))
    with ToolchainProfiler.profile_block('write_output_file'):
      finalize_output(outfile, post, function_table_data, bundled_args, metadata, DEBUG)
    success = True

  finally:
    outfile.close()
    if not success:
      shared.try_delete(outfile.name) # remove partial output


def compile_js(infile, temp_files, DEBUG):
  """Compile infile with asm.js backend, return the contents of the compiled js"""
  with temp_files.get_file('.4.js') as temp_js:
    backend_args = create_backend_args(infile, temp_js)

    if DEBUG:
      logger.debug('emscript: llvm backend: ' + ' '.join(backend_args))
      t = time.time()
    with ToolchainProfiler.profile_block('emscript_llvm_backend'):
      jsrun.timeout_run(subprocess.Popen(backend_args, stdout=subprocess.PIPE, universal_newlines=True), note_args=backend_args)
    if DEBUG:
      logger.debug('  emscript: llvm backend took %s seconds' % (time.time() - t))

    # Split up output
    backend_output = open(temp_js).read()
    # if DEBUG:
    #   print >> sys.stderr, backend_output
  return backend_output


def parse_backend_output(backend_output, DEBUG):
  start_funcs_marker = '// EMSCRIPTEN_START_FUNCTIONS'
  end_funcs_marker = '// EMSCRIPTEN_END_FUNCTIONS'
  metadata_split_marker = '// EMSCRIPTEN_METADATA'

  start_funcs = backend_output.index(start_funcs_marker)
  end_funcs = backend_output.rindex(end_funcs_marker)
  metadata_split = backend_output.rindex(metadata_split_marker)

  funcs = backend_output[start_funcs + len(start_funcs_marker):end_funcs]
  metadata_raw = backend_output[metadata_split + len(metadata_split_marker):]
  mem_init = backend_output[end_funcs + len(end_funcs_marker):metadata_split]

  # we no longer use the "Runtime" object. TODO: stop emiting it in the backend
  mem_init = mem_init.replace('Runtime.', '')

  try:
    metadata = json.loads(metadata_raw, object_pairs_hook=OrderedDict)
  except:
    logger.error('emscript: failure to parse metadata output from compiler backend. raw output is: \n' + metadata_raw)
    raise

  # functions marked llvm.used in the code are exports requested by the user
  shared.Building.user_requested_exports += metadata['exports']

  return funcs, metadata, mem_init


def fixup_metadata_tables(metadata):
  # if emulating pointer casts, force all tables to the size of the largest
  # (for wasm, we use binaryen's fpcast-emu pass, we don't need to do anything
  # here)
  if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS and not shared.Settings.WASM:
    max_size = 0
    for k, v in metadata['tables'].items():
      max_size = max(max_size, v.count(',') + 1)
    for k, v in metadata['tables'].items():
      curr = v.count(',') + 1
      if curr < max_size:
        if v.count('[]') == 1:
          metadata['tables'][k] = v.replace(']', (','.join(['0'] * (max_size - curr)) + ']'))
        else:
          metadata['tables'][k] = v.replace(']', (',0' * (max_size - curr)) + ']')

  if shared.Settings.SIDE_MODULE:
    for k in metadata['tables'].keys():
      metadata['tables'][k] = metadata['tables'][k].replace('var FUNCTION_TABLE_', 'var SIDE_FUNCTION_TABLE_')


def fixup_functions(funcs, metadata):
  # function table masks
  table_sizes = {}
  for k, v in metadata['tables'].items():
    # undercounts by one, but that is what we want
    table_sizes[k] = str(v.count(','))
    # if shared.Settings.ASSERTIONS >= 2 and table_sizes[k] == 0:
    #   logger.warning('no function pointers with signature ' + k + ', but there is a call, which will abort if it occurs (this can result from undefined behavior, check for compiler warnings on your source files and consider -Werror)'
  funcs = re.sub(r"#FM_(\w+)#", lambda m: table_sizes[m.groups(0)[0]], funcs)

  # fix +float into float.0, if not running js opts
  if not shared.Settings.RUNNING_JS_OPTS:
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


def compiler_glue(metadata, libraries, compiler_engine, temp_files, DEBUG):
  if DEBUG:
    logger.debug('emscript: js compiler glue')
    t = time.time()

  # Settings changes
  i64_funcs = ['i64Add', 'i64Subtract', '__muldi3', '__divdi3', '__udivdi3', '__remdi3', '__uremdi3']
  for i64_func in i64_funcs:
    if i64_func in metadata['declares']:
      shared.Settings.PRECISE_I64_MATH = 2
      break

  # FIXME: do these one by one as normal js lib funcs
  metadata['declares'] = [i64_func for i64_func in metadata['declares'] if i64_func not in ['getHigh32', 'setHigh32']]

  optimize_syscalls(metadata['declares'], DEBUG)
  update_settings_glue(metadata)

  assert not (metadata['simd'] and shared.Settings.WASM), 'SIMD is used, but not supported in WASM mode yet'
  assert not (shared.Settings.SIMD and shared.Settings.WASM), 'SIMD is requested, but not supported in WASM mode yet'

  glue, forwarded_data = compile_settings(compiler_engine, libraries, temp_files)

  if DEBUG:
    logger.debug('  emscript: glue took %s seconds' % (time.time() - t))

  return glue, forwarded_data


def function_tables_and_exports(funcs, metadata, mem_init, glue, forwarded_data, outfile, DEBUG):
  if DEBUG:
    logger.debug('emscript: python processing: function tables and exports')
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  # merge in information from llvm backend

  function_table_data = metadata['tables']

  # merge forwarded data
  shared.Settings.EXPORTED_FUNCTIONS = forwarded_json['EXPORTED_FUNCTIONS']

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  pre = memory_and_global_initializers(pre, metadata, mem_init)
  pre, funcs_js = get_js_funcs(pre, funcs)
  all_exported_functions = get_all_exported_functions(function_table_data)
  all_implemented = get_all_implemented(forwarded_json, metadata)
  check_all_implemented(all_implemented, pre)
  implemented_functions = get_implemented_functions(metadata)
  pre = include_asm_consts(pre, forwarded_json, metadata)
  outfile.write(pre)
  pre = None

  # Move preAsms to their right place
  def move_preasm(m):
    contents = m.groups(0)[0]
    outfile.write(contents + '\n')
    return ''

  if not shared.Settings.BOOTSTRAPPING_STRUCT_INFO and len(funcs_js) > 1:
    funcs_js[1] = re.sub(r'/\* PRE_ASM \*/(.*)\n', move_preasm, funcs_js[1])

  if 'pre' in function_table_data:
    pre_tables = function_table_data['pre']
    del function_table_data['pre']
  else:
    pre_tables = ''

  function_table_sigs = list(function_table_data.keys())

  in_table, debug_tables, function_tables_defs = make_function_tables_defs(
    implemented_functions, all_implemented, function_table_data, metadata)

  exported_implemented_functions = get_exported_implemented_functions(
    all_exported_functions, all_implemented, metadata)

  # List of function signatures of used 'invoke_xxx()' functions in the application
  # For backwards compatibility if one might be using a mismatching Emscripten compiler version, if 'invokeFuncs' is not present in metadata,
  # use the full list of signatures in function table and generate invoke_() functions for all signatures in the program (producing excessive code size)
  # we must also emit the full list if we are emitting code that can be linked later
  if 'invokeFuncs' in metadata and not shared.Settings.LINKABLE:
    invoke_function_names = metadata['invokeFuncs']
  else:
    invoke_function_names = ['invoke_' + x for x in function_table_sigs]

  asm_setup = create_asm_setup(debug_tables, function_table_data, invoke_function_names, metadata)
  basic_funcs = create_basic_funcs(function_table_sigs, invoke_function_names)
  basic_vars = create_basic_vars(exported_implemented_functions, forwarded_json, metadata)

  funcs_js += create_mftCall_funcs(function_table_data)

  exports = create_exports(exported_implemented_functions, in_table, function_table_data, metadata)

  # calculate globals
  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except:
    pass
  if not shared.Settings.RELOCATABLE:
    global_vars = metadata['externs']
  else:
    global_vars = [] # linkable code accesses globals through function calls
  global_funcs = set(key for key, value in forwarded_json['Functions']['libraryFunctions'].items() if value != 2)
  global_funcs = sorted(global_funcs.difference(set(global_vars)).difference(implemented_functions))
  if shared.Settings.RELOCATABLE:
    global_funcs += ['g$' + extern for extern in metadata['externs']]

  # Tracks the set of used (minified) function names in
  # JS symbols imported to asm.js module.
  minified_js_names = MinifiedJsNameGenerator()

  # Converts list of imports ['foo', 'bar', ...] to a dictionary of
  # name mappings in form { 'minified': 'unminified', ... }
  def define_asmjs_import_names(imports):
    if shared.Settings.MINIFY_ASMJS_IMPORT_NAMES:
      return [(minified_js_names.generate(), i) for i in imports]
    else:
      return [(i, i) for i in imports]

  basic_funcs = define_asmjs_import_names(basic_funcs)
  global_funcs = define_asmjs_import_names(global_funcs)
  basic_vars = define_asmjs_import_names(basic_vars)
  global_vars = define_asmjs_import_names(global_vars)

  bg_funcs = basic_funcs + global_funcs
  bg_vars = basic_vars + global_vars
  asm_global_funcs = create_asm_global_funcs(bg_funcs, metadata)
  asm_global_vars = create_asm_global_vars(bg_vars)

  the_global = create_the_global(metadata)
  sending_vars = bg_funcs + bg_vars
  sending = '{ ' + ', '.join(['"' + math_fix(minified) + '": ' + unminified for (minified, unminified) in sending_vars]) + ' }'

  receiving = create_receiving(function_table_data, function_tables_defs,
                               exported_implemented_functions)

  function_tables_impls = make_function_tables_impls(function_table_data)
  final_function_tables = '\n'.join(function_tables_impls) + '\n' + function_tables_defs
  if shared.Settings.EMULATED_FUNCTION_POINTERS:
    final_function_tables = (
      final_function_tables
      .replace("asm['", '')
      .replace("']", '')
      .replace('var SIDE_FUNCTION_TABLE_', 'var FUNCTION_TABLE_')
      .replace('var dynCall_', '//')
    )

  if DEBUG:
    logger.debug('asm text sizes' + str([
      [len(s) for s in funcs_js], len(asm_setup), len(asm_global_vars), len(asm_global_funcs), len(pre_tables),
      len('\n'.join(function_tables_impls)), len(function_tables_defs) + (function_tables_defs.count('\n') * len('  ')),
      len(exports), len(the_global), len(sending), len(receiving)]))
    logger.debug('  emscript: python processing: function tables and exports took %s seconds' % (time.time() - t))

  bundled_args = (funcs_js, asm_setup, the_global, sending, receiving, asm_global_vars,
                  asm_global_funcs, pre_tables, final_function_tables, exports)
  return (post, function_table_data, bundled_args)


def finalize_output(outfile, post, function_table_data, bundled_args, metadata, DEBUG):
  function_table_sigs = function_table_data.keys()
  module = create_module_asmjs(function_table_sigs, metadata, *bundled_args)

  if DEBUG:
    logger.debug('emscript: python processing: finalize')
    t = time.time()

  write_output_file(outfile, post, module)
  module = None

  if DEBUG:
    logger.debug('  emscript: python processing: finalize took %s seconds' % (time.time() - t))

  write_cyberdwarf_data(outfile, metadata)


def create_module_asmjs(function_table_sigs, metadata,
                        funcs_js, asm_setup, the_global, sending, receiving, asm_global_vars,
                        asm_global_funcs, pre_tables, final_function_tables, exports):
  receiving += create_named_globals(metadata)
  runtime_funcs = create_runtime_funcs_asmjs(exports)

  asm_start_pre = create_asm_start_pre(asm_setup, the_global, sending, metadata)
  asm_temp_vars = create_asm_temp_vars()
  asm_runtime_thread_local_vars = create_asm_runtime_thread_local_vars()
  asm_start = asm_start_pre + '\n' + asm_global_vars + asm_temp_vars + asm_runtime_thread_local_vars + '\n' + asm_global_funcs

  temp_float = '  var tempFloat = %s;\n' % ('Math_fround(0)' if provide_fround() else '0.0')
  async_state = '  var asyncState = 0;\n' if shared.Settings.EMTERPRETIFY_ASYNC else ''
  f0_fround = '  const f0 = Math_fround(0);\n' if provide_fround() else ''

  replace_memory = create_replace_memory()

  start_funcs_marker = '\n// EMSCRIPTEN_START_FUNCS\n'

  asm_end = create_asm_end(exports)

  module = [
    asm_start,
    temp_float,
    async_state,
    f0_fround,
    replace_memory,
    start_funcs_marker
  ] + runtime_funcs + funcs_js + [
    '\n  ',
    pre_tables, final_function_tables, asm_end,
    '\n', receiving, ';\n'
  ]

  if shared.Settings.SIDE_MODULE:
    module.append('''
parentModule['registerFunctions'](%s, Module);
''' % str([str(f) for f in function_table_sigs]))

  return module


def write_output_file(outfile, post, module):
  for i in range(len(module)): # do this loop carefully to save memory
    module[i] = normalize_line_endings(module[i])
    outfile.write(module[i])

  post = normalize_line_endings(post)
  outfile.write(post)


def write_cyberdwarf_data(outfile, metadata):
  if not shared.Settings.CYBERDWARF:
    return

  assert('cyberdwarf_data' in metadata)
  cd_file_name = outfile.name + ".cd"
  with open(cd_file_name, 'w') as f:
    json.dump({'cyberdwarf': metadata['cyberdwarf_data']}, f)


def create_backend_args(infile, temp_js):
  """Create args for asm.js backend from settings dict"""
  args = [
    shared.LLVM_COMPILER, infile, '-march=js', '-filetype=asm', '-o', temp_js,
    '-emscripten-stack-size=%d' % shared.Settings.TOTAL_STACK,
    '-O%s' % shared.Settings.OPT_LEVEL,
  ]
  if shared.Settings.PRECISE_F32:
    args += ['-emscripten-precise-f32']
  if shared.Settings.USE_PTHREADS:
    args += ['-emscripten-enable-pthreads']
  if shared.Settings.WARN_UNALIGNED:
    args += ['-emscripten-warn-unaligned']
  if shared.Settings.RESERVED_FUNCTION_POINTERS > 0:
    args += ['-emscripten-reserved-function-pointers=%d' % shared.Settings.RESERVED_FUNCTION_POINTERS]
  if shared.Settings.ASSERTIONS > 0:
    args += ['-emscripten-assertions=%d' % shared.Settings.ASSERTIONS]
  if shared.Settings.ALIASING_FUNCTION_POINTERS == 0:
    args += ['-emscripten-no-aliasing-function-pointers']
  if shared.Settings.EMULATED_FUNCTION_POINTERS:
    args += ['-emscripten-emulated-function-pointers']
  if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS:
    args += ['-emscripten-emulate-function-pointer-casts']
  if shared.Settings.RELOCATABLE:
    args += ['-emscripten-relocatable']
    args += ['-emscripten-global-base=0']
  elif shared.Settings.GLOBAL_BASE >= 0:
    args += ['-emscripten-global-base=%d' % shared.Settings.GLOBAL_BASE]
  if shared.Settings.SIDE_MODULE:
    args += ['-emscripten-side-module']
  if shared.Settings.LEGALIZE_JS_FFI != 1:
    args += ['-emscripten-legalize-javascript-ffi=0']
  if shared.Settings.DISABLE_EXCEPTION_CATCHING != 1:
    args += ['-enable-emscripten-cpp-exceptions']
    if shared.Settings.DISABLE_EXCEPTION_CATCHING == 2:
      args += ['-emscripten-cpp-exceptions-whitelist=' + ','.join(shared.Settings.EXCEPTION_CATCHING_WHITELIST or ['fake'])]
  if shared.Settings.ASYNCIFY:
    args += ['-emscripten-asyncify']
    args += ['-emscripten-asyncify-functions=' + ','.join(shared.Settings.ASYNCIFY_FUNCTIONS)]
    args += ['-emscripten-asyncify-whitelist=' + ','.join(shared.Settings.ASYNCIFY_WHITELIST)]
  if not shared.Settings.EXIT_RUNTIME:
    args += ['-emscripten-no-exit-runtime']
  if shared.Settings.WORKAROUND_IOS_9_RIGHT_SHIFT_BUG:
    args += ['-emscripten-asmjs-work-around-ios-9-right-shift-bug']
  if shared.Settings.WASM:
    args += ['-emscripten-wasm']
    if shared.Building.is_wasm_only():
      args += ['-emscripten-only-wasm']
  if shared.Settings.CYBERDWARF:
    args += ['-enable-cyberdwarf']
  return args


def optimize_syscalls(declares, DEBUG):
  """Disables filesystem if only a limited subset of syscalls is used.

  Our syscalls are static, and so if we see a very limited set of them - in particular,
  no open() syscall and just simple writing - then we don't need full filesystem support.
  If FORCE_FILESYSTEM is set, we can't do this. We also don't do it if INCLUDE_FULL_LIBRARY, since
  not including the filesystem would mean not including the full JS libraries, and the same for
  MAIN_MODULE since a side module might need the filesystem.
  """
  relevant_settings = ['FORCE_FILESYSTEM', 'INCLUDE_FULL_LIBRARY', 'MAIN_MODULE']
  if any(shared.Settings[s] for s in relevant_settings):
    return

  syscall_prefix = '__syscall'
  syscall_numbers = [d[len(syscall_prefix):] for d in declares if d.startswith(syscall_prefix)]
  syscalls = [int(s) for s in syscall_numbers if is_int(s)]
  if set(syscalls).issubset(set([6, 54, 140, 146])): # close, ioctl, llseek, writev
    if DEBUG:
      logger.debug('very limited syscalls (%s) so disabling full filesystem support', ', '.join(str(s) for s in syscalls))
    shared.Settings.FILESYSTEM = 0


def is_int(x):
  try:
    int(x)
    return True
  except:
    return False


def update_settings_glue(metadata):
  if shared.Settings.CYBERDWARF:
    shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.append("cyberdwarf_Debugger")
    shared.Settings.EXPORTED_FUNCTIONS.append("cyberdwarf_Debugger")

  # Integrate info from backend
  if shared.Settings.SIDE_MODULE:
    shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = [] # we don't need any JS library contents in side modules

  if metadata.get('cantValidate') and shared.Settings.ASM_JS != 2:
    logger.warning('disabling asm.js validation due to use of non-supported features: ' + metadata['cantValidate'])
    shared.Settings.ASM_JS = 2

  all_funcs = shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + [shared.JS.to_nice_ident(d) for d in metadata['declares']]
  implemented_funcs = [x[1:] for x in metadata['implementedFunctions']]
  shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = sorted(set(all_funcs).difference(implemented_funcs))
  shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [x[1:] for x in metadata['externs']]

  if metadata['simd']:
    shared.Settings.SIMD = 1
    if shared.Settings.ASM_JS != 2:
      logger.warning('disabling asm.js validation due to use of SIMD')
      shared.Settings.ASM_JS = 2

  shared.Settings.MAX_GLOBAL_ALIGN = metadata['maxGlobalAlign']
  shared.Settings.IMPLEMENTED_FUNCTIONS = metadata['implementedFunctions']

  # addFunction support for Wasm backend
  if shared.Settings.WASM_BACKEND and shared.Settings.RESERVED_FUNCTION_POINTERS > 0:
    start_index = metadata['jsCallStartIndex']
    # e.g. jsCallFunctionType ['v', 'ii'] -> sig2order{'v': 0, 'ii': 1}
    sig2order = {sig: i for i, sig in enumerate(metadata['jsCallFuncType'])}
    # Index in the Wasm function table in which jsCall thunk function starts
    shared.Settings.JSCALL_START_INDEX = start_index
    shared.Settings.JSCALL_SIG_ORDER = sig2order

  # Extract the list of function signatures that MAIN_THREAD_EM_ASM blocks in
  # the compiled code have, each signature will need a proxy function invoker
  # generated for it.
  def read_proxied_function_signatures(asmConsts):
    proxied_function_signatures = set()
    for _, sigs, proxying_types in asmConsts.values():
      for sig, proxying_type in zip(sigs, proxying_types):
        if proxying_type == 'sync_on_main_thread_':
          proxied_function_signatures.add(sig + '_sync')
        elif proxying_type == 'async_on_main_thread_':
          proxied_function_signatures.add(sig + '_async')
    return list(proxied_function_signatures)

  # Proxying EM_ASM calls is not yet implemented in Wasm backend
  if not shared.Settings.WASM_BACKEND:
    shared.Settings.PROXIED_FUNCTION_SIGNATURES = read_proxied_function_signatures(metadata['asmConsts'])


def compile_settings(compiler_engine, libraries, temp_files):
  # Save settings to a file to work around v8 issue 1579
  with temp_files.get_file('.txt') as settings_file:
    with open(settings_file, 'w') as s:
      json.dump(shared.Settings.to_dict(), s, sort_keys=True)

    # Call js compiler
    env = os.environ.copy()
    env['EMCC_BUILD_DIR'] = os.getcwd()
    out = jsrun.run_js_tool(path_from_root('src', 'compiler.js'), compiler_engine,
                            [settings_file] + libraries, stdout=subprocess.PIPE, stderr=STDERR_FILE,
                            cwd=path_from_root('src'), env=env)
  assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
  glue, forwarded_data = out.split('//FORWARDED_DATA:')
  return glue, forwarded_data


def memory_and_global_initializers(pre, metadata, mem_init):
  global_initializers = ', '.join('{ func: function() { %s() } }' % i for i in metadata['initializers'])

  if shared.Settings.SIMD == 1:
    pre = open(path_from_root(os.path.join('src', 'ecmascript_simd.js'))).read() + '\n\n' + pre

  staticbump = metadata['staticBump']
  while staticbump % 16 != 0:
    staticbump += 1
  pthread = ''
  if shared.Settings.USE_PTHREADS:
    pthread = 'if (!ENVIRONMENT_IS_PTHREAD)'
  pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''\
STATICTOP = STATIC_BASE + {staticbump};
/* global initializers */ {pthread} __ATINIT__.push({global_initializers});
{mem_init}'''.format(staticbump=staticbump,
                     pthread=pthread,
                     global_initializers=global_initializers,
                     mem_init=mem_init))

  if shared.Settings.SIDE_MODULE:
    pre = pre.replace('GLOBAL_BASE', 'gb')
  if shared.Settings.SIDE_MODULE or shared.Settings.WASM:
    pre = pre.replace('{{{ STATIC_BUMP }}}', str(staticbump))

  return pre


def get_js_funcs(pre, funcs):
  funcs_js = [funcs]
  parts = pre.split('// ASM_LIBRARY FUNCTIONS\n')
  if len(parts) > 1:
    pre = parts[0]
    funcs_js.append(parts[1])
  return pre, funcs_js


def get_all_exported_functions(function_table_data):
  # both asm.js and otherwise
  all_exported_functions = set(shared.expand_response(shared.Settings.EXPORTED_FUNCTIONS))

  # additional functions to export from asm, if they are implemented
  for additional_export in shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE:
    all_exported_functions.add('_' + additional_export)
  if shared.Settings.EXPORT_FUNCTION_TABLES:
    for table in function_table_data.values():
      for func in table.split('[')[1].split(']')[0].split(','):
        if func[0] == '_':
          all_exported_functions.add(func)
  return all_exported_functions


def get_all_implemented(forwarded_json, metadata):
  return metadata['implementedFunctions'] + list(forwarded_json['Functions']['implementedFunctions'].keys()) # XXX perf?


# Return the list of original exports, for error reporting. It may
# be a response file, in which case, load it
def get_original_exported_functions():
  ret = shared.Settings.ORIGINAL_EXPORTED_FUNCTIONS
  if ret and ret[0] == '@':
    ret = json.loads(open(ret[1:]).read())
  return ret


def check_all_implemented(all_implemented, pre):
  for requested in get_original_exported_functions():
    if not is_already_implemented(requested, pre, all_implemented):
      # could be a js library func
      if shared.Settings.ERROR_ON_UNDEFINED_SYMBOLS:
        exit_with_error('undefined exported function: "%s"', requested)
      elif shared.Settings.WARN_ON_UNDEFINED_SYMBOLS:
        logger.warning('undefined exported function: "%s"', requested)


def is_already_implemented(requested, pre, all_implemented):
  is_implemented = requested in all_implemented
  # special-case malloc, EXPORTED by default for internal use, but we bake in a
  # trivial allocator and warn at runtime if used in ASSERTIONS
  is_exception = requested == '_malloc'
  in_pre = ('function ' + asstr(requested)) in pre
  return is_implemented or is_exception or in_pre


def get_exported_implemented_functions(all_exported_functions, all_implemented, metadata):
  funcs = set(metadata['exports'])
  export_bindings = shared.Settings.EXPORT_BINDINGS
  export_all = shared.Settings.EXPORT_ALL
  for key in all_implemented:
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      funcs.add(key)

  funcs = list(funcs) + metadata['initializers']
  if not shared.Settings.ONLY_MY_CODE:
    funcs.append('runPostSets')
    if shared.Settings.ALLOW_MEMORY_GROWTH:
      funcs.append('_emscripten_replace_memory')
    if not shared.Settings.SIDE_MODULE:
      funcs += ['stackAlloc', 'stackSave', 'stackRestore', 'establishStackSpace']
    if shared.Settings.SAFE_HEAP:
      funcs += ['setDynamicTop']
    if not (shared.Settings.WASM and shared.Settings.SIDE_MODULE):
      funcs += ['setThrew']
    if shared.Settings.EMTERPRETIFY:
      funcs += ['emterpret']
      if shared.Settings.EMTERPRETIFY_ASYNC:
        funcs += ['setAsyncState', 'emtStackSave', 'emtStackRestore', 'getEmtStackMax', 'setEmtStackMax']
    if shared.Settings.ASYNCIFY and need_asyncify(funcs):
      funcs += ['setAsync']

  return sorted(set(funcs))


def get_implemented_functions(metadata):
  return set(metadata['implementedFunctions'])


def proxy_debug_print(call_type):
  if shared.Settings.PTHREADS_DEBUG:
    if call_type == 'sync_on_main_thread_':
      return 'warnOnce("sync proxying function " + code);'
    if call_type == 'async_on_main_thread_':
      return 'warnOnce("async proxying function " + code);'
  return ''


def include_asm_consts(pre, forwarded_json, metadata):
  if shared.Settings.WASM and shared.Settings.SIDE_MODULE:
    if metadata['asmConsts']:
      exit_with_error('EM_ASM is not yet supported in shared wasm module (it cannot be stored in the wasm itself, need some solution)')

  asm_consts, all_sigs = all_asm_consts(metadata)
  asm_const_funcs = []
  for sig, call_type in all_sigs:
    if 'j' in sig:
      exit_with_error('emscript: EM_ASM should not receive i64s as inputs, they are not valid in JS')
    if '_emscripten_asm_const_' + call_type + sig in forwarded_json['Functions']['libraryFunctions']:
      continue # Only one invoker needs to be emitted for each ASM_CONST (signature x call_type) item
    forwarded_json['Functions']['libraryFunctions']['_emscripten_asm_const_' + call_type + sig] = 1
    args = ['a%d' % i for i in range(len(sig) - 1)]
    all_args = ['code'] + args
    proxy_function = ''
    if shared.Settings.USE_PTHREADS:
      if call_type == 'sync_on_main_thread_':
        proxy_function = '_emscripten_sync_run_in_browser_thread_' + sig
      elif call_type == 'async_on_main_thread_':
        proxy_function = '_emscripten_async_run_in_browser_thread_' + sig

    # In proxied function calls, positive integers 1, 2, 3, ... denote pointers
    # to regular C compiled functions. Negative integers -1, -2, -3, ... denote
    # indices to EM_ASM() blocks, so remap the EM_ASM() indices from 0, 1, 2,
    # ... over to the negative integers starting at -1.
    proxy_args = '-1 - ' + ','.join(all_args)

    pre_asm_const = ''

    if proxy_function:
      pre_asm_const += '  if (ENVIRONMENT_IS_PTHREAD) { ' + proxy_debug_print(call_type) + 'return ' + proxy_function + '(' + proxy_args + '); } \n'

    if shared.Settings.EMTERPRETIFY_ASYNC and shared.Settings.ASSERTIONS:
      # we cannot have an EM_ASM on the stack when saving/loading
      pre_asm_const += "  assert(typeof EmterpreterAsync !== 'object' || EmterpreterAsync.state !== 2, 'cannot have an EM_ASM on the stack when emterpreter pauses/resumes - the JS is not emterpreted, so we would end up running it again from the start');\n"

    asm_const_funcs.append(r'''
function _emscripten_asm_const_%s(%s) {
%s  return ASM_CONSTS[code](%s);
}''' % (call_type + asstr(sig), ', '.join(all_args), pre_asm_const, ', '.join(args)))

  asm_consts_text = '\nvar ASM_CONSTS = [' + ',\n '.join(asm_consts) + '];\n'
  asm_funcs_text = '\n'.join(asm_const_funcs) + '\n'

  em_js_funcs = create_em_js(forwarded_json, metadata)
  em_js_text = '\n'.join(em_js_funcs) + '\n'

  body_marker = '// === Body ==='
  return pre.replace(body_marker, body_marker + '\n' + asm_consts_text + asstr(asm_funcs_text) + em_js_text)


# Test if the parentheses at body[openIdx] and body[closeIdx] are a match to
# each other.
def parentheses_match(body, openIdx, closeIdx):
  if closeIdx < 0:
    closeIdx += len(body)
  count = 1
  for i in range(openIdx + 1, closeIdx + 1):
    if body[i] == body[openIdx]:
      count += 1
    elif body[i] == body[closeIdx]:
      count -= 1
      if count <= 0:
        return i == closeIdx
  return False


def trim_asm_const_body(body):
  body = body.strip()
  orig = None
  while orig != body:
    orig = body
    if len(body) > 1 and body[0] == '"' and body[-1] == '"':
      body = body[1:-1].replace('\\"', '"').strip()
    if len(body) > 1 and body[0] == '{' and body[-1] == '}' and parentheses_match(body, 0, -1):
      body = body[1:-1].strip()
    if len(body) > 1 and body[0] == '(' and body[-1] == ')' and parentheses_match(body, 0, -1):
      body = body[1:-1].strip()
  return body


def all_asm_consts(metadata):
  asm_consts = [0] * len(metadata['asmConsts'])
  all_sigs = []
  for k, v in metadata['asmConsts'].items():
    const, sigs, call_types = v
    const = asstr(const)
    const = trim_asm_const_body(const)
    const = '{ ' + const + ' }'
    args = []
    arity = max(len(s) for s in sigs) - 1
    for i in range(arity):
      args.append('$' + str(i))
    const = 'function(' + ', '.join(args) + ') ' + const
    asm_consts[int(k)] = const
    assert(len(sigs) == len(call_types))
    for sig, call_type in zip(sigs, call_types):
      all_sigs.append((sig, call_type))
  return asm_consts, all_sigs


def unfloat(s):
  """lower float to double for ffis"""
  return 'd' if s == 'f' else s


def make_function_tables_defs(implemented_functions, all_implemented, function_table_data, metadata):
  class Counter(object):
    next_bad_item = 0
    next_item = 0
    pre = []

  in_table = set()
  debug_tables = {}

  def make_params(sig):
    return ','.join('p%d' % p for p in range(len(sig) - 1))

  def make_coerced_params(sig):
    return ','.join(shared.JS.make_coercion('p%d', unfloat(sig[p + 1])) % p for p in range(len(sig) - 1))

  def make_coercions(sig):
    return ';'.join('p%d = %s' % (p, shared.JS.make_coercion('p%d' % p, sig[p + 1])) for p in range(len(sig) - 1)) + ';'

  # when emulating function pointer casts, we need to know what is the target of each pointer
  if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS and not shared.Settings.WASM:
    function_pointer_targets = {}
    for sig, table in function_table_data.items():
      start = table.index('[')
      end = table.rindex(']')
      body = table[start + 1:end].split(',')
      for i, parsed in enumerate(x.strip() for x in body):
        if parsed != '0':
          assert i not in function_pointer_targets
          function_pointer_targets[i] = [sig, str(parsed)]

  def make_table(sig, raw):
    if '[]' in raw:
      return ('', '') # empty table
    params = make_params(sig)
    coerced_params = make_coerced_params(sig)
    coercions = make_coercions(sig)

    def make_bad(target=None):
      i = Counter.next_bad_item
      Counter.next_bad_item += 1
      if target is None:
        target = i
      name = 'b' + str(i)
      if not shared.Settings.ASSERTIONS:
        code = 'abort(%s);' % target
      else:
        code = 'nullFunc_' + sig + '(%d);' % target
      if sig[0] != 'v':
        code += 'return %s' % shared.JS.make_initializer(sig[0]) + ';'
      return name, make_func(name, code, params, coercions)

    bad, bad_func = make_bad() # the default bad func
    if shared.Settings.ASSERTIONS <= 1:
      Counter.pre = [bad_func]
    else:
      Counter.pre = []
    start = raw.index('[')
    end = raw.rindex(']')
    body = raw[start + 1:end].split(',')
    if shared.Settings.EMULATED_FUNCTION_POINTERS:
      def receive(item):
        if item == '0':
          return item
        if item not in all_implemented:
          # this is not implemented; it would normally be wrapped, but with emulation, we just use it directly outside
          return item
        in_table.add(item)
        return "asm['" + item + "']"

      body = [receive(b) for b in body]
    for j in range(shared.Settings.RESERVED_FUNCTION_POINTERS):
      curr = 'jsCall_%s_%s' % (sig, j)
      body[1 + j] = curr
      implemented_functions.add(curr)
    Counter.next_item = 0

    def fix_item(item):
      j = Counter.next_item
      Counter.next_item += 1
      newline = Counter.next_item % 30 == 29
      if item == '0':
        # emulate all non-null pointer calls, if asked to
        if j > 0 and shared.Settings.EMULATE_FUNCTION_POINTER_CASTS and not shared.Settings.WASM and j in function_pointer_targets:
          proper_sig, proper_target = function_pointer_targets[j]
          if shared.Settings.EMULATED_FUNCTION_POINTERS:
            if proper_target in all_implemented:
              proper_target = "asm['" + proper_target + "']"

          def make_emulated_param(i):
            if i >= len(sig):
              return shared.JS.make_initializer(proper_sig[i]) # extra param, just send a zero
            return shared.JS.make_coercion('p%d' % (i - 1), proper_sig[i], convert_from=sig[i])

          proper_code = proper_target + '(' + ','.join([make_emulated_param(i + 1) for i in range(len(proper_sig) - 1)]) + ')'
          if proper_sig[0] != 'v':
            # proper sig has a return, which the wrapper may or may not use
            proper_code = shared.JS.make_coercion(proper_code, proper_sig[0])
            if proper_sig[0] != sig[0]:
              # first coercion ensured we call the target ok; this one ensures we return the right type in the wrapper
              proper_code = shared.JS.make_coercion(proper_code, sig[0], convert_from=proper_sig[0])
            if sig[0] != 'v':
              proper_code = 'return ' + proper_code
          else:
            # proper sig has no return, we may need a fake return
            if sig[0] != 'v':
              proper_code = 'return ' + shared.JS.make_initializer(sig[0])
          name = 'fpemu_%s_%d' % (sig, j)
          wrapper = make_func(name, proper_code, params, coercions)
          Counter.pre.append(wrapper)
          return name if not newline else (name + '\n')

        if shared.Settings.ASSERTIONS <= 1:
          return bad if not newline else (bad + '\n')

        specific_bad, specific_bad_func = make_bad(j)
        Counter.pre.append(specific_bad_func)
        return specific_bad if not newline else (specific_bad + '\n')

      clean_item = item.replace("asm['", '').replace("']", '')
      # when emulating function pointers, we don't need wrappers
      # but if relocating, then we also have the copies in-module, and do
      # in wasm we never need wrappers though
      if clean_item not in implemented_functions and not (shared.Settings.EMULATED_FUNCTION_POINTERS and not shared.Settings.RELOCATABLE) and not shared.Settings.WASM:
        # this is imported into asm, we must wrap it
        call_ident = clean_item
        if call_ident in metadata['redirects']:
          call_ident = metadata['redirects'][call_ident]
        if not call_ident.startswith('_') and not call_ident.startswith('Math_'):
          call_ident = '_' + call_ident
        code = call_ident + '(' + coerced_params + ')'
        if sig[0] != 'v':
          # ffis cannot return float
          if sig[0] == 'f':
            code = '+' + code
          code = 'return ' + shared.JS.make_coercion(code, sig[0])
        code += ';'
        Counter.pre.append(make_func(clean_item + '__wrapper', code, params, coercions))
        assert not sig == 'X', 'must know the signature in order to create a wrapper for "%s" (TODO for shared wasm modules)' % item
        return clean_item + '__wrapper'
      return item if not newline else (item + '\n')

    if shared.Settings.ASSERTIONS >= 2:
      debug_tables[sig] = body
    body = ','.join(fix_item(b) for b in body)
    return ('\n'.join(Counter.pre), ''.join([raw[:start + 1], body, raw[end:]]))

  infos = [make_table(sig, raw) for sig, raw in function_table_data.items()]
  Counter.pre = []

  function_tables_defs = '\n'.join([info[0] for info in infos]) + '\n'
  function_tables_defs += '\n// EMSCRIPTEN_END_FUNCS\n'
  function_tables_defs += '\n'.join([info[1] for info in infos])
  return in_table, debug_tables, function_tables_defs


def make_func(name, code, params, coercions):
  return 'function %s(%s) {\n %s %s\n}' % (name, params, coercions, code)


def math_fix(g):
  return g if not g.startswith('Math_') else g.split('_')[1]


def make_function_tables_impls(function_table_data):
  function_tables_impls = []
  for sig, table in function_table_data.items():
    args = ','.join(['a' + str(i) for i in range(1, len(sig))])
    arg_coercions = ' '.join(['a' + str(i) + '=' + shared.JS.make_coercion('a' + str(i), sig[i]) + ';' for i in range(1, len(sig))])
    coerced_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i]) for i in range(1, len(sig))])
    sig_mask = str(table.count(','))
    ret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('FUNCTION_TABLE_%s[index&%s](%s)' % (sig, sig_mask, coerced_args), sig[0])
    if not shared.Settings.EMULATED_FUNCTION_POINTERS:
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

    ffi_args = ','.join([shared.JS.make_coercion('a' + str(i), sig[i], ffi_arg=True) for i in range(1, len(sig))])
    for i in range(shared.Settings.RESERVED_FUNCTION_POINTERS):
      jsret = ('return ' if sig[0] != 'v' else '') + shared.JS.make_coercion('jsCall_%s(%d%s%s)' % (sig, i, ',' if ffi_args else '', ffi_args), sig[0], ffi_result=True)
      function_tables_impls.append('''
function jsCall_%s_%s(%s) {
  %s
  %s;
}

''' % (sig, i, args, arg_coercions, jsret))
  return function_tables_impls


def create_mftCall_funcs(function_table_data):
  if not shared.Settings.EMULATED_FUNCTION_POINTERS:
    return []
  if shared.Settings.WASM or not shared.Settings.RELOCATABLE:
    return []

  mftCall_funcs = []
  # in wasm, emulated function pointers are just simple table calls
  for sig, table in function_table_data.items():
    return_type, sig_args = sig[0], sig[1:]
    num_args = len(sig_args)
    params = ','.join(['ptr'] + ['p%d' % i for i in range(num_args)])
    coerced_params = ','.join([shared.JS.make_coercion('ptr', 'i')] + [shared.JS.make_coercion('p%d' % i, unfloat(sig_args[i])) for i in range(num_args)])
    coercions = ';'.join(['ptr = ptr | 0'] + ['p%d = %s' % (i, shared.JS.make_coercion('p%d' % i, unfloat(sig_args[i]))) for i in range(num_args)]) + ';'
    mini_coerced_params = ','.join([shared.JS.make_coercion('p%d' % i, sig_args[i]) for i in range(num_args)])
    maybe_return = '' if return_type == 'v' else 'return'
    final_return = maybe_return + ' ' + shared.JS.make_coercion('ftCall_' + sig + '(' + coerced_params + ')', unfloat(return_type)) + ';'
    if shared.Settings.EMULATED_FUNCTION_POINTERS == 1:
      body = final_return
    else:
      sig_mask = str(table.count(','))
      body = ('if (((ptr|0) >= (fb|0)) & ((ptr|0) < (fb + ' + sig_mask + ' | 0))) { ' + maybe_return + ' ' +
              shared.JS.make_coercion(
                'FUNCTION_TABLE_' + sig + '[(ptr-fb)&' + sig_mask + '](' +
                mini_coerced_params + ')', return_type, ffi_arg=True
              ) + '; ' + ('return;' if return_type == 'v' else '') + ' }' + final_return)
    mftCall_funcs.append(make_func('mftCall_' + sig, body, params, coercions) + '\n')
  return mftCall_funcs


def get_function_pointer_error(sig, function_table_sigs):
  if shared.Settings.ASSERTIONS <= 1:
    extra = ' err("Build with ASSERTIONS=2 for more info.");'
    pointer = ' '
  else:
    pointer = ' \'" + x + "\' '
    extra = ' err("This pointer might make sense in another type signature: '
    # sort signatures, attempting to show most likely related ones first
    sigs = list(function_table_sigs)
    sigs.sort(key=signature_sort_key(sig))
    for other in sigs:
      if other != sig:
        extra += other + ': " + debug_table_' + other + '[x] + "  '
    extra += '"); '
  return 'err("Invalid function pointer' + pointer + 'called with signature \'' + sig + '\'. ' + \
         'Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? ' + \
         'Or calling a function with an incorrect type, which will fail? ' + \
         '(it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)' + \
         '"); ' + extra


def signature_sort_key(sig):
  def closure(other):
    ret = 0
    minlen = min(len(other), len(sig))
    maxlen = min(len(other), len(sig))
    if other.startswith(sig) or sig.startswith(other):
      ret -= 1000 # prioritize prefixes, could be dropped params
    ret -= 133 * difflib.SequenceMatcher(a=other, b=sig).ratio() # prioritize on diff similarity
    ret += 15 * abs(len(other) - len(sig)) / float(maxlen) # deprioritize the bigger the length difference is
    for i in range(minlen):
      if other[i] == sig[i]:
        ret -= 5 / float(maxlen) # prioritize on identically-placed params
    ret += 20 * len(other) # deprioritize on length
    return ret
  return closure


def create_asm_global_funcs(bg_funcs, metadata):
  maths = ['Math.' + func for func in ['floor', 'abs', 'sqrt', 'pow', 'cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'exp', 'log', 'ceil', 'imul', 'min', 'max', 'clz32']]
  if provide_fround():
    maths += ['Math.fround']

  asm_global_funcs = ''.join(['  var ' + g.replace('.', '_') + '=global' + access_quote(g) + ';\n' for g in maths])
  asm_global_funcs += ''.join(['  var ' + unminified + '=env' + access_quote(math_fix(minified)) + ';\n' for (minified, unminified) in bg_funcs])
  asm_global_funcs += global_simd_funcs(access_quote, metadata)
  if shared.Settings.USE_PTHREADS:
    asm_global_funcs += ''.join(['  var Atomics_' + ty + '=global' + access_quote('Atomics') + access_quote(ty) + ';\n' for ty in ['load', 'store', 'exchange', 'compareExchange', 'add', 'sub', 'and', 'or', 'xor']])
  return asm_global_funcs


def create_asm_global_vars(bg_vars):
  asm_global_vars = ''.join(['  var ' + unminified + '=env' + access_quote(minified) + '|0;\n' for (minified, unminified) in bg_vars])
  if shared.Settings.WASM and shared.Settings.SIDE_MODULE:
    # wasm side modules internally define their stack, these are set at module startup time
    asm_global_vars += '\n  var STACKTOP = 0, STACK_MAX = 0;\n'

  return asm_global_vars


def global_simd_funcs(access_quote, metadata):
  # Always import SIMD when building with -s SIMD=1, since in that mode memcpy is SIMD optimized.
  if not (metadata['simd'] or shared.Settings.SIMD):
    return ''

  def string_contains_any(s, str_list):
    return any(sub in s for sub in str_list)

  nonexisting_simd_symbols = ['Int8x16_fromInt8x16', 'Uint8x16_fromUint8x16', 'Int16x8_fromInt16x8', 'Uint16x8_fromUint16x8', 'Int32x4_fromInt32x4', 'Uint32x4_fromUint32x4', 'Float32x4_fromFloat32x4', 'Float64x2_fromFloat64x2']
  nonexisting_simd_symbols += ['Int32x4_addSaturate', 'Int32x4_subSaturate', 'Uint32x4_addSaturate', 'Uint32x4_subSaturate']
  nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8', 'Float64x2'] for y in ['load2', 'store2']]
  nonexisting_simd_symbols += [(x + '_' + y) for x in ['Int8x16', 'Uint8x16', 'Int16x8', 'Uint16x8'] for y in ['load1', 'store1']]

  simd = make_simd_types(metadata)

  simd_func_text = ''
  simd_func_text += ''.join(['  var SIMD_' + ty + '=global' + access_quote('SIMD') + access_quote(ty) + ';\n' for ty in simd['types']])

  def generate_symbols(types, funcs):
    symbols = ['  var SIMD_' + ty + '_' + g + '=SIMD_' + ty + access_quote(g) + ';\n' for ty in types for g in funcs]
    symbols = [x for x in symbols if not string_contains_any(x, nonexisting_simd_symbols)]
    return ''.join(symbols)

  simd_func_text += generate_symbols(simd['int_types'], simd['int_funcs'])
  simd_func_text += generate_symbols(simd['float_types'], simd['float_funcs'])
  simd_func_text += generate_symbols(simd['bool_types'], simd['bool_funcs'])

  # SIMD conversions (not bitcasts) between same lane sizes:
  def add_simd_cast(dst, src):
    return '  var SIMD_' + dst + '_from' + src + '=SIMD_' + dst + '.from' + src + ';\n'

  def add_simd_casts(t1, t2):
    return add_simd_cast(t1, t2) + add_simd_cast(t2, t1)

  # Bug: Skip importing conversions for int<->uint for now, they don't validate
  # as asm.js. https://bugzilla.mozilla.org/show_bug.cgi?id=1313512
  # This is not an issue when building SSEx code, because it doesn't use these.
  # (but it will be an issue if using SIMD.js intrinsics from vector.h to
  # explicitly call these)
  # if metadata['simdInt8x16'] and metadata['simdUint8x16']:
  #   simd_func_text += add_simd_casts('Int8x16', 'Uint8x16')
  # if metadata['simdInt16x8'] and metadata['simdUint16x8']:
  #   simd_func_text += add_simd_casts('Int16x8', 'Uint16x8')
  # if metadata['simdInt32x4'] and metadata['simdUint32x4']:
  #   simd_func_text += add_simd_casts('Int32x4', 'Uint32x4')

  if metadata['simdInt32x4'] and metadata['simdFloat32x4']:
    simd_func_text += add_simd_casts('Int32x4', 'Float32x4')
  if metadata['simdUint32x4'] and metadata['simdFloat32x4']:
    simd_func_text += add_simd_casts('Uint32x4', 'Float32x4')
  if metadata['simdInt32x4'] and metadata['simdFloat64x2']:
    simd_func_text += add_simd_cast('Int32x4', 'Float64x2') # Unofficial, needed for emscripten_int32x4_fromFloat64x2
  if metadata['simdUint32x4'] and metadata['simdFloat64x2']:
    simd_func_text += add_simd_cast('Uint32x4', 'Float64x2') # Unofficial, needed for emscripten_uint32x4_fromFloat64x2

  # Unofficial, Bool64x2 does not yet exist, but needed for Float64x2 comparisons.
  if metadata['simdFloat64x2']:
    simd_func_text += '  var SIMD_Int32x4_fromBool64x2Bits = global.SIMD.Int32x4.fromBool64x2Bits;\n'
  return simd_func_text


def make_simd_types(metadata):
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
  if metadata['simdInt32x4'] or shared.Settings.SIMD:
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


def asm_safe_heap():
  """optimized safe heap in asm, when we can"""
  return shared.Settings.SAFE_HEAP and not shared.Settings.SAFE_HEAP_LOG and not shared.Settings.RELOCATABLE


def provide_fround():
  return shared.Settings.PRECISE_F32 or shared.Settings.SIMD


def create_asm_setup(debug_tables, function_table_data, invoke_function_names, metadata):
  function_table_sigs = function_table_data.keys()

  asm_setup = ''
  if shared.Settings.ASSERTIONS >= 2:
    for sig in function_table_data:
      # if the table is empty, debug_tables will not contain it
      body = debug_tables.get(sig, [])
      asm_setup += '\nvar debug_table_' + sig + ' = ' + json.dumps(body) + ';'

  if shared.Settings.ASSERTIONS:
    for sig in function_table_sigs:
      asm_setup += '\nfunction nullFunc_' + sig + '(x) { ' + get_function_pointer_error(sig, function_table_sigs) + 'abort(x) }\n'

  if shared.Settings.WASM:
    def table_size(table):
      table_contents = table[table.index('[') + 1: table.index(']')]
      if len(table_contents) == 0: # empty table
        return 0
      return table_contents.count(',') + 1

    table_total_size = sum(table_size(s) for s in function_table_data.values())
    asm_setup += "\nModule['wasmTableSize'] = %d;\n" % table_total_size
    if not shared.Settings.EMULATED_FUNCTION_POINTERS:
      asm_setup += "\nModule['wasmMaxTableSize'] = %d;\n" % table_total_size

  if shared.Settings.RELOCATABLE:
    if not shared.Settings.SIDE_MODULE:
      asm_setup += 'var gb = GLOBAL_BASE, fb = 0;\n'
    side = 'parent' if shared.Settings.SIDE_MODULE else ''

    def check(extern):
      if shared.Settings.ASSERTIONS:
        return ('assert(' + side + 'Module["' + extern + '"], "external function \'' + extern +
                '\' is missing. perhaps a side module was not linked in? if this symbol was expected to arrive '
                'from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");')
      return ''

    for extern in metadata['externs']:
      asm_setup += 'var g$' + extern + ' = function() { ' + check(extern) + ' return ' + side + 'Module["' + extern + '"] };\n'

  asm_setup += create_invoke_wrappers(invoke_function_names)
  asm_setup += setup_function_pointers(function_table_sigs)

  if shared.Settings.EMULATED_FUNCTION_POINTERS:
    function_tables_impls = make_function_tables_impls(function_table_data)
    asm_setup += '\n' + '\n'.join(function_tables_impls) + '\n'

  return asm_setup


def setup_function_pointers(function_table_sigs):
  asm_setup = ''
  for sig in function_table_sigs:
    if shared.Settings.RESERVED_FUNCTION_POINTERS:
      asm_setup += '\n' + shared.JS.make_jscall(sig) + '\n'
    if shared.Settings.EMULATED_FUNCTION_POINTERS:
      args = ['a%d' % i for i in range(len(sig) - 1)]
      full_args = ['x'] + args
      if shared.Settings.WASM:
        if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS:
          # emulated function pointers in wasm use an internal i64-based ABI with a fixed number of arguments. we can't
          # call into it directly because it returns an i64, which is an error for the VM. instead, we use dynCalls
          dyn_call = "Module['asm']['dynCall_" + sig + "']"
          asm_setup += '''
function ftCall_%s(%s) {
  return %s(%s);
}
''' % (sig, ', '.join(full_args), dyn_call, ', '.join(full_args))
          # and we are done with this signature, continue
          continue
        else:
          # otherwise, wasm emulated function pointers *without* emulated casts can just all
          # into the table
          table_access = "Module['wasmTable']"
          table_read = table_access + '.get(x)'
      else:
        table_access = 'FUNCTION_TABLE_' + sig
        if shared.Settings.SIDE_MODULE:
          table_access = 'parentModule["' + table_access + '"]' # side module tables were merged into the parent, we need to access the global one
        table_read = table_access + '[x]'
      prelude = ''
      if shared.Settings.ASSERTIONS:
        prelude = '''
  if (x < 0 || x >= %s.length) { err("Function table mask error (out of range)"); %s ; abort(x) }''' % (table_access, get_function_pointer_error(sig, function_table_sigs))
      asm_setup += '''
function ftCall_%s(%s) {%s
  return %s(%s);
}
''' % (sig, ', '.join(full_args), prelude, table_read, ', '.join(args))
  return asm_setup


def create_basic_funcs(function_table_sigs, invoke_function_names):
  basic_funcs = ['abort', 'assert', 'enlargeMemory', 'getTotalMemory', 'setTempRet0', 'getTempRet0']
  if shared.Settings.ABORTING_MALLOC:
    basic_funcs += ['abortOnCannotGrowMemory']
  if shared.Settings.STACK_OVERFLOW_CHECK:
    basic_funcs += ['abortStackOverflow']
  if shared.Settings.EMTERPRETIFY:
    basic_funcs += ['abortStackOverflowEmterpreter']
  if shared.Settings.SAFE_HEAP:
    if asm_safe_heap():
      basic_funcs += ['segfault', 'alignfault', 'ftfault']
    else:
      basic_funcs += ['SAFE_HEAP_LOAD', 'SAFE_HEAP_LOAD_D', 'SAFE_HEAP_STORE', 'SAFE_HEAP_STORE_D', 'SAFE_FT_MASK']
  if shared.Settings.ASSERTIONS:
    for sig in function_table_sigs:
      basic_funcs += ['nullFunc_' + sig]

  basic_funcs += invoke_function_names

  for sig in function_table_sigs:
    if shared.Settings.RESERVED_FUNCTION_POINTERS:
      basic_funcs.append('jsCall_%s' % sig)
    if shared.Settings.EMULATED_FUNCTION_POINTERS:
      # in wasm, emulated function pointers are just simple table calls
      if not shared.Settings.WASM:
        basic_funcs.append('ftCall_%s' % sig)
  return basic_funcs


def create_basic_vars(exported_implemented_functions, forwarded_json, metadata):
  basic_vars = ['DYNAMICTOP_PTR', 'tempDoublePtr']
  if not (shared.Settings.WASM and shared.Settings.SIDE_MODULE):
    basic_vars += ['STACKTOP', 'STACK_MAX']
  if shared.Settings.RELOCATABLE:
    if not (shared.Settings.WASM and shared.Settings.SIDE_MODULE):
      basic_vars += ['gb', 'fb']
    else:
      # wasm side modules have a specific convention for these
      basic_vars += ['__memory_base', '__table_base']

  # See if we need ASYNCIFY functions
  # We might not need them even if ASYNCIFY is enabled
  if need_asyncify(exported_implemented_functions):
    basic_vars += ['___async', '___async_unwind', '___async_retval', '___async_cur_frame']

  if shared.Settings.EMTERPRETIFY:
    basic_vars += ['EMTSTACKTOP', 'EMT_STACK_MAX', 'eb']

  return basic_vars


def create_exports(exported_implemented_functions, in_table, function_table_data, metadata):
  asm_runtime_funcs = create_asm_runtime_funcs()
  all_exported = exported_implemented_functions + asm_runtime_funcs + function_tables(function_table_data)
  # In asm.js + emulated function pointers, export all the table because we use
  # JS to add the asm.js module's functions to the table (which is external
  # in this mode). In wasm, we don't need that since wasm modules can
  # directly add functions to the imported Table.
  if not shared.Settings.WASM and shared.Settings.EMULATED_FUNCTION_POINTERS:
    all_exported += in_table
  exports = []
  for export in sorted(set(all_exported)):
    exports.append(quote(export) + ": " + export)
  if shared.Settings.WASM and shared.Settings.SIDE_MODULE:
    # named globals in side wasm modules are exported globals from asm/wasm
    for k, v in metadata['namedGlobals'].items():
      exports.append(quote('_' + str(k)) + ': ' + str(v))
    # aliases become additional exports
    for k, v in metadata['aliases'].items():
      exports.append(quote(str(k)) + ': ' + str(v))
  # shared wasm emulated function pointer mode requires us to know the function pointer for
  # each function. export fp$func => function pointer for func
  if shared.Settings.WASM and shared.Settings.RELOCATABLE and shared.Settings.EMULATE_FUNCTION_POINTER_CASTS:
    for k, v in metadata['functionPointers'].items():
      exports.append(quote('fp$' + str(k)) + ': ' + str(v))
  return '{ ' + ', '.join(exports) + ' }'


def create_asm_runtime_funcs():
  funcs = []
  if not (shared.Settings.WASM and shared.Settings.SIDE_MODULE):
    funcs += ['stackAlloc', 'stackSave', 'stackRestore', 'establishStackSpace', 'setThrew']
  if shared.Settings.SAFE_HEAP:
    funcs += ['setDynamicTop']
  if shared.Settings.ONLY_MY_CODE:
    funcs = []
  return funcs


def function_tables(function_table_data):
  if not shared.Settings.EMULATED_FUNCTION_POINTERS:
    return ['dynCall_' + table for table in function_table_data]
  else:
    return []


def create_the_global(metadata):
  # the global is only needed for asm.js
  if shared.Settings.WASM and shared.Settings.BINARYEN_METHOD == 'native-wasm':
    return '{}'
  fundamentals = ['Math']
  fundamentals += ['Int8Array', 'Int16Array', 'Int32Array', 'Uint8Array', 'Uint16Array', 'Uint32Array', 'Float32Array', 'Float64Array']
  fundamentals += ['NaN', 'Infinity']
  if metadata['simd'] or shared.Settings.SIMD:
    # Always import SIMD when building with -s SIMD=1, since in that mode memcpy is SIMD optimized.
    fundamentals += ['SIMD']
  if shared.Settings.ALLOW_MEMORY_GROWTH:
    fundamentals.append('byteLength')
  return '{ ' + ', '.join(['"' + math_fix(s) + '": ' + s for s in fundamentals]) + ' }'


def create_receiving(function_table_data, function_tables_defs, exported_implemented_functions):
  receiving = ''
  if not shared.Settings.ASSERTIONS:
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
  if not shared.Settings.SWAPPABLE_ASM_MODULE:
    receiving += ';\n'.join(['var ' + s + ' = Module["' + s + '"] = asm["' + s + '"]' for s in exported_implemented_functions + function_tables(function_table_data)])
  else:
    receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + s + ' = Module["' + s + '"] = function() {' + runtime_assertions + '  return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions + function_tables(function_table_data)])
  receiving += ';\n'

  if shared.Settings.EXPORT_FUNCTION_TABLES and not shared.Settings.WASM:
    for table in function_table_data.values():
      tableName = table.split()[1]
      table = table.replace('var ' + tableName, 'var ' + tableName + ' = Module["' + tableName + '"]')
      receiving += table + '\n'

  if shared.Settings.EMULATED_FUNCTION_POINTERS:
    receiving += '\n' + function_tables_defs.replace('// EMSCRIPTEN_END_FUNCS\n', '') + '\n' + ''.join(['Module["dynCall_%s"] = dynCall_%s\n' % (sig, sig) for sig in function_table_data])
    if not shared.Settings.WASM:
      for sig in function_table_data.keys():
        name = 'FUNCTION_TABLE_' + sig
        fullname = name if not shared.Settings.SIDE_MODULE else ('SIDE_' + name)
        receiving += 'Module["' + name + '"] = ' + fullname + ';\n'

  return receiving


def create_named_globals(metadata):
  named_globals = ''
  if shared.Settings.RELOCATABLE:
    named_globals += '''
var NAMED_GLOBALS = { %s };
for (var named in NAMED_GLOBALS) {
  Module['_' + named] = gb + NAMED_GLOBALS[named];
}
Module['NAMED_GLOBALS'] = NAMED_GLOBALS;
''' % ', '.join('"' + k + '": ' + str(v) for k, v in metadata['namedGlobals'].items())
    if shared.Settings.WASM:
      # wasm side modules are pure wasm, and cannot create their g$..() methods, so we help them out
      # TODO: this works if we are the main module, but if the supplying module is later, it won't, so
      #       we'll need another solution for that. one option is to scan the module imports, if/when
      #       wasm supports that, then the loader can do this.
      named_globals += '''
for (var named in NAMED_GLOBALS) {
  (function(named) {
    var func = Module['_' + named];
    Module['g$_' + named] = function() { return func };
  })(named);
}
'''
    named_globals += ''.join(["Module['%s'] = Module['%s']\n" % (k, v) for k, v in metadata['aliases'].items()])
  return named_globals


def create_runtime_funcs_asmjs(exports):
  if shared.Settings.ONLY_MY_CODE:
    return []

  if shared.Settings.ASSERTIONS or shared.Settings.STACK_OVERFLOW_CHECK >= 2:
    stack_check = '  if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(size|0);\n'
  else:
    stack_check = ''

  funcs = ['''
function stackAlloc(size) {
  size = size|0;
  var ret = 0;
  ret = STACKTOP;
  STACKTOP = (STACKTOP + size)|0;
  STACKTOP = (STACKTOP + 15)&-16;
  %s
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
function setThrew(threw, value) {
  threw = threw|0;
  value = value|0;
  if ((__THREW__|0) == 0) {
    __THREW__ = threw;
    threwValue = value;
  }
}
''' % stack_check]

  if need_asyncify(exports):
    funcs.append('''
function setAsync() {
  ___async = 1;
}
''')

  if shared.Settings.EMTERPRETIFY:
    funcs.append('''
function emterpret(pc) { // this will be replaced when the emterpreter code is generated; adding it here allows validation until then
  pc = pc | 0;
  assert(0);
}''')

  if shared.Settings.EMTERPRETIFY_ASYNC:
    funcs.append('''
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
function getEmtStackMax() {
  return EMT_STACK_MAX | 0;
}
function setEmtStackMax(x) {
  x = x | 0;
  EMT_STACK_MAX = x;
}
''')

  if shared.Settings.SAFE_HEAP:
    funcs.append('''
function setDynamicTop(value) {
  value = value | 0;
  HEAP32[DYNAMICTOP_PTR>>2] = value;
}
''')

  if asm_safe_heap():
    funcs.append('''
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
''')

  return funcs


def create_asm_start_pre(asm_setup, the_global, sending, metadata):
  shared_array_buffer = ''
  if shared.Settings.USE_PTHREADS and not shared.Settings.WASM:
    shared_array_buffer = "Module.asmGlobalArg['Atomics'] = Atomics;"

  module_get = 'Module{access} = {val};'
  module_global = module_get.format(access=access_quote('asmGlobalArg'), val=the_global)
  module_library = module_get.format(access=access_quote('asmLibraryArg'), val=sending)

  asm_function_top = ('// EMSCRIPTEN_START_ASM\n'
                      'var asm = (/** @suppress {uselessCode} */ function(global, env, buffer) {')

  use_asm = "'almost asm';"
  if shared.Settings.ASM_JS == 1:
    use_asm = "'use asm';"

  lines = [
    asm_setup,
    module_global,
    shared_array_buffer,
    module_library,
    asm_function_top,
    use_asm,
    create_first_in_asm(),
    create_memory_views(),
  ]
  return '\n'.join(lines)


def create_asm_temp_vars():
  rtn = '''
  var __THREW__ = 0;
  var threwValue = 0;
  var setjmpId = 0;
  var undef = 0;
  var nan = global%s, inf = global%s;
  var tempInt = 0, tempBigInt = 0, tempBigIntS = 0, tempValue = 0, tempDouble = 0.0;
''' % (access_quote('NaN'), access_quote('Infinity'))

  return rtn


def create_asm_runtime_thread_local_vars():
  if not shared.Settings.USE_PTHREADS:
    return ''

  return '''
  var __pthread_ptr = 0;
  var __pthread_is_main_runtime_thread = 0;
  var __pthread_is_main_browser_thread = 0;
'''


def create_replace_memory():
  if not shared.Settings.ALLOW_MEMORY_GROWTH:
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


def create_asm_end(exports):
  return '''

  return %s;
})
// EMSCRIPTEN_END_ASM
(Module%s, Module%s, buffer);
''' % (exports, access_quote('asmGlobalArg'), access_quote('asmLibraryArg'))


def create_first_in_asm():
  return ''


def create_memory_views():
  """Generates memory views for the different heap types.

  Generated symbols:
    Int8View    Int16View   Int32View
    Uint8View   Uint16View  Uint32View
    Float32View Float64View
  """
  ret = '\n'
  grow_memory = shared.Settings.ALLOW_MEMORY_GROWTH
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


def emscript_wasm_backend(infile, outfile, memfile, libraries, compiler_engine,
                          temp_files, DEBUG):
  # Overview:
  #   * Run wasm-emscripten-finalize to extract metadata and modify the binary
  #     to use emscripten's wasm<->JS ABI
  #   * Use the metadata to generate the JS glue that goes with the wasm

  metadata = finalize_wasm(temp_files, infile, outfile, memfile, DEBUG)
  if shared.Settings.SIDE_MODULE:
    return

  # optimize syscalls

  optimize_syscalls(metadata['declares'], DEBUG)

  # js compiler

  if DEBUG:
    logger.debug('emscript: js compiler glue')

  update_settings_glue(metadata)

  if DEBUG:
    t = time.time()
  glue, forwarded_data = compile_settings(compiler_engine, libraries, temp_files)
  if DEBUG:
    logger.debug('  emscript: glue took %s seconds' % (time.time() - t))
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  # memory and global initializers

  global_initializers = ', '.join('{ func: function() { %s() } }' % i for i in metadata['initializers'])

  staticbump = metadata['staticBump']
  while staticbump % 16 != 0:
    staticbump += 1
  pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + %d;
/* global initializers */ %s __ATINIT__.push(%s);
''' % (staticbump,
       'if (!ENVIRONMENT_IS_PTHREAD)' if shared.Settings.USE_PTHREADS else '',
       global_initializers))

  pre = pre.replace('{{{ STATIC_BUMP }}}', str(staticbump))

  # merge forwarded data
  shared.Settings.EXPORTED_FUNCTIONS = forwarded_json['EXPORTED_FUNCTIONS']

  exported_implemented_functions = create_exported_implemented_functions_wasm(pre, forwarded_json, metadata)

  asm_consts, asm_const_funcs = create_asm_consts_wasm(forwarded_json, metadata)
  em_js_funcs = create_em_js(forwarded_json, metadata)
  pre = pre.replace(
    '// === Body ===',
    ('// === Body ===\n\nvar ASM_CONSTS = [' +
     ',\n '.join(asm_consts) + '];\n' +
     asstr('\n'.join(asm_const_funcs)) +
     '\n'.join(em_js_funcs) + '\n'))

  outfile.write(pre)
  pre = None

  invoke_funcs = metadata.get('invokeFuncs', [])
  # List of function signatures used in jsCall functions, e.g.['v', 'vi']
  jscall_sigs = metadata.get('jsCallFuncType', [])

  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except:
    pass

  sending = create_sending_wasm(invoke_funcs, jscall_sigs, forwarded_json,
                                metadata)
  receiving = create_receiving_wasm(exported_implemented_functions)

  module = create_module_wasm(sending, receiving, invoke_funcs, jscall_sigs,
                              exported_implemented_functions)

  write_output_file(outfile, post, module)
  module = None

  outfile.close()


def finalize_wasm(temp_files, infile, outfile, memfile, DEBUG):
  wasm_emscripten_finalize = os.path.join(shared.BINARYEN_ROOT, 'bin', 'wasm-emscripten-finalize')
  wasm_dis = os.path.join(shared.BINARYEN_ROOT, 'bin', 'wasm-dis')

  def debug_copy(src, dst):
    if DEBUG:
      shutil.copyfile(src, os.path.join(shared.CANONICAL_TEMP_DIR, dst))
      if src[-2:] == '.o' or src[-5:] == '.wasm':
        tmp = dst + '.wast'
        shared.check_call([wasm_dis, src, '-o', os.path.join(shared.CANONICAL_TEMP_DIR, tmp)])

  basename = shared.unsuffixed(outfile.name)
  metadata_file = basename + '.metadata'
  wasm = basename + '.wasm'
  base_wasm = infile
  debug_copy(infile, 'base.wasm')

  write_source_map = shared.Settings.DEBUG_LEVEL >= 4
  if write_source_map:
    base_source_map = base_wasm + '.map'
    sourcemap_cmd = [shared.PYTHON, path_from_root('tools', 'wasm-sourcemap.py'),
                     base_wasm,
                     '--dwarfdump=' + shared.LLVM_DWARFDUMP,
                     '-o',  base_source_map]
    if not shared.Settings.SOURCE_MAP_BASE:
      logger.warn("Wasm source map won't be usable in a browser without --source-map-base")
    shared.check_call(sourcemap_cmd)
    debug_copy(base_source_map, 'base_wasm.map')

  cmd = [wasm_emscripten_finalize, base_wasm, '-o', wasm,
         '--global-base=%s' % shared.Settings.GLOBAL_BASE,
         ('--emscripten-reserved-function-pointers=%d' %
          shared.Settings.RESERVED_FUNCTION_POINTERS)]
  if shared.Settings.DEBUG_LEVEL >= 2 or shared.Settings.PROFILING_FUNCS:
    cmd.append('-g')
  if shared.Settings.LEGALIZE_JS_FFI != 1:
    cmd.append('--no-legalize-javascript-ffi')
  if write_source_map:
    cmd.append('--input-source-map=' + base_source_map)
    cmd.append('--output-source-map=' + wasm + '.map')
    cmd.append('--output-source-map-url=' + shared.Settings.SOURCE_MAP_BASE + os.path.basename(shared.Settings.WASM_BINARY_FILE) + '.map')
  if not shared.Settings.MEM_INIT_IN_WASM:
    cmd.append('--separate-data-segments=' + memfile)
  shared.check_call(cmd, stdout=open(metadata_file, 'w'))
  if write_source_map:
    debug_copy(wasm + '.map', 'post_finalize.map')
  debug_copy(wasm, 'post_finalize.wasm')

  return create_metadata_wasm(open(metadata_file).read(), DEBUG)


def create_metadata_wasm(metadata_raw, DEBUG):
  metadata = load_metadata(metadata_raw)
  if DEBUG:
    logger.debug("Metadata parsed: " + pprint.pformat(metadata))
  return metadata


def create_exported_implemented_functions_wasm(pre, forwarded_json, metadata):
  exported_implemented_functions = set(metadata['exports'])

  all_exported_functions = set(shared.expand_response(shared.Settings.EXPORTED_FUNCTIONS)) # both asm.js and otherwise
  for additional_export in shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE: # additional functions to export from asm, if they are implemented
    all_exported_functions.add('_' + additional_export)
  all_implemented = get_all_implemented(forwarded_json, metadata)

  export_bindings = shared.Settings.EXPORT_BINDINGS
  export_all = shared.Settings.EXPORT_ALL
  for key in all_implemented:
    if key in all_exported_functions or export_all or (export_bindings and key.startswith('_emscripten_bind')):
      exported_implemented_functions.add(key)

  check_all_implemented(all_implemented, pre)
  exported_implemented_functions = sorted(exported_implemented_functions)
  return exported_implemented_functions


def create_asm_consts_wasm(forwarded_json, metadata):
  asm_consts = [0] * len(metadata['asmConsts'])
  all_sigs = []
  for k, v in metadata['asmConsts'].items():
    const = asstr(v[0])
    sigs = v[1]
    const = trim_asm_const_body(const)
    args = []
    max_arity = 16
    arity = 0
    for i in range(max_arity):
      if ('$' + str(i)) in const:
        arity = i + 1
    for i in range(arity):
      args.append('$' + str(i))
    const = 'function(' + ', '.join(args) + ') {' + const + '}'
    asm_consts[int(k)] = const
    all_sigs += sigs

  asm_const_funcs = []
  for sig in set(all_sigs):
    forwarded_json['Functions']['libraryFunctions']['_emscripten_asm_const_' + sig] = 1
    asm_const_funcs.append(r'''
function _emscripten_asm_const_%s(code, sig_ptr, argbuf) {
  var sig = AsciiToString(sig_ptr);
  var args = [];
  var align_to = function(ptr, align) {
    return (ptr+align-1) & ~(align-1);
  };
  var buf = argbuf;
  for (var i = 0; i < sig.length; i++) {
    var c = sig[i];
    if (c == 'd' || c == 'f') {
      buf = align_to(buf, 8);
      args.push(HEAPF64[(buf >> 3)]);
      buf += 8;
    } else if (c == 'i') {
      buf = align_to(buf, 4);
      args.push(HEAPU32[(buf >> 2)]);
      buf += 4;
    }
  }
  return ASM_CONSTS[code].apply(null, args);
}''' % sig)
  return asm_consts, asm_const_funcs


def create_em_js(forwarded_json, metadata):
  em_js_funcs = []
  separator = '<::>'
  for name, raw in metadata.get('emJsFuncs', {}).items():
    assert separator in raw
    args, body = raw.split(separator, 1)
    args = args[1:-1]
    if args == 'void':
      args = []
    else:
      args = args.split(',')
    arg_names = [arg.split()[-1] for arg in args if arg]
    func = 'function {}({}){}'.format(name, ','.join(arg_names), asstr(body))
    em_js_funcs.append(func)
    forwarded_json['Functions']['libraryFunctions'][name] = 1

  return em_js_funcs


def create_sending_wasm(invoke_funcs, jscall_sigs, forwarded_json, metadata):
  basic_funcs = ['assert', 'enlargeMemory', 'getTotalMemory']
  if shared.Settings.ABORTING_MALLOC:
    basic_funcs += ['abortOnCannotGrowMemory']
  if shared.Settings.SAFE_HEAP:
    basic_funcs += ['segfault', 'alignfault']

  basic_vars = ['STACKTOP', 'STACK_MAX', 'DYNAMICTOP_PTR']

  if not shared.Settings.RELOCATABLE:
    global_vars = metadata['externs']
  else:
    global_vars = [] # linkable code accesses globals through function calls

  implemented_functions = set(metadata['implementedFunctions'])
  library_funcs = set(k for k, v in forwarded_json['Functions']['libraryFunctions'].items() if v != 2)
  global_funcs = list(library_funcs.difference(set(global_vars)).difference(implemented_functions))

  jscall_funcs = ['jsCall_' + sig for sig in jscall_sigs]

  send_items = (basic_funcs + invoke_funcs + jscall_funcs + global_funcs +
                basic_vars + global_vars)

  def fix_import_name(g):
    if g.startswith('Math_'):
      return g.split('_')[1]
    # Unlike fastcomp the wasm backend doesn't use the '_' prefix for native
    # symbols.  Emscripten currently expects symbols to start with '_' so we
    # artificially add them to the output of emscripten-wasm-finalize and them
    # strip them again here.
    if g.startswith('_'):
      return g[1:]
    return g

  send_items_map = OrderedDict()
  for name in send_items:
    internal_name = fix_import_name(name)
    if internal_name in send_items_map:
      exit_with_error('duplicate symbol in exports to wasm: %s', name)
    send_items_map[internal_name] = name

  sorted_keys = sorted(send_items_map.keys())
  return '{ ' + ', '.join('"' + k + '": ' + send_items_map[k] for k in sorted_keys) + ' }'


def create_receiving_wasm(exported_implemented_functions):
  receiving = ''
  if shared.Settings.ASSERTIONS:
    # assert on the runtime being in a valid state when calling into compiled code. The only exceptions are
    # some support code
    receiving = '\n'.join(['var real_' + asmjs_mangle(s) + ' = asm["' + s + '"]; asm["' + s + '''"] = function() {
assert(runtimeInitialized, 'you need to wait for the runtime to be ready (e.g. wait for main() to be called)');
assert(!runtimeExited, 'the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
return real_''' + asmjs_mangle(s) + '''.apply(null, arguments);
};
''' for s in exported_implemented_functions if s not in ['_memcpy', '_memset', 'runPostSets', '_emscripten_replace_memory', '__start_module']])

  if not shared.Settings.SWAPPABLE_ASM_MODULE:
    receiving += ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = asm["' + s + '"]' for s in exported_implemented_functions])
  else:
    receiving += 'Module["asm"] = asm;\n' + ';\n'.join(['var ' + asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = function() { return Module["asm"]["' + s + '"].apply(null, arguments) }' for s in exported_implemented_functions])
  receiving += ';\n'
  return receiving


def create_module_wasm(sending, receiving, invoke_funcs, jscall_sigs,
                       exported_implemented_functions):
  invoke_wrappers = create_invoke_wrappers(invoke_funcs)
  jscall_funcs = create_jscall_funcs(jscall_sigs)

  if shared.Settings.USE_PTHREADS and not shared.Settings.WASM:
    shared_array_buffer = "if (typeof SharedArrayBuffer !== 'undefined') Module.asmGlobalArg['Atomics'] = Atomics;"
  else:
    shared_array_buffer = ''

  module = ['''
Module%s = {};
%s
Module%s = %s;
''' % (access_quote('asmGlobalArg'), shared_array_buffer, access_quote('asmLibraryArg'), sending) + '''
var asm = Module['asm'](Module%s, Module%s, buffer);
%s;
''' % (access_quote('asmGlobalArg'), access_quote('asmLibraryArg'), receiving)]

  module.append('''
var stackAlloc = Module['_stackAlloc'];
var stackSave = Module['_stackSave'];
var stackRestore = Module['_stackRestore'];
var establishStackSpace = Module['establishStackSpace'];
''')

  module.append(invoke_wrappers)
  module.append(jscall_funcs)
  return module


def load_metadata(metadata_raw):
  try:
    metadata_json = json.loads(metadata_raw)
  except Exception:
    logger.error('emscript: failure to parse metadata output from wasm-emscripten-finalize. raw output is: \n' + metadata_raw)
    raise

  metadata = {
    'declares': [],
    'implementedFunctions': [],
    'externs': [],
    'simd': False,
    'maxGlobalAlign': 0,
    'initializers': [],
    'exports': [],
    'emJsFuncs': {},
    'invokeFuncs': [],
  }

  for key, value in metadata_json.items():
    # json.loads returns `unicode` for strings but other code in this file
    # generally works with utf8 encoded `str` objects, and they don't alwasy
    # mix well.  e.g. s.replace(x, y) will blow up is `s` a uts8 str containing
    # non-ascii and either x or y are unicode objects.
    # TODO(sbc): Remove this encoding if we switch to unicode elsewhere
    # (specifically the glue returned from compile_settings)
    if type(value) == list:
      value = [asstr(v) for v in value]
    metadata[key] = value

  # Initializers call the global var version of the export, so they get the mangled name.
  metadata['initializers'] = [asmjs_mangle(i) for i in metadata['initializers']]

  # functions marked llvm.used in the code are exports requested by the user
  shared.Building.user_requested_exports += metadata['exports']

  return metadata


def create_invoke_wrappers(invoke_funcs):
  """Asm.js-style exception handling: invoke wrapper generation."""
  invoke_wrappers = ''
  for invoke in invoke_funcs:
    sig = invoke[len('invoke_'):]
    invoke_wrappers += '\n' + shared.JS.make_invoke(sig) + '\n'
  return invoke_wrappers


def create_jscall_funcs(sigs):
  jscall_funcs = ''
  for i, sig in enumerate(sigs):
    jscall_funcs += '\n' + shared.JS.make_jscall(sig, i) + '\n'
  return jscall_funcs


def asmjs_mangle(name):
  """Mangle a name the way asm.js/JSBackend globals are mangled.

  Prepends '_' and replaces non-alphanumerics with '_'.
  Used by wasm backend for JS library consistency with asm.js.
  """
  library_functions_in_module = ('setThrew', 'setTempRet0', 'getTempRet0')
  if name.startswith('dynCall_'):
    return name
  if name in library_functions_in_module:
    return name
  return '_' + ''.join(['_' if not c.isalnum() else c for c in name])


def normalize_line_endings(text):
  """Normalize to UNIX line endings.

  On Windows, writing to text file will duplicate \r\n to \r\r\n otherwise.
  """
  if WINDOWS:
    return text.replace('\r\n', '\n')
  return text


def main(infile, outfile, memfile, libraries):
  temp_files = get_configuration().get_temp_files()
  infile, outfile = substitute_response_files([infile, outfile])

  if not shared.Settings.BOOTSTRAPPING_STRUCT_INFO and not shared.Settings.ONLY_MY_CODE:
    generated_struct_info_name = 'generated_struct_info.json'

    def ensure_struct_info():
      with ToolchainProfiler.profile_block('gen_struct_info'):
        out = shared.Cache.get_path(generated_struct_info_name)
        gen_struct_info.main(['-qo', out, path_from_root('src', 'struct_info.json')])
        return out

    shared.Settings.STRUCT_INFO = shared.Cache.get(generated_struct_info_name, ensure_struct_info, extension='json')
  # do we need an else, to define it for the bootstrap case?

  outfile_obj = open(outfile, 'w')

  emscripter = emscript_wasm_backend if shared.Settings.WASM_BACKEND else emscript
  return temp_files.run_and_clean(lambda: emscripter(
      infile, outfile_obj, memfile, libraries, shared.COMPILER_ENGINE, temp_files, get_configuration().DEBUG)
  )
