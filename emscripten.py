# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""A small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
"""

from tools.toolchain_profiler import ToolchainProfiler

import os
import json
import subprocess
import time
import logging
import pprint
import shutil
from collections import OrderedDict

from tools import building
from tools import diagnostics
from tools import shared
from tools import gen_struct_info
from tools import webassembly
from tools.shared import WINDOWS, path_from_root, exit_with_error, asmjs_mangle
from tools.shared import treat_as_user_function, strip_prefix
from tools.settings import settings

logger = logging.getLogger('emscripten')

WASM_INIT_FUNC = '__wasm_call_ctors'


def compute_minimal_runtime_initializer_and_exports(post, exports, receiving):
  # Declare all exports out to global JS scope so that JS library functions can access them in a
  # way that minifies well with Closure
  # e.g. var a,b,c,d,e,f;
  exports_that_are_not_initializers = [x for x in exports if x not in WASM_INIT_FUNC]
  # In Wasm backend the exports are still unmangled at this point, so mangle the names here
  exports_that_are_not_initializers = [asmjs_mangle(x) for x in exports_that_are_not_initializers]

  # Decide whether we should generate the global dynCalls dictionary for the dynCall() function?
  if settings.DYNCALLS and '$dynCall' in settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE and len([x for x in exports_that_are_not_initializers if x.startswith('dynCall_')]) > 0:
    exports_that_are_not_initializers += ['dynCalls = {}']

  declares = 'var ' + ',\n '.join(exports_that_are_not_initializers) + ';'
  post = shared.do_replace(post, '<<< WASM_MODULE_EXPORTS_DECLARES >>>', declares)

  # Generate assignments from all wasm exports out to the JS variables above: e.g. a = asm['a']; b = asm['b'];
  post = shared.do_replace(post, '<<< WASM_MODULE_EXPORTS >>>', receiving)
  return post


def write_output_file(outfile, module):
  for i in range(len(module)): # do this loop carefully to save memory
    module[i] = normalize_line_endings(module[i])
    outfile.write(module[i])


def optimize_syscalls(declares, DEBUG):
  """Disables filesystem if only a limited subset of syscalls is used.

  Our syscalls are static, and so if we see a very limited set of them - in particular,
  no open() syscall and just simple writing - then we don't need full filesystem support.
  If FORCE_FILESYSTEM is set, we can't do this. We also don't do it if INCLUDE_FULL_LIBRARY, since
  not including the filesystem would mean not including the full JS libraries, and the same for
  MAIN_MODULE since a side module might need the filesystem.
  """
  relevant_settings = ['FORCE_FILESYSTEM', 'INCLUDE_FULL_LIBRARY', 'MAIN_MODULE']
  if any(settings[s] for s in relevant_settings):
    return

  if settings.FILESYSTEM == 0:
    # without filesystem support, it doesn't matter what syscalls need
    settings.SYSCALLS_REQUIRE_FILESYSTEM = 0
  else:
    syscall_prefixes = ('__sys', 'fd_')
    syscalls = [d for d in declares if d.startswith(syscall_prefixes)]
    # check if the only filesystem syscalls are in: close, ioctl, llseek, write
    # (without open, etc.. nothing substantial can be done, so we can disable
    # extra filesystem support in that case)
    if set(syscalls).issubset(set([
      '__sys_ioctl',
      'fd_seek',
      'fd_write',
      'fd_close',
    ])):
      if DEBUG:
        logger.debug('very limited syscalls (%s) so disabling full filesystem support', ', '.join(str(s) for s in syscalls))
      settings.SYSCALLS_REQUIRE_FILESYSTEM = 0


def is_int(x):
  try:
    int(x)
    return True
  except ValueError:
    return False


def align_memory(addr):
  return (addr + 15) & -16


def update_settings_glue(metadata, DEBUG):
  optimize_syscalls(metadata['declares'], DEBUG)

  # Integrate info from backend
  if settings.SIDE_MODULE:
    # we don't need any JS library contents in side modules
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = []

  all_funcs = settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + [shared.JS.to_nice_ident(d) for d in metadata['declares']]
  settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = sorted(set(all_funcs).difference(metadata['exports']))

  settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += metadata['globalImports']

  settings.WASM_EXPORTS = metadata['exports'] + list(metadata['namedGlobals'].keys())
  # Store function exports so that Closure and metadce can track these even in
  # -s DECLARE_ASM_MODULE_EXPORTS=0 builds.
  settings.WASM_FUNCTION_EXPORTS = metadata['exports']

  # start with the MVP features, and add any detected features.
  settings.BINARYEN_FEATURES = ['--mvp-features'] + metadata['features']
  if settings.USE_PTHREADS:
    assert '--enable-threads' in settings.BINARYEN_FEATURES
  if settings.MEMORY64:
    assert '--enable-memory64' in settings.BINARYEN_FEATURES

  if settings.RELOCATABLE:
    # When building relocatable output (e.g. MAIN_MODULE) the reported table
    # size does not include the reserved slot at zero for the null pointer.
    # Instead we use __table_base to offset the elements by 1.
    if settings.INITIAL_TABLE == -1:
      settings.INITIAL_TABLE = metadata['tableSize'] + 1

  settings.HAS_MAIN = settings.MAIN_MODULE or settings.STANDALONE_WASM or 'main' in settings.WASM_EXPORTS

  # When using dynamic linking the main function might be in a side module.
  # To be safe assume they do take input parametes.
  settings.MAIN_READS_PARAMS = metadata['mainReadsParams'] or settings.MAIN_MODULE

  if settings.STACK_OVERFLOW_CHECK and not settings.SIDE_MODULE:
    settings.EXPORTED_RUNTIME_METHODS += ['writeStackCookie', 'checkStackCookie']
    # writeStackCookie and checkStackCookie both rely on emscripten_stack_get_end being
    # exported.  In theory it should always be present since its defined in compiler-rt.
    assert 'emscripten_stack_get_end' in metadata['exports']


def apply_static_code_hooks(forwarded_json, code):
  code = shared.do_replace(code, '<<< ATINITS >>>', str(forwarded_json['ATINITS']))
  if settings.HAS_MAIN:
    code = shared.do_replace(code, '<<< ATMAINS >>>', str(forwarded_json['ATMAINS']))
  if settings.EXIT_RUNTIME:
    code = shared.do_replace(code, '<<< ATEXITS >>>', str(forwarded_json['ATEXITS']))
  return code


def compile_settings():
  stderr_file = os.environ.get('EMCC_STDERR_FILE')
  if stderr_file:
    stderr_file = os.path.abspath(stderr_file)
    logger.info('logging stderr in js compiler phase into %s' % stderr_file)
    stderr_file = open(stderr_file, 'w')

  # Save settings to a file to work around v8 issue 1579
  with shared.configuration.get_temp_files().get_file('.txt') as settings_file:
    with open(settings_file, 'w') as s:
      json.dump(settings.dict(), s, sort_keys=True)

    # Call js compiler
    env = os.environ.copy()
    env['EMCC_BUILD_DIR'] = os.getcwd()
    out = shared.run_js_tool(path_from_root('src', 'compiler.js'),
                             [settings_file], stdout=subprocess.PIPE, stderr=stderr_file,
                             cwd=path_from_root('src'), env=env)
  assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
  glue, forwarded_data = out.split('//FORWARDED_DATA:')
  return glue, forwarded_data


def set_memory(static_bump):
  stack_low = align_memory(settings.GLOBAL_BASE + static_bump)
  stack_high = align_memory(stack_low + settings.TOTAL_STACK)
  settings.STACK_BASE = stack_high
  settings.STACK_MAX = stack_low
  settings.HEAP_BASE = align_memory(stack_high)


def report_missing_symbols(js_library_funcs):
  # Report any symbol that was explicitly exported but is present neither
  # as a native function nor as a JS library function.
  defined_symbols = set(asmjs_mangle(e) for e in settings.WASM_EXPORTS).union(js_library_funcs)
  missing = set(settings.USER_EXPORTED_FUNCTIONS) - defined_symbols
  for symbol in sorted(missing):
    diagnostics.warning('undefined', f'undefined exported symbol: "{symbol}"')

  # Special hanlding for the `_main` symbol

  if settings.STANDALONE_WASM:
    # standalone mode doesn't use main, and it always reports missing entry point at link time.
    # In this mode we never expect _main in the export list.
    return

  if settings.IGNORE_MISSING_MAIN:
    # The default mode for emscripten is to ignore the missing main function allowing
    # maximum compatibility.
    return

  if settings.EXPECT_MAIN and 'main' not in settings.WASM_EXPORTS:
    # For compatibility with the output of wasm-ld we use the same wording here in our
    # error message as if wasm-ld had failed (i.e. in LLD_REPORT_UNDEFINED mode).
    exit_with_error('entry symbol not defined (pass --no-entry to suppress): main')


def proxy_debug_print(sync):
  if settings.PTHREADS_DEBUG:
    if sync:
      return 'warnOnce("sync proxying function " + code);'
    else:
      return 'warnOnce("async proxying function " + code);'
  return ''


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


def create_named_globals(metadata):
  named_globals = []
  for k, v in metadata['namedGlobals'].items():
    v = int(v)
    if settings.RELOCATABLE:
      v += settings.GLOBAL_BASE
    mangled = asmjs_mangle(k)
    if settings.MINIMAL_RUNTIME:
      named_globals.append("var %s = %s;" % (mangled, v))
    else:
      named_globals.append("var %s = Module['%s'] = %s;" % (mangled, mangled, v))

  return '\n'.join(named_globals)


def emscript(in_wasm, out_wasm, outfile_js, memfile, DEBUG):
  # Overview:
  #   * Run wasm-emscripten-finalize to extract metadata and modify the binary
  #     to use emscripten's wasm<->JS ABI
  #   * Use the metadata to generate the JS glue that goes with the wasm

  if settings.SINGLE_FILE:
    # placeholder strings for JS glue, to be replaced with subresource locations in do_binaryen
    settings.WASM_BINARY_FILE = '<<< WASM_BINARY_FILE >>>'
  else:
    # set file locations, so that JS glue can find what it needs
    settings.WASM_BINARY_FILE = shared.JS.escape_for_js_string(os.path.basename(out_wasm))

  metadata = finalize_wasm(in_wasm, out_wasm, memfile, DEBUG)

  update_settings_glue(metadata, DEBUG)

  if settings.SIDE_MODULE:
    if metadata['asmConsts']:
      exit_with_error('EM_ASM is not supported in side modules')
    if metadata['emJsFuncs']:
      exit_with_error('EM_JS is not supported in side modules')
    logger.debug('emscript: skipping remaining js glue generation')
    return

  if DEBUG:
    logger.debug('emscript: js compiler glue')
    t = time.time()

  # memory and global initializers

  if settings.RELOCATABLE:
    static_bump = align_memory(webassembly.parse_dylink_section(in_wasm).mem_size)
    set_memory(static_bump)
    logger.debug('stack_base: %d, stack_max: %d, heap_base: %d', settings.STACK_BASE, settings.STACK_MAX, settings.HEAP_BASE)

  glue, forwarded_data = compile_settings()
  if DEBUG:
    logger.debug('  emscript: glue took %s seconds' % (time.time() - t))
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  exports = metadata['exports']

  if settings.ASYNCIFY:
    exports += ['asyncify_start_unwind', 'asyncify_stop_unwind', 'asyncify_start_rewind', 'asyncify_stop_rewind']

  report_missing_symbols(forwarded_json['libraryFunctions'])

  if not outfile_js:
    logger.debug('emscript: skipping remaining js glue generation')
    return

  if settings.MINIMAL_RUNTIME:
    # In MINIMAL_RUNTIME, atinit exists in the postamble part
    post = apply_static_code_hooks(forwarded_json, post)
  else:
    # In regular runtime, atinits etc. exist in the preamble part
    pre = apply_static_code_hooks(forwarded_json, pre)

  asm_consts = create_asm_consts(metadata)
  em_js_funcs = create_em_js(metadata)
  asm_const_pairs = ['%s: %s' % (key, value) for key, value in asm_consts]
  asm_const_map = 'var ASM_CONSTS = {\n  ' + ',  \n '.join(asm_const_pairs) + '\n};\n'
  pre = pre.replace(
    '// === Body ===',
    ('// === Body ===\n\n' + asm_const_map +
     '\n'.join(em_js_funcs) + '\n'))

  with open(outfile_js, 'w') as out:
    out.write(normalize_line_endings(pre))
    pre = None

    invoke_funcs = metadata['invokeFuncs']
    sending = create_sending(invoke_funcs, metadata)
    receiving = create_receiving(exports)

    if settings.MINIMAL_RUNTIME:
      if settings.DECLARE_ASM_MODULE_EXPORTS:
        post = compute_minimal_runtime_initializer_and_exports(post, exports, receiving)
      receiving = ''

    module = create_module(sending, receiving, invoke_funcs, metadata)

    write_output_file(out, module)

    out.write(normalize_line_endings(post))
    module = None


def remove_trailing_zeros(memfile):
  with open(memfile, 'rb') as f:
    mem_data = f.read()
  end = len(mem_data)
  while end > 0 and (mem_data[end - 1] == b'\0' or mem_data[end - 1] == 0):
    end -= 1
  with open(memfile, 'wb') as f:
    f.write(mem_data[:end])


def finalize_wasm(infile, outfile, memfile, DEBUG):
  building.save_intermediate(infile, 'base.wasm')
  # tell binaryen to look at the features section, and if there isn't one, to use MVP
  # (which matches what llvm+lld has given us)
  args = ['--minimize-wasm-changes']

  # if we don't need to modify the wasm, don't tell finalize to emit a wasm file
  modify_wasm = False

  if settings.WASM2JS:
    # wasm2js requires full legalization (and will do extra wasm binary
    # later processing later anyhow)
    modify_wasm = True
  if settings.GENERATE_SOURCE_MAP:
    building.emit_wasm_source_map(infile, infile + '.map', outfile)
    building.save_intermediate(infile + '.map', 'base_wasm.map')
    args += ['--output-source-map-url=' + settings.SOURCE_MAP_BASE + os.path.basename(outfile) + '.map']
    modify_wasm = True
  if settings.DEBUG_LEVEL >= 2 or settings.ASYNCIFY_ADD or settings.ASYNCIFY_ADVISE or settings.ASYNCIFY_ONLY or settings.ASYNCIFY_REMOVE or settings.EMIT_SYMBOL_MAP or settings.PROFILING_FUNCS:
    args.append('-g')
  if settings.WASM_BIGINT:
    args.append('--bigint')
  if settings.DYNCALLS:
    # we need to add all dyncalls to the wasm
    modify_wasm = True
  else:
    if settings.WASM_BIGINT:
      args.append('--no-dyncalls')
    else:
      args.append('--dyncalls-i64')
      # we need to add some dyncalls to the wasm
      modify_wasm = True
  if settings.LEGALIZE_JS_FFI:
    # When we dynamically link our JS loader adds functions from wasm modules to
    # the table. It must add the original versions of them, not legalized ones,
    # so that indirect calls have the right type, so export those.
    if settings.RELOCATABLE:
      args.append('--pass-arg=legalize-js-interface-export-originals')
    modify_wasm = True
  else:
    args.append('--no-legalize-javascript-ffi')
  if memfile:
    args.append(f'--separate-data-segments={memfile}')
    args.append(f'--global-base={settings.GLOBAL_BASE}')
    modify_wasm = True
  if settings.SIDE_MODULE:
    args.append('--side-module')
  if settings.STACK_OVERFLOW_CHECK >= 2:
    args.append('--check-stack-overflow')
    modify_wasm = True
  if settings.STANDALONE_WASM:
    args.append('--standalone-wasm')

  if settings.DEBUG_LEVEL >= 3:
    args.append('--dwarf')
  stdout = building.run_binaryen_command('wasm-emscripten-finalize',
                                         infile=infile,
                                         outfile=outfile if modify_wasm else None,
                                         args=args,
                                         stdout=subprocess.PIPE)
  if modify_wasm:
    building.save_intermediate(infile, 'post_finalize.wasm')
  elif infile != outfile:
    shutil.copy(infile, outfile)
  if settings.GENERATE_SOURCE_MAP:
    building.save_intermediate(infile + '.map', 'post_finalize.map')

  if memfile:
    # we have a separate .mem file. binaryen did not strip any trailing zeros,
    # because it's an ABI question as to whether it is valid to do so or not.
    # we can do so here, since we make sure to zero out that memory (even in
    # the dynamic linking case, our loader zeros it out)
    remove_trailing_zeros(memfile)

  return load_metadata_wasm(stdout, DEBUG)


def create_asm_consts(metadata):
  asm_consts = {}
  for addr, const in metadata['asmConsts'].items():
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
    asm_consts[int(addr)] = const
  asm_consts = [(key, value) for key, value in asm_consts.items()]
  asm_consts.sort()
  return asm_consts


def create_em_js(metadata):
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
    arg_names = [arg.split()[-1].replace("*", "") for arg in args if arg]
    func = 'function {}({}){}'.format(name, ','.join(arg_names), body)
    em_js_funcs.append(func)

  return em_js_funcs


def add_standard_wasm_imports(send_items_map):
  if settings.IMPORTED_MEMORY:
    memory_import = 'wasmMemory'
    if settings.MODULARIZE and settings.USE_PTHREADS:
      # Pthreads assign wasmMemory in their worker startup. In MODULARIZE mode, they cannot assign inside the
      # Module scope, so lookup via Module as well.
      memory_import += " || Module['wasmMemory']"
    send_items_map['memory'] = memory_import

  if settings.SAFE_HEAP:
    send_items_map['segfault'] = 'segfault'
    send_items_map['alignfault'] = 'alignfault'

  if settings.RELOCATABLE:
    send_items_map['__indirect_function_table'] = 'wasmTable'

  if settings.MAYBE_WASM2JS or settings.AUTODEBUG or settings.LINKABLE:
    # legalization of i64 support code may require these in some modes
    send_items_map['setTempRet0'] = 'setTempRet0'
    send_items_map['getTempRet0'] = 'getTempRet0'

  if settings.AUTODEBUG:
    send_items_map['log_execution'] = '''function(loc) {
      console.log('log_execution ' + loc);
    }'''
    send_items_map['get_i32'] = '''function(loc, index, value) {
      console.log('get_i32 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['get_i64'] = '''function(loc, index, low, high) {
      console.log('get_i64 ' + [loc, index, low, high]);
      setTempRet0(high);
      return low;
    }'''
    send_items_map['get_f32'] = '''function(loc, index, value) {
      console.log('get_f32 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['get_f64'] = '''function(loc, index, value) {
      console.log('get_f64 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['get_anyref'] = '''function(loc, index, value) {
      console.log('get_anyref ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['get_exnref'] = '''function(loc, index, value) {
      console.log('get_exnref ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['set_i32'] = '''function(loc, index, value) {
      console.log('set_i32 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['set_i64'] = '''function(loc, index, low, high) {
      console.log('set_i64 ' + [loc, index, low, high]);
      setTempRet0(high);
      return low;
    }'''
    send_items_map['set_f32'] = '''function(loc, index, value) {
      console.log('set_f32 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['set_f64'] = '''function(loc, index, value) {
      console.log('set_f64 ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['set_anyref'] = '''function(loc, index, value) {
      console.log('set_anyref ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['set_exnref'] = '''function(loc, index, value) {
      console.log('set_exnref ' + [loc, index, value]);
      return value;
    }'''
    send_items_map['load_ptr'] = '''function(loc, bytes, offset, ptr) {
      console.log('load_ptr ' + [loc, bytes, offset, ptr]);
      return ptr;
    }'''
    send_items_map['load_val_i32'] = '''function(loc, value) {
      console.log('load_val_i32 ' + [loc, value]);
      return value;
    }'''
    send_items_map['load_val_i64'] = '''function(loc, low, high) {
      console.log('load_val_i64 ' + [loc, low, high]);
      setTempRet0(high);
      return low;
    }'''
    send_items_map['load_val_f32'] = '''function(loc, value) {
      console.log('load_val_f32 ' + [loc, value]);
      return value;
    }'''
    send_items_map['load_val_f64'] = '''function(loc, value) {
      console.log('load_val_f64 ' + [loc, value]);
      return value;
    }'''
    send_items_map['store_ptr'] = '''function(loc, bytes, offset, ptr) {
      console.log('store_ptr ' + [loc, bytes, offset, ptr]);
      return ptr;
    }'''
    send_items_map['store_val_i32'] = '''function(loc, value) {
      console.log('store_val_i32 ' + [loc, value]);
      return value;
    }'''
    send_items_map['store_val_i64'] = '''function(loc, low, high) {
      console.log('store_val_i64 ' + [loc, low, high]);
      setTempRet0(high);
      return low;
    }'''
    send_items_map['store_val_f32'] = '''function(loc, value) {
      console.log('store_val_f32 ' + [loc, value]);
      return value;
    }'''
    send_items_map['store_val_f64'] = '''function(loc, value) {
      console.log('store_val_f64 ' + [loc, value]);
      return value;
    }'''


def create_sending(invoke_funcs, metadata):
  em_js_funcs = set(metadata['emJsFuncs'].keys())
  declares = [asmjs_mangle(d) for d in metadata['declares']]
  externs = [asmjs_mangle(e) for e in metadata['globalImports']]
  send_items = set(invoke_funcs + declares + externs)
  send_items.update(em_js_funcs)

  def fix_import_name(g):
    # Unlike fastcomp the wasm backend doesn't use the '_' prefix for native
    # symbols.  Emscripten currently expects symbols to start with '_' so we
    # artificially add them to the output of emscripten-wasm-finalize and them
    # strip them again here.
    # note that we don't do this for EM_JS functions (which, rarely, may have
    # a '_' prefix)
    if g.startswith('_') and g not in em_js_funcs:
      return g[1:]
    return g

  send_items_map = OrderedDict()
  for name in send_items:
    internal_name = fix_import_name(name)
    if internal_name in send_items_map:
      exit_with_error('duplicate symbol in exports to wasm: %s', name)
    send_items_map[internal_name] = name

  add_standard_wasm_imports(send_items_map)

  sorted_keys = sorted(send_items_map.keys())
  return '{\n  ' + ',\n  '.join('"' + k + '": ' + send_items_map[k] for k in sorted_keys) + '\n}'


def make_export_wrappers(exports, delay_assignment):
  wrappers = []
  for name in exports:
    mangled = asmjs_mangle(name)
    # The emscripten stack functions are called very early (by writeStackCookie) before
    # the runtime is initialized so we can't create these wrappers that check for
    # runtimeInitialized.
    if settings.ASSERTIONS and not name.startswith('emscripten_stack_'):
      # With assertions enabled we create a wrapper that are calls get routed through, for
      # the lifetime of the program.
      if delay_assignment:
        wrappers.append('''\
/** @type {function(...*):?} */
var %(mangled)s = Module["%(mangled)s"] = createExportWrapper("%(name)s");
''' % {'mangled': mangled, 'name': name})
      else:
        wrappers.append('''\
/** @type {function(...*):?} */
var %(mangled)s = Module["%(mangled)s"] = createExportWrapper("%(name)s", asm);
''' % {'mangled': mangled, 'name': name})
    elif delay_assignment:
      # With assertions disabled the wrapper will replace the global var and Module var on
      # first use.
      wrappers.append('''\
/** @type {function(...*):?} */
var %(mangled)s = Module["%(mangled)s"] = function() {
  return (%(mangled)s = Module["%(mangled)s"] = Module["asm"]["%(name)s"]).apply(null, arguments);
};
''' % {'mangled': mangled, 'name': name})
    else:
      wrappers.append('''\
/** @type {function(...*):?} */
var %(mangled)s = Module["%(mangled)s"] = asm["%(name)s"]
''' % {'mangled': mangled, 'name': name})
  return wrappers


def create_receiving(exports):
  # When not declaring asm exports this section is empty and we instead programatically export
  # symbols on the global object by calling exportAsmFunctions after initialization
  if not settings.DECLARE_ASM_MODULE_EXPORTS:
    return ''

  exports_that_are_not_initializers = [x for x in exports if x != WASM_INIT_FUNC]

  receiving = []

  # with WASM_ASYNC_COMPILATION that asm object may not exist at this point in time
  # so we need to support delayed assignment.
  delay_assignment = settings.WASM_ASYNC_COMPILATION and not settings.MINIMAL_RUNTIME
  if not delay_assignment:
    if settings.MINIMAL_RUNTIME:
      # In Wasm exports are assigned inside a function to variables existing in top level JS scope, i.e.
      # var _main;
      # WebAssembly.instantiate(Module["wasm"], imports).then((function(output) {
      # var asm = output.instance.exports;
      # _main = asm["_main"];
      generate_dyncall_assignment = settings.DYNCALLS and '$dynCall' in settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE
      for s in exports_that_are_not_initializers:
        mangled = asmjs_mangle(s)
        dynCallAssignment = ('dynCalls["' + s.replace('dynCall_', '') + '"] = ') if generate_dyncall_assignment and mangled.startswith('dynCall_') else ''
        receiving += [dynCallAssignment + mangled + ' = asm["' + s + '"];']
    else:
      if settings.MINIMAL_RUNTIME:
        # In wasm2js exports can be directly processed at top level, i.e.
        # var asm = Module["asm"](asmLibraryArg, buffer);
        # var _main = asm["_main"];
        if settings.USE_PTHREADS and settings.MODULARIZE:
          # TODO: As a temp solution, multithreaded MODULARIZED MINIMAL_RUNTIME builds export all
          # symbols like regular runtime does.
          # Fix this by migrating worker.js code to reside inside the Module so it is in the same
          # scope as the rest of the JS code, or by defining an export syntax to MINIMAL_RUNTIME
          # that multithreaded MODULARIZEd builds can export on.
          receiving += [asmjs_mangle(s) + ' = Module["' + asmjs_mangle(s) + '"] = asm["' + s + '"];' for s in exports_that_are_not_initializers]
        else:
          receiving += ['var ' + asmjs_mangle(s) + ' = asm["' + asmjs_mangle(s) + '"];' for s in exports_that_are_not_initializers]
      else:
        receiving += make_export_wrappers(exports, delay_assignment)
  else:
    receiving += make_export_wrappers(exports, delay_assignment)

  if settings.MINIMAL_RUNTIME:
    return '\n  '.join(receiving) + '\n'
  else:
    return '\n'.join(receiving) + '\n'


def create_module(sending, receiving, invoke_funcs, metadata):
  invoke_wrappers = create_invoke_wrappers(invoke_funcs)
  receiving += create_named_globals(metadata)
  module = []

  module.append('var asmLibraryArg = %s;\n' % sending)
  if settings.ASYNCIFY and settings.ASSERTIONS:
    module.append('Asyncify.instrumentWasmImports(asmLibraryArg);\n')

  if not settings.MINIMAL_RUNTIME:
    module.append("var asm = createWasm();\n")

  module.append(receiving)
  module.append(invoke_wrappers)
  return module


def load_metadata_wasm(metadata_raw, DEBUG):
  try:
    metadata_json = json.loads(metadata_raw)
  except Exception:
    logger.error('emscript: failure to parse metadata output from wasm-emscripten-finalize. raw output is: \n' + metadata_raw)
    raise

  metadata = {
    'declares': [],
    'globalImports': [],
    'staticBump': 0,
    'tableSize': 0,
    'exports': [],
    'namedGlobals': {},
    'emJsFuncs': {},
    'asmConsts': {},
    'invokeFuncs': [],
    'features': [],
    'mainReadsParams': 1,
  }
  legacy_keys = set(['implementedFunctions', 'initializers', 'simd', 'externs'])

  assert 'tableSize' in metadata_json.keys()
  for key, value in metadata_json.items():
    if key in legacy_keys:
      continue
    if key not in metadata:
      exit_with_error('unexpected metadata key received from wasm-emscripten-finalize: %s', key)
    metadata[key] = value

  # TODO(sbc): Remove this once binaryen change to globalImports has been around for a while.
  if 'externs' in metadata_json:
    metadata['globalImports'] = [x[1:] for x in metadata_json['externs']]

  # Support older metadata when asmConsts values were lists.  We only use the first element
  # nowadays
  # TODO(sbc): remove this once binaryen has been changed to only emit the single element
  metadata['asmConsts'] = {k: v[0] if type(v) is list else v for k, v in metadata['asmConsts'].items()}

  if DEBUG:
    logger.debug("Metadata parsed: " + pprint.pformat(metadata))

  # Calculate the subset of exports that were explicitly marked with llvm.used.
  # These are any exports that were not requested on the command line and are
  # not known auto-generated system functions.
  unexpected_exports = [e for e in metadata['exports'] if treat_as_user_function(e)]
  unexpected_exports = [asmjs_mangle(e) for e in unexpected_exports]
  unexpected_exports = [e for e in unexpected_exports if e not in settings.EXPORTED_FUNCTIONS]
  building.user_requested_exports.update(unexpected_exports)
  settings.EXPORTED_FUNCTIONS.extend(unexpected_exports)

  return metadata


def create_invoke_wrappers(invoke_funcs):
  """Asm.js-style exception handling: invoke wrapper generation."""
  invoke_wrappers = ''
  for invoke in invoke_funcs:
    sig = strip_prefix(invoke, 'invoke_')
    invoke_wrappers += '\n' + shared.JS.make_invoke(sig) + '\n'
  return invoke_wrappers


def normalize_line_endings(text):
  """Normalize to UNIX line endings.

  On Windows, writing to text file will duplicate \r\n to \r\r\n otherwise.
  """
  if WINDOWS:
    return text.replace('\r\n', '\n')
  return text


def generate_struct_info():
  # If we are running in BOOTSTRAPPING_STRUCT_INFO we don't populate STRUCT_INFO
  # otherwise that would lead to infinite recursion.
  if settings.BOOTSTRAPPING_STRUCT_INFO:
    return

  generated_struct_info_name = 'generated_struct_info.json'

  @ToolchainProfiler.profile_block('gen_struct_info')
  def generate_struct_info(out):
    gen_struct_info.main(['-q', '-o', out])

  settings.STRUCT_INFO = shared.Cache.get(generated_struct_info_name, generate_struct_info)


def run(in_wasm, out_wasm, outfile_js, memfile):
  generate_struct_info()

  emscript(in_wasm, out_wasm, outfile_js, memfile, shared.DEBUG)
