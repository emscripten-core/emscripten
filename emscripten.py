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

from tools import building
from tools import diagnostics
from tools import js_manipulation
from tools import shared
from tools import utils
from tools import webassembly
from tools import extract_metadata
from tools.utils import exit_with_error, path_from_root, removeprefix
from tools.shared import DEBUG, asmjs_mangle
from tools.shared import treat_as_user_export
from tools.settings import settings

logger = logging.getLogger('emscripten')


def compute_minimal_runtime_initializer_and_exports(post, exports, receiving):
  # Declare all exports out to global JS scope so that JS library functions can access them in a
  # way that minifies well with Closure
  # e.g. var a,b,c,d,e,f;
  exports_that_are_not_initializers = [x for x in exports if x not in building.WASM_CALL_CTORS]
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
  for chunk in module:
    outfile.write(chunk)


def maybe_disable_filesystem(imports):
  """Disables filesystem if only a limited subset of syscalls is used.

  Our syscalls are static, and so if we see a very limited set of them - in particular,
  no open() syscall and just simple writing - then we don't need full filesystem support.
  If FORCE_FILESYSTEM is set, we can't do this. We also don't do it if INCLUDE_FULL_LIBRARY, since
  not including the filesystem would mean not including the full JS libraries, and the same for
  MAIN_MODULE=1 since a side module might need the filesystem.
  """
  if any(settings[s] for s in ['FORCE_FILESYSTEM', 'INCLUDE_FULL_LIBRARY']):
    return
  if settings.MAIN_MODULE == 1:
    return

  if settings.FILESYSTEM == 0:
    # without filesystem support, it doesn't matter what syscalls need
    settings.SYSCALLS_REQUIRE_FILESYSTEM = 0
  else:
    # TODO(sbc): Find a better way to identify wasi syscalls
    syscall_prefixes = ('__syscall_', 'fd_')
    side_module_imports = [shared.demangle_c_symbol_name(s) for s in settings.SIDE_MODULE_IMPORTS]
    all_imports = set(imports).union(side_module_imports)
    syscalls = {d for d in all_imports if d.startswith(syscall_prefixes) or d in ['path_open']}
    # check if the only filesystem syscalls are in: close, ioctl, llseek, write
    # (without open, etc.. nothing substantial can be done, so we can disable
    # extra filesystem support in that case)
    if syscalls.issubset({
      '__syscall_ioctl',
      'fd_seek',
      'fd_write',
      'fd_close',
      'fd_fdstat_get',
    }):
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


def get_weak_imports(main_wasm):
  dylink_sec = webassembly.parse_dylink_section(main_wasm)
  for symbols in dylink_sec.import_info.values():
    for symbol, flags in symbols.items():
      if flags & webassembly.SYMBOL_BINDING_MASK == webassembly.SYMBOL_BINDING_WEAK:
        settings.WEAK_IMPORTS.append(symbol)


def update_settings_glue(wasm_file, metadata):
  maybe_disable_filesystem(metadata.imports)

  # Integrate info from backend
  if settings.SIDE_MODULE:
    # we don't need any JS library contents in side modules
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = []
  else:
    syms = settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + metadata.imports
    syms = set(syms).difference(metadata.all_exports)
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = sorted(syms)
    if settings.MAIN_MODULE:
      get_weak_imports(wasm_file)

  settings.WASM_EXPORTS = metadata.all_exports
  settings.WASM_GLOBAL_EXPORTS = list(metadata.namedGlobals.keys())
  settings.HAVE_EM_ASM = bool(settings.MAIN_MODULE or len(metadata.asmConsts) != 0)

  # start with the MVP features, and add any detected features.
  settings.BINARYEN_FEATURES = ['--mvp-features'] + metadata.features
  if settings.ASYNCIFY == 2:
    settings.BINARYEN_FEATURES += ['--enable-reference-types']

  if settings.PTHREADS:
    assert '--enable-threads' in settings.BINARYEN_FEATURES
  if settings.MEMORY64:
    assert '--enable-memory64' in settings.BINARYEN_FEATURES

  settings.HAS_MAIN = bool(settings.MAIN_MODULE) or settings.PROXY_TO_PTHREAD or settings.STANDALONE_WASM or 'main' in settings.WASM_EXPORTS or '__main_argc_argv' in settings.WASM_EXPORTS
  if settings.HAS_MAIN and not settings.MINIMAL_RUNTIME:
    # Dependencies of `callMain`
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$exitJS', '$handleException']

  # When using dynamic linking the main function might be in a side module.
  # To be safe assume they do take input parametes.
  settings.MAIN_READS_PARAMS = metadata.mainReadsParams or bool(settings.MAIN_MODULE)
  if settings.MAIN_READS_PARAMS and not settings.STANDALONE_WASM:
    # callMain depends on this library function
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$stringToUTF8OnStack']

  if settings.STACK_OVERFLOW_CHECK and not settings.SIDE_MODULE:
    # writeStackCookie and checkStackCookie both rely on emscripten_stack_get_end being
    # exported.  In theory it should always be present since its defined in compiler-rt.
    assert 'emscripten_stack_get_end' in metadata.function_exports

  for deps in metadata.jsDeps:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.extend(deps.split(','))


def apply_static_code_hooks(forwarded_json, code):
  code = shared.do_replace(code, '<<< ATINITS >>>', str(forwarded_json['ATINITS']))
  if settings.HAS_MAIN:
    code = shared.do_replace(code, '<<< ATMAINS >>>', str(forwarded_json['ATMAINS']))
  if settings.EXIT_RUNTIME and (not settings.MINIMAL_RUNTIME or settings.HAS_MAIN):
    code = shared.do_replace(code, '<<< ATEXITS >>>', str(forwarded_json['ATEXITS']))
  return code


def compile_javascript(symbols_only=False):
  stderr_file = os.environ.get('EMCC_STDERR_FILE')
  if stderr_file:
    stderr_file = os.path.abspath(stderr_file)
    logger.info('logging stderr in js compiler phase into %s' % stderr_file)
    stderr_file = open(stderr_file, 'w')

  # Save settings to a file to work around v8 issue 1579
  with shared.get_temp_files().get_file('.json') as settings_file:
    with open(settings_file, 'w') as s:
      json.dump(settings.external_dict(), s, sort_keys=True, indent=2)

    # Call js compiler
    env = os.environ.copy()
    env['EMCC_BUILD_DIR'] = os.getcwd()
    args = [settings_file]
    if symbols_only:
      args += ['--symbols-only']
    out = shared.run_js_tool(path_from_root('src/compiler.js'),
                             args, stdout=subprocess.PIPE, stderr=stderr_file,
                             cwd=path_from_root('src'), env=env, encoding='utf-8')
  if symbols_only:
    glue = None
    forwarded_data = out
  else:
    assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
    glue, forwarded_data = out.split('//FORWARDED_DATA:')
  return glue, forwarded_data


def set_memory(static_bump):
  stack_low = align_memory(settings.GLOBAL_BASE + static_bump)
  stack_high = align_memory(stack_low + settings.STACK_SIZE)
  settings.STACK_HIGH = stack_high
  settings.STACK_LOW = stack_low
  settings.HEAP_BASE = align_memory(stack_high)


def report_missing_symbols(js_symbols):
  # Report any symbol that was explicitly exported but is present neither
  # as a native function nor as a JS library function.
  defined_symbols = set(asmjs_mangle(e) for e in settings.WASM_EXPORTS).union(js_symbols)
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

  if settings.EXPECT_MAIN and 'main' not in settings.WASM_EXPORTS and '__main_argc_argv' not in settings.WASM_EXPORTS:
    # For compatibility with the output of wasm-ld we use the same wording here in our
    # error message as if wasm-ld had failed.
    exit_with_error('entry symbol not defined (pass --no-entry to suppress): main')


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
  for k, v in metadata.namedGlobals.items():
    v = int(v)
    if settings.RELOCATABLE:
      v += settings.GLOBAL_BASE
    mangled = asmjs_mangle(k)
    if settings.MINIMAL_RUNTIME:
      named_globals.append("var %s = %s;" % (mangled, v))
    else:
      named_globals.append("var %s = Module['%s'] = %s;" % (mangled, mangled, v))

  return '\n'.join(named_globals)


def emscript(in_wasm, out_wasm, outfile_js, memfile, js_syms):
  # Overview:
  #   * Run wasm-emscripten-finalize to extract metadata and modify the binary
  #     to use emscripten's wasm<->JS ABI
  #   * Use the metadata to generate the JS glue that goes with the wasm

  if settings.SINGLE_FILE:
    # placeholder strings for JS glue, to be replaced with subresource locations in do_binaryen
    settings.WASM_BINARY_FILE = '<<< WASM_BINARY_FILE >>>'
  else:
    # set file locations, so that JS glue can find what it needs
    settings.WASM_BINARY_FILE = js_manipulation.escape_for_js_string(os.path.basename(out_wasm))

  metadata = finalize_wasm(in_wasm, out_wasm, memfile, js_syms)

  if settings.RELOCATABLE and settings.MEMORY64 == 2:
    metadata.imports += ['__memory_base32']

  if settings.ASYNCIFY == 1:
    metadata.function_exports['asyncify_start_unwind'] = 1
    metadata.function_exports['asyncify_stop_unwind'] = 0
    metadata.function_exports['asyncify_start_rewind'] = 1
    metadata.function_exports['asyncify_stop_rewind'] = 0

  update_settings_glue(out_wasm, metadata)

  if not settings.WASM_BIGINT and metadata.emJsFuncs:
    import_map = {}

    with webassembly.Module(in_wasm) as module:
      types = module.get_types()
      for imp in module.get_imports():
        if imp.module not in ('GOT.mem', 'GOT.func'):
          import_map[imp.field] = imp

    for em_js_func, raw in metadata.emJsFuncs.items():
      c_sig = raw.split('<::>')[0].strip('()')
      if not c_sig or c_sig == 'void':
        c_sig = []
      else:
        c_sig = c_sig.split(',')
      if em_js_func in import_map:
        imp = import_map[em_js_func]
        assert imp.kind == webassembly.ExternType.FUNC
        signature = types[imp.type]
        if len(signature.params) != len(c_sig):
          diagnostics.warning('em-js-i64', 'using 64-bit arguments in EM_JS function without WASM_BIGINT is not yet fully supported: `%s` (%s, %s)', em_js_func, c_sig, signature.params)

  if settings.SIDE_MODULE:
    logger.debug('emscript: skipping remaining js glue generation')
    return

  if DEBUG:
    logger.debug('emscript: js compiler glue')
    t = time.time()

  # memory and global initializers

  if settings.RELOCATABLE:
    dylink_sec = webassembly.parse_dylink_section(in_wasm)
    static_bump = align_memory(dylink_sec.mem_size)
    set_memory(static_bump)
    logger.debug('stack_low: %d, stack_high: %d, heap_base: %d', settings.STACK_LOW, settings.STACK_HIGH, settings.HEAP_BASE)

    # When building relocatable output (e.g. MAIN_MODULE) the reported table
    # size does not include the reserved slot at zero for the null pointer.
    # So we need to offset the elements by 1.
    if settings.INITIAL_TABLE == -1:
      settings.INITIAL_TABLE = dylink_sec.table_size + 1

    if settings.ASYNCIFY == 1:
      metadata.imports += ['__asyncify_state', '__asyncify_data']

  if metadata.invokeFuncs:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$getWasmTableEntry']

  glue, forwarded_data = compile_javascript()
  if DEBUG:
    logger.debug('  emscript: glue took %s seconds' % (time.time() - t))
    t = time.time()

  forwarded_json = json.loads(forwarded_data)

  if forwarded_json['warnings']:
    diagnostics.warning('js-compiler', 'warnings in JS library compilation')

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  if settings.ASSERTIONS:
    pre += "function checkIncomingModuleAPI() {\n"
    for sym in settings.ALL_INCOMING_MODULE_JS_API:
      if sym not in settings.INCOMING_MODULE_JS_API:
        pre += f"  ignoredModuleProp('{sym}');\n"
    pre += "}\n"

  report_missing_symbols(forwarded_json['librarySymbols'])

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
  extra_code = ''
  if asm_const_pairs or settings.MAIN_MODULE:
    extra_code += 'var ASM_CONSTS = {\n  ' + ',  \n '.join(asm_const_pairs) + '\n};\n'
  if em_js_funcs:
    extra_code += '\n'.join(em_js_funcs) + '\n'
  if extra_code:
    pre = pre.replace(
      '// === Body ===\n',
      '// === Body ===\n\n' + extra_code + '\n')

  with open(outfile_js, 'w', encoding='utf-8') as out:
    out.write(pre)
    pre = None

    receiving = create_receiving(metadata.function_exports)

    if settings.MINIMAL_RUNTIME:
      if settings.DECLARE_ASM_MODULE_EXPORTS:
        post = compute_minimal_runtime_initializer_and_exports(post, metadata.function_exports, receiving)
      receiving = ''

    module = create_module(receiving, metadata, forwarded_json['librarySymbols'])

    write_output_file(out, module)

    out.write(post)
    module = None


def remove_trailing_zeros(memfile):
  mem_data = utils.read_binary(memfile)
  mem_data = mem_data.rstrip(b'\0')
  utils.write_binary(memfile, mem_data)


@ToolchainProfiler.profile()
def get_metadata(infile, outfile, modify_wasm, args):
  metadata = extract_metadata.extract_metadata(infile)
  if modify_wasm:
    # In some cases we still need to modify the wasm file
    # using wasm-emscripten-finalize.
    building.run_binaryen_command('wasm-emscripten-finalize',
                                  infile=infile,
                                  outfile=outfile,
                                  args=args)
    # When we do this we can generate new imports, so
    # re-read parts of the metadata post-finalize
    extract_metadata.update_metadata(outfile, metadata)
  if DEBUG:
    logger.debug("Metadata: " + pprint.pformat(metadata.__dict__))
  return metadata


def finalize_wasm(infile, outfile, memfile, js_syms):
  building.save_intermediate(infile, 'base.wasm')
  args = []

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
  if settings.DEBUG_LEVEL >= 2 or settings.ASYNCIFY_ADD or settings.ASYNCIFY_ADVISE or settings.ASYNCIFY_ONLY or settings.ASYNCIFY_REMOVE or settings.EMIT_SYMBOL_MAP or settings.EMIT_NAME_SECTION:
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
  if settings.AUTODEBUG:
    # In AUTODEBUG mode we want to delay all legalization until later.  This is hack
    # to force wasm-emscripten-finalize not to do any legalization at all.
    args.append('--bigint')
  else:
    if settings.LEGALIZE_JS_FFI:
      # When we dynamically link our JS loader adds functions from wasm modules to
      # the table. It must add the original versions of them, not legalized ones,
      # so that indirect calls have the right type, so export those.
      args += building.js_legalization_pass_flags()
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

  metadata = get_metadata(infile, outfile, modify_wasm, args)
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

  expected_exports = set(settings.EXPORTED_FUNCTIONS)
  expected_exports.update(asmjs_mangle(s) for s in settings.REQUIRED_EXPORTS)
  # Assume that when JS symbol dependencies are exported it is because they
  # are needed by by a JS symbol and are not being explicitly exported due
  # to EMSCRIPTEN_KEEPALIVE (llvm.used).
  for deps in js_syms.values():
    expected_exports.update(asmjs_mangle(s) for s in deps)

  # Calculate the subset of exports that were explicitly marked as
  # EMSCRIPTEN_KEEPALIVE (llvm.used).
  # These are any exports that were not requested on the command line and are
  # not known auto-generated system functions.
  unexpected_exports = [e for e in metadata.all_exports if treat_as_user_export(e)]
  unexpected_exports = [asmjs_mangle(e) for e in unexpected_exports]
  unexpected_exports = [e for e in unexpected_exports if e not in expected_exports]
  if '_main' in unexpected_exports:
    logger.warning('main() is in the input files, but "_main" is not in EXPORTED_FUNCTIONS, which means it may be eliminated as dead code. Export it if you want main() to run.')
    unexpected_exports.remove('_main')

  building.user_requested_exports.update(unexpected_exports)
  settings.EXPORTED_FUNCTIONS.extend(unexpected_exports)

  return metadata


def create_asm_consts(metadata):
  asm_consts = {}
  for addr, const in metadata.asmConsts.items():
    body = trim_asm_const_body(const)
    args = []
    max_arity = 16
    arity = 0
    for i in range(max_arity):
      if ('$' + str(i)) in const:
        arity = i + 1
    for i in range(arity):
      args.append('$' + str(i))
    args = ', '.join(args)
    if 'arguments' in body:
      # arrow functions don't bind `arguments` so we have to use
      # the old function syntax in this case
      func = f'function({args}) {{ {body} }}'
    else:
      func = f'({args}) => {{ {body} }}'
    if settings.RELOCATABLE:
      addr += settings.GLOBAL_BASE
    asm_consts[addr] = func
  asm_consts = [(key, value) for key, value in asm_consts.items()]
  asm_consts.sort()
  return asm_consts


def type_to_sig(type):
  # These should match the conversion in $sigToWasmTypes.
  return {
    webassembly.Type.I32: 'i',
    webassembly.Type.I64: 'j',
    webassembly.Type.F32: 'f',
    webassembly.Type.F64: 'd',
    webassembly.Type.VOID: 'v'
  }[type]


def func_type_to_sig(type):
  parameters = [type_to_sig(param) for param in type.params]
  if type.returns:
    assert len(type.returns) == 1, "One return type expected."
    ret = type.returns[0]
  else:
    ret = webassembly.Type.VOID
  return type_to_sig(ret) + ''.join(parameters)


def create_em_js(metadata):
  em_js_funcs = []
  separator = '<::>'
  for name, raw in metadata.emJsFuncs.items():
    assert separator in raw
    args, body = raw.split(separator, 1)
    args = args[1:-1]
    if args == 'void':
      args = []
    else:
      args = args.split(',')
    arg_names = [arg.split()[-1].replace('*', '') for arg in args if arg]
    args = ','.join(arg_names)
    func = f'function {name}({args}) {body}'
    if (settings.MAIN_MODULE or settings.ASYNCIFY == 2) and name in metadata.emJsFuncTypes:
      sig = func_type_to_sig(metadata.emJsFuncTypes[name])
      func = func + f'\n{name}.sig = \'{sig}\';'
    em_js_funcs.append(func)

  return em_js_funcs


def add_standard_wasm_imports(send_items_map):
  extra_sent_items = []

  if settings.IMPORTED_MEMORY:
    memory_import = 'wasmMemory'
    if settings.MODULARIZE and settings.PTHREADS:
      # Pthreads assign wasmMemory in their worker startup. In MODULARIZE mode, they cannot assign inside the
      # Module scope, so lookup via Module as well.
      memory_import += " || Module['wasmMemory']"
    send_items_map['memory'] = memory_import

  if settings.SAFE_HEAP:
    extra_sent_items.append('segfault')
    extra_sent_items.append('alignfault')

  if settings.RELOCATABLE:
    send_items_map['__indirect_function_table'] = 'wasmTable'

  if settings.AUTODEBUG:
    extra_sent_items += [
      'log_execution',
      'get_i32',
      'get_i64',
      'get_f32',
      'get_f64',
      'get_anyref',
      'get_exnref',
      'set_i32',
      'set_i64',
      'set_f32',
      'set_f64',
      'set_anyref',
      'set_exnref',
      'load_ptr',
      'load_val_i32',
      'load_val_i64',
      'load_val_f32',
      'load_val_f64',
      'store_ptr',
      'store_val_i32',
      'store_val_i64',
      'store_val_f32',
      'store_val_f64',
    ]

  if settings.SPLIT_MODULE and settings.ASYNCIFY == 2:
    # Calls to this function are generated by binaryen so it must be manually
    # imported.
    extra_sent_items.append('__load_secondary_module')

  for s in extra_sent_items:
    send_items_map[s] = s


def create_sending(metadata, library_symbols):
  # Map of wasm imports to mangled/external/JS names
  send_items_map = {}

  for name in metadata.invokeFuncs:
    send_items_map[name] = name
  for name in metadata.imports:
    if name in metadata.emJsFuncs:
      send_items_map[name] = name
    else:
      send_items_map[name] = asmjs_mangle(name)

  add_standard_wasm_imports(send_items_map)

  if settings.MAIN_MODULE:
    # When including dynamic linking support, also add any JS library functions
    # that are part of EXPORTED_FUNCTIONS (or in the case of MAIN_MODULE=1 add
    # all JS library functions).  This allows `dlsym(RTLD_DEFAULT)` to lookup JS
    # library functions, since `wasmImports` acts as the global symbol table.
    wasm_exports = set(metadata.function_exports)
    library_symbols = set(library_symbols)
    if settings.MAIN_MODULE == 1:
      for f in library_symbols:
        if shared.is_c_symbol(f):
          demangled = shared.demangle_c_symbol_name(f)
          if demangled in wasm_exports:
            continue
          send_items_map[demangled] = f
    else:
      for f in settings.EXPORTED_FUNCTIONS + settings.SIDE_MODULE_IMPORTS:
        if f in library_symbols and shared.is_c_symbol(f):
          demangled = shared.demangle_c_symbol_name(f)
          if demangled in wasm_exports:
            continue
          send_items_map[demangled] = f

  sorted_items = sorted(send_items_map.items())
  prefix = ''
  if settings.USE_CLOSURE_COMPILER:
    # This prevents closure compiler from minifying the field names in this object.
    prefix = '/** @export */\n  '
  return '{\n  ' + ',\n  '.join(f'{prefix}{k}: {v}' for k, v in sorted_items) + '\n}'


def make_export_wrappers(function_exports, delay_assignment):
  wrappers = []

  # The emscripten stack functions are called very early (by writeStackCookie) before
  # the runtime is initialized so we can't create these wrappers that check for
  # runtimeInitialized.
  # Likewise `__trap` can occur before the runtime is initialized since it is used in
  # abort.
  # pthread_self and _emscripten_proxy_execute_task_queue are currently called in some
  # cases after the runtime has exited.
  # TODO: Look into removing these, and improving our robustness around thread termination.
  def install_wrapper(sym):
    if sym.startswith('_asan_') or sym.startswith('emscripten_stack_'):
      return False
    if sym in ('__trap', 'pthread_self', '_emscripten_proxy_execute_task_queue'):
      return False
    return True

  for name, nargs in function_exports.items():
    mangled = asmjs_mangle(name)
    wrapper = 'var %s = ' % mangled

    # TODO(sbc): Can we avoid exporting the dynCall_ functions on the module.
    should_export = settings.EXPORT_KEEPALIVE and mangled in settings.EXPORTED_FUNCTIONS
    if name.startswith('dynCall_') or should_export:
      exported = "Module['%s'] = " % mangled
    else:
      exported = ''
    wrapper += exported

    if settings.ASSERTIONS and install_wrapper(name):
      # With assertions enabled we create a wrapper that are calls get routed through, for
      # the lifetime of the program.
      wrapper += "createExportWrapper('%s');" % name
    elif delay_assignment:
      # With assertions disabled the wrapper will replace the global var and Module var on
      # first use.
      args = [f'a{i}' for i in range(nargs)]
      args = ', '.join(args)
      wrapper += f"({args}) => ({mangled} = {exported}wasmExports['{name}'])({args});"
    else:
      wrapper += 'asm["%s"]' % name

    wrappers.append(wrapper)
  return wrappers


def create_receiving(function_exports):
  # When not declaring asm exports this section is empty and we instead programatically export
  # symbols on the global object by calling exportAsmFunctions after initialization
  if not settings.DECLARE_ASM_MODULE_EXPORTS:
    return ''

  receiving = []

  # with WASM_ASYNC_COMPILATION that asm object may not exist at this point in time
  # so we need to support delayed assignment.
  delay_assignment = settings.WASM_ASYNC_COMPILATION and not settings.MINIMAL_RUNTIME
  if not delay_assignment:
    if settings.MINIMAL_RUNTIME:
      # In Wasm exports are assigned inside a function to variables
      # existing in top level JS scope, i.e.
      # var _main;
      # WebAssembly.instantiate(Module['wasm'], imports).then((output) => {
      #   var asm = output.instance.exports;
      #   _main = asm["_main"];
      generate_dyncall_assignment = settings.DYNCALLS and '$dynCall' in settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE
      exports_that_are_not_initializers = [x for x in function_exports if x != building.WASM_CALL_CTORS]

      for s in exports_that_are_not_initializers:
        mangled = asmjs_mangle(s)
        dynCallAssignment = ('dynCalls["' + s.replace('dynCall_', '') + '"] = ') if generate_dyncall_assignment and mangled.startswith('dynCall_') else ''
        should_export = settings.EXPORT_ALL or (settings.EXPORT_KEEPALIVE and mangled in settings.EXPORTED_FUNCTIONS)
        export_assignment = ''
        if settings.MODULARIZE and should_export:
          export_assignment = f"Module['{mangled}'] = "
        receiving += [f'{export_assignment}{dynCallAssignment}{mangled} = asm["{s}"]']
    else:
      receiving += make_export_wrappers(function_exports, delay_assignment)
  else:
    receiving += make_export_wrappers(function_exports, delay_assignment)

  if settings.MINIMAL_RUNTIME:
    return '\n  '.join(receiving) + '\n'
  else:
    return '\n'.join(receiving) + '\n'


def create_module(receiving, metadata, library_symbols):
  receiving += create_named_globals(metadata)
  module = []

  sending = create_sending(metadata, library_symbols)
  module.append('var wasmImports = %s;\n' % sending)
  if settings.ASYNCIFY and (settings.ASSERTIONS or settings.ASYNCIFY == 2):
    # instrumenting imports is used in asyncify in two ways: to add assertions
    # that check for proper import use, and for ASYNCIFY=2 we use them to set up
    # the Promise API on the import side.
    module.append('Asyncify.instrumentWasmImports(wasmImports);\n')

  if not settings.MINIMAL_RUNTIME:
    module.append("var asm = createWasm();\n")

  module.append(receiving)
  if settings.SUPPORT_LONGJMP == 'emscripten' or not settings.DISABLE_EXCEPTION_CATCHING:
    module.append(create_invoke_wrappers(metadata))
  else:
    assert not metadata.invokeFuncs, "invoke_ functions exported but exceptions and longjmp are both disabled"
  if settings.MEMORY64 or settings.CAN_ADDRESS_2GB:
    module.append(create_pointer_conversion_wrappers(metadata))
  return module


def create_invoke_wrappers(metadata):
  """Asm.js-style exception handling: invoke wrapper generation."""
  invoke_wrappers = ''
  for invoke in metadata.invokeFuncs:
    sig = removeprefix(invoke, 'invoke_')
    invoke_wrappers += '\n' + js_manipulation.make_invoke(sig) + '\n'
  return invoke_wrappers


def create_pointer_conversion_wrappers(metadata):
  # TODO(sbc): Move this into somewhere less static.  Maybe it can become
  # part of library.js file, even though this metadata relates specifically
  # to native (non-JS) functions.
  #
  # The signature format here is similar to the one used for JS libraries
  # but with the following as the only valid char:
  #  '_' - non-pointer argument (pass through unchanged)
  #  'p' - pointer/int53 argument (convert to/from BigInt)
  #  'P' - same as above but allow `undefined` too (requires extra check)
  mapping = {
    'sbrk': 'pP',
    'stackAlloc': 'pp',
    'emscripten_builtin_malloc': 'pp',
    'malloc': 'pp',
    '__getTypeName': 'pp',
    'setThrew': '_p',
    'free': '_p',
    'stackRestore': '_p',
    '__cxa_is_pointer_type': '_p',
    'stackSave': 'p',
    'fflush': '_p',
    'emscripten_stack_get_end': 'p',
    'emscripten_stack_get_base': 'p',
    'pthread_self': 'p',
    'emscripten_stack_get_current': 'p',
    '__errno_location': 'p',
    'emscripten_builtin_memalign': 'ppp',
    'emscripten_builtin_free': 'vp',
    'main': '__PP',
    '__main_argc_argv': '__PP',
    'emscripten_stack_set_limits': '_pp',
    '__set_stack_limits': '_pp',
    '__cxa_can_catch': '_ppp',
    '__cxa_increment_exception_refcount': '_p',
    '__cxa_decrement_exception_refcount': '_p',
    '_wasmfs_write_file': '_ppp',
    '__dl_seterr': '_pp',
    '_emscripten_run_in_main_runtime_thread_js': '___p_',
    '_emscripten_proxy_execute_task_queue': '_p',
    '_emscripten_thread_exit': '_p',
    '_emscripten_thread_init': '_p_____',
    '_emscripten_thread_free_data': '_p',
    '_emscripten_dlsync_self_async': '_p',
    '_emscripten_proxy_dlsync_async': '_pp',
    '_wasmfs_rmdir': '_p',
    '_wasmfs_unlink': '_p',
    '_wasmfs_mkdir': '_p_',
    '_wasmfs_open': '_p__',
    'emscripten_wasm_worker_initialize': '_p_',
    'asyncify_start_rewind': '_p',
    'asyncify_start_unwind': '_p',
    '__get_exception_message': '_ppp',
  }

  wrappers = '''
function applySignatureConversions(exports) {
  // First, make a copy of the incoming exports object
  exports = Object.assign({}, exports);
'''

  sigs_seen = set()
  wrap_functions = []
  for symbol in metadata.function_exports:
    sig = mapping.get(symbol)
    if sig:
      if settings.MEMORY64:
        if sig not in sigs_seen:
          wrappers += js_manipulation.make_wasm64_wrapper(sig)
          sigs_seen.add(sig)
        wrap_functions.append(symbol)
      elif sig[0] == 'p':
        if sig not in sigs_seen:
          wrappers += js_manipulation.make_unsign_pointer_wrapper(sig)
          sigs_seen.add(sig)
        wrap_functions.append(symbol)

  for f in wrap_functions:
    sig = mapping[f]
    wrappers += f"\n  exports['{f}'] = makeWrapper_{sig}(exports['{f}']);"
  wrappers += '\n  return exports\n}'

  return wrappers


def run(in_wasm, out_wasm, outfile_js, memfile, js_syms):
  emscript(in_wasm, out_wasm, outfile_js, memfile, js_syms)
