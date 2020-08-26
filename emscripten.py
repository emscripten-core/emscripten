# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""A small wrapper script around the core JS compiler. This calls that
compiler with the settings given to it. It can also read data from C/C++
header files (so that the JS compiler can see the constants in those
headers, for the libc implementation in JS).
"""

from __future__ import print_function

import os
import json
import subprocess
import time
import logging
import pprint
from collections import OrderedDict

from tools import building
from tools import diagnostics
from tools import shared
from tools import gen_struct_info
from tools.response_file import substitute_response_files
from tools.shared import WINDOWS, asstr, path_from_root, exit_with_error, asmjs_mangle, treat_as_user_function
from tools.toolchain_profiler import ToolchainProfiler

logger = logging.getLogger('emscripten')

STDERR_FILE = os.environ.get('EMCC_STDERR_FILE')
if STDERR_FILE:
  STDERR_FILE = os.path.abspath(STDERR_FILE)
  logger.info('logging stderr in js compiler phase into %s' % STDERR_FILE)
  STDERR_FILE = open(STDERR_FILE, 'w')


def compute_minimal_runtime_initializer_and_exports(post, initializers, exports, receiving):
  # Generate invocations for all global initializers directly off the asm export object, e.g. asm['__GLOBAL__INIT']();
  post = post.replace('/*** RUN_GLOBAL_INITIALIZERS(); ***/', '\n'.join(["asm['" + x + "']();" for x in global_initializer_funcs(initializers)]))

  if shared.Settings.WASM:
    # Declare all exports out to global JS scope so that JS library functions can access them in a
    # way that minifies well with Closure
    # e.g. var a,b,c,d,e,f;
    exports_that_are_not_initializers = [x for x in exports if x not in initializers]
    # In Wasm backend the exports are still unmangled at this point, so mangle the names here
    exports_that_are_not_initializers = [asmjs_mangle(x) for x in exports_that_are_not_initializers]
    post = post.replace('/*** ASM_MODULE_EXPORTS_DECLARES ***/', 'var ' + ','.join(exports_that_are_not_initializers) + ';')

    # Generate assignments from all asm.js/wasm exports out to the JS variables above: e.g. a = asm['a']; b = asm['b'];
    post = post.replace('/*** ASM_MODULE_EXPORTS ***/', receiving)
    receiving = ''

  return post, receiving


def global_initializer_funcs(initializers):
  # If we have at most one global ctor, no need to group global initializers.
  # Also in EVAL_CTORS mode, we want to try to evaluate the individual ctor functions, so in that mode,
  # do not group ctors into one.
  return ['globalCtors'] if (len(initializers) > 1 and not shared.Settings.EVAL_CTORS) else initializers


def write_output_file(outfile, post, module):
  for i in range(len(module)): # do this loop carefully to save memory
    module[i] = normalize_line_endings(module[i])
    outfile.write(module[i])

  post = normalize_line_endings(post)
  outfile.write(post)


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

  if shared.Settings.FILESYSTEM == 0:
    # without filesystem support, it doesn't matter what syscalls need
    shared.Settings.SYSCALLS_REQUIRE_FILESYSTEM = 0
  else:
    syscall_prefixes = ('__sys', 'fd_', '__wasi_fd_')
    syscalls = [d for d in declares if d.startswith(syscall_prefixes)]
    # check if the only filesystem syscalls are in: close, ioctl, llseek, write
    # (without open, etc.. nothing substantial can be done, so we can disable
    # extra filesystem support in that case)
    if set(syscalls).issubset(set([
      '__sys_ioctl',
      # legacy/fastcomp name for __sys_ioctl
      '__syscall6',
      'fd_seek', '__wasi_fd_seek',
      'fd_write', '__wasi_fd_write',
      'fd_close', '__wasi_fd_close',
    ])):
      if DEBUG:
        logger.debug('very limited syscalls (%s) so disabling full filesystem support', ', '.join(str(s) for s in syscalls))
      shared.Settings.SYSCALLS_REQUIRE_FILESYSTEM = 0


def is_int(x):
  try:
    int(x)
    return True
  except ValueError:
    return False


def align_memory(addr):
  return (addr + 15) & -16


def align_static_bump(metadata):
  metadata['staticBump'] = align_memory(metadata['staticBump'])
  return metadata['staticBump']


def update_settings_glue(metadata, DEBUG):
  optimize_syscalls(metadata['declares'], DEBUG)

  # Integrate info from backend
  if shared.Settings.SIDE_MODULE:
    # we don't need any JS library contents in side modules
    shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = []

  all_funcs = shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + [shared.JS.to_nice_ident(d) for d in metadata['declares']]
  implemented_funcs = [x[1:] for x in metadata['implementedFunctions']]
  shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = sorted(set(all_funcs).difference(implemented_funcs))

  shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [x[1:] for x in metadata['externs']]

  shared.Settings.MAX_GLOBAL_ALIGN = metadata['maxGlobalAlign']
  shared.Settings.IMPLEMENTED_FUNCTIONS = metadata['implementedFunctions']

  if metadata['asmConsts']:
    # emit the EM_ASM signature-reading helper function only if we have any EM_ASM
    # functions in the module.
    shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$readAsmConstArgs']

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

    shared.Settings.PROXIED_FUNCTION_SIGNATURES = read_proxied_function_signatures(metadata['asmConsts'])

  shared.Settings.STATIC_BUMP = align_static_bump(metadata)

  shared.Settings.BINARYEN_FEATURES = metadata['features']
  shared.Settings.WASM_TABLE_SIZE = metadata['tableSize']
  if shared.Settings.RELOCATABLE:
    # When building relocatable output (e.g. MAIN_MODULE) the reported table
    # size does not include the reserved slot at zero for the null pointer.
    # Instead we use __table_base to offset the elements by 1.
    shared.Settings.WASM_TABLE_SIZE += 1
  shared.Settings.MAIN_READS_PARAMS = metadata['mainReadsParams']


# static code hooks
class StaticCodeHooks:
  atinits = []
  atmains = []
  atexits = []


def apply_static_code_hooks(code):
  code = code.replace('{{{ ATINITS }}}', StaticCodeHooks.atinits)
  code = code.replace('{{{ ATMAINS }}}', StaticCodeHooks.atmains)
  code = code.replace('{{{ ATEXITS }}}', StaticCodeHooks.atexits)
  return code


def apply_forwarded_data(forwarded_data):
  forwarded_json = json.loads(forwarded_data)
  # Be aware of JS static allocations
  shared.Settings.STATIC_BUMP = forwarded_json['STATIC_BUMP']
  shared.Settings.DYNAMICTOP_PTR = forwarded_json['DYNAMICTOP_PTR']
  # Be aware of JS static code hooks
  StaticCodeHooks.atinits = str(forwarded_json['ATINITS'])
  StaticCodeHooks.atmains = str(forwarded_json['ATMAINS'])
  StaticCodeHooks.atexits = str(forwarded_json['ATEXITS'])


def compile_settings(temp_files):
  # Save settings to a file to work around v8 issue 1579
  with temp_files.get_file('.txt') as settings_file:
    with open(settings_file, 'w') as s:
      json.dump(shared.Settings.to_dict(), s, sort_keys=True)

    # Call js compiler
    env = os.environ.copy()
    env['EMCC_BUILD_DIR'] = os.getcwd()
    out = shared.run_js_tool(path_from_root('src', 'compiler.js'),
                             [settings_file], stdout=subprocess.PIPE, stderr=STDERR_FILE,
                             cwd=path_from_root('src'), env=env)
  assert '//FORWARDED_DATA:' in out, 'Did not receive forwarded data in pre output - process failed?'
  glue, forwarded_data = out.split('//FORWARDED_DATA:')

  apply_forwarded_data(forwarded_data)

  return glue, forwarded_data


class Memory():
  def __init__(self):
    # Note: if RELOCATABLE, then only relative sizes can be computed, and we don't
    #       actually write out any absolute memory locations ({{{ STACK_BASE }}}
    #       does not exist, etc.)

    # Memory layout:
    #  * first the static globals
    self.global_base = shared.Settings.GLOBAL_BASE
    self.static_bump = shared.Settings.STATIC_BUMP
    #  * then the stack (up on fastcomp, down on upstream)
    self.stack_low = align_memory(self.global_base + self.static_bump)
    self.stack_high = align_memory(self.stack_low + shared.Settings.TOTAL_STACK)
    self.stack_base = self.stack_high
    self.stack_max = self.stack_low
    #  * then dynamic memory begins
    self.dynamic_base = align_memory(self.stack_high)

    if self.dynamic_base >= shared.Settings.INITIAL_MEMORY:
     exit_with_error('Memory is not large enough for static data (%d) plus the stack (%d), please increase INITIAL_MEMORY (%d) to at least %d' % (self.static_bump, shared.Settings.TOTAL_STACK, shared.Settings.INITIAL_MEMORY, self.dynamic_base))


def apply_memory(js):
  # Apply the statically-at-compile-time computed memory locations.
  memory = Memory()

  # Write it all out
  js = js.replace('{{{ STATIC_BUMP }}}', str(memory.static_bump))
  js = js.replace('{{{ STACK_BASE }}}', str(memory.stack_base))
  js = js.replace('{{{ STACK_MAX }}}', str(memory.stack_max))
  js = js.replace('{{{ DYNAMIC_BASE }}}', str(memory.dynamic_base))

  logger.debug('global_base: %d stack_base: %d, stack_max: %d, dynamic_base: %d, static bump: %d', memory.global_base, memory.stack_base, memory.stack_max, memory.dynamic_base, memory.static_bump)

  shared.Settings.DYNAMIC_BASE = memory.dynamic_base
  shared.Settings.STACK_BASE = memory.stack_base

  return js


def apply_table(js):
  js = js.replace('{{{ WASM_TABLE_SIZE }}}', str(shared.Settings.WASM_TABLE_SIZE))

  return js


def report_missing_symbols(all_implemented, pre):
  # the initial list of missing functions are that the user explicitly exported
  # but were not implemented in compiled code
  missing = list(set(shared.Settings.USER_EXPORTED_FUNCTIONS) - all_implemented)

  for requested in missing:
    if ('function ' + asstr(requested)) in pre:
      continue
    # special-case malloc, EXPORTED by default for internal use, but we bake in a
    # trivial allocator and warn at runtime if used in ASSERTIONS
    if missing == '_malloc':
      continue
    diagnostics.warning('undefined', 'undefined exported function: "%s"', requested)

  # Special hanlding for the `_main` symbol

  if shared.Settings.STANDALONE_WASM:
    # standalone mode doesn't use main, and it always reports missing entry point at link time.
    # In this mode we never expect _main in the export list.
    return

  if shared.Settings.IGNORE_MISSING_MAIN:
    # The default mode for emscripten is to ignore the missing main function allowing
    # maximum compatibility.
    return

  if shared.Settings.EXPECT_MAIN and '_main' not in all_implemented:
    # For compatibility with the output of wasm-ld we use the same wording here in our
    # error message as if wasm-ld had failed (i.e. in LLD_REPORT_UNDEFINED mode).
    exit_with_error('entry symbol not defined (pass --no-entry to suppress): main')


def proxy_debug_print(sync):
  if shared.Settings.PTHREADS_DEBUG:
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


def create_fp_accessors(metadata):
  if not shared.Settings.RELOCATABLE:
    return ''

  # Create `fp$XXX` handlers for determining function pionters (table addresses)
  # at runtime.
  # For SIDE_MODULEs these are generated by the proxyHandler at runtime.
  accessors = []
  for fullname in metadata['declares']:
    if not fullname.startswith('fp$'):
      continue
    _, name, sig = fullname.split('$')
    mangled = asmjs_mangle(name)
    side = 'parent' if shared.Settings.SIDE_MODULE else ''
    assertion = ('\n  assert(%sModule["%s"] || typeof %s !== "undefined", "external function `%s` is missing.' % (side, mangled, mangled, name) +
                 'perhaps a side module was not linked in? if this symbol was expected to arrive '
                 'from a system library, try to build the MAIN_MODULE with '
                 'EMCC_FORCE_STDLIBS=XX in the environment");')
    # the name of the original function is generally the normal function
    # name, unless it is legalized, in which case the export is the legalized
    # version, and the original provided by orig$X
    if shared.Settings.LEGALIZE_JS_FFI and not shared.JS.is_legal_sig(sig):
      name = 'orig$' + name

    accessors.append('''
Module['%(full)s'] = function() {
  %(assert)s
  // Use the original wasm function itself, for the table, from the main module.
  var func = Module['asm']['%(original)s'];
  // Try an original version from a side module.
  if (!func) func = Module['_%(original)s'];
  // Otherwise, look for a regular function or JS library function.
  if (!func) func = Module['%(mangled)s'];
  if (!func) func = %(mangled)s;
  var fp = addFunction(func, '%(sig)s');
  Module['%(full)s'] = function() { return fp };
  return fp;
}
''' % {'full': asmjs_mangle(fullname), 'mangled': mangled, 'original': name, 'assert': assertion, 'sig': sig})

  return '\n'.join(accessors)


def create_named_globals(metadata):
  if not shared.Settings.RELOCATABLE:
    named_globals = []
    for k, v in metadata['namedGlobals'].items():
      # We keep __data_end alive internally so that wasm-emscripten-finalize knows where the
      # static data region ends.  Don't export this to JS like other user-exported global
      # address.
      if k not in ['__data_end']:
        named_globals.append("Module['_%s'] = %s;" % (k, v))
    return '\n'.join(named_globals)

  named_globals = '''
var NAMED_GLOBALS = {
  %s
};
for (var named in NAMED_GLOBALS) {
  Module['_' + named] = gb + NAMED_GLOBALS[named];
}
Module['NAMED_GLOBALS'] = NAMED_GLOBALS;
''' % ',\n  '.join('"' + k + '": ' + str(v) for k, v in metadata['namedGlobals'].items())

  if shared.Settings.WASM:
    # wasm side modules are pure wasm, and cannot create their g$..() methods, so we help them out
    # TODO: this works if we are the main module, but if the supplying module is later, it won't, so
    #       we'll need another solution for that. one option is to scan the module imports, if/when
    #       wasm supports that, then the loader can do this.
    named_globals += '''
for (var named in NAMED_GLOBALS) {
  (function(named) {
    var addr = Module['_' + named];
    Module['g$_' + named] = function() { return addr };
  })(named);
}
'''
  named_globals += ''.join(["Module['%s'] = Module['%s'];\n" % (k, v) for k, v in metadata['aliases'].items()])
  return named_globals


def emscript(infile, outfile, memfile, temp_files, DEBUG):
  # Overview:
  #   * Run wasm-emscripten-finalize to extract metadata and modify the binary
  #     to use emscripten's wasm<->JS ABI
  #   * Use the metadata to generate the JS glue that goes with the wasm

  metadata = finalize_wasm(temp_files, infile, outfile, memfile, DEBUG)

  update_settings_glue(metadata, DEBUG)

  if shared.Settings.SIDE_MODULE:
    return

  if DEBUG:
    logger.debug('emscript: js compiler glue')

  if DEBUG:
    t = time.time()
  glue, forwarded_data = compile_settings(temp_files)
  if DEBUG:
    logger.debug('  emscript: glue took %s seconds' % (time.time() - t))
    t = time.time()

  forwarded_json = json.loads(forwarded_data)
  # For the wasm backend the implementedFunctions from compiler.js should
  # always be empty. This only gets populated for __asm function when using
  # the JS backend.
  assert not forwarded_json['Functions']['implementedFunctions']

  pre, post = glue.split('// EMSCRIPTEN_END_FUNCS')

  # memory and global initializers

  global_initializers = ', '.join('{ func: function() { %s() } }' % i for i in metadata['initializers'])

  staticbump = shared.Settings.STATIC_BUMP

  if shared.Settings.MINIMAL_RUNTIME:
    # In minimal runtime, global initializers are run after the Wasm Module instantiation has finished.
    global_initializers = ''
  else:
    # In regular runtime, global initializers are recorded in an __ATINIT__ array.
    global_initializers = '''/* global initializers */ %s __ATINIT__.push(%s);
''' % ('if (!ENVIRONMENT_IS_PTHREAD)' if shared.Settings.USE_PTHREADS else '',
       global_initializers)

  pre = pre.replace('STATICTOP = STATIC_BASE + 0;', '''STATICTOP = STATIC_BASE + %d;
%s
''' % (staticbump, global_initializers))

  pre = apply_memory(pre)
  pre = apply_static_code_hooks(pre) # In regular runtime, atinits etc. exist in the preamble part
  post = apply_static_code_hooks(post) # In MINIMAL_RUNTIME, atinit exists in the postamble part

  if shared.Settings.RELOCATABLE and not shared.Settings.SIDE_MODULE:
    pre += 'var gb = GLOBAL_BASE, fb = 0;\n'

  # merge forwarded data
  shared.Settings.EXPORTED_FUNCTIONS = forwarded_json['EXPORTED_FUNCTIONS']

  exports = metadata['exports']

  # Store exports for Closure compiler to be able to track these as globals in
  # -s DECLARE_ASM_MODULE_EXPORTS=0 builds.
  shared.Settings.MODULE_EXPORTS = [(asmjs_mangle(f), f) for f in exports]

  if shared.Settings.ASYNCIFY:
    exports += ['asyncify_start_unwind', 'asyncify_stop_unwind', 'asyncify_start_rewind', 'asyncify_stop_rewind']

  report_missing_symbols(set([asmjs_mangle(f) for f in exports]), pre)

  asm_consts = create_asm_consts_wasm(forwarded_json, metadata)
  em_js_funcs = create_em_js(forwarded_json, metadata)
  asm_const_pairs = ['%s: %s' % (key, value) for key, value in asm_consts]
  asm_const_map = 'var ASM_CONSTS = {\n  ' + ',  \n '.join(asm_const_pairs) + '\n};\n'
  pre = pre.replace(
    '// === Body ===',
    ('// === Body ===\n\n' + asm_const_map +
     '\n'.join(em_js_funcs) + '\n'))
  pre = apply_table(pre)
  outfile.write(pre)
  pre = None

  invoke_funcs = metadata['invokeFuncs']
  try:
    del forwarded_json['Variables']['globals']['_llvm_global_ctors'] # not a true variable
  except KeyError:
    pass

  sending = create_sending_wasm(invoke_funcs, forwarded_json, metadata)
  receiving = create_receiving_wasm(exports, metadata['initializers'])

  if shared.Settings.MINIMAL_RUNTIME:
    post, receiving = compute_minimal_runtime_initializer_and_exports(post, metadata['initializers'], exports, receiving)

  module = create_module_wasm(sending, receiving, invoke_funcs, metadata)

  write_output_file(outfile, post, module)
  module = None

  outfile.close()


def remove_trailing_zeros(memfile):
  with open(memfile, 'rb') as f:
    mem_data = f.read()
  end = len(mem_data)
  while end > 0 and (mem_data[end - 1] == b'\0' or mem_data[end - 1] == 0):
    end -= 1
  with open(memfile, 'wb') as f:
    f.write(mem_data[:end])


def finalize_wasm(temp_files, infile, outfile, memfile, DEBUG):
  basename = shared.unsuffixed(outfile.name)
  wasm = basename + '.wasm'
  base_wasm = infile
  building.save_intermediate(infile, 'base.wasm')

  args = ['--detect-features']

  write_source_map = shared.Settings.DEBUG_LEVEL >= 4
  if write_source_map:
    building.emit_wasm_source_map(base_wasm, base_wasm + '.map')
    building.save_intermediate(base_wasm + '.map', 'base_wasm.map')
    args += ['--output-source-map-url=' + shared.Settings.SOURCE_MAP_BASE + os.path.basename(shared.Settings.WASM_BINARY_FILE) + '.map']

  # tell binaryen to look at the features section, and if there isn't one, to use MVP
  # (which matches what llvm+lld has given us)
  if shared.Settings.DEBUG_LEVEL >= 2 or shared.Settings.PROFILING_FUNCS or shared.Settings.EMIT_SYMBOL_MAP or shared.Settings.ASYNCIFY_ONLY or shared.Settings.ASYNCIFY_REMOVE or shared.Settings.ASYNCIFY_ADD:
    args.append('-g')
  if shared.Settings.WASM_BIGINT:
    args.append('--bigint')
  if shared.Settings.LEGALIZE_JS_FFI != 1:
    args.append('--no-legalize-javascript-ffi')
  if not shared.Settings.MEM_INIT_IN_WASM:
    args.append('--separate-data-segments=' + memfile)
  if shared.Settings.SIDE_MODULE:
    args.append('--side-module')
  else:
    # --global-base is used by wasm-emscripten-finalize to calculate the size
    # of the static data used.  The argument we supply here needs to match the
    # global based used by lld (see building.link_lld).  For relocatable this is
    # zero for the global base although at runtime __memory_base is used.
    # For non-relocatable output we used shared.Settings.GLOBAL_BASE.
    # TODO(sbc): Can we remove this argument infer this from the segment
    # initializer?
    if shared.Settings.RELOCATABLE:
      args.append('--global-base=0')
    else:
      args.append('--global-base=%s' % shared.Settings.GLOBAL_BASE)
  if shared.Settings.STACK_OVERFLOW_CHECK >= 2:
    args.append('--check-stack-overflow')
  if shared.Settings.STANDALONE_WASM:
    args.append('--standalone-wasm')
  # When we dynamically link our JS loader adds functions from wasm modules to
  # the table. It must add the original versions of them, not legalized ones,
  # so that indirect calls have the right type, so export those.
  if shared.Settings.RELOCATABLE:
    args.append('--pass-arg=legalize-js-interface-export-originals')
  if shared.Settings.DEBUG_LEVEL >= 3:
    args.append('--dwarf')
  args.append('--minimize-wasm-changes')
  stdout = building.run_binaryen_command('wasm-emscripten-finalize',
                                         infile=base_wasm,
                                         outfile=wasm,
                                         args=args,
                                         stdout=subprocess.PIPE)
  if write_source_map:
    building.save_intermediate(wasm + '.map', 'post_finalize.map')
  building.save_intermediate(wasm, 'post_finalize.wasm')

  if not shared.Settings.MEM_INIT_IN_WASM:
    # we have a separate .mem file. binaryen did not strip any trailing zeros,
    # because it's an ABI question as to whether it is valid to do so or not.
    # we can do so here, since we make sure to zero out that memory (even in
    # the dynamic linking case, our loader zeros it out)
    remove_trailing_zeros(memfile)

  return load_metadata_wasm(stdout, DEBUG)


def create_asm_consts_wasm(forwarded_json, metadata):
  asm_consts = {}
  for k, v in metadata['asmConsts'].items():
    const, sigs, call_types = v
    const = asstr(const)
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
  asm_consts = [(key, value) for key, value in asm_consts.items()]
  asm_consts.sort()
  return asm_consts


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
    arg_names = [arg.split()[-1].replace("*", "") for arg in args if arg]
    func = 'function {}({}){}'.format(name, ','.join(arg_names), asstr(body))
    em_js_funcs.append(func)
    forwarded_json['Functions']['libraryFunctions'][name] = 1

  return em_js_funcs


def add_standard_wasm_imports(send_items_map):
  # Normally we import these into the wasm (so that JS could use them even
  # before the wasm loads), while in standalone mode we do not depend
  # on JS to create them, but create them in the wasm and export them.
  if not shared.Settings.STANDALONE_WASM:
    memory_import = 'wasmMemory'
    if shared.Settings.MODULARIZE and shared.Settings.USE_PTHREADS:
      # Pthreads assign wasmMemory in their worker startup. In MODULARIZE mode, they cannot assign inside the
      # Module scope, so lookup via Module as well.
      memory_import += " || Module['wasmMemory']"
    send_items_map['memory'] = memory_import

    send_items_map['table'] = 'wasmTable'

  # With the wasm backend __memory_base and __table_base are only needed for
  # relocatable output.
  if shared.Settings.RELOCATABLE:
    send_items_map['__memory_base'] = str(shared.Settings.GLOBAL_BASE) # tell the memory segments where to place themselves
    # the wasm backend reserves slot 0 for the NULL function pointer
    table_base = '1'
    send_items_map['__table_base'] = table_base
  if shared.Settings.RELOCATABLE:
    send_items_map['__stack_pointer'] = 'STACK_BASE'

  if shared.Settings.MAYBE_WASM2JS or shared.Settings.AUTODEBUG or shared.Settings.LINKABLE:
    # legalization of i64 support code may require these in some modes
    send_items_map['setTempRet0'] = 'setTempRet0'
    send_items_map['getTempRet0'] = 'getTempRet0'

  if shared.Settings.AUTODEBUG:
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


def create_sending_wasm(invoke_funcs, forwarded_json, metadata):
  basic_funcs = []
  if shared.Settings.SAFE_HEAP:
    basic_funcs += ['segfault', 'alignfault']

  em_js_funcs = list(metadata['emJsFuncs'].keys())
  declared_items = ['_' + item for item in metadata['declares']]
  send_items = set(basic_funcs + invoke_funcs + em_js_funcs + declared_items)

  def fix_import_name(g):
    if g.startswith('Math_'):
      return g.split('_')[1]
    # Unlike fastcomp the wasm backend doesn't use the '_' prefix for native
    # symbols.  Emscripten currently expects symbols to start with '_' so we
    # artificially add them to the output of emscripten-wasm-finalize and them
    # strip them again here.
    # note that we don't do this for EM_JS functions (which, rarely, may have
    # a '_' prefix)
    if g.startswith('_') and g not in metadata['emJsFuncs']:
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
  return '{ ' + ', '.join('"' + k + '": ' + send_items_map[k] for k in sorted_keys) + ' }'


def make_export_wrappers(exports, delay_assignment):
  wrappers = []
  for name in exports:
    mangled = asmjs_mangle(name)
    if shared.Settings.ASSERTIONS:
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


def create_receiving_wasm(exports, initializers):
  # When not declaring asm exports this section is empty and we instead programatically export
  # symbols on the global object by calling exportAsmFunctions after initialization
  if not shared.Settings.DECLARE_ASM_MODULE_EXPORTS:
    return ''

  exports_that_are_not_initializers = [x for x in exports if x not in initializers]

  receiving = []

  # with WASM_ASYNC_COMPILATION that asm object may not exist at this point in time
  # so we need to support delayed assignment.
  delay_assignment = (shared.Settings.WASM and shared.Settings.WASM_ASYNC_COMPILATION) and not shared.Settings.MINIMAL_RUNTIME
  if not delay_assignment:
    if shared.Settings.WASM and shared.Settings.MINIMAL_RUNTIME:
      # In Wasm exports are assigned inside a function to variables existing in top level JS scope, i.e.
      # var _main;
      # WebAssembly.instantiate(Module["wasm"], imports).then((function(output) {
      # var asm = output.instance.exports;
      # _main = asm["_main"];
      receiving += [asmjs_mangle(s) + ' = asm["' + s + '"];' for s in exports_that_are_not_initializers]
    else:
      if shared.Settings.MINIMAL_RUNTIME:
        # In wasm2js exports can be directly processed at top level, i.e.
        # var asm = Module["asm"](asmGlobalArg, asmLibraryArg, buffer);
        # var _main = asm["_main"];
        if shared.Settings.USE_PTHREADS and shared.Settings.MODULARIZE:
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

  return '\n'.join(receiving) + '\n'


def create_module_wasm(sending, receiving, invoke_funcs, metadata):
  invoke_wrappers = create_invoke_wrappers(invoke_funcs)
  receiving += create_named_globals(metadata)
  receiving += create_fp_accessors(metadata)
  module = []
  module.append('var asmGlobalArg = {};\n')
  if shared.Settings.USE_PTHREADS and not shared.Settings.WASM:
    module.append("if (typeof SharedArrayBuffer !== 'undefined') asmGlobalArg['Atomics'] = Atomics;\n")

  module.append('var asmLibraryArg = %s;\n' % (sending))
  if shared.Settings.ASYNCIFY and shared.Settings.ASSERTIONS:
    module.append('Asyncify.instrumentWasmImports(asmLibraryArg);\n')

  if not shared.Settings.MINIMAL_RUNTIME:
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
    'aliases': {},
    'declares': [],
    'implementedFunctions': [],
    'externs': [],
    'simd': False, # Obsolete, always False
    'maxGlobalAlign': 0,
    'staticBump': 0,
    'tableSize': 0,
    'initializers': [],
    'exports': [],
    'namedGlobals': {},
    'emJsFuncs': {},
    'asmConsts': {},
    'invokeFuncs': [],
    'features': [],
    'mainReadsParams': 1,
  }

  assert 'tableSize' in metadata_json.keys()
  for key, value in metadata_json.items():
    # json.loads returns `unicode` for strings but other code in this file
    # generally works with utf8 encoded `str` objects, and they don't alwasy
    # mix well.  e.g. s.replace(x, y) will blow up is `s` a uts8 str containing
    # non-ascii and either x or y are unicode objects.
    # TODO(sbc): Remove this encoding if we switch to unicode elsewhere
    # (specifically the glue returned from compile_settings)
    if type(value) == list:
      value = [asstr(v) for v in value]
    if key not in metadata:
      exit_with_error('unexpected metadata key received from wasm-emscripten-finalize: %s', key)
    metadata[key] = value

  if not shared.Settings.MINIMAL_RUNTIME:
    # In regular runtime initializers call the global var version of the export, so they get the mangled name.
    # In MINIMAL_RUNTIME, the initializers are called directly off the export object for minimal code size.
    metadata['initializers'] = [asmjs_mangle(i) for i in metadata['initializers']]

  if DEBUG:
    logger.debug("Metadata parsed: " + pprint.pformat(metadata))

  # Calculate the subset of exports that were explicitly marked with llvm.used.
  # These are any exports that were not requested on the command line and are
  # not known auto-generated system functions.
  unexpected_exports = [e for e in metadata['exports'] if treat_as_user_function(e)]
  unexpected_exports = [asmjs_mangle(e) for e in unexpected_exports]
  unexpected_exports = [e for e in unexpected_exports if e not in shared.Settings.EXPORTED_FUNCTIONS]
  building.user_requested_exports += unexpected_exports

  # With the wasm backend the set of implemented functions is identical to the set of exports
  # Set this key here simply so that the shared code that handle it.
  metadata['implementedFunctions'] = [asmjs_mangle(x) for x in metadata['exports']]

  return metadata


def create_invoke_wrappers(invoke_funcs):
  """Asm.js-style exception handling: invoke wrapper generation."""
  invoke_wrappers = ''
  for invoke in invoke_funcs:
    sig = invoke[len('invoke_'):]
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
  if not shared.Settings.STRUCT_INFO and not shared.Settings.BOOTSTRAPPING_STRUCT_INFO:
    generated_struct_info_name = 'generated_struct_info.json'

    def generate_struct_info():
      with ToolchainProfiler.profile_block('gen_struct_info'):
        out = shared.Cache.get_path(generated_struct_info_name)
        gen_struct_info.main(['-q', '-c', '-o', out])
        return out

    shared.Settings.STRUCT_INFO = shared.Cache.get(generated_struct_info_name, generate_struct_info)
  # do we need an else, to define it for the bootstrap case?


def run(infile, outfile, memfile):
  temp_files = shared.configuration.get_temp_files()
  infile, outfile = substitute_response_files([infile, outfile])
  generate_struct_info()

  outfile_obj = open(outfile, 'w')

  return temp_files.run_and_clean(lambda: emscript(
      infile, outfile_obj, memfile, temp_files, shared.DEBUG)
  )
