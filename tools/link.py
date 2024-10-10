# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from .toolchain_profiler import ToolchainProfiler

import base64
import glob
import hashlib
import json
import logging
import os
import re
import shlex
import stat
import shutil
import time
from subprocess import PIPE
from urllib.parse import quote

from . import building
from . import cache
from . import config
from . import diagnostics
from . import emscripten
from . import feature_matrix
from . import filelock
from . import js_manipulation
from . import ports
from . import shared
from . import system_libs
from . import utils
from . import webassembly
from . import extract_metadata
from .utils import read_file, write_file, delete_file
from .utils import removeprefix, exit_with_error
from .shared import in_temp, safe_copy, do_replace, OFormat
from .shared import DEBUG, WINDOWS, DYNAMICLIB_ENDINGS, STATICLIB_ENDINGS
from .shared import unsuffixed, unsuffixed_basename, get_file_suffix
from .settings import settings, default_setting, user_settings, JS_ONLY_SETTINGS, DEPRECATED_SETTINGS
from .minimal_runtime_shell import generate_minimal_runtime_html

import tools.line_endings

logger = logging.getLogger('link')

DEFAULT_SHELL_HTML = utils.path_from_root('src/shell.html')

DEFAULT_ASYNCIFY_IMPORTS = ['__asyncjs__*']

DEFAULT_ASYNCIFY_EXPORTS = [
  'main',
  '__main_argc_argv',
]

VALID_ENVIRONMENTS = ('web', 'webview', 'worker', 'node', 'shell')

EXECUTABLE_ENDINGS = ['.wasm', '.html', '.js', '.mjs', '.out', '']

# Supported LLD flags which we will pass through to the linker.
SUPPORTED_LINKER_FLAGS = (
    '--start-group', '--end-group',
    '-(', '-)',
    '--whole-archive', '--no-whole-archive',
    '-whole-archive', '-no-whole-archive'
)

# Unsupported LLD flags which we will ignore.
# Maps to true if the flag takes an argument.
UNSUPPORTED_LLD_FLAGS = {
    # macOS-specific linker flag that libtool (ltmain.sh) will if macOS is detected.
    '-bind_at_load': False,
    # wasm-ld doesn't support soname or other dynamic linking flags (yet).   Ignore them
    # in order to aid build systems that want to pass these flags.
    '-soname': True,
    '-allow-shlib-undefined': False,
    '-rpath': True,
    '-rpath-link': True,
    '-version-script': True,
    '-install_name': True,
}

UBSAN_SANITIZERS = {
  'alignment',
  'bool',
  'builtin',
  'bounds',
  'enum',
  'float-cast-overflow',
  'float-divide-by-zero',
  'function',
  'implicit-unsigned-integer-truncation',
  'implicit-signed-integer-truncation',
  'implicit-integer-sign-change',
  'integer-divide-by-zero',
  'nonnull-attribute',
  'null',
  'nullability-arg',
  'nullability-assign',
  'nullability-return',
  'object-size',
  'pointer-overflow',
  'return',
  'returns-nonnull-attribute',
  'shift',
  'signed-integer-overflow',
  'unreachable',
  'unsigned-integer-overflow',
  'vla-bound',
  'vptr',
  'undefined',
  'undefined-trap',
  'implicit-integer-truncation',
  'implicit-integer-arithmetic-value-change',
  'implicit-conversion',
  'integer',
  'nullability',
}


final_js = None


# this function uses the global 'final' variable, which contains the current
# final output file. if a method alters final, and calls this method, then it
# must modify final globally (i.e. it can't receive final as a param and
# return it)
# TODO: refactor all this, a singleton that abstracts over the final output
#       and saving of intermediates
def save_intermediate(name, suffix='js'):
  if not DEBUG:
    return
  if not final_js:
    logger.debug(f'(not saving intermediate {name} because not generating JS)')
    return
  building.save_intermediate(final_js, f'{name}.{suffix}')


def save_intermediate_with_wasm(name, wasm_binary):
  if not DEBUG:
    return
  save_intermediate(name) # save the js
  building.save_intermediate(wasm_binary, name + '.wasm')


def base64_encode(filename):
  data = utils.read_binary(filename)
  b64 = base64.b64encode(data)
  return b64.decode('ascii')


def align_to_wasm_page_boundary(address):
  page_size = webassembly.WASM_PAGE_SIZE
  return ((address + (page_size - 1)) // page_size) * page_size


def will_metadce():
  # The metadce JS parsing code does not currently support the JS that gets generated
  # when assertions are enabled.
  if settings.ASSERTIONS:
    return False
  return settings.OPT_LEVEL >= 3 or settings.SHRINK_LEVEL >= 1


def setup_environment_settings():
  # Environment setting based on user input
  environments = settings.ENVIRONMENT.split(',')
  if any([x for x in environments if x not in VALID_ENVIRONMENTS]):
    exit_with_error(f'Invalid environment specified in "ENVIRONMENT": {settings.ENVIRONMENT}. Should be one of: {",".join(VALID_ENVIRONMENTS)}')

  settings.ENVIRONMENT_MAY_BE_WEB = not settings.ENVIRONMENT or 'web' in environments
  settings.ENVIRONMENT_MAY_BE_WEBVIEW = not settings.ENVIRONMENT or 'webview' in environments
  settings.ENVIRONMENT_MAY_BE_NODE = not settings.ENVIRONMENT or 'node' in environments
  settings.ENVIRONMENT_MAY_BE_SHELL = not settings.ENVIRONMENT or 'shell' in environments

  # The worker case also includes Node.js workers when pthreads are
  # enabled and Node.js is one of the supported environments for the build to
  # run on. Node.js workers are detected as a combination of
  # ENVIRONMENT_IS_WORKER and ENVIRONMENT_IS_NODE.
  settings.ENVIRONMENT_MAY_BE_WORKER = \
      not settings.ENVIRONMENT or \
      'worker' in environments or \
      (settings.ENVIRONMENT_MAY_BE_NODE and settings.PTHREADS)

  if not settings.ENVIRONMENT_MAY_BE_WORKER and settings.PROXY_TO_WORKER:
    exit_with_error('if you specify --proxy-to-worker and specify a "-sENVIRONMENT=" directive, it must include "worker" as a target! (Try e.g. -sENVIRONMENT=web,worker)')

  if not settings.ENVIRONMENT_MAY_BE_WORKER and settings.SHARED_MEMORY:
    exit_with_error('when building with multithreading enabled and a "-sENVIRONMENT=" directive is specified, it must include "worker" as a target! (Try e.g. -sENVIRONMENT=web,worker)')


def generate_js_sym_info():
  # Runs the js compiler to generate a list of all symbols available in the JS
  # libraries.  This must be done separately for each linker invocation since the
  # list of symbols depends on what settings are used.
  # TODO(sbc): Find a way to optimize this.  Potentially we could add a super-set
  # mode of the js compiler that would generate a list of all possible symbols
  # that could be checked in.
  _, forwarded_data = emscripten.compile_javascript(symbols_only=True)
  # When running in symbols_only mode compiler.mjs outputs a flat list of C symbols.
  return json.loads(forwarded_data)


@ToolchainProfiler.profile_block('JS symbol generation')
def get_js_sym_info():
  # Avoiding using the cache when generating struct info since
  # this step is performed while the cache is locked.
  if DEBUG or settings.BOOTSTRAPPING_STRUCT_INFO or config.FROZEN_CACHE:
    return generate_js_sym_info()

  # We define a cache hit as when the settings and `--js-library` contents are
  # identical.
  # Ignore certain settings that can are no relevant to library deps.  Here we
  # skip PRE_JS_FILES/POST_JS_FILES which don't effect the library symbol list
  # and can contain full paths to temporary files.
  skip_settings = {'PRE_JS_FILES', 'POST_JS_FILES'}
  input_files = [json.dumps(settings.external_dict(skip_keys=skip_settings), sort_keys=True, indent=2)]
  for jslib in sorted(glob.glob(utils.path_from_root('src') + '/library*.js')):
    input_files.append(read_file(jslib))
  for jslib in settings.JS_LIBRARIES:
    if not os.path.isabs(jslib):
      jslib = utils.path_from_root('src', jslib)
    input_files.append(read_file(jslib))
  content = '\n'.join(input_files)
  content_hash = hashlib.sha1(content.encode('utf-8')).hexdigest()

  def build_symbol_list(filename):
    """Only called when there is no existing symbol list for a given content hash.
    """
    library_syms = generate_js_sym_info()

    write_file(filename, json.dumps(library_syms, separators=(',', ':'), indent=2))

  # We need to use a separate lock here for symbol lists because, unlike with system libraries,
  # it's normally for these file to get pruned as part of normal operation.  This means that it
  # can be deleted between the `cache.get()` then the `read_file`.
  with filelock.FileLock(cache.get_path(cache.get_path('symbol_lists.lock'))):
    filename = cache.get(f'symbol_lists/{content_hash}.json', build_symbol_list)
    library_syms = json.loads(read_file(filename))

    # Limit of the overall size of the cache to 100 files.
    # This code will get test coverage since a full test run of `other` or `core`
    # generates ~1000 unique symbol lists.
    cache_limit = 500
    root = cache.get_path('symbol_lists')
    if len(os.listdir(root)) > cache_limit:
      files = []
      for f in os.listdir(root):
        f = os.path.join(root, f)
        files.append((f, os.path.getmtime(f)))
      files.sort(key=lambda x: x[1])
      # Delete all but the newest N files
      for f, _ in files[:-cache_limit]:
        delete_file(f)

  return library_syms


def filter_link_flags(flags, using_lld):
  def is_supported(f):
    if using_lld:
      for flag, takes_arg in UNSUPPORTED_LLD_FLAGS.items():
        # lld allows various flags to have either a single -foo or double --foo
        if f.startswith(flag) or f.startswith('-' + flag):
          diagnostics.warning('linkflags', 'ignoring unsupported linker flag: `%s`', f)
          # Skip the next argument if this linker flag takes and argument and that
          # argument was not specified as a separately (i.e. it was specified as
          # single arg containing an `=` char.)
          skip_next = takes_arg and '=' not in f
          return False, skip_next
      return True, False
    else:
      if f in SUPPORTED_LINKER_FLAGS:
        return True, False
      # Silently ignore -l/-L flags when not using lld.  If using lld allow
      # them to pass through the linker
      if f.startswith('-l') or f.startswith('-L'):
        return False, False
      diagnostics.warning('linkflags', 'ignoring unsupported linker flag: `%s`', f)
      return False, False

  results = []
  skip_next = False
  for f in flags:
    if skip_next:
      skip_next = False
      continue
    keep, skip_next = is_supported(f[1])
    if keep:
      results.append(f)

  return results


def fix_windows_newlines(text):
  # Avoid duplicating \r\n to \r\r\n when writing out text.
  if WINDOWS:
    text = text.replace('\r\n', '\n')
  return text


def read_js_files(files):
  contents = []
  for f in files:
    content = read_file(f)
    if content.startswith('#preprocess\n'):
      contents.append(shared.read_and_preprocess(f, expand_macros=True))
    else:
      contents.append(content)
  contents = '\n'.join(contents)
  return fix_windows_newlines(contents)


def should_run_binaryen_optimizer():
  # run the binaryen optimizer in -O2+. in -O0 we don't need it obviously, while
  # in -O1 we don't run it as the LLVM optimizer has been run, and it does the
  # great majority of the work; not running the binaryen optimizer in that case
  # keeps -O1 mostly-optimized while compiling quickly and without rewriting
  # DWARF etc.
  return settings.OPT_LEVEL >= 2


def get_binaryen_passes():
  passes = []
  optimizing = should_run_binaryen_optimizer()
  # wasm-emscripten-finalize will strip the features section for us
  # automatically, but if we did not modify the wasm then we didn't run it,
  # and in an optimized build we strip it manually here. (note that in an
  # unoptimized build we might end up with the features section, if we neither
  # optimize nor run wasm-emscripten-finalize, but a few extra bytes in the
  # binary don't matter in an unoptimized build)
  if optimizing:
    passes += ['--strip-target-features']
  # safe heap must run before post-emscripten, so post-emscripten can apply the sbrk ptr
  if settings.SAFE_HEAP:
    passes += ['--safe-heap']
  # sign-ext is enabled by default by llvm.  If the target browser settings don't support
  # this we lower it away here using a binaryen pass.
  if not feature_matrix.caniuse(feature_matrix.Feature.SIGN_EXT):
    logger.debug('lowering sign-ext feature due to incompatible target browser engines')
    passes += ['--signext-lowering']
  if optimizing:
    passes += ['--post-emscripten']
    if settings.SIDE_MODULE:
      passes += ['--pass-arg=post-emscripten-side-module']
  if optimizing:
    passes += [building.opt_level_to_str(settings.OPT_LEVEL, settings.SHRINK_LEVEL)]
  # when optimizing, use the fact that low memory is never used (1024 is a
  # hardcoded value in the binaryen pass). we also cannot do it when the stack
  # is first, as then the stack is in the low memory that should be unused.
  if optimizing and settings.GLOBAL_BASE >= 1024 and not settings.STACK_FIRST:
    passes += ['--low-memory-unused']
  if settings.AUTODEBUG:
    # adding '--flatten' here may make these even more effective
    passes += ['--instrument-locals']
    passes += ['--log-execution']
    passes += ['--instrument-memory']
    if settings.LEGALIZE_JS_FFI:
      # legalize it again now, as the instrumentation may need it
      passes += ['--legalize-js-interface']
      passes += building.js_legalization_pass_flags()
  if settings.EMULATE_FUNCTION_POINTER_CASTS:
    # note that this pass must run before asyncify, as if it runs afterwards we only
    # generate the  byn$fpcast_emu  functions after asyncify runs, and so we wouldn't
    # be able to further process them.
    passes += ['--fpcast-emu']
  if settings.ASYNCIFY == 1:
    passes += ['--asyncify']
    if settings.MAIN_MODULE or settings.SIDE_MODULE:
      passes += ['--pass-arg=asyncify-relocatable']
    if settings.ASSERTIONS:
      passes += ['--pass-arg=asyncify-asserts']
    if settings.ASYNCIFY_ADVISE:
      passes += ['--pass-arg=asyncify-verbose']
    if settings.ASYNCIFY_IGNORE_INDIRECT:
      passes += ['--pass-arg=asyncify-ignore-indirect']
    if settings.ASYNCIFY_PROPAGATE_ADD:
      passes += ['--pass-arg=asyncify-propagate-addlist']
    passes += ['--pass-arg=asyncify-imports@%s' % ','.join(settings.ASYNCIFY_IMPORTS)]

    # shell escaping can be confusing; try to emit useful warnings
    def check_human_readable_list(items):
      for item in items:
        if item.count('(') != item.count(')'):
          logger.warning('emcc: ASYNCIFY list contains an item without balanced parentheses ("(", ")"):')
          logger.warning('   ' + item)
          logger.warning('This may indicate improper escaping that led to splitting inside your names.')
          logger.warning('Try using a response file. e.g: -sASYNCIFY_ONLY=@funcs.txt. The format is a simple')
          logger.warning('text file, one line per function.')
          break

    if settings.ASYNCIFY_REMOVE:
      check_human_readable_list(settings.ASYNCIFY_REMOVE)
      passes += ['--pass-arg=asyncify-removelist@%s' % ','.join(settings.ASYNCIFY_REMOVE)]
    if settings.ASYNCIFY_ADD:
      check_human_readable_list(settings.ASYNCIFY_ADD)
      passes += ['--pass-arg=asyncify-addlist@%s' % ','.join(settings.ASYNCIFY_ADD)]
    if settings.ASYNCIFY_ONLY:
      check_human_readable_list(settings.ASYNCIFY_ONLY)
      passes += ['--pass-arg=asyncify-onlylist@%s' % ','.join(settings.ASYNCIFY_ONLY)]

  if settings.MEMORY64 == 2:
    passes += ['--memory64-lowering', '--table64-lowering']

  if settings.BINARYEN_IGNORE_IMPLICIT_TRAPS:
    passes += ['--ignore-implicit-traps']
  # normally we can assume the memory, if imported, has not been modified
  # beforehand (in fact, in most cases the memory is not even imported anyhow,
  # but it is still safe to pass the flag), and is therefore filled with zeros.
  # the one exception is dynamic linking of a side module: the main module is ok
  # as it is loaded first, but the side module may be assigned memory that was
  # previously used.
  if optimizing and not settings.SIDE_MODULE:
    passes += ['--zero-filled-memory']
  # LLVM output always has immutable initial table contents: the table is
  # fixed and may only be appended to at runtime (that is true even in
  # relocatable mode)
  if optimizing:
    passes += ['--pass-arg=directize-initial-contents-immutable']

  if settings.BINARYEN_EXTRA_PASSES:
    # BINARYEN_EXTRA_PASSES is comma-separated, and we support both '-'-prefixed and
    # unprefixed pass names
    extras = settings.BINARYEN_EXTRA_PASSES.split(',')
    passes += [('--' + p) if p[0] != '-' else p for p in extras if p]

  # Run the translator to the new EH instructions with exnref
  if settings.WASM_EXNREF:
    passes += ['--emit-exnref']

  # If we are going to run metadce then that means we will be running binaryen
  # tools after the main invocation, whose flags are determined here
  # (specifically we will run metadce and possibly also wasm-opt for import/
  # export minification). And when we run such a tool it will "undo" any
  # StackIR optimizations (since the conversion to BinaryenIR undoes them as it
  # restructures the code). We could re-run those opts, but it is most efficient
  # to just not do them now if we'll invoke other tools later, and we'll do them
  # only in the very last invocation.
  if will_metadce():
    passes += ['--no-stack-ir']

  return passes


def make_js_executable(script):
  src = read_file(script)
  cmd = config.NODE_JS
  if settings.MEMORY64 == 1:
    cmd += shared.node_memory64_flags()
  elif settings.WASM_BIGINT:
    cmd += shared.node_bigint_flags(config.NODE_JS)
  if len(cmd) > 1 or not os.path.isabs(cmd[0]):
    # Using -S (--split-string) here means that arguments to the executable are
    # correctly parsed.  We don't do this by default because old versions of env
    # don't support -S.
    cmd = '/usr/bin/env -S ' + shared.shlex_join(cmd)
  else:
    cmd = shared.shlex_join(cmd)
  logger.debug('adding `#!` to JavaScript file: %s' % cmd)
  # add shebang
  with open(script, 'w') as f:
    f.write('#!%s\n' % cmd)
    f.write(src)
  try:
    os.chmod(script, stat.S_IMODE(os.stat(script).st_mode) | stat.S_IXUSR) # make executable
  except OSError:
    pass # can fail if e.g. writing the executable to /dev/null


def do_split_module(wasm_file, options):
  os.replace(wasm_file, wasm_file + '.orig')
  args = ['--instrument']
  if options.requested_debug:
    # Tell wasm-split to preserve function names.
    args += ['-g']
  building.run_binaryen_command('wasm-split', wasm_file + '.orig', outfile=wasm_file, args=args)


def get_worker_js_suffix():
  return '.worker.mjs' if settings.EXPORT_ES6 else '.worker.js'


def setup_pthreads():
  if settings.RELOCATABLE:
    # pthreads + dynamic linking has certain limitations
    if settings.SIDE_MODULE:
      diagnostics.warning('experimental', '-sSIDE_MODULE + pthreads is experimental')
    elif settings.MAIN_MODULE:
      diagnostics.warning('experimental', '-sMAIN_MODULE + pthreads is experimental')
    elif settings.LINKABLE:
      diagnostics.warning('experimental', '-sLINKABLE + pthreads is experimental')
  if settings.ALLOW_MEMORY_GROWTH:
    diagnostics.warning('pthreads-mem-growth', '-pthread + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271')

  default_setting('DEFAULT_PTHREAD_STACK_SIZE', settings.STACK_SIZE)

  if not settings.MINIMAL_RUNTIME and 'instantiateWasm' not in settings.INCOMING_MODULE_JS_API:
    # pthreads runtime depends on overriding instantiateWasm
    settings.INCOMING_MODULE_JS_API.append('instantiateWasm')

  # Functions needs by runtime_pthread.js
  settings.REQUIRED_EXPORTS += [
    '_emscripten_thread_free_data',
    '_emscripten_thread_crashed',
    'emscripten_main_runtime_thread_id',
    'emscripten_main_thread_process_queued_calls',
    '_emscripten_run_on_main_thread_js',
    'emscripten_stack_set_limits',
  ]

  if settings.EMBIND:
    settings.REQUIRED_EXPORTS.append('_embind_initialize_bindings')

  if settings.MAIN_MODULE:
    settings.REQUIRED_EXPORTS += [
      '_emscripten_dlsync_self',
      '_emscripten_dlsync_self_async',
      '_emscripten_proxy_dlsync',
      '_emscripten_proxy_dlsync_async',
      '__dl_seterr',
    ]

  # runtime_pthread.js depends on these library symbols
  settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
    '$PThread',
    '$establishStackSpace',
    '$invokeEntryPoint',
  ]

  if settings.MINIMAL_RUNTIME:
    building.user_requested_exports.add('exit')


def set_initial_memory():
  user_specified_initial_heap = 'INITIAL_HEAP' in user_settings

  # INITIAL_HEAP cannot be used when the memory object is created in JS: we don't know
  # the size of static data here and thus the total initial memory size.
  if settings.IMPORTED_MEMORY:
    if user_specified_initial_heap:
      # Some of these could (and should) be implemented.
      exit_with_error('INITIAL_HEAP is currently not compatible with IMPORTED_MEMORY (which is enabled indirectly via SHARED_MEMORY, RELOCATABLE, ASYNCIFY_LAZY_LOAD_CODE)')
    # The default for imported memory is to fall back to INITIAL_MEMORY.
    settings.INITIAL_HEAP = -1

  if not user_specified_initial_heap:
    # For backwards compatibility, we will only use INITIAL_HEAP by default when the user
    # specified neither INITIAL_MEMORY nor MAXIMUM_MEMORY. Both place an upper bounds on
    # the overall initial linear memory (stack + static data + heap), and we do not know
    # the size of static data at this stage. Setting any non-zero initial heap value in
    # this scenario would risk pushing users over the limit they have set.
    user_specified_initial = settings.INITIAL_MEMORY != -1
    user_specified_maximum = 'MAXIMUM_MEMORY' in user_settings or 'WASM_MEM_MAX' in user_settings or 'BINARYEN_MEM_MAX' in user_settings
    if user_specified_initial or user_specified_maximum:
      settings.INITIAL_HEAP = -1

  # Apply the default if we are going with INITIAL_MEMORY.
  if settings.INITIAL_HEAP == -1 and settings.INITIAL_MEMORY == -1:
    default_setting('INITIAL_MEMORY', 16 * 1024 * 1024)

  def check_memory_setting(setting):
    if settings[setting] % webassembly.WASM_PAGE_SIZE != 0:
      exit_with_error(f'{setting} must be a multiple of WebAssembly page size (64KiB), was {settings[setting]}')
    if settings[setting] >= 2**53:
      exit_with_error(f'{setting} must be smaller than 2^53 bytes due to JS Numbers (doubles) being used to hold pointer addresses in JS side')

  # Due to the aforementioned lack of knowledge about the static data size, we delegate
  # checking the overall consistency of these settings to wasm-ld.
  if settings.INITIAL_HEAP != -1:
    check_memory_setting('INITIAL_HEAP')

  if settings.INITIAL_MEMORY != -1:
    check_memory_setting('INITIAL_MEMORY')
    if settings.INITIAL_MEMORY < settings.STACK_SIZE:
      exit_with_error(f'INITIAL_MEMORY must be larger than STACK_SIZE, was {settings.INITIAL_MEMORY} (STACK_SIZE={settings.STACK_SIZE})')

  check_memory_setting('MAXIMUM_MEMORY')
  if settings.MEMORY_GROWTH_LINEAR_STEP != -1:
    check_memory_setting('MEMORY_GROWTH_LINEAR_STEP')


# Set an upper estimate of what MAXIMUM_MEMORY should be. Take note that this value
# may not be precise, and is only an upper bound of the exact value calculated later
# by the linker.
def set_max_memory():
  # With INITIAL_HEAP, we only know the lower bound on initial memory size.
  initial_memory_known = settings.INITIAL_MEMORY != -1

  if not settings.ALLOW_MEMORY_GROWTH:
    if 'MAXIMUM_MEMORY' in user_settings:
      diagnostics.warning('unused-command-line-argument', 'MAXIMUM_MEMORY is only meaningful with ALLOW_MEMORY_GROWTH')
    # Optimization: lower the default maximum memory to initial memory if possible.
    if initial_memory_known:
      settings.MAXIMUM_MEMORY = settings.INITIAL_MEMORY

  # Automaticaly up the default maximum when the user requested a large minimum.
  if 'MAXIMUM_MEMORY' not in user_settings:
    if settings.ALLOW_MEMORY_GROWTH:
      if any([settings.INITIAL_HEAP != -1 and settings.INITIAL_HEAP >= 2 * 1024 * 1024 * 1024,
              initial_memory_known and settings.INITIAL_MEMORY > 2 * 1024 * 1024 * 1024]):
        settings.MAXIMUM_MEMORY = 4 * 1024 * 1024 * 1024

    # INITIAL_MEMORY sets a lower bound for MAXIMUM_MEMORY
    if initial_memory_known and settings.INITIAL_MEMORY > settings.MAXIMUM_MEMORY:
      settings.MAXIMUM_MEMORY = settings.INITIAL_MEMORY

  # A similar check for INITIAL_HEAP would not be precise and so is delegated to wasm-ld.
  if initial_memory_known and settings.MAXIMUM_MEMORY < settings.INITIAL_MEMORY:
    exit_with_error('MAXIMUM_MEMORY cannot be less than INITIAL_MEMORY')


def inc_initial_memory(delta):
  # Both INITIAL_HEAP and INITIAL_MEMORY can be set at the same time. Increment both.
  if settings.INITIAL_HEAP != -1:
    settings.INITIAL_HEAP += delta
  if settings.INITIAL_MEMORY != -1:
    settings.INITIAL_MEMORY += delta


def check_browser_versions():
  # Map of setting all VM version settings to the minimum version
  # we support.
  min_version_settings = {
    'MIN_FIREFOX_VERSION': feature_matrix.OLDEST_SUPPORTED_FIREFOX,
    'MIN_CHROME_VERSION': feature_matrix.OLDEST_SUPPORTED_CHROME,
    'MIN_SAFARI_VERSION': feature_matrix.OLDEST_SUPPORTED_SAFARI,
    'MIN_NODE_VERSION': feature_matrix.OLDEST_SUPPORTED_NODE,
  }

  if settings.LEGACY_VM_SUPPORT:
    # Default all browser versions to zero
    for key in min_version_settings.keys():
      default_setting(key, 0)

  for key, oldest in min_version_settings.items():
    if settings[key] != 0 and settings[key] < oldest:
      exit_with_error(f'{key} older than {oldest} is not supported')


@ToolchainProfiler.profile_block('linker_setup')
def phase_linker_setup(options, state, newargs):
  system_libpath = '-L' + str(cache.get_lib_dir(absolute=True))
  state.append_link_flag(system_libpath)

  # We used to do this check during on startup during `check_sanity`, but
  # we now only do it when linking, in order to reduce the overhead when
  # only compiling.
  if not shared.SKIP_SUBPROCS:
    shared.check_llvm_version()

  autoconf = os.environ.get('EMMAKEN_JUST_CONFIGURE') or 'conftest.c' in state.orig_args or 'conftest.cpp' in state.orig_args
  if autoconf:
    # configure tests want a more shell-like style, where we emit return codes on exit()
    settings.EXIT_RUNTIME = 1
    # use node.js raw filesystem access, to behave just like a native executable
    settings.NODERAWFS = 1
    # Add `#!` line to output JS and make it executable.
    options.executable = True

  if 'noExitRuntime' in settings.INCOMING_MODULE_JS_API:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.append('$noExitRuntime')

  if settings.OPT_LEVEL >= 1:
    default_setting('ASSERTIONS', 0)

  if options.emrun:
    options.pre_js.append(utils.path_from_root('src/emrun_prejs.js'))
    options.post_js.append(utils.path_from_root('src/emrun_postjs.js'))
    # emrun mode waits on program exit
    settings.EXIT_RUNTIME = 1

  if options.cpu_profiler:
    options.post_js.append(utils.path_from_root('src/cpuprofiler.js'))

  if not settings.RUNTIME_DEBUG:
    settings.RUNTIME_DEBUG = bool(settings.LIBRARY_DEBUG or
                                  settings.GL_DEBUG or
                                  settings.DYLINK_DEBUG or
                                  settings.OPENAL_DEBUG or
                                  settings.SYSCALL_DEBUG or
                                  settings.WEBSOCKET_DEBUG or
                                  settings.SOCKET_DEBUG or
                                  settings.FETCH_DEBUG or
                                  settings.EXCEPTION_DEBUG or
                                  settings.PTHREADS_DEBUG or
                                  settings.ASYNCIFY_DEBUG)

  if options.memory_profiler:
    settings.MEMORYPROFILER = 1

  if settings.PTHREADS_PROFILING:
    if not settings.ASSERTIONS:
      exit_with_error('PTHREADS_PROFILING only works with ASSERTIONS enabled')
    options.post_js.append(utils.path_from_root('src/threadprofiler.js'))

  options.extern_pre_js = read_js_files(options.extern_pre_js)
  options.extern_post_js = read_js_files(options.extern_post_js)

  # TODO: support source maps with js_transform
  if options.js_transform and settings.GENERATE_SOURCE_MAP:
    logger.warning('disabling source maps because a js transform is being done')
    settings.GENERATE_SOURCE_MAP = 0

  # options.output_file is the user-specified one, target is what we will generate
  if options.output_file:
    target = options.output_file
    # check for the existence of the output directory now, to avoid having
    # to do so repeatedly when each of the various output files (.mem, .wasm,
    # etc) are written. This gives a more useful error message than the
    # IOError and python backtrace that users would otherwise see.
    dirname = os.path.dirname(target)
    if dirname and not os.path.isdir(dirname):
      exit_with_error("specified output file (%s) is in a directory that does not exist" % target)
  elif autoconf:
    # Autoconf expects the executable output file to be called `a.out`
    target = 'a.out'
  elif settings.SIDE_MODULE:
    target = 'a.out.wasm'
  else:
    target = 'a.out.js'

  final_suffix = get_file_suffix(target)

  for s, reason in DEPRECATED_SETTINGS.items():
    if s in user_settings:
      diagnostics.warning('deprecated', f'{s} is deprecated ({reason}). Please open a bug if you have a continuing need for this setting')

  if settings.EXTRA_EXPORTED_RUNTIME_METHODS:
    settings.EXPORTED_RUNTIME_METHODS += settings.EXTRA_EXPORTED_RUNTIME_METHODS

  # If no output format was specified we try to deduce the format based on
  # the output filename extension
  if not options.oformat and (options.relocatable or (options.shared and not settings.SIDE_MODULE)):
    # Until we have a better story for actually producing runtime shared libraries
    # we support a compatibility mode where shared libraries are actually just
    # object files linked with `wasm-ld --relocatable` or `llvm-link` in the case
    # of LTO.
    if final_suffix in EXECUTABLE_ENDINGS:
      diagnostics.warning('emcc', '-shared/-r used with executable output suffix. This behaviour is deprecated.  Please remove -shared/-r to build an executable or avoid the executable suffix (%s) when building object files.' % final_suffix)
    else:
      if options.shared:
        diagnostics.warning('emcc', 'linking a library with `-shared` will emit a static object file.  This is a form of emulation to support existing build systems.  If you want to build a runtime shared library use the SIDE_MODULE setting.')
      options.oformat = OFormat.OBJECT

  if not options.oformat:
    if settings.SIDE_MODULE or final_suffix == '.wasm':
      options.oformat = OFormat.WASM
    elif final_suffix == '.mjs':
      options.oformat = OFormat.MJS
    elif final_suffix == '.html':
      options.oformat = OFormat.HTML
    else:
      options.oformat = OFormat.JS

  if options.oformat in (OFormat.WASM, OFormat.OBJECT):
    for s in JS_ONLY_SETTINGS:
      if s in user_settings:
        diagnostics.warning('unused-command-line-argument', f'{s} is only valid when generating JavaScript output')

  if options.oformat == OFormat.MJS:
    settings.EXPORT_ES6 = 1
    settings.MODULARIZE = 1

  if options.oformat in (OFormat.WASM, OFormat.BARE):
    if options.emit_tsd:
      exit_with_error('Wasm only output is not compatible --emit-tsd')
    # If the user asks directly for a wasm file then this *is* the target
    wasm_target = target
  elif settings.SINGLE_FILE or settings.WASM == 0:
    # In SINGLE_FILE or WASM2JS mode the wasm file is not part of the output at
    # all so we generate it the temp directory.
    wasm_target = in_temp(shared.replace_suffix(target, '.wasm'))
  else:
    # Otherwise the wasm file is produced alongside the final target.
    wasm_target = get_secondary_target(target, '.wasm')

  if settings.SAFE_HEAP not in [0, 1, 2]:
    exit_with_error('emcc: SAFE_HEAP must be 0, 1 or 2')

  if not settings.WASM:
    # When the user requests non-wasm output, we enable wasm2js. that is,
    # we still compile to wasm normally, but we compile the final output
    # to js.
    settings.WASM = 1
    settings.WASM2JS = 1
  if settings.WASM == 2:
    # Requesting both Wasm and Wasm2JS support
    settings.WASM2JS = 1

  if options.oformat == OFormat.WASM and not settings.SIDE_MODULE:
    # if the output is just a wasm file, it will normally be a standalone one,
    # as there is no JS. an exception are side modules, as we can't tell at
    # compile time whether JS will be involved or not - the main module may
    # have JS, and the side module is expected to link against that.
    # we also do not support standalone mode in fastcomp.
    settings.STANDALONE_WASM = 1

  if settings.LZ4:
    settings.EXPORTED_RUNTIME_METHODS += ['LZ4']

  if settings.PURE_WASI:
    settings.STANDALONE_WASM = 1
    settings.WASM_BIGINT = 1
    # WASI does not support Emscripten (JS-based) exception catching, which the
    # JS-based longjmp support also uses. Emscripten EH is by default disabled
    # so we don't need to do anything here.
    if not settings.WASM_EXCEPTIONS:
      default_setting('SUPPORT_LONGJMP', 0)

  if options.no_entry:
    settings.EXPECT_MAIN = 0
  elif settings.STANDALONE_WASM:
    if '_main' in settings.EXPORTED_FUNCTIONS:
      # TODO(sbc): Make this into a warning?
      logger.debug('including `_main` in EXPORTED_FUNCTIONS is not necessary in standalone mode')
  else:
    # In normal non-standalone mode we have special handling of `_main` in EXPORTED_FUNCTIONS.
    # 1. If the user specifies exports, but doesn't include `_main` we assume they want to build a
    #    reactor.
    # 2. If the user doesn't export anything we default to exporting `_main` (unless `--no-entry`
    #    is specified (see above).
    if 'EXPORTED_FUNCTIONS' in user_settings:
      if '_main' in settings.USER_EXPORTS:
        settings.EXPORTED_FUNCTIONS.remove('_main')
        settings.EXPORT_IF_DEFINED.append('main')
      else:
        settings.EXPECT_MAIN = 0
    else:
      settings.EXPORT_IF_DEFINED.append('main')

  if settings.STANDALONE_WASM:
    # In STANDALONE_WASM mode we either build a command or a reactor.
    # See https://github.com/WebAssembly/WASI/blob/main/design/application-abi.md
    # For a command we always want EXIT_RUNTIME=1
    # For a reactor we always want EXIT_RUNTIME=0
    if 'EXIT_RUNTIME' in user_settings:
      exit_with_error('explicitly setting EXIT_RUNTIME not compatible with STANDALONE_WASM.  EXIT_RUNTIME will always be True for programs (with a main function) and False for reactors (not main function).')
    settings.EXIT_RUNTIME = settings.EXPECT_MAIN
    settings.IGNORE_MISSING_MAIN = 0
    # the wasm must be runnable without the JS, so there cannot be anything that
    # requires JS legalization
    default_setting('LEGALIZE_JS_FFI', 0)
    if 'MEMORY_GROWTH_LINEAR_STEP' in user_settings:
      exit_with_error('MEMORY_GROWTH_LINEAR_STEP is not compatible with STANDALONE_WASM')
    if 'MEMORY_GROWTH_GEOMETRIC_CAP' in user_settings:
      exit_with_error('MEMORY_GROWTH_GEOMETRIC_CAP is not compatible with STANDALONE_WASM')
    if settings.MINIMAL_RUNTIME:
      exit_with_error('MINIMAL_RUNTIME reduces JS size, and is incompatible with STANDALONE_WASM which focuses on ignoring JS anyhow and being 100% wasm')

  # Note the exports the user requested
  building.user_requested_exports.update(settings.EXPORTED_FUNCTIONS)

  if '_main' in settings.EXPORTED_FUNCTIONS or 'main' in settings.EXPORT_IF_DEFINED:
    settings.EXPORT_IF_DEFINED.append('__main_argc_argv')
  elif settings.ASSERTIONS and not settings.STANDALONE_WASM:
    # In debug builds when `main` is not explicitly requested as an
    # export we still add it to EXPORT_IF_DEFINED so that we can warn
    # users who forget to explicitly export `main`.
    # See other.test_warn_unexported_main.
    # This is not needed in STANDALONE_WASM mode since we export _start
    # (unconditionally) rather than main.
    settings.EXPORT_IF_DEFINED += ['main', '__main_argc_argv']

  if settings.ASSERTIONS:
    # Exceptions are thrown with a stack trace by default when ASSERTIONS is
    # set and when building with either -fexceptions or -fwasm-exceptions.
    if 'EXCEPTION_STACK_TRACES' in user_settings and not settings.EXCEPTION_STACK_TRACES:
      exit_with_error('EXCEPTION_STACK_TRACES cannot be disabled when ASSERTIONS are enabled')
    if settings.WASM_EXCEPTIONS or not settings.DISABLE_EXCEPTION_CATCHING:
      settings.EXCEPTION_STACK_TRACES = 1

    # -sASSERTIONS implies basic stack overflow checks, and ASSERTIONS=2
    # implies full stack overflow checks. However, we don't set this default in
    # PURE_WASI, or when we are linking without standard libraries because
    # STACK_OVERFLOW_CHECK depends on emscripten_stack_get_end which is defined
    # in libcompiler-rt.
    if not settings.PURE_WASI and '-nostdlib' not in newargs and '-nodefaultlibs' not in newargs:
      default_setting('STACK_OVERFLOW_CHECK', max(settings.ASSERTIONS, settings.STACK_OVERFLOW_CHECK))

  # For users that opt out of WARN_ON_UNDEFINED_SYMBOLS we assume they also
  # want to opt out of ERROR_ON_UNDEFINED_SYMBOLS.
  if user_settings.get('WARN_ON_UNDEFINED_SYMBOLS') == '0':
    default_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0)

  # It is unlikely that developers targeting "native web" APIs with MINIMAL_RUNTIME need
  # errno support by default.
  if settings.MINIMAL_RUNTIME:
    # Require explicit -lfoo.js flags to link with JS libraries.
    default_setting('AUTO_JS_LIBRARIES', 0)
    # When using MINIMAL_RUNTIME, symbols should only be exported if requested.
    default_setting('EXPORT_KEEPALIVE', 0)

  if settings.STRICT_JS and (settings.MODULARIZE or settings.EXPORT_ES6):
    exit_with_error("STRICT_JS doesn't work with MODULARIZE or EXPORT_ES6")

  if not options.shell_path:
    # Minimal runtime uses a different default shell file
    if settings.MINIMAL_RUNTIME:
      options.shell_path = options.shell_path = utils.path_from_root('src/shell_minimal_runtime.html')
    else:
      options.shell_path = DEFAULT_SHELL_HTML

  if settings.STRICT:
    if not settings.MODULARIZE and not settings.EXPORT_ES6:
      default_setting('STRICT_JS', 1)
    default_setting('DEFAULT_TO_CXX', 0)
    default_setting('IGNORE_MISSING_MAIN', 0)
    default_setting('AUTO_NATIVE_LIBRARIES', 0)
    if settings.MAIN_MODULE != 1:
      # These two settings cannot be disabled with MAIN_MODULE=1 because all symbols
      # are needed in this mode.
      default_setting('AUTO_JS_LIBRARIES', 0)
      default_setting('ALLOW_UNIMPLEMENTED_SYSCALLS', 0)
    if options.oformat == OFormat.HTML and options.shell_path == DEFAULT_SHELL_HTML:
      # Out default shell.html file has minimal set of INCOMING_MODULE_JS_API elements that it expects
      default_setting('INCOMING_MODULE_JS_API', 'canvas,monitorRunDependencies,onAbort,onExit,print,setStatus'.split(','))
    else:
      default_setting('INCOMING_MODULE_JS_API', [])

  if not settings.MINIMAL_RUNTIME and not settings.STRICT:
    # Export the HEAP object by default, when not running in STRICT mode
    settings.EXPORTED_RUNTIME_METHODS.extend([
      'HEAPF32',
      'HEAPF64',
      'HEAP_DATA_VIEW',
      'HEAP8',  'HEAPU8',
      'HEAP16', 'HEAPU16',
      'HEAP32', 'HEAPU32',
      'HEAP64', 'HEAPU64',
    ])

  # Default to TEXTDECODER=2 (always use TextDecoder to decode UTF-8 strings)
  # in -Oz builds, since custom decoder for UTF-8 takes up space.
  # In pthreads enabled builds, TEXTDECODER==2 may not work, see
  # https://github.com/whatwg/encoding/issues/172
  # When supporting shell environments, do not do this as TextDecoder is not
  # widely supported there.
  if settings.SHRINK_LEVEL >= 2 and not settings.SHARED_MEMORY and \
     not settings.ENVIRONMENT_MAY_BE_SHELL:
    default_setting('TEXTDECODER', 2)

  # If set to 1, we will run the autodebugger (the automatic debugging tool, see
  # tools/autodebugger).  Note that this will disable inclusion of libraries. This
  # is useful because including dlmalloc makes it hard to compare native and js
  # builds
  if os.environ.get('EMCC_AUTODEBUG'):
    settings.AUTODEBUG = 1

  # Use settings

  if settings.DEBUG_LEVEL > 1 and options.use_closure_compiler:
    diagnostics.warning('emcc', 'disabling closure because debug info was requested')
    options.use_closure_compiler = False

  if settings.WASM == 2 and settings.SINGLE_FILE:
    exit_with_error('cannot have both WASM=2 and SINGLE_FILE enabled at the same time')

  if settings.SEPARATE_DWARF and settings.WASM2JS:
    exit_with_error('cannot have both SEPARATE_DWARF and WASM2JS at the same time (as there is no wasm file)')

  if settings.MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION and settings.MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION:
    exit_with_error('MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION and MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION are mutually exclusive!')

  if options.emrun:
    if settings.MINIMAL_RUNTIME:
      exit_with_error('--emrun is not compatible with MINIMAL_RUNTIME')

  if options.use_closure_compiler:
    settings.USE_CLOSURE_COMPILER = 1

  if 'CLOSURE_WARNINGS' in user_settings:
    if settings.CLOSURE_WARNINGS not in ['quiet', 'warn', 'error']:
      exit_with_error('invalid option -sCLOSURE_WARNINGS=%s specified! Allowed values are "quiet", "warn" or "error".' % settings.CLOSURE_WARNINGS)

    diagnostics.warning('deprecated', 'CLOSURE_WARNINGS is deprecated, use -Wclosure/-Wno-closure instead')
    closure_warnings = diagnostics.manager.warnings['closure']
    if settings.CLOSURE_WARNINGS == 'error':
      closure_warnings['error'] = True
      closure_warnings['enabled'] = True
    elif settings.CLOSURE_WARNINGS == 'warn':
      closure_warnings['error'] = False
      closure_warnings['enabled'] = True
    elif settings.CLOSURE_WARNINGS == 'quiet':
      closure_warnings['error'] = False
      closure_warnings['enabled'] = False

  if not settings.MINIMAL_RUNTIME:
    if not settings.BOOTSTRAPPING_STRUCT_INFO:
      if settings.DYNCALLS:
        # Include dynCall() function by default in DYNCALLS builds in classic runtime; in MINIMAL_RUNTIME, must add this explicitly.
        settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$dynCall']

      if settings.ASSERTIONS:
        # "checkUnflushedContent()" and "missingLibrarySymbol()" depend on warnOnce
        settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$warnOnce']

      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$getValue', '$setValue']

    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$ExitStatus']

  if not settings.BOOTSTRAPPING_STRUCT_INFO and settings.SAFE_HEAP:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$getValue_safe', '$setValue_safe']

  if settings.ABORT_ON_WASM_EXCEPTIONS or settings.SPLIT_MODULE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$wasmTable']

  if settings.MAIN_MODULE:
    assert not settings.SIDE_MODULE
    if settings.MAIN_MODULE == 1:
      settings.INCLUDE_FULL_LIBRARY = 1
    # Called from preamble.js once the main module is instantiated.
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$loadDylibs']

  if settings.MAIN_MODULE == 1 or settings.SIDE_MODULE == 1:
    settings.LINKABLE = 1

  if settings.LINKABLE and settings.USER_EXPORTS:
    diagnostics.warning('unused-command-line-argument', 'EXPORTED_FUNCTIONS is not valid with LINKABLE set (normally due to SIDE_MODULE=1/MAIN_MODULE=1) since all functions are exported this mode.  To export only a subset use SIDE_MODULE=2/MAIN_MODULE=2')

  if settings.MAIN_MODULE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
      '$getDylinkMetadata',
      '$mergeLibSymbols',
    ]

  if settings.PTHREADS:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
      '$registerTLSInit',
    ]

  if settings.RELOCATABLE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
      '$reportUndefinedSymbols',
      '$relocateExports',
      '$GOTHandler',
      '__heap_base',
      '__stack_pointer',
    ]

    if settings.ASYNCIFY == 1:
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
        '__asyncify_state',
        '__asyncify_data'
      ]

    if settings.MINIMAL_RUNTIME:
      exit_with_error('MINIMAL_RUNTIME is not compatible with relocatable output')
    if settings.WASM2JS:
      exit_with_error('WASM2JS is not compatible with relocatable output')
    # shared modules need memory utilities to allocate their memory
    settings.ALLOW_TABLE_GROWTH = 1

  # various settings require sbrk() access
  if settings.DETERMINISTIC or \
     settings.EMSCRIPTEN_TRACING or \
     settings.SAFE_HEAP or \
     settings.MEMORYPROFILER:
    settings.REQUIRED_EXPORTS += ['sbrk']

  if settings.MEMORYPROFILER:
    settings.REQUIRED_EXPORTS += ['__heap_base',
                                  'emscripten_stack_get_base',
                                  'emscripten_stack_get_end',
                                  'emscripten_stack_get_current']

  if settings.ASYNCIFY_LAZY_LOAD_CODE:
    settings.ASYNCIFY = 1

  if settings.ASYNCIFY == 1:
    # See: https://github.com/emscripten-core/emscripten/issues/12065
    # See: https://github.com/emscripten-core/emscripten/issues/12066
    settings.DYNCALLS = 1

  settings.ASYNCIFY_ADD = unmangle_symbols_from_cmdline(settings.ASYNCIFY_ADD)
  settings.ASYNCIFY_REMOVE = unmangle_symbols_from_cmdline(settings.ASYNCIFY_REMOVE)
  settings.ASYNCIFY_ONLY = unmangle_symbols_from_cmdline(settings.ASYNCIFY_ONLY)

  if settings.EMULATE_FUNCTION_POINTER_CASTS:
    # Emulated casts forces a wasm ABI of (i64, i64, ...) in the table, which
    # means all table functions are illegal for JS to call directly. Use
    # dyncalls which call into the wasm, which then does an indirect call.
    settings.DYNCALLS = 1

  if options.oformat != OFormat.OBJECT and final_suffix in ('.o', '.bc', '.so', '.dylib') and not settings.SIDE_MODULE:
    diagnostics.warning('emcc', 'object file output extension (%s) used for non-object output.  If you meant to build an object file please use `-c, `-r`, or `-shared`' % final_suffix)

  if settings.SUPPORT_BIG_ENDIAN:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
      '$LE_HEAP_STORE_U16',
      '$LE_HEAP_STORE_I16',
      '$LE_HEAP_STORE_U32',
      '$LE_HEAP_STORE_I32',
      '$LE_HEAP_STORE_F32',
      '$LE_HEAP_STORE_F64',
      '$LE_HEAP_LOAD_U16',
      '$LE_HEAP_LOAD_I16',
      '$LE_HEAP_LOAD_U32',
      '$LE_HEAP_LOAD_I32',
      '$LE_HEAP_LOAD_F32',
      '$LE_HEAP_LOAD_F64'
    ]

  if settings.RUNTIME_DEBUG or settings.ASSERTIONS or settings.STACK_OVERFLOW_CHECK or settings.PTHREADS_PROFILING or settings.GL_ASSERTIONS:
    # Lots of code in debug/assertion blocks uses ptrToString.
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$ptrToString']

  if settings.STACK_OVERFLOW_CHECK:
    settings.REQUIRED_EXPORTS += [
      'emscripten_stack_get_end',
      'emscripten_stack_get_free',
      'emscripten_stack_get_base',
      'emscripten_stack_get_current',
    ]

    # We call one of these two functions during startup which caches the stack limits
    # in wasm globals allowing get_base/get_free to be super fast.
    # See compiler-rt/stack_limits.S.
    if settings.RELOCATABLE:
      settings.REQUIRED_EXPORTS += ['emscripten_stack_set_limits']
    else:
      settings.REQUIRED_EXPORTS += ['emscripten_stack_init']

  if settings.STACK_OVERFLOW_CHECK >= 2:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$setStackLimits']

  if settings.MODULARIZE:
    if settings.PROXY_TO_WORKER:
      exit_with_error('-sMODULARIZE is not compatible with --proxy-to-worker (if you want to run in a worker with -sMODULARIZE, you likely want to do the worker side setup manually)')
    # in MINIMAL_RUNTIME we may not need to emit the Promise code, as the
    # HTML output creates a singleton instance, and it does so without the
    # Promise. However, in Pthreads mode the Promise is used for worker
    # creation.
    if settings.MINIMAL_RUNTIME and options.oformat == OFormat.HTML and not settings.PTHREADS:
      settings.USE_READY_PROMISE = 0

  if settings.WASM2JS and settings.LEGACY_VM_SUPPORT:
    settings.POLYFILL_OLD_MATH_FUNCTIONS = 1

  check_browser_versions()

  if settings.MIN_NODE_VERSION >= 150000:
    default_setting('NODEJS_CATCH_REJECTION', 0)

  # Do not catch rejections or exits in modularize mode, as these options
  # are for use when running emscripten modules standalone
  # see https://github.com/emscripten-core/emscripten/issues/18723#issuecomment-1429236996
  if settings.MODULARIZE:
    default_setting('NODEJS_CATCH_REJECTION', 0)
    default_setting('NODEJS_CATCH_EXIT', 0)
    if settings.NODEJS_CATCH_REJECTION or settings.NODEJS_CATCH_EXIT:
      exit_with_error('cannot use -sNODEJS_CATCH_REJECTION or -sNODEJS_CATCH_EXIT with -sMODULARIZE')

  setup_environment_settings()

  if settings.POLYFILL:
    # Emscripten requires certain ES6+ constructs by default in library code
    # - (various ES6 operators available in all browsers listed below)
    # - https://caniuse.com/mdn-javascript_operators_nullish_coalescing:
    #                                          FF:72 CHROME:80 SAFARI:13.1 NODE:14
    # - https://caniuse.com/mdn-javascript_operators_optional_chaining:
    #                                          FF:74 CHROME:80 SAFARI:13.1 NODE:14
    # - https://caniuse.com/mdn-javascript_operators_logical_or_assignment:
    #                                          FF:79 CHROME:85 SAFARI:14 NODE:16
    # Taking the highest requirements gives is our minimum:
    #                             Max Version: FF:79 CHROME:85 SAFARI:14 NODE:16
    # TODO: replace this with feature matrix in the future.
    settings.TRANSPILE = (settings.MIN_FIREFOX_VERSION < 79 or
                          settings.MIN_CHROME_VERSION < 85 or
                          settings.MIN_SAFARI_VERSION < 140000 or
                          settings.MIN_NODE_VERSION < 160000)

  # https://caniuse.com/class: FF:45 CHROME:49 SAFARI:9
  supports_es6_classes = (settings.MIN_FIREFOX_VERSION >= 45 and
                          settings.MIN_CHROME_VERSION >= 49 and
                          settings.MIN_SAFARI_VERSION >= 90000)

  if not settings.DISABLE_EXCEPTION_CATCHING and settings.EXCEPTION_STACK_TRACES and not supports_es6_classes:
    diagnostics.warning('transpile', '-sEXCEPTION_STACK_TRACES requires an engine that support ES6 classes.')
    settings.EXCEPTION_STACK_TRACES = 0

  # Silently drop any individual backwards compatibility emulation flags that are known never to occur on browsers that support WebAssembly.
  if not settings.WASM2JS:
    settings.POLYFILL_OLD_MATH_FUNCTIONS = 0

  if settings.STB_IMAGE:
    state.append_link_flag('-lstb_image')
    settings.EXPORTED_FUNCTIONS += ['_stbi_load', '_stbi_load_from_memory', '_stbi_image_free']

  if settings.USE_WEBGL2:
    settings.MAX_WEBGL_VERSION = 2

  # MIN_WEBGL_VERSION=2 implies MAX_WEBGL_VERSION=2
  if settings.MIN_WEBGL_VERSION == 2:
    default_setting('MAX_WEBGL_VERSION', 2)

  if settings.MIN_WEBGL_VERSION > settings.MAX_WEBGL_VERSION:
    exit_with_error('MIN_WEBGL_VERSION must be smaller or equal to MAX_WEBGL_VERSION!')

  if not settings.GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS and settings.GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS:
    exit_with_error('-sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0 only makes sense with -sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0!')

  if options.use_preload_plugins or len(options.preload_files) or len(options.embed_files):
    if settings.NODERAWFS:
      exit_with_error('--preload-file and --embed-file cannot be used with NODERAWFS which disables virtual filesystem')
    # if we include any files, or intend to use preload plugins, then we definitely need filesystem support
    settings.FORCE_FILESYSTEM = 1

  if settings.FORCE_FILESYSTEM and not settings.FILESYSTEM:
    exit_with_error('`-sFORCE_FILESYSTEM` cannot be used with `-sFILESYSTEM=0`')

  if settings.WASMFS:
    if settings.NODERAWFS:
      # wasmfs will be included normally in system_libs.py, but we must include
      # noderawfs in a forced manner so that it is always linked in (the hook it
      # implements can remain unimplemented, so it won't be linked in
      # automatically)
      # TODO: find a better way to do this
      state.append_link_flag('--whole-archive')
      state.append_link_flag('-lwasmfs_noderawfs')
      state.append_link_flag('--no-whole-archive')
    settings.FILESYSTEM = 1
    settings.SYSCALLS_REQUIRE_FILESYSTEM = 0
    settings.JS_LIBRARIES.append((0, 'library_wasmfs.js'))
    if settings.ASSERTIONS:
      # used in assertion checks for unflushed content
      settings.REQUIRED_EXPORTS += ['wasmfs_flush']
    if settings.FORCE_FILESYSTEM or settings.INCLUDE_FULL_LIBRARY:
      # Add exports for the JS API. Like the old JS FS, WasmFS by default
      # includes just what JS parts it actually needs, and FORCE_FILESYSTEM is
      # required to force all of it to be included if the user wants to use the
      # JS API directly. (INCLUDE_FULL_LIBRARY also causes this code to be
      # included, as the entire JS library can refer to things that require
      # these exports.)
      settings.REQUIRED_EXPORTS += [
        'emscripten_builtin_memalign',
        'wasmfs_create_file',
        '_wasmfs_mount',
        '_wasmfs_unmount',
        '_wasmfs_read_file',
        '_wasmfs_write_file',
        '_wasmfs_open',
        '_wasmfs_allocate',
        '_wasmfs_close',
        '_wasmfs_write',
        '_wasmfs_pwrite',
        '_wasmfs_rename',
        '_wasmfs_mkdir',
        '_wasmfs_unlink',
        '_wasmfs_chdir',
        '_wasmfs_mknod',
        '_wasmfs_rmdir',
        '_wasmfs_mmap',
        '_wasmfs_munmap',
        '_wasmfs_msync',
        '_wasmfs_read',
        '_wasmfs_pread',
        '_wasmfs_symlink',
        '_wasmfs_truncate',
        '_wasmfs_ftruncate',
        '_wasmfs_stat',
        '_wasmfs_lstat',
        '_wasmfs_chmod',
        '_wasmfs_fchmod',
        '_wasmfs_lchmod',
        '_wasmfs_utime',
        '_wasmfs_llseek',
        '_wasmfs_identify',
        '_wasmfs_readlink',
        '_wasmfs_readdir_start',
        '_wasmfs_readdir_get',
        '_wasmfs_readdir_finish',
        '_wasmfs_get_cwd',
      ]

  if settings.FETCH:
    state.append_link_flag('-lfetch')

  if settings.DEMANGLE_SUPPORT:
    settings.REQUIRED_EXPORTS += ['__cxa_demangle', 'free']
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$demangle', '$stackTrace']

  if settings.FULL_ES3:
    settings.FULL_ES2 = 1
    settings.MAX_WEBGL_VERSION = max(2, settings.MAX_WEBGL_VERSION)

  if settings.MAIN_READS_PARAMS and not settings.STANDALONE_WASM:
    # callMain depends on _emscripten_stack_alloc
    settings.REQUIRED_EXPORTS += ['_emscripten_stack_alloc']

  if settings.SUPPORT_LONGJMP == 'emscripten' or not settings.DISABLE_EXCEPTION_CATCHING:
    # make_invoke depends on stackSave and stackRestore
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$stackSave', '$stackRestore']

  if settings.RELOCATABLE:
    # TODO(https://reviews.llvm.org/D128515): Make this mandatory once
    # llvm change lands
    settings.EXPORT_IF_DEFINED.append('__wasm_apply_data_relocs')

  if settings.SIDE_MODULE and 'GLOBAL_BASE' in user_settings:
    diagnostics.warning('unused-command-line-argument', 'GLOBAL_BASE is not compatible with SIDE_MODULE')

  if settings.PROXY_TO_WORKER or options.use_preload_plugins:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$Browser']

  if not settings.BOOTSTRAPPING_STRUCT_INFO:
    if settings.DYNAMIC_EXECUTION == 2 and not settings.MINIMAL_RUNTIME:
      # Used by makeEval in the DYNAMIC_EXECUTION == 2 case
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$stackTrace']

    if not settings.STANDALONE_WASM and (settings.EXIT_RUNTIME or settings.ASSERTIONS):
      # to flush streams on FS exit, we need to be able to call fflush
      # we only include it if the runtime is exitable, or when ASSERTIONS
      # (ASSERTIONS will check that streams do not need to be flushed,
      # helping people see when they should have enabled EXIT_RUNTIME)
      settings.EXPORT_IF_DEFINED += ['fflush']

  if settings.SAFE_HEAP:
    # SAFE_HEAP check includes calling emscripten_get_sbrk_ptr() from wasm
    settings.REQUIRED_EXPORTS += ['emscripten_get_sbrk_ptr', 'emscripten_stack_get_base']
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$unSign']

  if not settings.DECLARE_ASM_MODULE_EXPORTS:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$exportWasmSymbols']

  if settings.ALLOW_MEMORY_GROWTH:
    # Setting ALLOW_MEMORY_GROWTH turns off ABORTING_MALLOC, as in that mode we default to
    # the behavior of trying to grow and returning 0 from malloc on failure, like
    # a standard system would. However, if the user sets the flag it
    # overrides that.
    default_setting('ABORTING_MALLOC', 0)

  if state.has_link_flag('-lembind'):
    settings.EMBIND = 1

  if settings.EMBIND:
    # Workaround for embind+LTO issue:
    # https://github.com/emscripten-core/emscripten/issues/21653
    settings.REQUIRED_EXPORTS.append('__getTypeName')

  if options.emit_tsd:
    settings.EMIT_TSD = True

  if settings.PTHREADS:
    setup_pthreads()
    settings.JS_LIBRARIES.append((0, 'library_pthread.js'))
    if settings.PROXY_TO_PTHREAD:
      settings.PTHREAD_POOL_SIZE_STRICT = 0
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$runtimeKeepalivePush']
  else:
    if settings.PROXY_TO_PTHREAD:
      exit_with_error('-sPROXY_TO_PTHREAD requires -pthread to work!')
    settings.JS_LIBRARIES.append((0, 'library_pthread_stub.js'))

  if settings.MEMORY64:
    # Any "pointers" passed to JS will now be i64's, in both modes.
    settings.WASM_BIGINT = 1

  if settings.MEMORY64 and settings.RELOCATABLE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.append('__table_base32')

  if settings.WASM_WORKERS:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$_wasmWorkerInitializeRuntime']
    # set location of Wasm Worker bootstrap JS file
    if settings.WASM_WORKERS == 1:
      settings.WASM_WORKER_FILE = unsuffixed(os.path.basename(target)) + '.ww.js'
    settings.JS_LIBRARIES.append((0, 'library_wasm_worker.js'))

  # Set min browser versions based on certain settings such as WASM_BIGINT,
  # PTHREADS, AUDIO_WORKLET
  # Such setting must be set before this point
  feature_matrix.apply_min_browser_versions()

  # TODO(sbc): Find make a generic way to expose the feature matrix to JS
  # compiler rather then adding them all ad-hoc as internal settings
  settings.SUPPORTS_GLOBALTHIS = feature_matrix.caniuse(feature_matrix.Feature.GLOBALTHIS)
  settings.SUPPORTS_PROMISE_ANY = feature_matrix.caniuse(feature_matrix.Feature.PROMISE_ANY)
  if not settings.BULK_MEMORY:
    settings.BULK_MEMORY = feature_matrix.caniuse(feature_matrix.Feature.BULK_MEMORY)

  if settings.AUDIO_WORKLET:
    if settings.AUDIO_WORKLET == 1:
      settings.AUDIO_WORKLET_FILE = unsuffixed(os.path.basename(target)) + '.aw.js'
    settings.JS_LIBRARIES.append((0, shared.path_from_root('src', 'library_webaudio.js')))
    if not settings.MINIMAL_RUNTIME:
      # If we are in the audio worklet environment, we can only access the Module object
      # and not the global scope of the main JS script. Therefore we need to export
      # all symbols that the audio worklet scope needs onto the Module object.
      # MINIMAL_RUNTIME exports these manually, since this export mechanism is placed
      # in global scope that is not suitable for MINIMAL_RUNTIME loader.
      settings.EXPORTED_RUNTIME_METHODS += ['stackSave', 'stackAlloc', 'stackRestore', 'wasmTable']

  if settings.FORCE_FILESYSTEM and not settings.MINIMAL_RUNTIME:
    # when the filesystem is forced, we export by default methods that filesystem usage
    # may need, including filesystem usage from standalone file packager output (i.e.
    # file packages not built together with emcc, but that are loaded at runtime
    # separately, and they need emcc's output to contain the support they need)
    settings.EXPORTED_RUNTIME_METHODS += [
      'FS_createPath',
      'FS_createDataFile',
      'FS_createPreloadedFile',
      'FS_unlink'
    ]
    if not settings.WASMFS:
      # The old FS has some functionality that WasmFS lacks.
      settings.EXPORTED_RUNTIME_METHODS += [
        'FS_createLazyFile',
        'FS_createDevice'
      ]

    settings.EXPORTED_RUNTIME_METHODS += [
      'addRunDependency',
      'removeRunDependency',
    ]

  if settings.PTHREADS or settings.WASM_WORKERS or settings.RELOCATABLE or settings.ASYNCIFY_LAZY_LOAD_CODE:
    settings.IMPORTED_MEMORY = 1

  set_initial_memory()

  if settings.EXPORT_ES6:
    if not settings.MODULARIZE:
      # EXPORT_ES6 requires output to be a module
      if 'MODULARIZE' in user_settings:
        exit_with_error('EXPORT_ES6 requires MODULARIZE to be set')
      settings.MODULARIZE = 1
    if shared.target_environment_may_be('node') and not settings.USE_ES6_IMPORT_META:
      # EXPORT_ES6 + ENVIRONMENT=*node* requires the use of import.meta.url
      if 'USE_ES6_IMPORT_META' in user_settings:
        exit_with_error('EXPORT_ES6 and ENVIRONMENT=*node* requires USE_ES6_IMPORT_META to be set')
      settings.USE_ES6_IMPORT_META = 1

  if settings.MODULARIZE and not settings.DECLARE_ASM_MODULE_EXPORTS:
    # When MODULARIZE option is used, currently requires declaring all module exports
    # individually - TODO: this could be optimized
    exit_with_error('DECLARE_ASM_MODULE_EXPORTS=0 is not compatible with MODULARIZE')

  # When not declaring wasm module exports in outer scope one by one, disable minifying
  # wasm module export names so that the names can be passed directly to the outer scope.
  # Also, if using library_exports.js API, disable minification so that the feature can work.
  if not settings.DECLARE_ASM_MODULE_EXPORTS or state.has_link_flag('-lexports.js'):
    settings.MINIFY_WASM_EXPORT_NAMES = 0

  # Enable minification of wasm imports and exports when appropriate, if we
  # are emitting an optimized JS+wasm combo (then the JS knows how to load the minified names).
  # Things that process the JS after this operation would be done must disable this.
  # For example, ASYNCIFY_LAZY_LOAD_CODE needs to identify import names.
  # ASYNCIFY=2 does not support this optimization yet as it has a hardcoded
  # check for 'main' as an export name. TODO
  if will_metadce() and \
      settings.OPT_LEVEL >= 2 and \
      settings.DEBUG_LEVEL <= 2 and \
      options.oformat not in (OFormat.WASM, OFormat.BARE) and \
      settings.ASYNCIFY != 2 and \
      not settings.LINKABLE and \
      not settings.STANDALONE_WASM and \
      not settings.AUTODEBUG and \
      not settings.ASSERTIONS and \
      not settings.RELOCATABLE and \
      not settings.ASYNCIFY_LAZY_LOAD_CODE and \
          settings.MINIFY_WASM_EXPORT_NAMES:
    settings.MINIFY_WASM_IMPORTS_AND_EXPORTS = 1
    settings.MINIFY_WASM_IMPORTED_MODULES = 1

  if settings.MINIMAL_RUNTIME and settings.ASSERTIONS:
    # In ASSERTIONS-builds, functions UTF8ArrayToString() and stringToUTF8Array()
    # (which are not JS library functions), both use warnOnce(), which in
    # MINIMAL_RUNTIME is a JS library function, so explicitly have to mark
    # dependency to warnOnce() in that case. If string functions are turned to
    # library functions in the future, then JS dependency tracking can be
    # used and this special directive can be dropped.
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$warnOnce']

  if settings.MODULARIZE and not (settings.EXPORT_ES6 and not settings.SINGLE_FILE) and \
     settings.EXPORT_NAME == 'Module' and options.oformat == OFormat.HTML and \
     (options.shell_path == DEFAULT_SHELL_HTML or options.shell_path == utils.path_from_root('src/shell_minimal.html')):
    exit_with_error(f'Due to collision in variable name "Module", the shell file "{options.shell_path}" is not compatible with build options "-sMODULARIZE -sEXPORT_NAME=Module". Either provide your own shell file, change the name of the export to something else to avoid the name collision. (see https://github.com/emscripten-core/emscripten/issues/7950 for details)')

  if settings.WASM_BIGINT:
    settings.LEGALIZE_JS_FFI = 0

  if settings.SINGLE_FILE:
    settings.GENERATE_SOURCE_MAP = 0

  if settings.EVAL_CTORS:
    if settings.WASM2JS:
      # code size/memory and correctness issues TODO
      exit_with_error('EVAL_CTORS is not compatible with wasm2js yet')
    elif settings.RELOCATABLE:
      exit_with_error('EVAL_CTORS is not compatible with relocatable yet (movable segments)')
    elif settings.ASYNCIFY:
      # In Asyncify exports can be called more than once, and this seems to not
      # work properly yet (see test_emscripten_scan_registers).
      exit_with_error('EVAL_CTORS is not compatible with asyncify yet')

  if options.use_closure_compiler == 2 and not settings.WASM2JS:
    exit_with_error('closure compiler mode 2 assumes the code is asm.js, so not meaningful for wasm')

  if settings.WASM2JS:
    settings.MAYBE_WASM2JS = 1

  if settings.AUTODEBUG:
    settings.REQUIRED_EXPORTS += ['_emscripten_tempret_set']

  if settings.LEGALIZE_JS_FFI:
    settings.REQUIRED_EXPORTS += ['__get_temp_ret', '__set_temp_ret']

  if settings.SPLIT_MODULE and settings.ASYNCIFY == 2:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['_load_secondary_module']

  # wasm side modules have suffix .wasm
  if settings.SIDE_MODULE and shared.suffix(target) == '.js':
    diagnostics.warning('emcc', 'output suffix .js requested, but wasm side modules are just wasm files; emitting only a .wasm, no .js')

  sanitize = set()

  for arg in newargs:
    if arg.startswith('-fsanitize='):
      sanitize.update(arg.split('=', 1)[1].split(','))
    elif arg.startswith('-fno-sanitize='):
      sanitize.difference_update(arg.split('=', 1)[1].split(','))

  if sanitize:
    settings.USE_OFFSET_CONVERTER = 1
    # These symbols are needed by `withBuiltinMalloc` which used to implement
    # the `__noleakcheck` attribute.  However this dependency is not yet represented in the JS
    # symbol graph generated when we run the compiler with `--symbols-only`.
    settings.REQUIRED_EXPORTS += [
      'malloc',
      'calloc',
      'memalign',
      'free',
      'emscripten_builtin_malloc',
      'emscripten_builtin_calloc',
      'emscripten_builtin_memalign',
      'emscripten_builtin_free',
    ]

  if ('leak' in sanitize or 'address' in sanitize) and not settings.ALLOW_MEMORY_GROWTH:
    # Increase the minimum memory requirements to account for extra memory
    # that the sanitizers might need (in addition to the shadow memory
    # requirements handled below).
    # These values are designed be an over-estimate of the actual requirements and
    # are based on experimentation with different tests/programs under asan and
    # lsan.
    inc_initial_memory(50 * 1024 * 1024)
    if settings.PTHREADS:
      inc_initial_memory(50 * 1024 * 1024)

  if settings.USE_OFFSET_CONVERTER:
    if settings.WASM2JS:
      exit_with_error('wasm2js is not compatible with USE_OFFSET_CONVERTER (see #14630)')
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.append('$UTF8ArrayToString')

  if sanitize & UBSAN_SANITIZERS:
    if '-fsanitize-minimal-runtime' in newargs:
      settings.UBSAN_RUNTIME = 1
    else:
      settings.UBSAN_RUNTIME = 2

  if 'leak' in sanitize:
    settings.USE_LSAN = 1
    default_setting('EXIT_RUNTIME', 1)

  if 'address' in sanitize:
    settings.USE_ASAN = 1
    default_setting('EXIT_RUNTIME', 1)
    if not settings.UBSAN_RUNTIME:
      settings.UBSAN_RUNTIME = 2

    settings.REQUIRED_EXPORTS += emscripten.ASAN_C_HELPERS

    if settings.ASYNCIFY and not settings.ASYNCIFY_ONLY:
      # we do not want asyncify to instrument these helpers - they just access
      # memory as small getters/setters, so they cannot pause anyhow, and also
      # we access them in the runtime as we prepare to rewind, which would hit
      # an asyncify assertion, if asyncify instrumented them.
      #
      # note that if ASYNCIFY_ONLY was set by the user then we do not need to
      # do anything (as the user's list won't contain these functions), and if
      # we did add them, the pass would assert on incompatible lists, hence the
      # condition in the above if.
      settings.ASYNCIFY_REMOVE += emscripten.ASAN_C_HELPERS

    if settings.ASAN_SHADOW_SIZE != -1:
      diagnostics.warning('emcc', 'ASAN_SHADOW_SIZE is ignored and will be removed in a future release')

    if 'GLOBAL_BASE' in user_settings:
      exit_with_error("ASan does not support custom GLOBAL_BASE")

    # Increase the INITIAL_MEMORY and shift GLOBAL_BASE to account for
    # the ASan shadow region which starts at address zero.
    # The shadow region is 1/8th the size of the total memory and is
    # itself part of the total memory.
    # We use the following variables in this calculation:
    # - user_mem : memory usable/visible by the user program.
    # - shadow_size : memory used by asan for shadow memory.
    # - total_mem : the sum of the above. this is the size of the wasm memory (and must be aligned to WASM_PAGE_SIZE)
    user_mem = settings.MAXIMUM_MEMORY
    if not settings.ALLOW_MEMORY_GROWTH and settings.INITIAL_MEMORY != -1:
      user_mem = settings.INITIAL_MEMORY

    # Given the know value of user memory size we can work backwards
    # to find the total memory and the shadow size based on the fact
    # that the user memory is 7/8ths of the total memory.
    # (i.e. user_mem == total_mem * 7 / 8
    # TODO-Bug?: this does not look to handle 4GB MAXIMUM_MEMORY correctly.
    total_mem = user_mem * 8 / 7

    # But we might need to re-align to wasm page size
    total_mem = int(align_to_wasm_page_boundary(total_mem))

    # The shadow size is 1/8th the resulting rounded up size
    shadow_size = total_mem // 8

    # We start our global data after the shadow memory.
    # We don't need to worry about alignment here.  wasm-ld will take care of that.
    settings.GLOBAL_BASE = shadow_size

    # Adjust INITIAL_MEMORY (if needed) to account for the shifted global base.
    if settings.INITIAL_MEMORY != -1:
      if settings.ALLOW_MEMORY_GROWTH:
        settings.INITIAL_MEMORY += align_to_wasm_page_boundary(shadow_size)
      else:
        settings.INITIAL_MEMORY = total_mem

    if settings.SAFE_HEAP:
      # SAFE_HEAP instruments ASan's shadow memory accesses.
      # Since the shadow memory starts at 0, the act of accessing the shadow memory is detected
      # by SAFE_HEAP as a null pointer dereference.
      exit_with_error('ASan does not work with SAFE_HEAP')

    if settings.MEMORY64:
      exit_with_error('MEMORY64 does not yet work with ASAN')

  if settings.USE_ASAN or settings.SAFE_HEAP:
    # ASan and SAFE_HEAP check address 0 themselves
    settings.CHECK_NULL_WRITES = 0

  if sanitize and settings.GENERATE_SOURCE_MAP:
    settings.LOAD_SOURCE_MAP = 1

  if 'GLOBAL_BASE' not in user_settings and not settings.SHRINK_LEVEL and not settings.OPT_LEVEL and not settings.USE_ASAN:
    # When optimizing for size it helps to put static data first before
    # the stack (since this makes instructions for accessing this data
    # use a smaller LEB encoding).
    # However, for debugability is better to have the stack come first
    # (because stack overflows will trap rather than corrupting data).
    settings.STACK_FIRST = True

  if state.has_link_flag('--stack-first'):
    settings.STACK_FIRST = True
    if settings.USE_ASAN:
      exit_with_error('--stack-first is not compatible with asan')
    if 'GLOBAL_BASE' in user_settings:
      exit_with_error('--stack-first is not compatible with -sGLOBAL_BASE')

  set_max_memory()

  # check if we can address the 2GB mark and higher.
  if not settings.MEMORY64 and settings.MAXIMUM_MEMORY > 2 * 1024 * 1024 * 1024:
    settings.CAN_ADDRESS_2GB = 1

  if settings.MAX_WEBGL_VERSION >= 2:
    settings.WEBGL_USE_GARBAGE_FREE_APIS = 1
    # Some browsers have issues using the WebGL2 garbage-free APIs when the
    # memory offsets are over 2^31 or 2^32
    # For firefox see: https://bugzilla.mozilla.org/show_bug.cgi?id=1838218
    if settings.MIN_FIREFOX_VERSION != feature_matrix.UNSUPPORTED and settings.MAXIMUM_MEMORY > 2 ** 31:
      settings.WEBGL_USE_GARBAGE_FREE_APIS = 0
    # For chrome see: https://crbug.com/324992397
    if settings.MIN_CHROME_VERSION != feature_matrix.UNSUPPORTED and settings.MEMORY64 and settings.MAXIMUM_MEMORY > 2 ** 32:
      settings.WEBGL_USE_GARBAGE_FREE_APIS = 0
    if settings.WEBGL_USE_GARBAGE_FREE_APIS and settings.MIN_WEBGL_VERSION >= 2:
      settings.INCLUDE_WEBGL1_FALLBACK = 0

  if settings.MINIMAL_RUNTIME:
    if settings.EXIT_RUNTIME:
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['proc_exit', '$callRuntimeCallbacks']
  else:
    # MINIMAL_RUNTIME only needs callRuntimeCallbacks in certain cases, but the normal runtime
    # always does.
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$callRuntimeCallbacks']

  if settings.EXIT_RUNTIME and not settings.STANDALONE_WASM:
    # Internal function implemented in musl that calls any functions registered
    # via `atexit` et al.  With STANDALONE_WASM this is all taken care of via
    # _start and exit handling in musl, but with the normal emscripten ABI we
    # need to be able to call these explicitly.
    settings.REQUIRED_EXPORTS += ['__funcs_on_exit']

  # The worker code in src/postamble.js depends on malloc/free being exported
  if settings.BUILD_AS_WORKER:
    settings.REQUIRED_EXPORTS += ['malloc', 'free']

  if not settings.DISABLE_EXCEPTION_CATCHING:
    settings.REQUIRED_EXPORTS += [
      # For normal builds the entries in deps_info.py are enough to include
      # these symbols whenever __cxa_find_matching_catch_* functions are
      # found.  However, under LTO these symbols don't exist prior to linking
      # so we include then unconditionally when exceptions are enabled.
      '__cxa_can_catch',

      # __cxa_begin_catch depends on this but we can't use deps info in this
      # case because that only works for user-level code, and __cxa_begin_catch
      # can be used by the standard library.
      '__cxa_increment_exception_refcount',
      # Same for __cxa_end_catch
      '__cxa_decrement_exception_refcount',

      # Emscripten exception handling can generate invoke calls, and they call
      # setThrew(). We cannot handle this using deps_info as the invokes are not
      # emitted because of library function usage, but by codegen itself.
      'setThrew',
      '__cxa_free_exception',
    ]

  if settings.ASYNCIFY:
    if not settings.ASYNCIFY_IGNORE_INDIRECT:
      # if we are not ignoring indirect calls, then we must treat invoke_* as if
      # they are indirect calls, since that is what they do - we can't see their
      # targets statically.
      settings.ASYNCIFY_IMPORTS += ['invoke_*']
    # add the default imports
    settings.ASYNCIFY_IMPORTS += DEFAULT_ASYNCIFY_IMPORTS
    # add the default exports (only used for ASYNCIFY == 2)
    settings.ASYNCIFY_EXPORTS += DEFAULT_ASYNCIFY_EXPORTS

    # return the full import name, including module. The name may
    # already have a module prefix; if not, we assume it is "env".
    def get_full_import_name(name):
      if '.' in name:
        return name
      return 'env.' + name

    settings.ASYNCIFY_IMPORTS = [get_full_import_name(i) for i in settings.ASYNCIFY_IMPORTS]

    if settings.ASYNCIFY == 2:
      diagnostics.warning('experimental', '-sASYNCIFY=2 (JSPI) is still experimental')

  if settings.WASM2JS:
    if settings.GENERATE_SOURCE_MAP:
      exit_with_error('wasm2js does not support source maps yet (debug in wasm for now)')
    if settings.MEMORY64:
      exit_with_error('wasm2js does not support MEMORY64')
    if settings.WASM_BIGINT:
      exit_with_error('wasm2js does not support WASM_BIGINT')
    if settings.CAN_ADDRESS_2GB:
      exit_with_error('wasm2js does not support >2gb address space')

  if settings.NODE_CODE_CACHING:
    if settings.WASM_ASYNC_COMPILATION:
      exit_with_error('NODE_CODE_CACHING requires sync compilation (WASM_ASYNC_COMPILATION=0)')
    if not shared.target_environment_may_be('node'):
      exit_with_error('NODE_CODE_CACHING only works in node, but target environments do not include it')
    if settings.SINGLE_FILE:
      exit_with_error('NODE_CODE_CACHING saves a file on the side and is not compatible with SINGLE_FILE')

  if not js_manipulation.isidentifier(settings.EXPORT_NAME):
    exit_with_error(f'EXPORT_NAME is not a valid JS identifier: `{settings.EXPORT_NAME}`')

  if settings.EMSCRIPTEN_TRACING and settings.ALLOW_MEMORY_GROWTH:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['emscripten_trace_report_memory_layout']
    settings.REQUIRED_EXPORTS += ['emscripten_stack_get_current',
                                  'emscripten_stack_get_base',
                                  'emscripten_stack_get_end']

  settings.EMSCRIPTEN_VERSION = utils.EMSCRIPTEN_VERSION
  settings.SOURCE_MAP_BASE = options.source_map_base or ''

  settings.LINK_AS_CXX = (shared.run_via_emxx or settings.DEFAULT_TO_CXX) and '-nostdlib++' not in newargs

  # WASMFS itself is written in C++, and needs C++ standard libraries
  if settings.WASMFS:
    settings.LINK_AS_CXX = True

  # Some settings make no sense when not linking as C++
  if not settings.LINK_AS_CXX:
    cxx_only_settings = [
      'DEMANGLE_SUPPORT',
      'EXCEPTION_DEBUG',
      'DISABLE_EXCEPTION_CATCHING',
      'EXCEPTION_CATCHING_ALLOWED',
      'DISABLE_EXCEPTION_THROWING',
    ]
    for setting in cxx_only_settings:
      if setting in user_settings:
        diagnostics.warning('linkflags', 'setting `%s` is not meaningful unless linking as C++', setting)

  if settings.WASM_EXCEPTIONS:
    settings.REQUIRED_EXPORTS += ['__trap']

  if settings.EXCEPTION_STACK_TRACES:
    # If the user explicitly gave EXCEPTION_STACK_TRACES=1 without enabling EH,
    # errors out.
    if settings.DISABLE_EXCEPTION_CATCHING and not settings.WASM_EXCEPTIONS:
      exit_with_error('EXCEPTION_STACK_TRACES requires either of -fexceptions or -fwasm-exceptions')
    # EXCEPTION_STACK_TRACES implies EXPORT_EXCEPTION_HANDLING_HELPERS
    settings.EXPORT_EXCEPTION_HANDLING_HELPERS = True

  # Make `getExceptionMessage` and other necessary functions available for use.
  if settings.EXPORT_EXCEPTION_HANDLING_HELPERS:
    # If the user explicitly gave EXPORT_EXCEPTION_HANDLING_HELPERS=1 without
    # enabling EH, errors out.
    if settings.DISABLE_EXCEPTION_CATCHING and not settings.WASM_EXCEPTIONS:
      exit_with_error('EXPORT_EXCEPTION_HANDLING_HELPERS requires either of -fexceptions or -fwasm-exceptions')
    # We also export refcount increasing and decreasing functions because if you
    # catch an exception, be it an Emscripten exception or a Wasm exception, in
    # JS, you may need to manipulate the refcount manually not to leak memory.
    # What you need to do is different depending on the kind of EH you use
    # (https://github.com/emscripten-core/emscripten/issues/17115).
    settings.EXPORTED_FUNCTIONS += ['getExceptionMessage', 'incrementExceptionRefcount', 'decrementExceptionRefcount']
    if settings.WASM_EXCEPTIONS:
      settings.REQUIRED_EXPORTS += ['__cpp_exception']

  if settings.SIDE_MODULE:
    # For side modules, we ignore all REQUIRED_EXPORTS that might have been added above.
    # They all come from either libc or compiler-rt.  The exception is __wasm_call_ctors
    # and _emscripten_tls_init which are per-module exports.
    settings.REQUIRED_EXPORTS.clear()

  if not settings.STANDALONE_WASM:
    # in standalone mode, crt1 will call the constructors from inside the wasm
    settings.REQUIRED_EXPORTS.append('__wasm_call_ctors')
  if settings.PTHREADS:
    settings.REQUIRED_EXPORTS.append('_emscripten_tls_init')

  settings.PRE_JS_FILES = [os.path.abspath(f) for f in options.pre_js]
  settings.POST_JS_FILES = [os.path.abspath(f) for f in options.post_js]

  settings.MINIFY_WHITESPACE = settings.OPT_LEVEL >= 2 and settings.DEBUG_LEVEL == 0 and not options.no_minify

  # Closure might be run if we run it ourselves, or if whitespace is not being
  # minifed. In the latter case we keep both whitespace and comments, and the
  # purpose of the comments might be closure compiler, so also perform all
  # adjustments necessary to ensure that works (which amounts to a few more
  # comments; adding some more of them is not an issue in such a build which
  # includes all comments and whitespace anyhow).
  if settings.USE_CLOSURE_COMPILER or not settings.MINIFY_WHITESPACE:
    settings.MAYBE_CLOSURE_COMPILER = 1

  return target, wasm_target


@ToolchainProfiler.profile_block('calculate system libraries')
def phase_calculate_system_libraries(linker_arguments, newargs):
  extra_files_to_link = []
  # Link in ports and system libraries, if necessary
  if not settings.SIDE_MODULE:
    # Ports are always linked into the main module, never the side module.
    extra_files_to_link += ports.get_libs(settings)
  extra_files_to_link += system_libs.calculate(newargs)
  linker_arguments.extend(extra_files_to_link)


@ToolchainProfiler.profile_block('link')
def phase_link(linker_arguments, wasm_target, js_syms):
  logger.debug(f'linking: {linker_arguments}')

  # Make a final pass over settings.EXPORTED_FUNCTIONS to remove any
  # duplication between functions added by the driver/libraries and function
  # specified by the user
  settings.EXPORTED_FUNCTIONS = dedup_list(settings.EXPORTED_FUNCTIONS)
  settings.REQUIRED_EXPORTS = dedup_list(settings.REQUIRED_EXPORTS)
  settings.EXPORT_IF_DEFINED = dedup_list(settings.EXPORT_IF_DEFINED)

  rtn = None
  if settings.LINKABLE and not settings.EXPORT_ALL:
    # In LINKABLE mode we pass `--export-dynamic` along with `--whole-archive`.  This results
    # in over 7000 exports, which cannot be distinguished from the few symbols we explicitly
    # export via EMSCRIPTEN_KEEPALIVE or EXPORTED_FUNCTIONS.
    # In order to avoid unnecessary exported symbols on the `Module` object we run the linker
    # twice in this mode:
    # 1. Without `--export-dynamic` to get the base exports
    # 2. With `--export-dynamic` to get the actual linkable Wasm binary
    # TODO(sbc): Remove this double execution of wasm-ld if we ever find a way to
    # distinguish EMSCRIPTEN_KEEPALIVE exports from `--export-dynamic` exports.
    settings.LINKABLE = False
    building.link_lld(linker_arguments, wasm_target, external_symbols=js_syms)
    settings.LINKABLE = True
    rtn = extract_metadata.extract_metadata(wasm_target)

  building.link_lld(linker_arguments, wasm_target, external_symbols=js_syms)
  return rtn


@ToolchainProfiler.profile_block('post link')
def phase_post_link(options, state, in_wasm, wasm_target, target, js_syms, base_metadata=None, linker_inputs=None):
  global final_js

  target_basename = unsuffixed_basename(target)

  if options.oformat != OFormat.WASM:
    final_js = in_temp(target_basename + '.js')

  settings.TARGET_BASENAME = unsuffixed_basename(target)

  if options.oformat in (OFormat.JS, OFormat.MJS):
    state.js_target = target
  else:
    state.js_target = get_secondary_target(target, '.js')

  settings.TARGET_JS_NAME = os.path.basename(state.js_target)

  metadata = phase_emscript(in_wasm, wasm_target, js_syms, base_metadata)

  if settings.EMBIND_AOT:
    phase_embind_aot(wasm_target, js_syms, linker_inputs)

  if options.emit_tsd:
    phase_emit_tsd(options, wasm_target, state.js_target, js_syms, metadata, linker_inputs)

  if options.js_transform:
    phase_source_transforms(options)

  phase_binaryen(target, options, wasm_target)

  # If we are not emitting any JS then we are all done now
  if options.oformat != OFormat.WASM:
    phase_final_emitting(options, state, target, wasm_target)


@ToolchainProfiler.profile_block('emscript')
def phase_emscript(in_wasm, wasm_target, js_syms, base_metadata):
  # Emscripten
  logger.debug('emscript')

  # No need to support base64 embeddeding in wasm2js mode since
  # the module is already in JS format.
  settings.SUPPORT_BASE64_EMBEDDING = settings.SINGLE_FILE and not settings.WASM2JS

  if shared.SKIP_SUBPROCS:
    return

  metadata = emscripten.emscript(in_wasm, wasm_target, final_js, js_syms, base_metadata=base_metadata)
  save_intermediate('original')
  return metadata


def run_embind_gen(wasm_target, js_syms, extra_settings, linker_inputs):
  # Save settings so they can be restored after TS generation.
  original_settings = settings.backup()
  settings.attrs.update(extra_settings)

  if settings.MAIN_MODULE and linker_inputs:
    # Copy libraries to the temp directory so they can be used when running
    # in node.
    for _i, linker_input in linker_inputs:
      if building.is_wasm_dylib(linker_input):
        safe_copy(linker_input, in_temp(''))

  settings.EXPORTED_RUNTIME_METHODS = []
  # Ignore any options or settings that can conflict with running the TS
  # generation output.
  # Don't invoke the program's `main` function.
  settings.INVOKE_RUN = False
  # Ignore -sMODULARIZE which could otherwise effect how we run the module
  # to generate the bindings.
  settings.MODULARIZE = False
  # Don't include any custom user JS or files.
  settings.PRE_JS_FILES = []
  settings.POST_JS_FILES = []
  # Force node since that is where the tool runs.
  # When SHARED_MEMORY or PROXY_TO_WORKER are enabled, add the required 'worker' environment.
  settings.ENVIRONMENT = 'node' + (',worker' if settings.SHARED_MEMORY or settings.PROXY_TO_WORKER else '')
  settings.MINIMAL_RUNTIME = 0
  # Required function to trigger TS generation.
  settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$callRuntimeCallbacks']
  settings.EXPORT_ES6 = False
  # Disable proxying and thread pooling so a worker is not automatically created.
  settings.PROXY_TO_PTHREAD = False
  settings.PTHREAD_POOL_SIZE = 0
  # Assume wasm support at binding generation time
  settings.WASM2JS = 0
  # Disable minify since the binaryen pass has not been run yet to change the
  # import names.
  settings.MINIFY_WASM_IMPORTED_MODULES = False
  setup_environment_settings()
  # Use a separate Wasm file so the JS does not need to be modified after emscripten.emscript.
  settings.SINGLE_FILE = False
  # Embind may be included multiple times, de-duplicate the list first.
  settings.JS_LIBRARIES = dedup_list(settings.JS_LIBRARIES)
  # Replace embind with the TypeScript generation version.
  embind_index = settings.JS_LIBRARIES.index('embind/embind.js')
  settings.JS_LIBRARIES[embind_index] = 'embind/embind_gen.js'
  if settings.MEMORY64:
    settings.MIN_NODE_VERSION = 160000
  outfile_js = in_temp('tsgen.js')
  # The Wasm outfile may be modified by emscripten.emscript, so use a temporary file.
  outfile_wasm = in_temp('tsgen.wasm')
  emscripten.emscript(wasm_target, outfile_wasm, outfile_js, js_syms, finalize=False)
  # Build the flags needed by Node.js to properly run the output file.
  node_args = []
  if settings.MEMORY64:
    node_args += shared.node_memory64_flags()
    # Currently we don't have any engines that support table64 so we need
    # to lower it in order to run the output.
    # In the normal flow this happens later in `phase_binaryen`
    building.run_wasm_opt(outfile_wasm, outfile_wasm, ['--table64-lowering'])
  if settings.WASM_EXCEPTIONS:
    node_args += shared.node_exception_flags(config.NODE_JS)
  # Run the generated JS file with the proper flags to generate the TypeScript bindings.
  out = shared.run_js_tool(outfile_js, [], node_args, stdout=PIPE)
  settings.restore(original_settings)
  return out


@ToolchainProfiler.profile_block('emit tsd')
def phase_emit_tsd(options, wasm_target, js_target, js_syms, metadata, linker_inputs):
  logger.debug('emit tsd')
  filename = options.emit_tsd
  embind_tsd = ''
  if settings.EMBIND:
    embind_tsd = run_embind_gen(wasm_target, js_syms, {'EMBIND_AOT': False}, linker_inputs)
  all_tsd = emscripten.create_tsd(metadata, embind_tsd)
  out_file = os.path.join(os.path.dirname(js_target), filename)
  write_file(out_file, all_tsd)


@ToolchainProfiler.profile_block('embind aot js')
def phase_embind_aot(wasm_target, js_syms, linker_inputs):
  out = run_embind_gen(wasm_target, js_syms, {}, linker_inputs)
  if DEBUG:
    write_file(in_temp('embind_aot.js'), out)
  src = read_file(final_js)
  src = do_replace(src, '<<< EMBIND_AOT_OUTPUT >>>', out)
  write_file(final_js, src)


# for Popen, we cannot have doublequotes, so provide functionality to
# remove them when needed.
def remove_quotes(arg):
  if isinstance(arg, list):
    return [remove_quotes(a) for a in arg]

  if arg.startswith('"') and arg.endswith('"'):
    return arg[1:-1].replace('\\"', '"')
  elif arg.startswith("'") and arg.endswith("'"):
    return arg[1:-1].replace("\\'", "'")
  else:
    return arg


@ToolchainProfiler.profile_block('source transforms')
def phase_source_transforms(options):
  # Apply a source code transformation, if requested
  global final_js
  safe_copy(final_js, final_js + '.tr.js')
  final_js += '.tr.js'
  posix = not shared.WINDOWS
  logger.debug('applying transform: %s', options.js_transform)
  shared.check_call(remove_quotes(shlex.split(options.js_transform, posix=posix) + [os.path.abspath(final_js)]))
  save_intermediate('transformed')


# Unmangle previously mangled `import.meta` and `await import` references in
# both main code and libraries.
# See also: `preprocess` in parseTools.js.
def fix_es6_import_statements(js_file):
  if not settings.EXPORT_ES6 or not settings.USE_ES6_IMPORT_META:
    return

  src = read_file(js_file)
  write_file(js_file, src
             .replace('EMSCRIPTEN$IMPORT$META', 'import.meta')
             .replace('EMSCRIPTEN$AWAIT$IMPORT', 'await import'))
  save_intermediate('es6-module')


def create_worker_file(input_file, target_dir, output_file, options):
  output_file = os.path.join(target_dir, output_file)
  input_file = utils.path_from_root(input_file)
  contents = shared.read_and_preprocess(input_file, expand_macros=True)
  write_file(output_file, contents)

  fix_es6_import_statements(output_file)

  # Minify the worker JS file, if JS minification is enabled.
  if settings.MINIFY_WHITESPACE:
    contents = building.acorn_optimizer(output_file, ['--minify-whitespace'], return_output=True, worker_js=True)
    write_file(output_file, contents)

  tools.line_endings.convert_line_endings_in_file(output_file, os.linesep, options.output_eol)


@ToolchainProfiler.profile_block('final emitting')
def phase_final_emitting(options, state, target, wasm_target):
  global final_js

  if shared.SKIP_SUBPROCS:
    return

  target_dir = os.path.dirname(os.path.abspath(target))

  # Deploy the Wasm Worker bootstrap file as an output file (*.ww.js)
  if settings.WASM_WORKERS == 1:
    create_worker_file('src/wasm_worker.js', target_dir, settings.WASM_WORKER_FILE, options)

  # Deploy the Audio Worklet module bootstrap file (*.aw.js)
  if settings.AUDIO_WORKLET == 1:
    create_worker_file('src/audio_worklet.js', target_dir, settings.AUDIO_WORKLET_FILE, options)

  if settings.MODULARIZE:
    modularize()
  elif settings.USE_CLOSURE_COMPILER:
    module_export_name_substitution()

  # Run a final optimization pass to clean up items that were not possible to
  # optimize by Closure, or unoptimalities that were left behind by processing
  # steps that occurred after Closure.
  if settings.MINIMAL_RUNTIME == 2 and settings.USE_CLOSURE_COMPILER and settings.DEBUG_LEVEL == 0:
    args = [final_js, '-o', final_js]
    if not settings.MINIFY_WHITESPACE:
      args.append('--pretty')
    shared.run_js_tool(utils.path_from_root('tools/unsafe_optimizations.mjs'), args, cwd=utils.path_from_root('.'))
    save_intermediate('unsafe-optimizations')
    # Finally, rerun Closure compile with simple optimizations. It will be able
    # to further minify the code. (n.b. it would not be safe to run in advanced
    # mode)
    final_js = building.closure_compiler(final_js, advanced=False, extra_closure_args=options.closure_args)
    # Run unsafe_optimizations.js once more.  This allows the cleanup of newly
    # unused things that closure compiler leaves behind (e.g `new Float64Array(x)`).
    shared.run_js_tool(utils.path_from_root('tools/unsafe_optimizations.mjs'), [final_js, '-o', final_js], cwd=utils.path_from_root('.'))
    save_intermediate('unsafe-optimizations2')

  fix_es6_import_statements(final_js)

  # Apply pre and postjs files
  if options.extern_pre_js or options.extern_post_js:
    logger.debug('applying extern pre/postjses')
    src = read_file(final_js)
    final_js += '.epp.js'
    with open(final_js, 'w', encoding='utf-8') as f:
      f.write(options.extern_pre_js)
      f.write(src)
      f.write(options.extern_post_js)
    save_intermediate('extern-pre-post')

  js_manipulation.handle_license(final_js)

  js_target = state.js_target

  # The JS is now final. Move it to its final location
  move_file(final_js, js_target)

  target_basename = unsuffixed_basename(target)

  # If we were asked to also generate HTML, do that
  if options.oformat == OFormat.HTML:
    generate_html(target, options, js_target, target_basename,
                  wasm_target)
  elif settings.PROXY_TO_WORKER:
    generate_worker_js(target, js_target, target_basename)

  if settings.SPLIT_MODULE:
    diagnostics.warning('experimental', 'the SPLIT_MODULE setting is experimental and subject to change')
    do_split_module(wasm_target, options)

  if not settings.SINGLE_FILE:
    tools.line_endings.convert_line_endings_in_file(js_target, os.linesep, options.output_eol)

  if options.executable:
    make_js_executable(js_target)


@ToolchainProfiler.profile_block('binaryen')
def phase_binaryen(target, options, wasm_target):
  global final_js
  logger.debug('using binaryen')
  # whether we need to emit -g (function name debug info) in the final wasm
  debug_function_names = settings.DEBUG_LEVEL >= 2 or settings.EMIT_NAME_SECTION
  # whether we need to emit -g in the intermediate binaryen invocations (but not
  # necessarily at the very end). this is necessary if we depend on debug info
  # during compilation, even if we do not emit it at the end.
  # we track the number of causes for needing intermdiate debug info so
  # that we can stop emitting it when possible - in particular, that is
  # important so that we stop emitting it before the end, and it is not in the
  # final binary (if it shouldn't be)
  intermediate_debug_info = 0
  if debug_function_names:
    intermediate_debug_info += 1
  if options.emit_symbol_map:
    intermediate_debug_info += 1
  if settings.ASYNCIFY == 1:
    intermediate_debug_info += 1

  # run wasm-opt if we have work for it: either passes, or if we are using
  # source maps (which requires some extra processing to keep the source map
  # but remove DWARF)
  passes = get_binaryen_passes()
  if passes:
    # if asyncify is used, we will use it in the next stage, and so if it is
    # the only reason we need intermediate debug info, we can stop keeping it
    if settings.ASYNCIFY == 1:
      intermediate_debug_info -= 1
    # currently binaryen's DWARF support will limit some optimizations; warn on
    # that. see https://github.com/emscripten-core/emscripten/issues/15269
    if settings.GENERATE_DWARF:
      diagnostics.warning('limited-postlink-optimizations', 'running limited binaryen optimizations because DWARF info requested (or indirectly required)')
    with ToolchainProfiler.profile_block('wasm_opt'):
      building.run_wasm_opt(wasm_target,
                            wasm_target,
                            args=passes,
                            debug=intermediate_debug_info)
      building.save_intermediate(wasm_target, 'byn.wasm')

  if settings.EVAL_CTORS:
    with ToolchainProfiler.profile_block('eval_ctors'):
      building.eval_ctors(final_js, wasm_target, debug_info=intermediate_debug_info)
      building.save_intermediate(wasm_target, 'ctors.wasm')

  # after generating the wasm, do some final operations

  if final_js:
    if settings.SUPPORT_BIG_ENDIAN:
      with ToolchainProfiler.profile_block('little_endian_heap'):
        final_js = building.little_endian_heap(final_js)

    # >=2GB heap support requires pointers in JS to be unsigned. rather than
    # require all pointers to be unsigned by default, which increases code size
    # a little, keep them signed, and just unsign them here if we need that.
    if settings.CAN_ADDRESS_2GB:
      with ToolchainProfiler.profile_block('use_unsigned_pointers_in_js'):
        final_js = building.use_unsigned_pointers_in_js(final_js)

    # shared memory growth requires some additional JS fixups.
    # note that we must do this after handling of unsigned pointers. unsigning
    # adds some >>> 0 things, while growth will replace a HEAP8 with a call to
    # a method to get the heap, and that call would not be recognized by the
    # unsigning pass
    if settings.SHARED_MEMORY and settings.ALLOW_MEMORY_GROWTH:
      with ToolchainProfiler.profile_block('apply_wasm_memory_growth'):
        final_js = building.apply_wasm_memory_growth(final_js)

    if settings.USE_ASAN:
      final_js = building.instrument_js_for_asan(final_js)

    if settings.SAFE_HEAP:
      final_js = building.instrument_js_for_safe_heap(final_js)

    if settings.OPT_LEVEL >= 2 and settings.DEBUG_LEVEL <= 2:
      # minify the JS. Do not minify whitespace if Closure is used, so that
      # Closure can print out readable error messages (Closure will then
      # minify whitespace afterwards)
      with ToolchainProfiler.profile_block('minify_wasm'):
        save_intermediate_with_wasm('preclean', wasm_target)
        final_js = building.minify_wasm_js(js_file=final_js,
                                           wasm_file=wasm_target,
                                           expensive_optimizations=will_metadce(),
                                           debug_info=intermediate_debug_info)
        save_intermediate_with_wasm('postclean', wasm_target)

    if options.use_closure_compiler:
      with ToolchainProfiler.profile_block('closure_compile'):
        final_js = building.closure_compiler(final_js, extra_closure_args=options.closure_args)
      save_intermediate('closure')

    if settings.TRANSPILE:
      with ToolchainProfiler.profile_block('transpile'):
        final_js = building.transpile(final_js)
      save_intermediate('transpile')
      # Run acorn one more time to minify whitespace after babel runs
      if settings.MINIFY_WHITESPACE:
        final_js = building.acorn_optimizer(final_js, ['--minify-whitespace'])

  if settings.ASYNCIFY_LAZY_LOAD_CODE:
    with ToolchainProfiler.profile_block('asyncify_lazy_load_code'):
      building.asyncify_lazy_load_code(wasm_target, debug=intermediate_debug_info)

  symbols_file = None
  if options.emit_symbol_map:
    symbols_file = shared.replace_or_append_suffix(target, '.symbols')

  if settings.WASM2JS:
    symbols_file_js = None
    if settings.WASM == 2:
      # With normal wasm2js mode this file gets included as part of the
      # preamble, but with WASM=2 its a separate file.
      wasm2js_polyfill = shared.read_and_preprocess(utils.path_from_root('src/wasm2js.js'), expand_macros=True)
      wasm2js_template = wasm_target + '.js'
      write_file(wasm2js_template, wasm2js_polyfill)
      # generate secondary file for JS symbols
      if options.emit_symbol_map:
        symbols_file_js = shared.replace_or_append_suffix(wasm2js_template, '.symbols')
    else:
      wasm2js_template = final_js
      if options.emit_symbol_map:
        symbols_file_js = shared.replace_or_append_suffix(target, '.symbols')

    wasm2js = building.wasm2js(wasm2js_template,
                               wasm_target,
                               opt_level=settings.OPT_LEVEL,
                               use_closure_compiler=options.use_closure_compiler,
                               debug_info=debug_function_names,
                               symbols_file=symbols_file,
                               symbols_file_js=symbols_file_js)

    shared.get_temp_files().note(wasm2js)

    if settings.WASM == 2:
      safe_copy(wasm2js, wasm2js_template)

    if settings.WASM != 2:
      final_js = wasm2js

    save_intermediate('wasm2js')

  generating_wasm = settings.WASM == 2 or not settings.WASM2JS

  # emit the final symbols, either in the binary or in a symbol map.
  # this will also remove debug info if we only kept it around in the intermediate invocations.
  # note that if we aren't emitting a binary (like in wasm2js) then we don't
  # have anything to do here.
  if options.emit_symbol_map:
    intermediate_debug_info -= 1
    if generating_wasm:
      building.handle_final_wasm_symbols(wasm_file=wasm_target, symbols_file=symbols_file, debug_info=intermediate_debug_info)
      save_intermediate_with_wasm('symbolmap', wasm_target)

  if settings.DEBUG_LEVEL >= 3 and settings.SEPARATE_DWARF and generating_wasm:
    # if the dwarf filename wasn't provided, use the default target + a suffix
    wasm_file_with_dwarf = settings.SEPARATE_DWARF
    if wasm_file_with_dwarf is True:
      # Historically this file has been called `.wasm.debug.wasm`
      # TODO(sbc): Should this just be `.debug.wasm`
      wasm_file_with_dwarf = get_secondary_target(target, '.wasm.debug.wasm')
    building.emit_debug_on_side(wasm_target, wasm_file_with_dwarf)

  # we have finished emitting the wasm, and so intermediate debug info will
  # definitely no longer be used tracking it.
  if debug_function_names:
    intermediate_debug_info -= 1
  assert intermediate_debug_info == 0
  # strip debug info if it was not already stripped by the last command
  if not debug_function_names and building.binaryen_kept_debug_info and generating_wasm:
    with ToolchainProfiler.profile_block('strip_name_section'):
      building.strip(wasm_target, wasm_target, debug=False, sections=["name"])

  # replace placeholder strings with correct subresource locations
  if final_js and settings.SINGLE_FILE and not settings.WASM2JS:
    js = read_file(final_js)

    if settings.MINIMAL_RUNTIME:
      js = do_replace(js, '<<< WASM_BINARY_DATA >>>', base64_encode(wasm_target))
    else:
      js = do_replace(js, '<<< WASM_BINARY_FILE >>>', get_subresource_location(wasm_target))
    delete_file(wasm_target)
    write_file(final_js, js)


def node_es6_imports():
  if not settings.EXPORT_ES6 or not shared.target_environment_may_be('node'):
    return ''

  # Multi-environment builds uses `await import` in `shell.js`
  if shared.target_environment_may_be('web'):
    return ''

  # Use static import declaration if we only target Node.js
  return '''
import { createRequire } from 'module';
const require = createRequire(import.meta.url);
'''


def node_pthread_detection():
  # Under node we detect that we are running in a pthread by checking the
  # workerData property.
  if settings.EXPORT_ES6:
    return "(await import('worker_threads')).workerData === 'em-pthread';\n"
  else:
    return "require('worker_threads').workerData === 'em-pthread'\n"


def modularize():
  global final_js
  logger.debug(f'Modularizing, assigning to var {settings.EXPORT_NAME}')
  src = read_file(final_js)

  # Multi-environment ES6 builds require an async function
  async_emit = ''
  if settings.EXPORT_ES6 and \
     shared.target_environment_may_be('node') and \
     shared.target_environment_may_be('web'):
    async_emit = 'async '

  # TODO: Remove when https://bugs.webkit.org/show_bug.cgi?id=223533 is resolved.
  if async_emit != '' and settings.EXPORT_NAME == 'config':
    diagnostics.warning('emcc', 'EXPORT_NAME should not be named "config" when targeting Safari')

  src = '''
%(maybe_async)sfunction(moduleArg = {}) {
  var moduleRtn;

%(src)s

  return moduleRtn;
}
''' % {
    'maybe_async': async_emit,
    'src': src,
  }

  if settings.MINIMAL_RUNTIME and not settings.PTHREADS:
    # Single threaded MINIMAL_RUNTIME programs do not need access to
    # document.currentScript, so a simple export declaration is enough.
    src = '/** @nocollapse */ var %s = %s' % (settings.EXPORT_NAME, src)
  else:
    script_url_node = ''
    # When MODULARIZE this JS may be executed later,
    # after document.currentScript is gone, so we save it.
    # In EXPORT_ES6 + PTHREADS the 'thread' is actually an ES6 module
    # webworker running in strict mode, so doesn't have access to 'document'.
    # In this case use 'import.meta' instead.
    if settings.EXPORT_ES6 and settings.USE_ES6_IMPORT_META:
      script_url = 'import.meta.url'
    else:
      script_url = "typeof document != 'undefined' ? document.currentScript?.src : undefined"
      if shared.target_environment_may_be('node'):
        script_url_node = "if (typeof __filename != 'undefined') _scriptName = _scriptName || __filename;"
    src = '''%(node_imports)s
var %(EXPORT_NAME)s = (() => {
  var _scriptName = %(script_url)s;
  %(script_url_node)s
  return (%(src)s);
})();
''' % {
      'node_imports': node_es6_imports(),
      'EXPORT_NAME': settings.EXPORT_NAME,
      'script_url': script_url,
      'script_url_node': script_url_node,
      'src': src,
    }

  # Given the async nature of how the Module function and Module object
  # come into existence in AudioWorkletGlobalScope, store the Module
  # function under a different variable name so that AudioWorkletGlobalScope
  # will be able to reference it without aliasing/conflicting with the
  # Module variable name. This should happen even in MINIMAL_RUNTIME builds
  # for MODULARIZE and EXPORT_ES6 to work correctly.
  if settings.AUDIO_WORKLET:
    src += f'globalThis.AudioWorkletModule = {settings.EXPORT_NAME};\n'

  # Export using a UMD style export, or ES6 exports if selected
  if settings.EXPORT_ES6:
    src += 'export default %s;\n' % settings.EXPORT_NAME

  elif not settings.MINIMAL_RUNTIME:
    src += '''\
if (typeof exports === 'object' && typeof module === 'object')
  module.exports = %(EXPORT_NAME)s;
else if (typeof define === 'function' && define['amd'])
  define([], () => %(EXPORT_NAME)s);
''' % {'EXPORT_NAME': settings.EXPORT_NAME}

  if settings.PTHREADS:
    # Create code for detecting if we are running in a pthread.
    # Normally this detection is done when the module is itself run but
    # when running in MODULARIZE mode we need use this to know if we should
    # run the module constructor on startup (true only for pthreads).
    if settings.ENVIRONMENT_MAY_BE_WEB or settings.ENVIRONMENT_MAY_BE_WORKER:
      src += "var isPthread = globalThis.self?.name?.startsWith('em-pthread');\n"
      # In order to support both web and node we also need to detect node here.
      if settings.ENVIRONMENT_MAY_BE_NODE:
        src += "var isNode = typeof globalThis.process?.versions?.node == 'string';\n"
        src += f'if (isNode) isPthread = {node_pthread_detection()}\n'
    elif settings.ENVIRONMENT_MAY_BE_NODE:
      src += f'var isPthread = {node_pthread_detection()}\n'
    src += '// When running as a pthread, construct a new instance on startup\n'
    src += 'isPthread && %s();\n' % settings.EXPORT_NAME

  final_js += '.modular.js'
  write_file(final_js, src)
  shared.get_temp_files().note(final_js)
  save_intermediate('modularized')


def module_export_name_substitution():
  assert not settings.MODULARIZE
  global final_js
  logger.debug(f'Private module export name substitution with {settings.EXPORT_NAME}')
  src = read_file(final_js)
  final_js += '.module_export_name_substitution.js'
  if settings.MINIMAL_RUNTIME and not settings.ENVIRONMENT_MAY_BE_NODE and not settings.ENVIRONMENT_MAY_BE_SHELL and not settings.AUDIO_WORKLET:
    # On the web, with MINIMAL_RUNTIME, the Module object is always provided
    # via the shell html in order to provide the .asm.js/.wasm content.
    replacement = settings.EXPORT_NAME
  else:
    replacement = "typeof %(EXPORT_NAME)s != 'undefined' ? %(EXPORT_NAME)s : {}" % {"EXPORT_NAME": settings.EXPORT_NAME}
  new_src = re.sub(r'{\s*[\'"]?__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__[\'"]?:\s*1\s*}', replacement, src)
  assert new_src != src, 'Unable to find Closure syntax __EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__ in source!'
  write_file(final_js, new_src)
  shared.get_temp_files().note(final_js)
  save_intermediate('module_export_name_substitution')


def generate_traditional_runtime_html(target, options, js_target, target_basename,
                                      wasm_target):
  script = ScriptSource()

  if settings.EXPORT_NAME != 'Module' and options.shell_path == DEFAULT_SHELL_HTML:
    # the minimal runtime shell HTML is designed to support changing the export
    # name, but the normal one does not support that currently
    exit_with_error('customizing EXPORT_NAME requires that the HTML be customized to use that name (see https://github.com/emscripten-core/emscripten/issues/10086)')

  shell = shared.read_and_preprocess(options.shell_path)
  if '{{{ SCRIPT }}}' not in shell:
    exit_with_error('HTML shell must contain {{{ SCRIPT }}}, see src/shell.html for an example')
  base_js_target = os.path.basename(js_target)

  if settings.PROXY_TO_WORKER:
    proxy_worker_filename = (settings.PROXY_TO_WORKER_FILENAME or target_basename) + '.js'
    worker_js = worker_js_script(proxy_worker_filename)
    script.inline = ('''
  var filename = '%s';
  if ((',' + window.location.search.substr(1) + ',').indexOf(',noProxy,') < 0) {
    console.log('running code in a web worker');
''' % get_subresource_location_js(proxy_worker_filename)) + worker_js + '''
  } else {
    console.log('running code on the main thread');
    var fileBytes = tryParseAsDataURI(filename);
    var script = document.createElement('script');
    if (fileBytes) {
      script.innerHTML = intArrayToString(fileBytes);
    } else {
      script.src = filename;
    }
    document.body.appendChild(script);
  }
'''
    # add required helper functions such as tryParseAsDataURI
    for filename in ('arrayUtils.js', 'base64Utils.js', 'URIUtils.js'):
      content = shared.read_and_preprocess(utils.path_from_root('src', filename))
      script.inline = content + script.inline

    script.inline = 'var ASSERTIONS = %s;\n%s' % (settings.ASSERTIONS, script.inline)
  else:
    # Normal code generation path
    script.src = base_js_target

  if settings.SINGLE_FILE:
    # In SINGLE_FILE mode we either inline the script, or in the case
    # of SHARED_MEMORY convert the entire thing into a data URL.
    if settings.SHARED_MEMORY:
      assert not script.inline
      script.src = get_subresource_location(js_target)
    else:
      js_contents = script.inline or ''
      if script.src:
        js_contents += read_file(js_target)
      script.src = None
      script.inline = read_file(js_target)
    delete_file(js_target)
  else:
    if not settings.WASM_ASYNC_COMPILATION:
      # We need to load the wasm file before anything else, since it
      # has be synchronously ready.
      script.un_src()
      script.inline = '''
          fetch('%s').then((result) => result.arrayBuffer())
                     .then((buf) => {
                             Module.wasmBinary = buf;
                             %s;
                           });
''' % (get_subresource_location(wasm_target), script.inline)

    if settings.WASM == 2:
      # If target browser does not support WebAssembly, we need to load
      # the .wasm.js file before the main .js file.
      script.un_src()
      script.inline = '''
          function loadMainJs() {
%s
          }
          if (!window.WebAssembly || location.search.indexOf('_rwasm=0') > 0) {
            // Current browser does not support WebAssembly, load the .wasm.js JavaScript fallback
            // before the main JS runtime.
            var wasm2js = document.createElement('script');
            wasm2js.src = '%s';
            wasm2js.onload = loadMainJs;
            document.body.appendChild(wasm2js);
          } else {
            // Current browser supports Wasm, proceed with loading the main JS runtime.
            loadMainJs();
          }
''' % (script.inline, get_subresource_location_js(wasm_target + '.js'))

  shell = do_replace(shell, '{{{ SCRIPT }}}', script.replacement())
  shell = shell.replace('{{{ SHELL_CSS }}}', utils.read_file(utils.path_from_root('src/shell.css')))
  logo_filename = utils.path_from_root('media/powered_by_logo_shell.png')
  logo_b64 = base64_encode(logo_filename)
  shell = shell.replace('{{{ SHELL_LOGO }}}', f'<img id="emscripten_logo" src="data:image/png;base64,{logo_b64}">')

  check_output_file(target)
  write_file(target, shell)


def minify_html(filename):
  if settings.DEBUG_LEVEL >= 2:
    return

  opts = []
  # -g1 and greater retain whitespace and comments in source
  if settings.DEBUG_LEVEL == 0:
    opts += ['--collapse-whitespace',
             '--remove-comments',
             '--remove-tag-whitespace',
             '--sort-attributes',
             '--sort-class-name']
  # -g2 and greater do not minify HTML at all
  if settings.DEBUG_LEVEL <= 1:
    opts += ['--decode-entities',
             '--collapse-boolean-attributes',
             '--remove-attribute-quotes',
             '--remove-redundant-attributes',
             '--remove-script-type-attributes',
             '--remove-style-link-type-attributes',
             '--use-short-doctype',
             '--minify-css', 'true',
             '--minify-js', 'true']

  # html-minifier also has the following options, but they look unsafe for use:
  # '--collapse-inline-tag-whitespace': removes whitespace between inline tags in visible text,
  #                                     causing words to be joined together. See
  #                                     https://github.com/terser/html-minifier-terser/issues/179
  #                                     https://github.com/emscripten-core/emscripten/issues/22188
  # '--remove-optional-tags': removes e.g. <head></head> and <body></body> tags from the page.
  #                           (Breaks at least browser.test_sdl2glshader)
  # '--remove-empty-attributes': removes all attributes with whitespace-only values.
  #                              (Breaks at least browser.test_asmfs_hello_file)
  # '--remove-empty-elements': removes all elements with empty contents.
  #                            (Breaks at least browser.test_asm_swapping)

  logger.debug(f'minifying HTML file {filename}')
  size_before = os.path.getsize(filename)
  start_time = time.time()
  shared.check_call(shared.get_npm_cmd('html-minifier-terser') + [filename, '-o', filename] + opts, env=shared.env_with_node_in_path())

  elapsed_time = time.time() - start_time
  size_after = os.path.getsize(filename)
  delta = size_after - size_before
  logger.debug(f'HTML minification took {elapsed_time:.2f} seconds, and shrunk size of {filename} from {size_before} to {size_after} bytes, delta={delta} ({delta * 100.0 / size_before:+.2f}%)')


def generate_html(target, options, js_target, target_basename, wasm_target):
  logger.debug('generating HTML')

  if settings.MINIMAL_RUNTIME:
    generate_minimal_runtime_html(target, options, js_target, target_basename)
  else:
    generate_traditional_runtime_html(target, options, js_target, target_basename, wasm_target)

  if settings.MINIFY_HTML and (settings.OPT_LEVEL >= 1 or settings.SHRINK_LEVEL >= 1):
    minify_html(target)

  tools.line_endings.convert_line_endings_in_file(target, os.linesep, options.output_eol)


def generate_worker_js(target, js_target, target_basename):
  if settings.SINGLE_FILE:
    # compiler output is embedded as base64 data URL
    proxy_worker_filename = get_subresource_location_js(js_target)
  else:
    # compiler output goes in .worker.js file
    move_file(js_target, shared.replace_suffix(js_target, get_worker_js_suffix()))
    worker_target_basename = target_basename + '.worker'
    proxy_worker_filename = (settings.PROXY_TO_WORKER_FILENAME or worker_target_basename) + '.js'

  target_contents = worker_js_script(proxy_worker_filename)
  write_file(target, target_contents)


def worker_js_script(proxy_worker_filename):
  web_gl_client_src = read_file(utils.path_from_root('src/webGLClient.js'))
  proxy_client_src = shared.read_and_preprocess(utils.path_from_root('src/proxyClient.js'), expand_macros=True)
  if not settings.SINGLE_FILE and not os.path.dirname(proxy_worker_filename):
    proxy_worker_filename = './' + proxy_worker_filename
  proxy_client_src = do_replace(proxy_client_src, '<<< filename >>>', proxy_worker_filename)
  return web_gl_client_src + '\n' + proxy_client_src


def find_library(lib, lib_dirs):
  for lib_dir in lib_dirs:
    path = os.path.join(lib_dir, lib)
    if os.path.isfile(path):
      logger.debug('found library "%s" at %s', lib, path)
      return path
  return None


def map_to_js_libs(library_name):
  """Given the name of a special Emscripten-implemented system library, returns an
  pair containing
  1. Array of absolute paths to JS library files, inside emscripten/src/ that corresponds to the
     library name. `None` means there is no mapping and the library will be processed by the linker
     as a require for normal native library.
  2. Optional name of a corresponding native library to link in.
  """
  # Some native libraries are implemented in Emscripten as system side JS libraries
  library_map = {
    'embind': ['embind/embind.js', 'embind/emval.js'],
    'EGL': ['library_egl.js'],
    'GL': ['library_webgl.js', 'library_html5_webgl.js'],
    'webgl.js': ['library_webgl.js', 'library_html5_webgl.js'],
    'GLESv2': ['library_webgl.js'],
    # N.b. there is no GLESv3 to link to (note [f] in https://www.khronos.org/registry/implementers_guide.html)
    'GLEW': ['library_glew.js'],
    'glfw': ['library_glfw.js'],
    'glfw3': ['library_glfw.js'],
    'GLU': [],
    'glut': ['library_glut.js'],
    'openal': ['library_openal.js'],
    'X11': ['library_xlib.js'],
    'SDL': ['library_sdl.js'],
    'uuid': ['library_uuid.js'],
    'fetch': ['library_fetch.js'],
    'websocket': ['library_websocket.js'],
    # These 4 libraries are separate under glibc but are all rolled into
    # libc with musl.  For compatibility with glibc we just ignore them
    # completely.
    'dl': [],
    'm': [],
    'rt': [],
    'pthread': [],
    # This is the name of GNU's C++ standard library. We ignore it here
    # for compatibility with GNU toolchains.
    'stdc++': [],
    'SDL2_mixer': [],
  }
  settings_map = {
    'glfw': {'USE_GLFW': 2},
    'glfw3': {'USE_GLFW': 3},
    'SDL': {'USE_SDL': 1},
    'SDL2_mixer': {'USE_SDL_MIXER': 2},
  }

  if library_name in settings_map:
    for key, value in settings_map[library_name].items():
      default_setting(key, value)

  if library_name in library_map:
    libs = library_map[library_name]
    logger.debug('Mapping library `%s` to JS libraries: %s' % (library_name, libs))
    return libs

  if library_name.endswith('.js') and os.path.isfile(utils.path_from_root('src', f'library_{library_name}')):
    return [f'library_{library_name}']

  return None


def process_libraries(state, linker_inputs):
  new_flags = []
  libraries = []
  suffixes = STATICLIB_ENDINGS + DYNAMICLIB_ENDINGS
  system_libs_map = system_libs.Library.get_usable_variations()

  # Find library files
  for i, flag in state.link_flags:
    if not flag.startswith('-l'):
      new_flags.append((i, flag))
      continue
    lib = removeprefix(flag, '-l')

    logger.debug('looking for library "%s"', lib)

    js_libs = map_to_js_libs(lib)
    if js_libs is not None:
      libraries += [(i, js_lib) for js_lib in js_libs]

    # We don't need to resolve system libraries to absolute paths here, we can just
    # let wasm-ld handle that.  However, we do want to map to the correct variant.
    # For example we map `-lc` to `-lc-mt` if we are building with threading support.
    if 'lib' + lib in system_libs_map:
      lib = system_libs_map['lib' + lib].get_link_flag()
      new_flags.append((i, lib))
      continue

    if js_libs is not None:
      continue

    path = None
    for suff in suffixes:
      name = 'lib' + lib + suff
      path = find_library(name, state.lib_dirs)
      if path:
        break

    if path:
      linker_inputs.append((i, path))
      continue

    new_flags.append((i, flag))

  settings.JS_LIBRARIES += libraries

  # At this point processing JS_LIBRARIES is finished, no more items will be added to it.
  # Sort the input list from (order, lib_name) pairs to a flat array in the right order.
  settings.JS_LIBRARIES.sort(key=lambda lib: lib[0])
  settings.JS_LIBRARIES = [lib[1] for lib in settings.JS_LIBRARIES]
  state.link_flags = new_flags


class ScriptSource:
  def __init__(self):
    self.src = None # if set, we have a script to load with a src attribute
    self.inline = None # if set, we have the contents of a script to write inline in a script

  def un_src(self):
    """Use this if you want to modify the script and need it to be inline."""
    if self.src is None:
      return
    quoted_src = quote(self.src)
    if settings.EXPORT_ES6:
      self.inline = f'''
        import("./{quoted_src}").then(exports => exports.default(Module))
      '''
    else:
      self.inline = f'''
            var script = document.createElement('script');
            script.src = "{quoted_src}";
            document.body.appendChild(script);
      '''
    self.src = None

  def replacement(self):
    """Returns the script tag to replace the {{{ SCRIPT }}} tag in the target"""
    assert (self.src or self.inline) and not (self.src and self.inline)
    if self.src:
      src = self.src
      if src.startswith('data:'):
        filename = src
      else:
        src = quote(self.src)
        filename = f'./{src}'
      if settings.EXPORT_ES6:
        return f'''
        <script type="module">
          import initModule from "{filename}";
          initModule(Module);
        </script>
        '''
      else:
        return f'<script async type="text/javascript" src="{src}"></script>'
    else:
      return '<script>\n%s\n</script>' % self.inline


def filter_out_dynamic_libs(options, inputs):
  # Filters out "fake" dynamic libraries that are really just intermediate object files.
  def check(input_file):
    if get_file_suffix(input_file) in DYNAMICLIB_ENDINGS and not building.is_wasm_dylib(input_file):
      if not options.ignore_dynamic_linking:
        diagnostics.warning('emcc', 'ignoring dynamic library %s because not compiling to JS or HTML, remember to link it when compiling to JS or HTML at the end', os.path.basename(input_file))
      return False
    else:
      return True

  return [f for f in inputs if check(f)]


def filter_out_duplicate_dynamic_libs(inputs):
  seen = set()

  # Filter out duplicate "fake" shared libraries (intermediate object files).
  # See test_core.py:test_redundant_link
  def check(input_file):
    if get_file_suffix(input_file) in DYNAMICLIB_ENDINGS and not building.is_wasm_dylib(input_file):
      abspath = os.path.abspath(input_file)
      if abspath in seen:
        return False
      seen.add(abspath)
    return True

  return [f for f in inputs if check(f)]


def process_dynamic_libs(dylibs, lib_dirs):
  extras = []
  seen = set()
  to_process = dylibs.copy()
  while to_process:
    dylib = to_process.pop()
    dylink = webassembly.parse_dylink_section(dylib)
    for needed in dylink.needed:
      if needed in seen:
        continue
      path = find_library(needed, lib_dirs)
      if path:
        extras.append(path)
        seen.add(needed)
      else:
        exit_with_error(f'{os.path.normpath(dylib)}: shared library dependency not found in library path: `{needed}`. (library path: {lib_dirs}')
      to_process.append(path)

  dylibs += extras
  for dylib in dylibs:
    exports = webassembly.get_exports(dylib)
    exports = set(e.name for e in exports)
    # EM_JS function are exports with a special prefix.  We need to strip
    # this prefix to get the actual symbol name.  For the main module, this
    # is handled by extract_metadata.py.
    exports = [removeprefix(e, '__em_js__') for e in exports]
    settings.SIDE_MODULE_EXPORTS.extend(sorted(exports))

    imports = webassembly.get_imports(dylib)
    imports = [i.field for i in imports if i.kind in (webassembly.ExternType.FUNC, webassembly.ExternType.GLOBAL, webassembly.ExternType.TAG)]
    # For now we ignore `invoke_` functions imported by side modules and rely
    # on the dynamic linker to create them on the fly.
    # TODO(sbc): Integrate with metadata.invoke_funcs that comes from the
    # main module to avoid creating new invoke functions at runtime.
    imports = set(imports)
    imports = set(i for i in imports if not i.startswith('invoke_'))
    weak_imports = webassembly.get_weak_imports(dylib)
    strong_imports = sorted(imports.difference(weak_imports))
    logger.debug('Adding symbols requirements from `%s`: %s', dylib, imports)

    mangled_imports = [shared.asmjs_mangle(e) for e in sorted(imports)]
    mangled_strong_imports = [shared.asmjs_mangle(e) for e in strong_imports]
    for sym in weak_imports:
      mangled = shared.asmjs_mangle(sym)
      if mangled not in settings.SIDE_MODULE_IMPORTS and mangled not in building.user_requested_exports:
        settings.WEAK_IMPORTS.append(sym)
    settings.SIDE_MODULE_IMPORTS.extend(mangled_imports)
    settings.EXPORT_IF_DEFINED.extend(sorted(imports))
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.extend(sorted(imports))
    building.user_requested_exports.update(mangled_strong_imports)


def unmangle_symbols_from_cmdline(symbols):
  def unmangle(x):
    return x.replace('.', ' ').replace('#', '&').replace('?', ',')

  if type(symbols) is list:
    return [unmangle(x) for x in symbols]
  return unmangle(symbols)


def get_secondary_target(target, ext):
  # Depending on the output format emscripten creates zero or more secondary
  # output files (e.g. the .wasm file when creating JS output, or the
  # .js and the .wasm file when creating html output.
  # Thus function names the secondary output files, while ensuring they
  # never collide with the primary one.
  base = unsuffixed(target)
  if get_file_suffix(target) == ext:
    base += '_'
  return base + ext


def dedup_list(lst):
  # Since we require python 3.6, that ordering of dictionaries is guaranteed
  # to be insertion order so we can use 'dict' here but not 'set'.
  return list(dict.fromkeys(lst))


def check_output_file(f):
  if os.path.isdir(f):
    exit_with_error(f'cannot write output file `{f}`: Is a directory')


def move_file(src, dst):
  logging.debug('move: %s -> %s', src, dst)
  check_output_file(dst)
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if src == dst:
    return
  if dst == os.devnull:
    return
  shutil.move(src, dst)


# Returns the subresource location for run-time access
def get_subresource_location(path, mimetype='application/octet-stream'):
  if settings.SINGLE_FILE:
    return f'data:{mimetype};base64,{base64_encode(path)}'
  else:
    return os.path.basename(path)


def get_subresource_location_js(path):
  return get_subresource_location(path, 'text/javascript')


@ToolchainProfiler.profile()
def package_files(options, target):
  rtn = []
  logger.debug('setting up files')
  file_args = ['--from-emcc']
  if options.preload_files:
    file_args.append('--preload')
    file_args += options.preload_files
  if options.embed_files:
    file_args.append('--embed')
    file_args += options.embed_files
  if options.exclude_files:
    file_args.append('--exclude')
    file_args += options.exclude_files
  if options.use_preload_cache:
    file_args.append('--use-preload-cache')
  if settings.LZ4:
    file_args.append('--lz4')
  if options.use_preload_plugins:
    file_args.append('--use-preload-plugins')
  if not settings.ENVIRONMENT_MAY_BE_NODE:
    file_args.append('--no-node')
  if options.embed_files:
    if settings.MEMORY64:
      file_args += ['--wasm64']
    object_file = in_temp('embedded_files.o')
    file_args += ['--obj-output=' + object_file]
    rtn.append(object_file)

  cmd = [shared.FILE_PACKAGER, shared.replace_suffix(target, '.data')] + file_args
  if options.preload_files:
    # Preloading files uses --pre-js code that runs before the module is loaded.
    file_code = shared.check_call(cmd, stdout=PIPE).stdout
    js_manipulation.add_files_pre_js(settings.PRE_JS_FILES, file_code)
  else:
    # Otherwise, we are embedding files, which does not require --pre-js code,
    # and instead relies on a static constructor to populate the filesystem.
    shared.check_call(cmd)

  return rtn


@ToolchainProfiler.profile_block('calculate linker inputs')
def phase_calculate_linker_inputs(options, state, linker_inputs):
  using_lld = not (options.oformat == OFormat.OBJECT and settings.LTO)
  state.link_flags = filter_link_flags(state.link_flags, using_lld)

  # Decide what we will link
  process_libraries(state, linker_inputs)

  # Interleave the linker inputs with the linker flags while maintainging their
  # relative order on the command line (both of these list are pairs, with the
  # first element being their command line position).
  linker_args = [val for _, val in sorted(linker_inputs + state.link_flags)]

  # If we are linking to an intermediate object then ignore other
  # "fake" dynamic libraries, since otherwise we will end up with
  # multiple copies in the final executable.
  if options.oformat == OFormat.OBJECT or options.ignore_dynamic_linking:
    linker_args = filter_out_dynamic_libs(options, linker_args)
  else:
    linker_args = filter_out_duplicate_dynamic_libs(linker_args)

  if settings.MAIN_MODULE:
    dylibs = [a for a in linker_args if building.is_wasm_dylib(a)]
    process_dynamic_libs(dylibs, state.lib_dirs)

  return linker_args


def run_post_link(wasm_input, options, state, newargs):
  settings.limit_settings(None)
  target, wasm_target = phase_linker_setup(options, state, newargs)
  process_libraries(state, [])
  phase_post_link(options, state, wasm_input, wasm_target, target, {})


def run(linker_inputs, options, state, newargs):
  # We have now passed the compile phase, allow reading/writing of all settings.
  settings.limit_settings(None)

  if not linker_inputs and not state.link_flags:
    exit_with_error('no input files')

  if options.output_file and options.output_file.startswith('-'):
    exit_with_error(f'invalid output filename: `{options.output_file}`')

  target, wasm_target = phase_linker_setup(options, state, newargs)

  # Link object files using wasm-ld or llvm-link (for bitcode linking)
  linker_arguments = phase_calculate_linker_inputs(options, state, linker_inputs)

  # Embed and preload files
  if len(options.preload_files) or len(options.embed_files):
    linker_arguments += package_files(options, target)

  if options.oformat == OFormat.OBJECT:
    logger.debug(f'link_to_object: {linker_arguments} -> {target}')
    building.link_to_object(linker_arguments, target)
    logger.debug('stopping after linking to object file')
    return 0

  phase_calculate_system_libraries(linker_arguments, newargs)

  js_syms = {}
  if (not settings.SIDE_MODULE or settings.ASYNCIFY) and not shared.SKIP_SUBPROCS:
    js_info = get_js_sym_info()
    if not settings.SIDE_MODULE:
      js_syms = js_info['deps']
      if settings.LINKABLE:
        for native_deps in js_syms.values():
          settings.REQUIRED_EXPORTS += native_deps
      else:
        def add_js_deps(sym):
          if sym in js_syms:
            native_deps = js_syms[sym]
            if native_deps:
              settings.REQUIRED_EXPORTS += native_deps

        for sym in settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE:
          add_js_deps(sym)
        for sym in js_info['extraLibraryFuncs']:
          add_js_deps(sym)
        for sym in settings.EXPORTED_RUNTIME_METHODS:
          add_js_deps(shared.demangle_c_symbol_name(sym))
        for sym in settings.EXPORTED_FUNCTIONS:
          add_js_deps(shared.demangle_c_symbol_name(sym))
    if settings.ASYNCIFY:
      settings.ASYNCIFY_IMPORTS_EXCEPT_JS_LIBS = settings.ASYNCIFY_IMPORTS[:]
      settings.ASYNCIFY_IMPORTS += ['*.' + x for x in js_info['asyncFuncs']]

  base_metadata = phase_link(linker_arguments, wasm_target, js_syms)

  # Special handling for when the user passed '-Wl,--version'.  In this case the linker
  # does not create the output file, but just prints its version and exits with 0.
  if '--version' in linker_arguments:
    return 0

  # TODO(sbc): In theory we should really run the whole pipeline even if the output is
  # /dev/null, but that will take some refactoring
  if target == os.devnull:
    return 0

  # Perform post-link steps (unless we are running bare mode)
  if options.oformat != OFormat.BARE:
    phase_post_link(options, state, wasm_target, wasm_target, target, js_syms, base_metadata, linker_inputs)

  return 0
