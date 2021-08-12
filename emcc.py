#!/usr/bin/env python3
# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emcc - compiler helper script
=============================

emcc is a drop-in replacement for a compiler like gcc or clang.

See  emcc --help  for details.

emcc can be influenced by a few environment variables:

  EMCC_DEBUG - "1" will log out useful information during compilation, as well as
               save each compiler step as an emcc-* file in the temp dir
               (by default /tmp/emscripten_temp). "2" will save additional emcc-*
               steps, that would normally not be separately produced (so this
               slows down compilation).
"""

from tools.toolchain_profiler import ToolchainProfiler

import base64
import json
import logging
import os
import re
import shlex
import shutil
import stat
import sys
import time
from enum import Enum, unique, auto
from subprocess import PIPE
from urllib.parse import quote


import emscripten
from tools import shared, system_libs
from tools import colored_logger, diagnostics, building
from tools.shared import unsuffixed, unsuffixed_basename, WINDOWS, safe_copy
from tools.shared import run_process, read_and_preprocess, exit_with_error, DEBUG
from tools.shared import do_replace, strip_prefix
from tools.response_file import substitute_response_files
from tools.minimal_runtime_shell import generate_minimal_runtime_html
import tools.line_endings
from tools import js_manipulation
from tools import wasm2c
from tools import webassembly
from tools import config
from tools.settings import settings, MEM_SIZE_SETTINGS, COMPILE_TIME_SETTINGS
from tools.utils import read_file, write_file, read_binary

logger = logging.getLogger('emcc')

# endings = dot + a suffix, safe to test by  filename.endswith(endings)
C_ENDINGS = ('.c', '.i')
CXX_ENDINGS = ('.cpp', '.cxx', '.cc', '.c++', '.CPP', '.CXX', '.C', '.CC', '.C++', '.ii')
OBJC_ENDINGS = ('.m', '.mi')
OBJCXX_ENDINGS = ('.mm', '.mii')
ASSEMBLY_CPP_ENDINGS = ('.S',)
SPECIAL_ENDINGLESS_FILENAMES = (os.devnull,)

SOURCE_ENDINGS = C_ENDINGS + CXX_ENDINGS + OBJC_ENDINGS + OBJCXX_ENDINGS + SPECIAL_ENDINGLESS_FILENAMES + ASSEMBLY_CPP_ENDINGS
C_ENDINGS = C_ENDINGS + SPECIAL_ENDINGLESS_FILENAMES # consider the special endingless filenames like /dev/null to be C

EXECUTABLE_ENDINGS = ('.wasm', '.html', '.js', '.mjs', '.out', '')
DYNAMICLIB_ENDINGS = ('.dylib', '.so') # Windows .dll suffix is not included in this list, since those are never linked to directly on the command line.
STATICLIB_ENDINGS = ('.a',)
ASSEMBLY_ENDINGS = ('.ll', '.s')
HEADER_ENDINGS = ('.h', '.hxx', '.hpp', '.hh', '.H', '.HXX', '.HPP', '.HH')

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
    '-M': False,
    # wasm-ld doesn't support soname or other dynamic linking flags (yet).   Ignore them
    # in order to aid build systems that want to pass these flags.
    '-soname': True,
    '-allow-shlib-undefined': False,
    '-rpath': True,
    '-rpath-link': True,
    '-version-script': True,
}

DEFAULT_ASYNCIFY_IMPORTS = [
  'emscripten_sleep', 'emscripten_wget', 'emscripten_wget_data', 'emscripten_idb_load',
  'emscripten_idb_store', 'emscripten_idb_delete', 'emscripten_idb_exists',
  'emscripten_idb_load_blob', 'emscripten_idb_store_blob', 'SDL_Delay',
  'emscripten_scan_registers', 'emscripten_lazy_load_code',
  'emscripten_fiber_swap',
  'wasi_snapshot_preview1.fd_sync', '__wasi_fd_sync', '_emval_await',
  'dlopen', '__asyncjs__*'
]

# Target options
final_js = None

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


VALID_ENVIRONMENTS = ('web', 'webview', 'worker', 'node', 'shell')
SIMD_INTEL_FEATURE_TOWER = ['-msse', '-msse2', '-msse3', '-mssse3', '-msse4.1', '-msse4.2', '-mavx']
SIMD_NEON_FLAGS = ['-mfpu=neon']


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


def base64_encode(b):
  b64 = base64.b64encode(b)
  return b64.decode('ascii')


@unique
class OFormat(Enum):
  # Output a relocatable object file.  We use this
  # today for `-r` and `-shared`.
  OBJECT = auto()
  WASM = auto()
  JS = auto()
  MJS = auto()
  HTML = auto()
  BARE = auto()


@unique
class Mode(Enum):
  PREPROCESS_ONLY = auto()
  PCH = auto()
  COMPILE_ONLY = auto()
  POST_LINK_ONLY = auto()
  COMPILE_AND_LINK = auto()


class EmccState:
  def __init__(self, args):
    self.mode = Mode.COMPILE_AND_LINK
    self.orig_args = args
    self.has_dash_c = False
    self.has_dash_E = False
    self.has_dash_S = False
    self.link_flags = []
    self.lib_dirs = []
    self.forced_stdlibs = []


def add_link_flag(state, i, f):
  if f.startswith('-L'):
    state.lib_dirs.append(f[2:])

  state.link_flags.append((i, f))


class EmccOptions:
  def __init__(self):
    self.output_file = None
    self.post_link = False
    self.executable = False
    self.compiler_wrapper = None
    self.oformat = None
    self.requested_debug = ''
    self.profiling_funcs = False
    self.tracing = False
    self.emit_symbol_map = False
    self.use_closure_compiler = None
    self.closure_args = []
    self.js_transform = None
    self.pre_js = '' # before all js
    self.post_js = '' # after all js
    self.extern_pre_js = '' # before all js, external to optimized code
    self.extern_post_js = '' # after all js, external to optimized code
    self.preload_files = []
    self.embed_files = []
    self.exclude_files = []
    self.ignore_dynamic_linking = False
    self.shell_path = shared.path_from_root('src', 'shell.html')
    self.source_map_base = ''
    self.emrun = False
    self.cpu_profiler = False
    self.thread_profiler = False
    self.memory_profiler = False
    self.memory_init_file = None
    self.use_preload_cache = False
    self.use_preload_plugins = False
    self.default_object_extension = '.o'
    self.valid_abspaths = []
    self.cfi = False
    # Specifies the line ending format to use for all generated text files.
    # Defaults to using the native EOL on each platform (\r\n on Windows, \n on
    # Linux & MacOS)
    self.output_eol = os.linesep
    self.no_entry = False
    self.shared = False
    self.relocatable = False


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
      (settings.ENVIRONMENT_MAY_BE_NODE and settings.USE_PTHREADS)

  if not settings.ENVIRONMENT_MAY_BE_WORKER and settings.PROXY_TO_WORKER:
    exit_with_error('If you specify --proxy-to-worker and specify a "-s ENVIRONMENT=" directive, it must include "worker" as a target! (Try e.g. -s ENVIRONMENT=web,worker)')

  if not settings.ENVIRONMENT_MAY_BE_WORKER and settings.USE_PTHREADS:
    exit_with_error('When building with multithreading enabled and a "-s ENVIRONMENT=" directive is specified, it must include "worker" as a target! (Try e.g. -s ENVIRONMENT=web,worker)')


def minify_whitespace():
  return settings.OPT_LEVEL >= 2 and settings.DEBUG_LEVEL == 0


def embed_memfile():
  return (settings.SINGLE_FILE or
          (settings.MEM_INIT_METHOD == 0 and
           (not settings.MAIN_MODULE and
            not settings.SIDE_MODULE and
            not settings.GENERATE_SOURCE_MAP)))


def expand_byte_size_suffixes(value):
  """Given a string with KB/MB size suffixes, such as "32MB", computes how
  many bytes that is and returns it as an integer.
  """
  value = value.strip()
  match = re.match(r'^(\d+)\s*([kmgt]?b)?$', value, re.I)
  if not match:
    exit_with_error("invalid byte size `%s`.  Valid suffixes are: kb, mb, gb, tb" % value)
  value, suffix = match.groups()
  value = int(value)
  if suffix:
    size_suffixes = {suffix: 1024 ** i for i, suffix in enumerate(['b', 'kb', 'mb', 'gb', 'tb'])}
    value *= size_suffixes[suffix.lower()]
  return value


def apply_settings(changes):
  """Take a map of users settings {NAME: VALUE} and apply them to the global
  settings object.
  """

  def standardize_setting_change(key, value):
    # boolean NO_X settings are aliases for X
    # (note that *non*-boolean setting values have special meanings,
    # and we can't just flip them, so leave them as-is to be
    # handled in a special way later)
    if key.startswith('NO_') and value in ('0', '1'):
      key = strip_prefix(key, 'NO_')
      value = str(1 - int(value))
    return key, value

  for key, value in changes.items():
    key, value = standardize_setting_change(key, value)

    if key in settings.internal_settings:
      exit_with_error('%s is an internal setting and cannot be set from command line', key)

    # map legacy settings which have aliases to the new names
    # but keep the original key so errors are correctly reported via the `setattr` below
    user_key = key
    if key in settings.legacy_settings and key in settings.alt_names:
      key = settings.alt_names[key]

    # In those settings fields that represent amount of memory, translate suffixes to multiples of 1024.
    if key in MEM_SIZE_SETTINGS:
      value = str(expand_byte_size_suffixes(value))

    filename = None
    if value and value[0] == '@':
      filename = strip_prefix(value, '@')
      if not os.path.exists(filename):
        exit_with_error('%s: file not found parsing argument: %s=%s' % (filename, key, value))
      value = read_file(filename).strip()
    else:
      value = value.replace('\\', '\\\\')

    existing = getattr(settings, user_key, None)
    expect_list = type(existing) == list

    if filename and expect_list and value.strip()[0] != '[':
      # Prefer simpler one-line-per value parser
      value = parse_symbol_list_file(value)
    else:
      try:
        value = parse_value(value, expect_list)
      except Exception as e:
        exit_with_error('a problem occurred in evaluating the content after a "-s", specifically "%s=%s": %s', key, value, str(e))

    # Do some basic type checking by comparing to the existing settings.
    # Sadly we can't do this generically in the SettingsManager since there are settings
    # that so change types internally over time.
    # We only currently worry about lists vs non-lists.
    if expect_list != (type(value) == list):
      exit_with_error('setting `%s` expects `%s` but got `%s`' % (user_key, type(existing), type(value)))

    setattr(settings, user_key, value)

    if key == 'EXPORTED_FUNCTIONS':
      # used for warnings in emscripten.py
      settings.USER_EXPORTED_FUNCTIONS = settings.EXPORTED_FUNCTIONS.copy()

    # TODO(sbc): Remove this legacy way.
    if key == 'WASM_OBJECT_FILES':
      settings.LTO = 0 if value else 'full'


def is_ar_file_with_missing_index(archive_file):
  # We parse the archive header outselves because llvm-nm --print-armap is slower and less
  # reliable.
  # See: https://github.com/emscripten-core/emscripten/issues/10195
  archive_header = b'!<arch>\n'
  file_header_size = 60

  with open(archive_file, 'rb') as f:
    header = f.read(len(archive_header))
    if header != archive_header:
      # This is not even an ar file
      return False
    file_header = f.read(file_header_size)
    if len(file_header) != file_header_size:
      # We don't have any file entires at all so we don't consider the index missing
      return False

  name = file_header[:16].strip()
  # If '/' is the name of the first file we have an index
  return name != b'/'


def ensure_archive_index(archive_file):
  # Fastcomp linking works without archive indexes.
  if not settings.AUTO_ARCHIVE_INDEXES:
    return
  if is_ar_file_with_missing_index(archive_file):
    diagnostics.warning('emcc', '%s: archive is missing an index; Use emar when creating libraries to ensure an index is created', archive_file)
    diagnostics.warning('emcc', '%s: adding index', archive_file)
    run_process([shared.LLVM_RANLIB, archive_file])


@ToolchainProfiler.profile_block('JS symbol generation')
def get_all_js_syms():
  # Runs the js compiler to generate a list of all symbols available in the JS
  # libraries.  This must be done separately for each linker invokation since the
  # list of symbols depends on what settings are used.
  # TODO(sbc): Find a way to optimize this.  Potentially we could add a super-set
  # mode of the js compiler that would generate a list of all possible symbols
  # that could be checked in.
  old_full = settings.INCLUDE_FULL_LIBRARY
  try:
    # Temporarily define INCLUDE_FULL_LIBRARY since we want a full list
    # of all available JS library functions.
    settings.INCLUDE_FULL_LIBRARY = True
    settings.ONLY_CALC_JS_SYMBOLS = True
    emscripten.generate_struct_info()
    glue, forwarded_data = emscripten.compile_settings()
    forwarded_json = json.loads(forwarded_data)
    library_syms = set()
    for name in forwarded_json['libraryFunctions']:
      if shared.is_c_symbol(name):
        name = shared.demangle_c_symbol_name(name)
        library_syms.add(name)
  finally:
    settings.ONLY_CALC_JS_SYMBOLS = False
    settings.INCLUDE_FULL_LIBRARY = old_full

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


def cxx_to_c_compiler(cxx):
  # Convert C++ compiler name into C compiler name
  dirname, basename = os.path.split(cxx)
  basename = basename.replace('clang++', 'clang').replace('g++', 'gcc').replace('em++', 'emcc')
  return os.path.join(dirname, basename)


def get_binaryen_passes():
  # run the binaryen optimizer in -O2+. in -O0 we don't need it obviously, while
  # in -O1 we don't run it as the LLVM optimizer has been run, and it does the
  # great majority of the work; not running the binaryen optimizer in that case
  # keeps -O1 mostly-optimized while compiling quickly and without rewriting
  # DWARF etc.
  run_binaryen_optimizer = settings.OPT_LEVEL >= 2

  passes = []
  # safe heap must run before post-emscripten, so post-emscripten can apply the sbrk ptr
  if settings.SAFE_HEAP:
    passes += ['--safe-heap']
  if settings.MEMORY64 == 2:
    passes += ['--memory64-lowering']
  if run_binaryen_optimizer:
    passes += ['--post-emscripten']
    if not settings.EXIT_RUNTIME:
      passes += ['--no-exit-runtime']
  if run_binaryen_optimizer:
    passes += [building.opt_level_to_str(settings.OPT_LEVEL, settings.SHRINK_LEVEL)]
  # when optimizing, use the fact that low memory is never used (1024 is a
  # hardcoded value in the binaryen pass)
  if run_binaryen_optimizer and settings.GLOBAL_BASE >= 1024:
    passes += ['--low-memory-unused']
  if settings.AUTODEBUG:
    # adding '--flatten' here may make these even more effective
    passes += ['--instrument-locals']
    passes += ['--log-execution']
    passes += ['--instrument-memory']
    if settings.LEGALIZE_JS_FFI:
      # legalize it again now, as the instrumentation may need it
      passes += ['--legalize-js-interface']
  if settings.EMULATE_FUNCTION_POINTER_CASTS:
    # note that this pass must run before asyncify, as if it runs afterwards we only
    # generate the  byn$fpcast_emu  functions after asyncify runs, and so we wouldn't
    # be able to further process them.
    passes += ['--fpcast-emu']
  if settings.ASYNCIFY:
    passes += ['--asyncify']
    if settings.ASSERTIONS:
      passes += ['--pass-arg=asyncify-asserts']
    if settings.ASYNCIFY_ADVISE:
      passes += ['--pass-arg=asyncify-verbose']
    if settings.ASYNCIFY_IGNORE_INDIRECT:
      passes += ['--pass-arg=asyncify-ignore-indirect']
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
  if settings.BINARYEN_IGNORE_IMPLICIT_TRAPS:
    passes += ['--ignore-implicit-traps']
  # normally we can assume the memory, if imported, has not been modified
  # beforehand (in fact, in most cases the memory is not even imported anyhow,
  # but it is still safe to pass the flag), and is therefore filled with zeros.
  # the one exception is dynamic linking of a side module: the main module is ok
  # as it is loaded first, but the side module may be assigned memory that was
  # previously used.
  if run_binaryen_optimizer and not settings.SIDE_MODULE:
    passes += ['--zero-filled-memory']

  if settings.BINARYEN_EXTRA_PASSES:
    # BINARYEN_EXTRA_PASSES is comma-separated, and we support both '-'-prefixed and
    # unprefixed pass names
    extras = settings.BINARYEN_EXTRA_PASSES.split(',')
    passes += [('--' + p) if p[0] != '-' else p for p in extras if p]

  return passes


def make_js_executable(script):
  src = read_file(script)
  cmd = shared.shlex_join(config.JS_ENGINE)
  if not os.path.isabs(config.JS_ENGINE[0]):
    # TODO: use whereis etc. And how about non-*NIX?
    cmd = '/usr/bin/env -S ' + cmd
  logger.debug('adding `#!` to JavaScript file: %s' % cmd)
  # add shebang
  with open(script, 'w') as f:
    f.write('#!%s\n' % cmd)
    f.write(src)
  try:
    os.chmod(script, stat.S_IMODE(os.stat(script).st_mode) | stat.S_IXUSR) # make executable
  except OSError:
    pass # can fail if e.g. writing the executable to /dev/null


def do_split_module(wasm_file):
  os.rename(wasm_file, wasm_file + '.orig')
  args = ['--instrument']
  building.run_binaryen_command('wasm-split', wasm_file + '.orig', outfile=wasm_file, args=args)


def is_dash_s_for_emcc(args, i):
  # -s OPT=VALUE or -s OPT or -sOPT are all interpreted as emscripten flags.
  # -s by itself is a linker option (alias for --strip-all)
  if args[i] == '-s':
    if len(args) <= i + 1:
      return False
    arg = args[i + 1]
  else:
    arg = strip_prefix(args[i], '-s')
  arg = arg.split('=')[0]
  return arg.isidentifier() and arg.isupper()


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
        exit_with_error(f'{os.path.normpath(dylib)}: shared library dependency not found: `{needed}`')
      to_process.append(path)

  dylibs += extras
  for dylib in dylibs:
    exports = webassembly.get_exports(dylib)
    exports = set(e.name for e in exports)
    settings.SIDE_MODULE_EXPORTS.extend(exports)

    imports = webassembly.get_imports(dylib)
    imports = [i.field for i in imports if i.kind in (webassembly.ExternType.FUNC, webassembly.ExternType.GLOBAL)]
    # For now we ignore `invoke_` functions imported by side modules and rely
    # on the dynamic linker to create them on the fly.
    # TODO(sbc): Integrate with metadata['invokeFuncs'] that comes from the
    # main module to avoid creating new invoke functions at runtime.
    imports = set(i for i in imports if not i.startswith('invoke_'))
    weak_imports = imports.intersection(exports)
    strong_imports = imports.difference(exports)
    logger.debug('Adding symbols requirements from `%s`: %s', dylib, imports)

    mangled_imports = [shared.asmjs_mangle(e) for e in imports]
    mangled_strong_imports = [shared.asmjs_mangle(e) for e in strong_imports]
    settings.SIDE_MODULE_IMPORTS.extend(mangled_imports)
    settings.EXPORTED_FUNCTIONS.extend(mangled_strong_imports)
    settings.EXPORT_IF_DEFINED.extend(weak_imports)
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.extend(strong_imports)
    building.user_requested_exports.update(mangled_strong_imports)


def unmangle_symbols_from_cmdline(symbols):
  def unmangle(x):
    return x.replace('.', ' ').replace('#', '&').replace('?', ',')

  if type(symbols) is list:
    return [unmangle(x) for x in symbols]
  return unmangle(symbols)


def parse_s_args(args):
  settings_changes = []
  for i in range(len(args)):
    if args[i].startswith('-s'):
      if is_dash_s_for_emcc(args, i):
        if args[i] == '-s':
          key = args[i + 1]
          args[i + 1] = ''
        else:
          key = strip_prefix(args[i], '-s')
        args[i] = ''

        # If not = is specified default to 1
        if '=' not in key:
          key += '=1'

        # Special handling of browser version targets. A version -1 means that the specific version
        # is not supported at all. Replace those with INT32_MAX to make it possible to compare e.g.
        # #if MIN_FIREFOX_VERSION < 68
        if re.match(r'MIN_.*_VERSION(=.*)?', key):
          try:
            if int(key.split('=')[1]) < 0:
              key = key.split('=')[0] + '=0x7FFFFFFF'
          except Exception:
            pass

        settings_changes.append(key)

  newargs = [a for a in args if a]
  return (settings_changes, newargs)


def emsdk_ldflags(user_args):
  if os.environ.get('EMMAKEN_NO_SDK'):
    return []

  library_paths = [
     shared.Cache.get_lib_dir(absolute=True)
  ]
  ldflags = ['-L' + l for l in library_paths]

  if '-nostdlib' in user_args:
    return ldflags

  return ldflags


def emsdk_cflags(user_args):
  cflags = ['--sysroot=' + shared.Cache.get_sysroot(absolute=True)]

  def array_contains_any_of(hay, needles):
    for n in needles:
      if n in hay:
        return True

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER) or array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    if '-msimd128' not in user_args:
      exit_with_error('Passing any of ' + ', '.join(SIMD_INTEL_FEATURE_TOWER + SIMD_NEON_FLAGS) + ' flags also requires passing -msimd128!')
    cflags += ['-D__SSE__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[1:]):
    cflags += ['-D__SSE2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[2:]):
    cflags += ['-D__SSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[3:]):
    cflags += ['-D__SSSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[4:]):
    cflags += ['-D__SSE4_1__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[5:]):
    cflags += ['-D__SSE4_2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[6:]):
    cflags += ['-D__AVX__=1']

  if array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    cflags += ['-D__ARM_NEON__=1']

  return cflags + ['-Xclang', '-iwithsysroot' + os.path.join('/include', 'compat')]


def get_clang_flags():
  return ['-target', get_llvm_target()]


def get_llvm_target():
  if settings.MEMORY64:
    return 'wasm64-unknown-emscripten'
  else:
    return 'wasm32-unknown-emscripten'


cflags = None


def get_cflags(options, user_args):
  global cflags
  if cflags:
    return cflags

  # Flags we pass to the compiler when building C/C++ code
  # We add these to the user's flags (newargs), but not when building .s or .S assembly files
  cflags = get_clang_flags()

  if options.tracing:
    cflags.append('-D__EMSCRIPTEN_TRACING__=1')

  if settings.USE_PTHREADS:
    cflags.append('-D__EMSCRIPTEN_PTHREADS__=1')

  if not settings.STRICT:
    # The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code
    # in strict mode. Code should use the define __EMSCRIPTEN__ instead.
    cflags.append('-DEMSCRIPTEN')

  # if exception catching is disabled, we can prevent that code from being
  # generated in the frontend
  if settings.DISABLE_EXCEPTION_CATCHING and not settings.EXCEPTION_HANDLING:
    cflags.append('-fignore-exceptions')

  if settings.INLINING_LIMIT:
    cflags.append('-fno-inline-functions')

  if settings.RELOCATABLE:
    cflags.append('-fPIC')
    cflags.append('-fvisibility=default')

  if settings.LTO:
    if not any(a.startswith('-flto') for a in user_args):
      cflags.append('-flto=' + settings.LTO)
  else:
    # In LTO mode these args get passed instead at link time when the backend runs.
    for a in building.llvm_backend_args():
      cflags += ['-mllvm', a]

  # Set the LIBCPP ABI version to at least 2 so that we get nicely aligned string
  # data and other nice fixes.
  cflags += [# '-fno-threadsafe-statics', # disabled due to issue 1289
             '-D__EMSCRIPTEN_major__=' + str(shared.EMSCRIPTEN_VERSION_MAJOR),
             '-D__EMSCRIPTEN_minor__=' + str(shared.EMSCRIPTEN_VERSION_MINOR),
             '-D__EMSCRIPTEN_tiny__=' + str(shared.EMSCRIPTEN_VERSION_TINY),
             '-D_LIBCPP_ABI_VERSION=2']

  # For compatability with the fastcomp compiler that defined these
  cflags += ['-Dunix',
             '-D__unix',
             '-D__unix__']

  # Changes to default clang behavior

  # Implicit functions can cause horribly confusing function pointer type errors, see #2175
  # If your codebase really needs them - very unrecommended! - you can disable the error with
  #   -Wno-error=implicit-function-declaration
  # or disable even a warning about it with
  #   -Wno-implicit-function-declaration
  cflags += ['-Werror=implicit-function-declaration']

  system_libs.add_ports_cflags(cflags, settings)

  if os.environ.get('EMMAKEN_NO_SDK') or '-nostdinc' in user_args:
    return cflags

  cflags += emsdk_cflags(user_args)
  return cflags


def get_file_suffix(filename):
  """Parses the essential suffix of a filename, discarding Unix-style version
  numbers in the name. For example for 'libz.so.1.2.8' returns '.so'"""
  if filename in SPECIAL_ENDINGLESS_FILENAMES:
    return filename
  while filename:
    filename, suffix = os.path.splitext(filename)
    if not suffix[1:].isdigit():
      return suffix
  return ''


def get_library_basename(filename):
  """Similar to get_file_suffix this strips off all numeric suffixes and then
  then final non-numeric one.  For example for 'libz.so.1.2.8' returns 'libz'"""
  filename = os.path.basename(filename)
  while filename:
    filename, suffix = os.path.splitext(filename)
    # Keep stipping suffixes until we strip a non-numeric one.
    if not suffix[1:].isdigit():
      return filename


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


def in_temp(name):
  temp_dir = shared.get_emscripten_temp_dir()
  return os.path.join(temp_dir, os.path.basename(name))


def dedup_list(lst):
  rtn = []
  for item in lst:
    if item not in rtn:
      rtn.append(item)
  return rtn


def move_file(src, dst):
  logging.debug('move: %s -> %s', src, dst)
  if os.path.isdir(dst):
    exit_with_error(f'cannot write output file `{dst}`: Is a directory')
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if src == dst:
    return
  if dst == os.devnull:
    return
  shutil.move(src, dst)


run_via_emxx = False


#
# Main run() function
#
def run(args):
  # Additional compiler flags that we treat as if they were passed to us on the
  # commandline
  EMCC_CFLAGS = os.environ.get('EMCC_CFLAGS')
  if DEBUG:
    cmd = shared.shlex_join(args)
    if EMCC_CFLAGS:
      cmd += ' + ' + EMCC_CFLAGS
    logger.warning(f'invocation: {cmd} (in {os.getcwd()})')
  if EMCC_CFLAGS:
    args.extend(shlex.split(EMCC_CFLAGS))

  # Strip args[0] (program name)
  args = args[1:]

  misc_temp_files = shared.configuration.get_temp_files()

  # Handle some global flags

  # read response files very early on
  try:
    args = substitute_response_files(args)
  except IOError as e:
    exit_with_error(e)

  if '--help' in args:
    # Documentation for emcc and its options must be updated in:
    #    site/source/docs/tools_reference/emcc.rst
    # This then gets built (via: `make -C site text`) to:
    #    site/build/text/docs/tools_reference/emcc.txt
    # This then needs to be copied to its final home in docs/emcc.txt from where
    # we read it here.  We have CI rules that ensure its always up-to-date.
    with open(shared.path_from_root('docs', 'emcc.txt'), 'r') as f:
      print(f.read())

    print('''
------------------------------------------------------------------

emcc: supported targets: llvm bitcode, WebAssembly, NOT elf
(autoconf likes to see elf above to enable shared object support)
''')
    return 0

  if '--version' in args:

    print(version_string())
    print('''\
Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''')
    return 0

  if run_via_emxx:
    clang = shared.CLANG_CXX
  else:
    clang = shared.CLANG_CC

  if len(args) == 1 and args[0] == '-v': # -v with no inputs
    # autoconf likes to see 'GNU' in the output to enable shared object support
    print(version_string(), file=sys.stderr)
    return shared.check_call([clang, '-v'] + get_clang_flags(), check=False).returncode

  if '-dumpmachine' in args:
    print(get_llvm_target())
    return 0

  if '-dumpversion' in args: # gcc's doc states "Print the compiler version [...] and don't do anything else."
    print(shared.EMSCRIPTEN_VERSION)
    return 0

  if '--cflags' in args:
    # fake running the command, to see the full args we pass to clang
    args = [x for x in args if x != '--cflags']
    with misc_temp_files.get_file(suffix='.o') as temp_target:
      input_file = 'hello_world.c'
      cmd = [shared.PYTHON, sys.argv[0], shared.path_from_root('tests', input_file), '-v', '-c', '-o', temp_target] + args
      proc = run_process(cmd, stderr=PIPE, check=False)
      if proc.returncode != 0:
        print(proc.stderr)
        exit_with_error('error getting cflags')
      lines = [x for x in proc.stderr.splitlines() if clang in x and input_file in x]
      parts = shlex.split(lines[0].replace('\\', '\\\\'))
      parts = [x for x in parts if x not in ['-c', '-o', '-v', '-emit-llvm'] and input_file not in x and temp_target not in x]
      print(shared.shlex_join(parts[1:]))
    return 0

  shared.check_sanity()

  if '-print-search-dirs' in args:
    return run_process([clang, '-print-search-dirs'], check=False).returncode

  EMMAKEN_CFLAGS = os.environ.get('EMMAKEN_CFLAGS')
  if EMMAKEN_CFLAGS:
    args += shlex.split(EMMAKEN_CFLAGS)

  if 'EMMAKEN_NO_SDK' in os.environ:
    diagnostics.warning('deprecated', 'We hope to deprecated EMMAKEN_NO_SDK.  See https://github.com/emscripten-core/emscripten/issues/14050 if use use this feature.')

  ## Process argument and setup the compiler
  state = EmccState(args)
  options, newargs, settings_map = phase_parse_arguments(state)

  # For internal consistency, ensure we don't attempt or read or write any link time
  # settings until we reach the linking phase.
  settings.limit_settings(COMPILE_TIME_SETTINGS)

  newargs, input_files = phase_setup(options, state, newargs, settings_map)

  if state.mode == Mode.POST_LINK_ONLY:
    settings.limit_settings(None)
    target, wasm_target = phase_linker_setup(options, state, newargs, settings_map)
    process_libraries(state, [])
    if len(input_files) != 1:
      exit_with_error('--post-link requires a single input file')
    phase_post_link(options, state, input_files[0][1], wasm_target, target)
    return 0

  ## Compile source code to object files
  linker_inputs = phase_compile_inputs(options, state, newargs, input_files)

  if state.mode != Mode.COMPILE_AND_LINK:
    logger.debug('stopping after compile phase')
    for flag in state.link_flags:
      diagnostics.warning('unused-command-line-argument', "argument unused during compilation: '%s'" % flag[1])
    for f in linker_inputs:
      diagnostics.warning('unused-command-line-argument', "%s: linker input file unused because linking not done" % f[1])

    return 0

  # We have now passed the compile phase, allow reading/writing of all settings.
  settings.limit_settings(None)

  if options.output_file and options.output_file.startswith('-'):
    exit_with_error(f'invalid output filename: `{options.output_file}`')

  target, wasm_target = phase_linker_setup(options, state, newargs, settings_map)

  # Link object files using wasm-ld or llvm-link (for bitcode linking)
  linker_arguments = phase_calculate_linker_inputs(options, state, linker_inputs)

  if options.oformat == OFormat.OBJECT:
    logger.debug(f'link_to_object: {linker_arguments} -> {target}')
    building.link_to_object(linker_arguments, target)
    logger.debug('stopping after linking to object file')
    return 0

  phase_calculate_system_libraries(state, linker_arguments, linker_inputs, newargs)

  phase_link(linker_arguments, wasm_target)

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
    phase_post_link(options, state, wasm_target, wasm_target, target)

  return 0


@ToolchainProfiler.profile_block('calculate linker inputs')
def phase_calculate_linker_inputs(options, state, linker_inputs):
  using_lld = not (options.oformat == OFormat.OBJECT and settings.LTO)
  state.link_flags = filter_link_flags(state.link_flags, using_lld)

  # Decide what we will link
  process_libraries(state, linker_inputs)

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


@ToolchainProfiler.profile_block('parse arguments')
def phase_parse_arguments(state):
  """The first phase of the compiler.  Parse command line argument and
  populate settings.
  """
  newargs = state.orig_args.copy()

  # Scan and strip emscripten specific cmdline warning flags.
  # This needs to run before other cmdline flags have been parsed, so that
  # warnings are properly printed during arg parse.
  newargs = diagnostics.capture_warnings(newargs)

  for i in range(len(newargs)):
    if newargs[i] in ('-l', '-L', '-I'):
      # Scan for individual -l/-L/-I arguments and concatenate the next arg on
      # if there is no suffix
      newargs[i] += newargs[i + 1]
      newargs[i + 1] = ''

  options, settings_changes, user_js_defines, newargs = parse_args(newargs)

  if options.post_link or options.oformat == OFormat.BARE:
    diagnostics.warning('experimental', '--oformat=base/--post-link are experimental and subject to change.')

  explicit_settings_changes, newargs = parse_s_args(newargs)
  settings_changes += explicit_settings_changes

  user_settings = {}
  for s in settings_changes:
    key, value = s.split('=', 1)
    user_settings[key] = value

  # STRICT is used when applying settings so it needs to be applied first before
  # called ing `apply_settings`.
  strict_cmdline = user_settings.get('STRICT')
  if strict_cmdline:
    settings.STRICT = int(strict_cmdline)

  # Apply user -jsD settings
  for s in user_js_defines:
    settings[s[0]] = s[1]

  # Apply -s settings in newargs here (after optimization levels, so they can override them)
  apply_settings(user_settings)

  return options, newargs, user_settings


@ToolchainProfiler.profile_block('setup')
def phase_setup(options, state, newargs, settings_map):
  """Second phase: configure and setup the compiler based on the specified settings and arguments.
  """

  if settings.RUNTIME_LINKED_LIBS:
    diagnostics.warning('deprecated', 'RUNTIME_LINKED_LIBS is deprecated; you can simply list the libraries directly on the commandline now')
    newargs += settings.RUNTIME_LINKED_LIBS

  def default_setting(name, new_default):
    if name not in settings_map:
      setattr(settings, name, new_default)

  if settings.STRICT:
    default_setting('DEFAULT_TO_CXX', 0)

  # Find input files

  # These three arrays are used to store arguments of different types for
  # type-specific processing. In order to shuffle the arguments back together
  # after processing, all of these arrays hold tuples (original_index, value).
  # Note that the index part of the tuple can have a fractional part for input
  # arguments that expand into multiple processed arguments, as in -Wl,-f1,-f2.
  input_files = []

  # find input files with a simple heuristic. we should really analyze
  # based on a full understanding of gcc params, right now we just assume that
  # what is left contains no more |-x OPT| things
  skip = False
  has_header_inputs = False
  for i in range(len(newargs)):
    if skip:
      skip = False
      continue

    arg = newargs[i]
    if arg in ('-MT', '-MF', '-MJ', '-MQ', '-D', '-U', '-o', '-x',
               '-Xpreprocessor', '-include', '-imacros', '-idirafter',
               '-iprefix', '-iwithprefix', '-iwithprefixbefore',
               '-isysroot', '-imultilib', '-A', '-isystem', '-iquote',
               '-install_name', '-compatibility_version',
               '-current_version', '-I', '-L', '-include-pch',
               '-Xlinker', '-Xclang'):
      skip = True

    if not arg.startswith('-'):
      # we already removed -o <target>, so all these should be inputs
      newargs[i] = ''
      # os.devnul should always be reported as existing but there is bug in windows
      # python before 3.8:
      # https://bugs.python.org/issue1311
      if not os.path.exists(arg) and arg != os.devnull:
        exit_with_error('%s: No such file or directory ("%s" was expected to be an input file, based on the commandline arguments provided)', arg, arg)
      file_suffix = get_file_suffix(arg)
      if file_suffix in HEADER_ENDINGS:
        has_header_inputs = True
      if file_suffix in STATICLIB_ENDINGS and not building.is_ar(arg):
        if building.is_bitcode(arg):
          message = f'{arg}: File has a suffix of a static library {STATICLIB_ENDINGS}, but instead is an LLVM bitcode file! When linking LLVM bitcode files use .bc or .o.'
        else:
          message = arg + ': Unknown format, not a static library!'
        exit_with_error(message)
      if file_suffix in DYNAMICLIB_ENDINGS and not building.is_bitcode(arg) and not building.is_wasm(arg):
        # For shared libraries that are neither bitcode nor wasm, assuming its local native
        # library and attempt to find a library by the same name in our own library path.
        # TODO(sbc): Do we really need this feature?  See test_other.py:test_local_link
        libname = strip_prefix(get_library_basename(arg), 'lib')
        flag = '-l' + libname
        diagnostics.warning('map-unrecognized-libraries', f'unrecognized file type: `{arg}`.  Mapping to `{flag}` and hoping for the best')
        add_link_flag(state, i, flag)
      else:
        input_files.append((i, arg))
    elif arg.startswith('-L'):
      add_link_flag(state, i, arg)
      newargs[i] = ''
    elif arg.startswith('-l'):
      add_link_flag(state, i, arg)
      newargs[i] = ''
    elif arg.startswith('-Wl,'):
      # Multiple comma separated link flags can be specified. Create fake
      # fractional indices for these: -Wl,a,b,c,d at index 4 becomes:
      # (4, a), (4.25, b), (4.5, c), (4.75, d)
      link_flags_to_add = arg.split(',')[1:]
      for flag_index, flag in enumerate(link_flags_to_add):
        add_link_flag(state, i + float(flag_index) / len(link_flags_to_add), flag)
      newargs[i] = ''
    elif arg == '-Xlinker':
      add_link_flag(state, i + 1, newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif arg == '-s':
      # -s and some other compiler flags are normally passed onto the linker
      # TODO(sbc): Pass this and other flags through when using lld
      # link_flags.append((i, arg))
      newargs[i] = ''
    elif arg == '-':
      input_files.append((i, arg))
      newargs[i] = ''

  if not input_files and not state.link_flags:
    exit_with_error('no input files')

  newargs = [a for a in newargs if a]

  # SSEx is implemented on top of SIMD128 instruction set, but do not pass SSE flags to LLVM
  # so it won't think about generating native x86 SSE code.
  newargs = [x for x in newargs if x not in SIMD_INTEL_FEATURE_TOWER and x not in SIMD_NEON_FLAGS]

  state.has_dash_c = '-c' in newargs
  state.has_dash_S = '-S' in newargs
  state.has_dash_E = '-E' in newargs

  if options.post_link:
    state.mode = Mode.POST_LINK_ONLY
  elif state.has_dash_E or '-M' in newargs or '-MM' in newargs or '-fsyntax-only' in newargs:
    state.mode = Mode.PREPROCESS_ONLY
  elif has_header_inputs:
    state.mode = Mode.PCH
  elif state.has_dash_c or state.has_dash_S:
    state.mode = Mode.COMPILE_ONLY

  if state.mode in (Mode.COMPILE_ONLY, Mode.PREPROCESS_ONLY):
    for key in settings_map:
       if key not in COMPILE_TIME_SETTINGS:
         diagnostics.warning('unused-command-line-argument', "linker setting ignored during compilation: '%s'" % key)
    if state.has_dash_c:
      if '-emit-llvm' in newargs:
        options.default_object_extension = '.bc'
    elif state.has_dash_S:
      if '-emit-llvm' in newargs:
        options.default_object_extension = '.ll'
      else:
        options.default_object_extension = '.s'
    elif '-M' in newargs or '-MM' in newargs:
      options.default_object_extension = '.mout' # not bitcode, not js; but just dependency rule of the input file

    if options.output_file and len(input_files) > 1:
      exit_with_error('cannot specify -o with -c/-S/-E/-M and multiple source files')

  if settings.MAIN_MODULE or settings.SIDE_MODULE:
    settings.RELOCATABLE = 1

  if settings.USE_PTHREADS and '-pthread' not in newargs:
    newargs += ['-pthread']

  if 'DISABLE_EXCEPTION_CATCHING' in settings_map and 'EXCEPTION_CATCHING_ALLOWED' in settings_map:
    # If we get here then the user specified both DISABLE_EXCEPTION_CATCHING and EXCEPTION_CATCHING_ALLOWED
    # on the command line.  This is no longer valid so report either an error or a warning (for
    # backwards compat with the old `DISABLE_EXCEPTION_CATCHING=2`
    if settings_map['DISABLE_EXCEPTION_CATCHING'] in ('0', '2'):
      diagnostics.warning('deprecated', 'DISABLE_EXCEPTION_CATCHING=X is no longer needed when specifying EXCEPTION_CATCHING_ALLOWED')
    else:
      exit_with_error('DISABLE_EXCEPTION_CATCHING and EXCEPTION_CATCHING_ALLOWED are mutually exclusive')

  if settings.EXCEPTION_CATCHING_ALLOWED:
    settings.DISABLE_EXCEPTION_CATCHING = 0

  if settings.DISABLE_EXCEPTION_THROWING and not settings.DISABLE_EXCEPTION_CATCHING:
    exit_with_error("DISABLE_EXCEPTION_THROWING was set (probably from -fno-exceptions) but is not compatible with enabling exception catching (DISABLE_EXCEPTION_CATCHING=0). If you don't want exceptions, set DISABLE_EXCEPTION_CATCHING to 1; if you do want exceptions, don't link with -fno-exceptions")

  return (newargs, input_files)


@ToolchainProfiler.profile_block('linker_setup')
def phase_linker_setup(options, state, newargs, settings_map):
  autoconf = os.environ.get('EMMAKEN_JUST_CONFIGURE') or 'conftest.c' in state.orig_args
  if autoconf:
    # configure tests want a more shell-like style, where we emit return codes on exit()
    settings.EXIT_RUNTIME = 1
    # use node.js raw filesystem access, to behave just like a native executable
    settings.NODERAWFS = 1
    # Add `#!` line to output JS and make it executable.
    options.executable = True

  ldflags = emsdk_ldflags(newargs)
  for f in ldflags:
    add_link_flag(state, sys.maxsize, f)

  if options.emrun:
    options.pre_js += read_file(shared.path_from_root('src', 'emrun_prejs.js')) + '\n'
    options.post_js += read_file(shared.path_from_root('src', 'emrun_postjs.js')) + '\n'
    # emrun mode waits on program exit
    settings.EXIT_RUNTIME = 1

  if options.cpu_profiler:
    options.post_js += read_file(shared.path_from_root('src', 'cpuprofiler.js')) + '\n'

  if options.memory_profiler:
    settings.MEMORYPROFILER = 1

  if options.thread_profiler:
    options.post_js += read_file(shared.path_from_root('src', 'threadprofiler.js')) + '\n'

  if options.memory_init_file is None:
    options.memory_init_file = settings.OPT_LEVEL >= 2

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

  if settings.EXTRA_EXPORTED_RUNTIME_METHODS:
    diagnostics.warning('deprecated', 'EXTRA_EXPORTED_RUNTIME_METHODS is deprecated, please use EXPORTED_RUNTIME_METHODS instead')
    settings.EXPORTED_RUNTIME_METHODS += settings.EXTRA_EXPORTED_RUNTIME_METHODS

  # If no output format was sepecific we try to imply the format based on
  # the output filename extension.
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

  if options.oformat == OFormat.MJS:
    settings.EXPORT_ES6 = 1
    settings.MODULARIZE = 1

  if options.oformat in (OFormat.WASM, OFormat.BARE):
    # If the user asks directly for a wasm file then this *is* the target
    wasm_target = target
  else:
    # Otherwise the wasm file is produced alongside the final target.
    wasm_target = get_secondary_target(target, '.wasm')

  if settings.SAFE_HEAP not in [0, 1]:
    exit_with_error('emcc: SAFE_HEAP must be 0 or 1')

  if not settings.WASM:
    # When the user requests non-wasm output, we enable wasm2js. that is,
    # we still compile to wasm normally, but we compile the final output
    # to js.
    settings.WASM = 1
    settings.WASM2JS = 1
  if settings.WASM == 2:
    # Requesting both Wasm and Wasm2JS support
    settings.WASM2JS = 1

  if (options.oformat == OFormat.WASM or settings.PURE_WASI) and not settings.SIDE_MODULE:
    # if the output is just a wasm file, it will normally be a standalone one,
    # as there is no JS. an exception are side modules, as we can't tell at
    # compile time whether JS will be involved or not - the main module may
    # have JS, and the side module is expected to link against that.
    # we also do not support standalone mode in fastcomp.
    settings.STANDALONE_WASM = 1

  if settings.LZ4:
    settings.EXPORTED_RUNTIME_METHODS += ['LZ4']

  if settings.WASM2C:
    # wasm2c only makes sense with standalone wasm - there will be no JS,
    # just wasm and then C
    settings.STANDALONE_WASM = 1
    # wasm2c doesn't need any special handling of i64, we have proper i64
    # handling on the FFI boundary, which is exactly like the case of JS with
    # BigInt support
    settings.WASM_BIGINT = 1

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
    if 'EXPORTED_FUNCTIONS' in settings_map:
      if '_main' not in settings.USER_EXPORTED_FUNCTIONS:
        settings.EXPECT_MAIN = 0
    else:
      assert not settings.EXPORTED_FUNCTIONS
      settings.EXPORTED_FUNCTIONS = ['_main']

  if settings.STANDALONE_WASM:
    # In STANDALONE_WASM mode we either build a command or a reactor.
    # See https://github.com/WebAssembly/WASI/blob/main/design/application-abi.md
    # For a command we always want EXIT_RUNTIME=1
    # For a reactor we always want EXIT_RUNTIME=0
    if 'EXIT_RUNTIME' in settings_map:
      exit_with_error('Explictly setting EXIT_RUNTIME not compatible with STANDALONE_WASM.  EXIT_RUNTIME will always be True for programs (with a main function) and False for reactors (not main function).')
    settings.EXIT_RUNTIME = settings.EXPECT_MAIN

  # Note the exports the user requested
  building.user_requested_exports.update(settings.EXPORTED_FUNCTIONS)

  def default_setting(name, new_default):
    if name not in settings_map:
      setattr(settings, name, new_default)

  if settings.OPT_LEVEL >= 1:
    default_setting('ASSERTIONS', 0)
  if settings.SHRINK_LEVEL >= 2:
    default_setting('EVAL_CTORS', 1)

  # -s ASSERTIONS=1 implies basic stack overflow checks, and ASSERTIONS=2
  # implies full stack overflow checks.
  if settings.ASSERTIONS:
    # However, we don't set this default in PURE_WASI, or when we are linking without standard
    # libraries because STACK_OVERFLOW_CHECK depends on emscripten_stack_get_end which is defined
    # in libcompiler-rt.
    if not settings.PURE_WASI and '-nostdlib' not in newargs and '-nodefaultlibs' not in newargs:
      default_setting('STACK_OVERFLOW_CHECK', max(settings.ASSERTIONS, settings.STACK_OVERFLOW_CHECK))

  if settings.LLD_REPORT_UNDEFINED or settings.STANDALONE_WASM:
    # Reporting undefined symbols at wasm-ld time requires us to know if we have a `main` function
    # or not, as does standalone wasm mode.
    # TODO(sbc): Remove this once this becomes the default
    settings.IGNORE_MISSING_MAIN = 0

  # For users that opt out of WARN_ON_UNDEFINED_SYMBOLS we assume they also
  # want to opt out of ERROR_ON_UNDEFINED_SYMBOLS.
  if settings_map.get('WARN_ON_UNDEFINED_SYMBOLS') == '0':
    default_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0)

  # It is unlikely that developers targeting "native web" APIs with MINIMAL_RUNTIME need
  # errno support by default.
  if settings.MINIMAL_RUNTIME:
    default_setting('SUPPORT_ERRNO', 0)
    # Require explicit -lfoo.js flags to link with JS libraries.
    default_setting('AUTO_JS_LIBRARIES', 0)

  if settings.STRICT:
    default_setting('STRICT_JS', 1)
    default_setting('AUTO_JS_LIBRARIES', 0)
    default_setting('AUTO_NATIVE_LIBRARIES', 0)
    default_setting('AUTO_ARCHIVE_INDEXES', 0)
    default_setting('IGNORE_MISSING_MAIN', 0)
    default_setting('ALLOW_UNIMPLEMENTED_SYSCALLS', 0)

  if not settings.AUTO_JS_LIBRARIES:
    default_setting('USE_SDL', 0)

  # Default to TEXTDECODER=2 (always use TextDecoder to decode UTF-8 strings)
  # in -Oz builds, since custom decoder for UTF-8 takes up space.
  # In pthreads enabled builds, TEXTDECODER==2 may not work, see
  # https://github.com/whatwg/encoding/issues/172
  # When supporting shell environments, do not do this as TextDecoder is not
  # widely supported there.
  if settings.SHRINK_LEVEL >= 2 and not settings.USE_PTHREADS and \
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
      exit_with_error('--emrun is not compatible with -s MINIMAL_RUNTIME=1')
    settings.EXPORTED_RUNTIME_METHODS.append('addOnExit')

  if options.use_closure_compiler:
    settings.USE_CLOSURE_COMPILER = options.use_closure_compiler

  if settings.CLOSURE_WARNINGS not in ['quiet', 'warn', 'error']:
    exit_with_error('Invalid option -s CLOSURE_WARNINGS=%s specified! Allowed values are "quiet", "warn" or "error".' % settings.CLOSURE_WARNINGS)

  # Include dynCall() function by default in DYNCALLS builds in classic runtime; in MINIMAL_RUNTIME, must add this explicitly.
  if settings.DYNCALLS and not settings.MINIMAL_RUNTIME:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$dynCall']

  if settings.MAIN_MODULE:
    assert not settings.SIDE_MODULE
    if settings.MAIN_MODULE == 1:
      settings.INCLUDE_FULL_LIBRARY = 1
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$preloadDylibs']
  elif settings.SIDE_MODULE:
    assert not settings.MAIN_MODULE
    # memory init file is not supported with side modules, must be executable synchronously (for dlopen)
    options.memory_init_file = False

  # If we are including the entire JS library then we know for sure we will, by definition,
  # require all the reverse dependencies.
  if settings.INCLUDE_FULL_LIBRARY:
    default_setting('REVERSE_DEPS', 'all')

  if settings.MAIN_MODULE == 1 or settings.SIDE_MODULE == 1:
    settings.LINKABLE = 1
    settings.EXPORT_ALL = 1

  if settings.MAIN_MODULE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$getDylinkMetadata', '$mergeLibSymbols']

  if settings.RELOCATABLE:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
        '$reportUndefinedSymbols',
        '$relocateExports',
        '$GOTHandler',
        '__heap_base',
        '__stack_pointer',
    ]
    settings.EXPORTED_FUNCTIONS += [
        # This needs to be exported on the Module object too so it's visible
        # to side modules too.
        '___heap_base',
        # Unconditional dependency in library_dylink.js
        '_setThrew',
    ]
    if settings.MINIMAL_RUNTIME:
      exit_with_error('MINIMAL_RUNTIME is not compatible with relocatable output')
    if settings.WASM2JS:
      exit_with_error('WASM2JS is not compatible with relocatable output')
    # shared modules need memory utilities to allocate their memory
    settings.EXPORTED_RUNTIME_METHODS += ['allocate']
    settings.ALLOW_TABLE_GROWTH = 1

  # various settings require sbrk() access
  if settings.DETERMINISTIC or \
     settings.EMSCRIPTEN_TRACING or \
     settings.SAFE_HEAP or \
     settings.MEMORYPROFILER:
    settings.EXPORTED_FUNCTIONS += ['_sbrk']

  if settings.MEMORYPROFILER:
    settings.EXPORTED_FUNCTIONS += ['___heap_base',
                                    '_emscripten_stack_get_base',
                                    '_emscripten_stack_get_end',
                                    '_emscripten_stack_get_current']

  if settings.ASYNCIFY_LAZY_LOAD_CODE:
    settings.ASYNCIFY = 1

  if settings.ASYNCIFY:
    # See: https://github.com/emscripten-core/emscripten/issues/12065
    # See: https://github.com/emscripten-core/emscripten/issues/12066
    settings.DYNCALLS = 1
    settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_get_base',
                                    '_emscripten_stack_get_end',
                                    '_emscripten_stack_set_limits']

  settings.ASYNCIFY_ADD = unmangle_symbols_from_cmdline(settings.ASYNCIFY_ADD)
  settings.ASYNCIFY_REMOVE = unmangle_symbols_from_cmdline(settings.ASYNCIFY_REMOVE)
  settings.ASYNCIFY_ONLY = unmangle_symbols_from_cmdline(settings.ASYNCIFY_ONLY)

  if state.mode == Mode.COMPILE_AND_LINK and final_suffix in ('.o', '.bc', '.so', '.dylib') and not settings.SIDE_MODULE:
    diagnostics.warning('emcc', 'generating an executable with an object extension (%s).  If you meant to build an object file please use `-c, `-r`, or `-shared`' % final_suffix)

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

  if settings.STACK_OVERFLOW_CHECK:
    # The basic writeStackCookie/checkStackCookie mechanism just needs to know where the end
    # of the stack is.
    settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_get_end', '_emscripten_stack_get_free']
    if settings.STACK_OVERFLOW_CHECK == 2:
      # The full checking done by binaryen's `StackCheck` pass also needs to know the base of the
      # stack.
      settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_get_base']

    # We call one of these two functions during startup which caches the stack limits
    # in wasm globals allowing get_base/get_free to be super fast.
    # See compiler-rt/stack_limits.S.
    if settings.RELOCATABLE:
      settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_set_limits']
    else:
      settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_init']

  if settings.MODULARIZE:
    if settings.PROXY_TO_WORKER:
      exit_with_error('-s MODULARIZE=1 is not compatible with --proxy-to-worker (if you want to run in a worker with -s MODULARIZE=1, you likely want to do the worker side setup manually)')
    # in MINIMAL_RUNTIME we may not need to emit the Promise code, as the
    # HTML output creates a singleton instance, and it does so without the
    # Promise. However, in Pthreads mode the Promise is used for worker
    # creation.
    if settings.MINIMAL_RUNTIME and options.oformat == OFormat.HTML and not settings.USE_PTHREADS:
      settings.EXPORT_READY_PROMISE = 0

  if settings.LEGACY_VM_SUPPORT:
    if settings.WASM2JS:
      settings.POLYFILL_OLD_MATH_FUNCTIONS = 1

    # Support all old browser versions
    settings.MIN_FIREFOX_VERSION = 0
    settings.MIN_SAFARI_VERSION = 0
    settings.MIN_IE_VERSION = 0
    settings.MIN_EDGE_VERSION = 0
    settings.MIN_CHROME_VERSION = 0

  if settings.MIN_CHROME_VERSION <= 37:
    settings.WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG = 1

  setup_environment_settings()

  # Silently drop any individual backwards compatibility emulation flags that are known never to occur on browsers that support WebAssembly.
  if not settings.WASM2JS:
    settings.POLYFILL_OLD_MATH_FUNCTIONS = 0
    settings.WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG = 0

  if settings.STB_IMAGE and final_suffix in EXECUTABLE_ENDINGS:
    state.forced_stdlibs.append('libstb_image')
    settings.EXPORTED_FUNCTIONS += ['_stbi_load', '_stbi_load_from_memory', '_stbi_image_free']

  if settings.USE_WEBGL2:
    settings.MAX_WEBGL_VERSION = 2

  # MIN_WEBGL_VERSION=2 implies MAX_WEBGL_VERSION=2
  if settings.MIN_WEBGL_VERSION == 2:
    default_setting('MAX_WEBGL_VERSION', 2)

  if settings.MIN_WEBGL_VERSION > settings.MAX_WEBGL_VERSION:
    exit_with_error('MIN_WEBGL_VERSION must be smaller or equal to MAX_WEBGL_VERSION!')

  if not settings.GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS and settings.GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS:
    exit_with_error('-s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0 only makes sense with -s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0!')

  if settings.ASMFS and final_suffix in EXECUTABLE_ENDINGS:
    state.forced_stdlibs.append('libasmfs')
    settings.FILESYSTEM = 0
    settings.SYSCALLS_REQUIRE_FILESYSTEM = 0
    settings.FETCH = 1
    settings.JS_LIBRARIES.append((0, 'library_asmfs.js'))

  # Explicitly drop linking in a malloc implementation if program is not using any dynamic allocation calls.
  if not settings.USES_DYNAMIC_ALLOC:
    settings.MALLOC = 'none'

  if settings.FETCH and final_suffix in EXECUTABLE_ENDINGS:
    state.forced_stdlibs.append('libfetch')
    settings.JS_LIBRARIES.append((0, 'library_fetch.js'))
    if settings.USE_PTHREADS:
      settings.FETCH_WORKER_FILE = unsuffixed(os.path.basename(target)) + '.fetch.js'

  if settings.DEMANGLE_SUPPORT:
    settings.EXPORTED_FUNCTIONS += ['___cxa_demangle']

  if settings.FULL_ES3:
    settings.FULL_ES2 = 1
    settings.MAX_WEBGL_VERSION = max(2, settings.MAX_WEBGL_VERSION)

  if settings.EMBIND:
    state.forced_stdlibs.append('libembind')

  settings.EXPORTED_FUNCTIONS += ['_stackSave', '_stackRestore', '_stackAlloc']
  if not settings.STANDALONE_WASM:
    # in standalone mode, crt1 will call the constructors from inside the wasm
    settings.EXPORTED_FUNCTIONS.append('___wasm_call_ctors')

  if settings.RELOCATABLE and not settings.DYNAMIC_EXECUTION:
    exit_with_error('cannot have both DYNAMIC_EXECUTION=0 and RELOCATABLE enabled at the same time, since RELOCATABLE needs to eval()')

  if settings.SIDE_MODULE and settings.GLOBAL_BASE != -1:
    exit_with_error('Cannot set GLOBAL_BASE when building SIDE_MODULE')

  # When building a side module we currently have to assume that any undefined
  # symbols that exist at link time will be satisfied by the main module or JS.
  if settings.SIDE_MODULE:
    default_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0)
    default_setting('WARN_ON_UNDEFINED_SYMBOLS', 0)
  else:
    settings.EXPORT_IF_DEFINED.append('__start_em_asm')
    settings.EXPORT_IF_DEFINED.append('__stop_em_asm')

  if options.use_preload_plugins or len(options.preload_files) or len(options.embed_files):
    if settings.NODERAWFS:
      exit_with_error('--preload-file and --embed-file cannot be used with NODERAWFS which disables virtual filesystem')
    # if we include any files, or intend to use preload plugins, then we definitely need filesystem support
    settings.FORCE_FILESYSTEM = 1

  if settings.PROXY_TO_WORKER or options.use_preload_plugins:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$Browser']

  if not settings.MINIMAL_RUNTIME:
    # In non-MINIMAL_RUNTIME, the core runtime depends on these functions to be present. (In MINIMAL_RUNTIME, they are
    # no longer always bundled in)
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += [
      '$demangle',
      '$demangleAll',
      '$jsStackTrace',
      '$stackTrace'
    ]

  if settings.FILESYSTEM and not settings.BOOTSTRAPPING_STRUCT_INFO and not settings.STANDALONE_WASM:
    # to flush streams on FS exit, we need to be able to call fflush
    # we only include it if the runtime is exitable, or when ASSERTIONS
    # (ASSERTIONS will check that streams do not need to be flushed,
    # helping people see when they should have enabled EXIT_RUNTIME)
    if settings.EXIT_RUNTIME or settings.ASSERTIONS:
      settings.EXPORTED_FUNCTIONS += ['_fflush']

  if settings.SUPPORT_ERRNO and not settings.BOOTSTRAPPING_STRUCT_INFO:
    # so setErrNo JS library function can report errno back to C
    settings.EXPORTED_FUNCTIONS += ['___errno_location']

  if settings.SAFE_HEAP:
    # SAFE_HEAP check includes calling emscripten_get_sbrk_ptr() from wasm
    settings.EXPORTED_FUNCTIONS += ['_emscripten_get_sbrk_ptr', '_emscripten_stack_get_base']
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$unSign']

  if not settings.DECLARE_ASM_MODULE_EXPORTS:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$exportAsmFunctions']

  if settings.ALLOW_MEMORY_GROWTH:
    # Setting ALLOW_MEMORY_GROWTH turns off ABORTING_MALLOC, as in that mode we default to
    # the behavior of trying to grow and returning 0 from malloc on failure, like
    # a standard system would. However, if the user sets the flag it
    # overrides that.
    default_setting('ABORTING_MALLOC', 0)

  if settings.USE_PTHREADS:
    if settings.USE_PTHREADS == 2:
      exit_with_error('USE_PTHREADS=2 is no longer supported')
    if settings.ALLOW_MEMORY_GROWTH:
      diagnostics.warning('pthreads-mem-growth', 'USE_PTHREADS + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271')
    settings.JS_LIBRARIES.append((0, 'library_pthread.js'))
    settings.EXPORTED_FUNCTIONS += [
      '___emscripten_pthread_data_constructor',
      '__emscripten_call_on_thread',
      '__emscripten_main_thread_futex',
      '__emscripten_thread_init',
      '__emscripten_thread_exit',
      '_emscripten_current_thread_process_queued_calls',
      '__emscripten_allow_main_runtime_queued_calls',
      '_emscripten_futex_wake',
      '_emscripten_get_global_libc',
      '_emscripten_main_browser_thread_id',
      '_emscripten_main_thread_process_queued_calls',
      '_emscripten_run_in_main_runtime_thread_js',
      '_emscripten_stack_set_limits',
      '_emscripten_sync_run_in_main_thread_2',
      '_emscripten_sync_run_in_main_thread_4',
      '_emscripten_tls_init',
      '_pthread_self',
      '_pthread_testcancel',
    ]
    # Some of these symbols are using by worker.js but otherwise unreferenced.
    # Because emitDCEGraph only considered the main js file, and not worker.js
    # we have explicitly mark these symbols as user-exported so that they will
    # kept alive through DCE.
    # TODO: Find a less hacky way to do this, perhaps by also scanning worker.js
    # for roots.
    building.user_requested_exports.add('_emscripten_tls_init')
    building.user_requested_exports.add('_emscripten_current_thread_process_queued_calls')

    # set location of worker.js
    settings.PTHREAD_WORKER_FILE = unsuffixed(os.path.basename(target)) + '.worker.js'
  else:
    settings.JS_LIBRARIES.append((0, 'library_pthread_stub.js'))

  if settings.FORCE_FILESYSTEM and not settings.MINIMAL_RUNTIME:
    # when the filesystem is forced, we export by default methods that filesystem usage
    # may need, including filesystem usage from standalone file packager output (i.e.
    # file packages not built together with emcc, but that are loaded at runtime
    # separately, and they need emcc's output to contain the support they need)
    if not settings.ASMFS:
      settings.EXPORTED_RUNTIME_METHODS += [
        'FS_createPath',
        'FS_createDataFile',
        'FS_createPreloadedFile',
        'FS_createLazyFile',
        'FS_createDevice',
        'FS_unlink'
      ]

    settings.EXPORTED_RUNTIME_METHODS += [
      'addRunDependency',
      'removeRunDependency',
    ]

  if not settings.MINIMAL_RUNTIME or settings.EXIT_RUNTIME:
    # MINIMAL_RUNTIME only needs callRuntimeCallbacks in certain cases, but the normal runtime
    # always does.
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$callRuntimeCallbacks']

  if settings.USE_PTHREADS:
    # memalign is used to ensure allocated thread stacks are aligned.
    settings.EXPORTED_FUNCTIONS += ['_memalign']

    if settings.MINIMAL_RUNTIME:
      building.user_requested_exports.add('exit')

    if settings.PROXY_TO_PTHREAD:
      settings.EXPORTED_FUNCTIONS += ['_emscripten_proxy_main']

    # pthread stack setup and other necessary utilities
    def include_and_export(name):
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$' + name]
      settings.EXPORTED_FUNCTIONS += [name]

    include_and_export('establishStackSpace')
    include_and_export('invokeEntryPoint')
    if not settings.MINIMAL_RUNTIME:
      # keepRuntimeAlive does not apply to MINIMAL_RUNTIME.
      settings.EXPORTED_RUNTIME_METHODS += ['keepRuntimeAlive']

    if settings.MODULARIZE:
      if not settings.EXPORT_ES6 and settings.EXPORT_NAME == 'Module':
        exit_with_error('pthreads + MODULARIZE currently require you to set -s EXPORT_NAME=Something (see settings.js) to Something != Module, so that the .worker.js file can work')

      # MODULARIZE+USE_PTHREADS mode requires extra exports out to Module so that worker.js
      # can access them:

      # general threading variables:
      settings.EXPORTED_RUNTIME_METHODS += ['PThread']

      # To keep code size to minimum, MINIMAL_RUNTIME does not utilize the global ExitStatus
      # object, only regular runtime has it.
      if not settings.MINIMAL_RUNTIME:
        settings.EXPORTED_RUNTIME_METHODS += ['ExitStatus']

    if settings.RELOCATABLE:
      # phtreads + dyanmic linking has certain limitations
      if settings.SIDE_MODULE:
        diagnostics.warning('experimental', '-s SIDE_MODULE + pthreads is experimental')
      elif settings.MAIN_MODULE:
        diagnostics.warning('experimental', '-s MAIN_MODULE + pthreads is experimental')
      elif settings.LINKABLE:
        diagnostics.warning('experimental', '-s LINKABLE + pthreads is experimental')

      default_setting('SUPPORT_LONGJMP', 0)
      if settings.SUPPORT_LONGJMP:
        exit_with_error('SUPPORT_LONGJMP is not compatible with pthreads + dynamic linking')

    if settings.PROXY_TO_WORKER:
      exit_with_error('--proxy-to-worker is not supported with -s USE_PTHREADS>0! Use the option -s PROXY_TO_PTHREAD=1 if you want to run the main thread of a multithreaded application in a web worker.')
  elif settings.PROXY_TO_PTHREAD:
    exit_with_error('-s PROXY_TO_PTHREAD=1 requires -s USE_PTHREADS to work!')

  def check_memory_setting(setting):
    if settings[setting] % webassembly.WASM_PAGE_SIZE != 0:
      exit_with_error(f'{setting} must be a multiple of WebAssembly page size (64KiB), was {settings[setting]}')

  check_memory_setting('INITIAL_MEMORY')
  check_memory_setting('MAXIMUM_MEMORY')
  if settings.INITIAL_MEMORY >= 2 * 1024 * 1024 * 1024:
    exit_with_error('INITIAL_MEMORY must be less than 2GB due to current spec limitations')
  if settings.INITIAL_MEMORY < settings.TOTAL_STACK:
    exit_with_error(f'INITIAL_MEMORY must be larger than TOTAL_STACK, was {settings.INITIAL_MEMORY} (TOTAL_STACK={settings.TOTAL_STACK})')
  if settings.MEMORY_GROWTH_LINEAR_STEP != -1:
    check_memory_setting('MEMORY_GROWTH_LINEAR_STEP')

  if 'MAXIMUM_MEMORY' in settings_map and not settings.ALLOW_MEMORY_GROWTH:
    diagnostics.warning('unused-command-line-argument', 'MAXIMUM_MEMORY is only meaningful with ALLOW_MEMORY_GROWTH')

  if settings.EXPORT_ES6 and not settings.MODULARIZE:
    # EXPORT_ES6 requires output to be a module
    if 'MODULARIZE' in settings_map:
      exit_with_error('EXPORT_ES6 requires MODULARIZE to be set')
    settings.MODULARIZE = 1

  if settings.MODULARIZE and not settings.DECLARE_ASM_MODULE_EXPORTS:
    # When MODULARIZE option is used, currently requires declaring all module exports
    # individually - TODO: this could be optimized
    exit_with_error('DECLARE_ASM_MODULE_EXPORTS=0 is not compatible with MODULARIZE')

  # When not declaring wasm module exports in outer scope one by one, disable minifying
  # wasm module export names so that the names can be passed directly to the outer scope.
  # Also, if using library_exports.js API, disable minification so that the feature can work.
  if not settings.DECLARE_ASM_MODULE_EXPORTS or '-lexports.js' in [x for _, x in state.link_flags]:
    settings.MINIFY_ASMJS_EXPORT_NAMES = 0

  # Enable minification of wasm imports and exports when appropriate, if we
  # are emitting an optimized JS+wasm combo (then the JS knows how to load the minified names).
  # Things that process the JS after this operation would be done must disable this.
  # For example, ASYNCIFY_LAZY_LOAD_CODE needs to identify import names.
  if will_metadce() and \
      settings.OPT_LEVEL >= 2 and \
      settings.DEBUG_LEVEL <= 2 and \
      options.oformat not in (OFormat.WASM, OFormat.BARE) and \
      not settings.LINKABLE and \
      not settings.STANDALONE_WASM and \
      not settings.AUTODEBUG and \
      not settings.ASSERTIONS and \
      not settings.RELOCATABLE and \
      not settings.ASYNCIFY_LAZY_LOAD_CODE and \
          settings.MINIFY_ASMJS_EXPORT_NAMES:
    settings.MINIFY_WASM_IMPORTS_AND_EXPORTS = 1
    settings.MINIFY_WASM_IMPORTED_MODULES = 1

  if settings.MINIMAL_RUNTIME:
    # Minimal runtime uses a different default shell file
    if options.shell_path == shared.path_from_root('src', 'shell.html'):
      options.shell_path = shared.path_from_root('src', 'shell_minimal_runtime.html')

    if settings.ASSERTIONS and settings.MINIMAL_RUNTIME:
      # In ASSERTIONS-builds, functions UTF8ArrayToString() and stringToUTF8Array() (which are not JS library functions), both
      # use warnOnce(), which in MINIMAL_RUNTIME is a JS library function, so explicitly have to mark dependency to warnOnce()
      # in that case. If string functions are turned to library functions in the future, then JS dependency tracking can be
      # used and this special directive can be dropped.
      settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$warnOnce']

  if settings.MODULARIZE and not (settings.EXPORT_ES6 and not settings.SINGLE_FILE) and \
     settings.EXPORT_NAME == 'Module' and options.oformat == OFormat.HTML and \
     (options.shell_path == shared.path_from_root('src', 'shell.html') or options.shell_path == shared.path_from_root('src', 'shell_minimal.html')):
    exit_with_error(f'Due to collision in variable name "Module", the shell file "{options.shell_path}" is not compatible with build options "-s MODULARIZE=1 -s EXPORT_NAME=Module". Either provide your own shell file, change the name of the export to something else to avoid the name collision. (see https://github.com/emscripten-core/emscripten/issues/7950 for details)')

  if settings.STANDALONE_WASM:
    if settings.USE_PTHREADS:
      exit_with_error('STANDALONE_WASM does not support pthreads yet')
    if settings.MINIMAL_RUNTIME:
      exit_with_error('MINIMAL_RUNTIME reduces JS size, and is incompatible with STANDALONE_WASM which focuses on ignoring JS anyhow and being 100% wasm')
    # the wasm must be runnable without the JS, so there cannot be anything that
    # requires JS legalization
    settings.LEGALIZE_JS_FFI = 0

  # TODO(sbc): Remove WASM2JS here once the size regression it would introduce has been fixed.
  if settings.USE_PTHREADS or settings.RELOCATABLE or settings.ASYNCIFY_LAZY_LOAD_CODE or settings.WASM2JS:
    settings.IMPORTED_MEMORY = 1

  if settings.WASM_BIGINT:
    settings.LEGALIZE_JS_FFI = 0

  if settings.SINGLE_FILE:
    settings.GENERATE_SOURCE_MAP = 0

  if options.use_closure_compiler == 2 and not settings.WASM2JS:
    exit_with_error('closure compiler mode 2 assumes the code is asm.js, so not meaningful for wasm')

  if 'MEM_INIT_METHOD' in settings_map:
    exit_with_error('MEM_INIT_METHOD is not supported in wasm. Memory will be embedded in the wasm binary if threads are not used, and included in a separate file if threads are used.')

  if settings.WASM2JS:
    settings.MAYBE_WASM2JS = 1
    # when using wasm2js, if the memory segments are in the wasm then they
    # end up converted by wasm2js into base64 encoded JS. alternatively, we
    # can use a .mem file like asm.js used to.
    # generally we follow what the options tell us to do (which is to use
    # a .mem file in most cases, since it is binary & compact). however, for
    # pthreads we must keep the memory segments in the wasm as they will be
    # passive segments which the .mem format cannot handle.
    settings.MEM_INIT_IN_WASM = not options.memory_init_file or settings.SINGLE_FILE or settings.USE_PTHREADS
  else:
    # wasm includes the mem init in the wasm binary. The exception is
    # wasm2js, which behaves more like js.
    options.memory_init_file = True
    settings.MEM_INIT_IN_WASM = True

  # wasm side modules have suffix .wasm
  if settings.SIDE_MODULE and target.endswith('.js'):
    diagnostics.warning('emcc', 'output suffix .js requested, but wasm side modules are just wasm files; emitting only a .wasm, no .js')

  sanitize = set()

  for arg in newargs:
    if arg.startswith('-fsanitize='):
      sanitize.update(arg.split('=', 1)[1].split(','))
    elif arg.startswith('-fno-sanitize='):
      sanitize.difference_update(arg.split('=', 1)[1].split(','))

  if sanitize:
    settings.USE_OFFSET_CONVERTER = 1
    settings.EXPORTED_FUNCTIONS += [
        '_memalign',
        '_emscripten_builtin_memalign',
        '_emscripten_builtin_malloc',
        '_emscripten_builtin_free',
        '___heap_base',
        '___global_base'
    ]

  if settings.USE_OFFSET_CONVERTER and settings.WASM2JS:
    exit_with_error('wasm2js is not compatible with USE_OFFSET_CONVERTER (see #14630)')

  if sanitize & UBSAN_SANITIZERS:
    if '-fsanitize-minimal-runtime' in newargs:
      settings.UBSAN_RUNTIME = 1
    else:
      settings.UBSAN_RUNTIME = 2

  if 'leak' in sanitize:
    settings.USE_LSAN = 1
    settings.EXIT_RUNTIME = 1

    if settings.LINKABLE:
      exit_with_error('LSan does not support dynamic linking')

  if 'address' in sanitize:
    settings.USE_ASAN = 1
    if not settings.UBSAN_RUNTIME:
      settings.UBSAN_RUNTIME = 2

    settings.EXPORTED_FUNCTIONS += [
      '_emscripten_builtin_memset',
      '_asan_c_load_1', '_asan_c_load_1u',
      '_asan_c_load_2', '_asan_c_load_2u',
      '_asan_c_load_4', '_asan_c_load_4u',
      '_asan_c_load_f', '_asan_c_load_d',
      '_asan_c_store_1', '_asan_c_store_1u',
      '_asan_c_store_2', '_asan_c_store_2u',
      '_asan_c_store_4', '_asan_c_store_4u',
      '_asan_c_store_f', '_asan_c_store_d',
    ]

    if settings.ASAN_SHADOW_SIZE != -1:
      diagnostics.warning('emcc', 'ASAN_SHADOW_SIZE is ignored and will be removed in a future release')

    if settings.GLOBAL_BASE != -1:
      exit_with_error("ASan does not support custom GLOBAL_BASE")

    max_mem = settings.INITIAL_MEMORY
    if settings.ALLOW_MEMORY_GROWTH:
      max_mem = settings.MAXIMUM_MEMORY

    shadow_size = max_mem // 8
    settings.GLOBAL_BASE = shadow_size

    sanitizer_mem = (shadow_size + webassembly.WASM_PAGE_SIZE) & ~webassembly.WASM_PAGE_SIZE
    # sanitizers do at least 9 page allocs of a single page during startup.
    sanitizer_mem += webassembly.WASM_PAGE_SIZE * 9
    # we also allocate at least 11 "regions". Each region is kRegionSize (2 << 20) but
    # MmapAlignedOrDieOnFatalError adds another 2 << 20 for alignment.
    sanitizer_mem += (1 << 21) * 11
    # When running in the threaded mode asan needs to allocate an array of kMaxNumberOfThreads
    # (1 << 22) pointers.  See compiler-rt/lib/asan/asan_thread.cpp.
    if settings.USE_PTHREADS:
      sanitizer_mem += (1 << 22) * 4

    # Increase the size of the initial memory according to how much memory
    # we think the sanitizers will use.
    settings.INITIAL_MEMORY += sanitizer_mem

    if settings.SAFE_HEAP:
      # SAFE_HEAP instruments ASan's shadow memory accesses.
      # Since the shadow memory starts at 0, the act of accessing the shadow memory is detected
      # by SAFE_HEAP as a null pointer dereference.
      exit_with_error('ASan does not work with SAFE_HEAP')

    if settings.LINKABLE:
      exit_with_error('ASan does not support dynamic linking')

  if sanitize and settings.GENERATE_SOURCE_MAP:
    settings.LOAD_SOURCE_MAP = 1

  if settings.GLOBAL_BASE == -1:
    # default if nothing else sets it
    # a higher global base is useful for optimizing load/store offsets, as it
    # enables the --post-emscripten pass
    settings.GLOBAL_BASE = 1024

  # various settings require malloc/free support from JS
  if settings.RELOCATABLE or \
     settings.BUILD_AS_WORKER or \
     settings.USE_WEBGPU or \
     settings.USE_PTHREADS or \
     settings.OFFSCREENCANVAS_SUPPORT or \
     settings.LEGACY_GL_EMULATION or \
     not settings.DISABLE_EXCEPTION_CATCHING or \
     settings.ASYNCIFY or \
     settings.ASMFS or \
     settings.DEMANGLE_SUPPORT or \
     settings.FORCE_FILESYSTEM or \
     settings.STB_IMAGE or \
     settings.EMBIND or \
     settings.FETCH or \
     settings.PROXY_POSIX_SOCKETS or \
     options.memory_profiler or \
     sanitize:
    settings.EXPORTED_FUNCTIONS += ['_malloc', '_free']

  if not settings.DISABLE_EXCEPTION_CATCHING:
    settings.EXPORTED_FUNCTIONS += [
      # For normal builds the entries in deps_info.py are enough to include
      # these symbols whenever __cxa_find_matching_catch_* functions are
      # found.  However, under LTO these symbols don't exist prior to linking
      # so we include then unconditionally when exceptions are enabled.
      '___cxa_is_pointer_type',
      '___cxa_can_catch',

      # Emscripten exception handling can generate invoke calls, and they call
      # setThrew(). We cannot handle this using deps_info as the invokes are not
      # emitted because of library function usage, but by codegen itself.
      '_setThrew',
    ]

  if settings.ASYNCIFY:
    if not settings.ASYNCIFY_IGNORE_INDIRECT:
      # if we are not ignoring indirect calls, then we must treat invoke_* as if
      # they are indirect calls, since that is what they do - we can't see their
      # targets statically.
      settings.ASYNCIFY_IMPORTS += ['invoke_*']
    # with pthreads we may call main through the __call_main mechanism, which can
    # therefore reach anything in the program, so mark it as possibly causing a
    # sleep (the asyncify analysis doesn't look through JS, just wasm, so it can't
    # see what it itself calls)
    if settings.USE_PTHREADS:
      settings.ASYNCIFY_IMPORTS += ['__call_main']
    # add the default imports
    settings.ASYNCIFY_IMPORTS += DEFAULT_ASYNCIFY_IMPORTS

    # return the full import name, including module. The name may
    # already have a module prefix; if not, we assume it is "env".
    def get_full_import_name(name):
      if '.' in name:
        return name
      return 'env.' + name

    settings.ASYNCIFY_IMPORTS = [get_full_import_name(i) for i in settings.ASYNCIFY_IMPORTS]

  if settings.WASM2JS and settings.GENERATE_SOURCE_MAP:
    exit_with_error('wasm2js does not support source maps yet (debug in wasm for now)')

  if settings.NODE_CODE_CACHING:
    if settings.WASM_ASYNC_COMPILATION:
      exit_with_error('NODE_CODE_CACHING requires sync compilation (WASM_ASYNC_COMPILATION=0)')
    if not shared.target_environment_may_be('node'):
      exit_with_error('NODE_CODE_CACHING only works in node, but target environments do not include it')
    if settings.SINGLE_FILE:
      exit_with_error('NODE_CODE_CACHING saves a file on the side and is not compatible with SINGLE_FILE')

  if not shared.JS.isidentifier(settings.EXPORT_NAME):
    exit_with_error(f'EXPORT_NAME is not a valid JS identifier: `{settings.EXPORT_NAME}`')

  if options.tracing and settings.ALLOW_MEMORY_GROWTH:
    settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['emscripten_trace_report_memory_layout']
    settings.EXPORTED_FUNCTIONS += ['_emscripten_stack_get_current',
                                    '_emscripten_stack_get_base',
                                    '_emscripten_stack_get_end']

  # Any "pointers" passed to JS will now be i64's, in both modes.
  if settings.MEMORY64:
    if settings_map.get('WASM_BIGINT') == '0':
      exit_with_error('MEMORY64 is not compatible with WASM_BIGINT=0')
    settings.WASM_BIGINT = 1

  # check if we can address the 2GB mark and higher: either if we start at
  # 2GB, or if we allow growth to either any amount or to 2GB or more.
  if settings.INITIAL_MEMORY > 2 * 1024 * 1024 * 1024 or \
     (settings.ALLOW_MEMORY_GROWTH and
      (settings.MAXIMUM_MEMORY < 0 or
       settings.MAXIMUM_MEMORY > 2 * 1024 * 1024 * 1024)):
    settings.CAN_ADDRESS_2GB = 1

  settings.EMSCRIPTEN_VERSION = shared.EMSCRIPTEN_VERSION
  settings.PROFILING_FUNCS = options.profiling_funcs
  settings.SOURCE_MAP_BASE = options.source_map_base or ''

  return target, wasm_target


@ToolchainProfiler.profile_block('compile inputs')
def phase_compile_inputs(options, state, newargs, input_files):
  def is_link_flag(flag):
    if flag.startswith('-nostdlib'):
      return True
    return flag.startswith(('-l', '-L', '-Wl,'))

  CXX = [shared.CLANG_CXX]
  CC = [shared.CLANG_CC]
  if config.COMPILER_WRAPPER:
    logger.debug('using compiler wrapper: %s', config.COMPILER_WRAPPER)
    CXX.insert(0, config.COMPILER_WRAPPER)
    CC.insert(0, config.COMPILER_WRAPPER)

  if 'EMMAKEN_COMPILER' in os.environ:
    diagnostics.warning('deprecated', '`EMMAKEN_COMPILER` is deprecated.\n'
                        'To use an alteranative LLVM build set `LLVM_ROOT` in the config file (or `EM_LLVM_ROOT` env var).\n'
                        'To wrap invocations of clang use the `COMPILER_WRAPPER` setting (or `EM_COMPILER_WRAPPER` env var.\n')
    CXX = [os.environ['EMMAKEN_COMPILER']]
    CC = [cxx_to_c_compiler(os.environ['EMMAKEN_COMPILER'])]

  compile_args = [a for a in newargs if a and not is_link_flag(a)]
  system_libs.ensure_sysroot()

  def get_language_mode(args):
    return_next = False
    for item in args:
      if return_next:
        return item
      if item == '-x':
        return_next = True
        continue
      if item.startswith('-x'):
        return strip_prefix(item, '-x')
    return ''

  language_mode = get_language_mode(newargs)

  def use_cxx(src):
    if 'c++' in language_mode or run_via_emxx:
      return True
    # Next consider the filename
    if src.endswith(C_ENDINGS + OBJC_ENDINGS):
      return False
    if src.endswith(CXX_ENDINGS):
      return True
    # Finally fall back to the default
    if settings.DEFAULT_TO_CXX:
      # Default to using C++ even when run as `emcc`.
      # This means that emcc will act as a C++ linker when no source files are
      # specified.
      # This differs to clang and gcc where the default is always C unless run as
      # clang++/g++.
      return True
    return False

  def get_compiler(cxx):
    if cxx:
      return CXX
    return CC

  def get_clang_command(src_file):
    return get_compiler(use_cxx(src_file)) + get_cflags(options, state.orig_args) + compile_args + [src_file]

  def get_clang_command_asm(src_file):
    return get_compiler(use_cxx(src_file)) + get_clang_flags() + compile_args + [src_file]

  # preprocessor-only (-E) support
  if state.mode == Mode.PREPROCESS_ONLY:
    for input_file in [x[1] for x in input_files]:
      cmd = get_clang_command(input_file)
      if options.output_file:
        cmd += ['-o', options.output_file]
      # Do not compile, but just output the result from preprocessing stage or
      # output the dependency rule. Warning: clang and gcc behave differently
      # with -MF! (clang seems to not recognize it)
      logger.debug(('just preprocessor ' if state.has_dash_E else 'just dependencies: ') + ' '.join(cmd))
      shared.check_call(cmd)
    return []

  # Precompiled headers support
  if state.mode == Mode.PCH:
    headers = [header for _, header in input_files]
    for header in headers:
      if not header.endswith(HEADER_ENDINGS):
        exit_with_error(f'cannot mix precompiled headers with non-header inputs: {headers} : {header}')
      cmd = get_clang_command(header)
      if options.output_file:
        cmd += ['-o', options.output_file]
      logger.debug(f"running (for precompiled headers): {cmd[0]} {' '.join(cmd[1:])}")
      shared.check_call(cmd)
      return []

  linker_inputs = []
  seen_names = {}

  def uniquename(name):
    if name not in seen_names:
      seen_names[name] = str(len(seen_names))
    return unsuffixed(name) + '_' + seen_names[name] + shared.suffix(name)

  def get_object_filename(input_file):
    if state.mode == Mode.COMPILE_ONLY:
      # In compile-only mode we don't use any temp file.  The object files
      # are written directly to their final output locations.
      if options.output_file:
        assert len(input_files) == 1
        return options.output_file
      else:
        return unsuffixed_basename(input_file) + options.default_object_extension
    else:
      return in_temp(unsuffixed(uniquename(input_file)) + options.default_object_extension)

  def compile_source_file(i, input_file):
    logger.debug('compiling source file: ' + input_file)
    output_file = get_object_filename(input_file)
    if state.mode not in (Mode.COMPILE_ONLY, Mode.PREPROCESS_ONLY):
      linker_inputs.append((i, output_file))
    if get_file_suffix(input_file) in ASSEMBLY_ENDINGS:
      cmd = get_clang_command_asm(input_file)
    else:
      cmd = get_clang_command(input_file)
    if not state.has_dash_c:
      cmd += ['-c']
    cmd += ['-o', output_file]
    if state.mode == Mode.COMPILE_AND_LINK and '-gsplit-dwarf' in newargs:
      # When running in COMPILE_AND_LINK mode we compile to temporary location
      # but we want the `.dwo` file to be generated in the current working directory,
      # like it is under clang.  We could avoid this hack if we use the clang driver
      # to generate the temporary files, but that would also involve using the clang
      # driver to perform linking which would be big change.
      cmd += ['-Xclang', '-split-dwarf-file', '-Xclang', unsuffixed_basename(input_file) + '.dwo']
      cmd += ['-Xclang', '-split-dwarf-output', '-Xclang', unsuffixed_basename(input_file) + '.dwo']
    shared.check_call(cmd)
    if output_file not in ('-', os.devnull):
      assert os.path.exists(output_file)

  # First, generate LLVM bitcode. For each input file, we get base.o with bitcode
  for i, input_file in input_files:
    file_suffix = get_file_suffix(input_file)
    if file_suffix in SOURCE_ENDINGS + ASSEMBLY_ENDINGS or (state.has_dash_c and file_suffix == '.bc'):
      compile_source_file(i, input_file)
    elif file_suffix in DYNAMICLIB_ENDINGS:
      logger.debug('using shared library: ' + input_file)
      linker_inputs.append((i, input_file))
    elif building.is_ar(input_file):
      logger.debug('using static library: ' + input_file)
      ensure_archive_index(input_file)
      linker_inputs.append((i, input_file))
    elif language_mode:
      compile_source_file(i, input_file)
    elif input_file == '-':
      exit_with_error('-E or -x required when input is from standard input')
    else:
      # Default to assuming the inputs are object files and pass them to the linker
      logger.debug('using object file: ' + input_file)
      linker_inputs.append((i, input_file))

  return linker_inputs


@ToolchainProfiler.profile_block('calculate system libraries')
def phase_calculate_system_libraries(state, linker_arguments, linker_inputs, newargs):
  extra_files_to_link = []
  # link in ports and system libraries, if necessary
  if not settings.SIDE_MODULE:
    # Ports are always linked into the main module, never the size module.
    extra_files_to_link += system_libs.get_ports_libs(settings)
  if '-nostdlib' not in newargs and '-nodefaultlibs' not in newargs:
    settings.LINK_AS_CXX = run_via_emxx
    # Traditionally we always link as C++.  For compatibility we continue to do that,
    # unless running in strict mode.
    if not settings.STRICT and '-nostdlib++' not in newargs:
      settings.LINK_AS_CXX = True
    extra_files_to_link += system_libs.calculate([f for _, f in sorted(linker_inputs)] + extra_files_to_link, forced=state.forced_stdlibs)
  linker_arguments.extend(extra_files_to_link)


@ToolchainProfiler.profile_block('link')
def phase_link(linker_arguments, wasm_target):
  logger.debug(f'linking: {linker_arguments}')

  # Make a final pass over settings.EXPORTED_FUNCTIONS to remove any
  # duplication between functions added by the driver/libraries and function
  # specified by the user
  settings.EXPORTED_FUNCTIONS = dedup_list(settings.EXPORTED_FUNCTIONS)

  # if  EMCC_DEBUG=2  then we must link now, so the temp files are complete.
  # if using the wasm backend, we might be using vanilla LLVM, which does not allow our
  # fastcomp deferred linking opts.
  # TODO: we could check if this is a fastcomp build, and still speed things up here
  js_syms = None
  if settings.LLD_REPORT_UNDEFINED and settings.ERROR_ON_UNDEFINED_SYMBOLS:
    js_syms = get_all_js_syms()
  building.link_lld(linker_arguments, wasm_target, external_symbols=js_syms)


@ToolchainProfiler.profile_block('post_link')
def phase_post_link(options, state, in_wasm, wasm_target, target):
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

  if settings.MEM_INIT_IN_WASM:
    memfile = None
  else:
    memfile = shared.replace_or_append_suffix(target, '.mem')

  phase_emscript(options, in_wasm, wasm_target, memfile)

  phase_source_transforms(options, target)

  if memfile and not settings.MINIMAL_RUNTIME:
    # MINIMAL_RUNTIME doesn't use `var memoryInitializer` but instead expects Module['mem'] to
    # be loaded before the module.  See src/postamble_minimal.js.
    phase_memory_initializer(memfile)

  phase_binaryen(target, options, wasm_target)

  # If we are not emitting any JS then we are all done now
  if options.oformat != OFormat.WASM:
    phase_final_emitting(options, state, target, wasm_target, memfile)


@ToolchainProfiler.profile_block('emscript')
def phase_emscript(options, in_wasm, wasm_target, memfile):
  # Emscripten
  logger.debug('emscript')
  if options.memory_init_file:
    settings.MEM_INIT_METHOD = 1
  else:
    assert settings.MEM_INIT_METHOD != 1

  if embed_memfile():
    settings.SUPPORT_BASE64_EMBEDDING = 1

  emscripten.run(in_wasm, wasm_target, final_js, memfile)
  save_intermediate('original')


@ToolchainProfiler.profile_block('source transforms')
def phase_source_transforms(options, target):
  global final_js

  # Embed and preload files
  if len(options.preload_files) or len(options.embed_files):
    logger.debug('setting up files')
    file_args = ['--from-emcc', '--export-name=' + settings.EXPORT_NAME]
    if len(options.preload_files):
      file_args.append('--preload')
      file_args += options.preload_files
    if len(options.embed_files):
      file_args.append('--embed')
      file_args += options.embed_files
    if len(options.exclude_files):
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
    file_code = shared.check_call([shared.FILE_PACKAGER, unsuffixed(target) + '.data'] + file_args, stdout=PIPE).stdout
    options.pre_js = js_manipulation.add_files_pre_js(options.pre_js, file_code)

  # Apply pre and postjs files
  if final_js and (options.pre_js or options.post_js):
    logger.debug('applying pre/postjses')
    src = read_file(final_js)
    final_js += '.pp.js'
    with open(final_js, 'w') as f:
      # pre-js code goes right after the Module integration code (so it
      # can use Module), we have a marker for it
      f.write(do_replace(src, '// {{PRE_JSES}}', fix_windows_newlines(options.pre_js)))
      f.write(fix_windows_newlines(options.post_js))
    options.pre_js = src = options.post_js = None
    save_intermediate('pre-post')

  # Apply a source code transformation, if requested
  if options.js_transform:
    safe_copy(final_js, final_js + '.tr.js')
    final_js += '.tr.js'
    posix = not shared.WINDOWS
    logger.debug('applying transform: %s', options.js_transform)
    shared.check_call(building.remove_quotes(shlex.split(options.js_transform, posix=posix) + [os.path.abspath(final_js)]))
    save_intermediate('transformed')


@ToolchainProfiler.profile_block('memory initializer')
def phase_memory_initializer(memfile):
  # For the wasm backend, we don't have any memory info in JS. All we need to do
  # is set the memory initializer url.
  global final_js

  src = read_file(final_js)
  src = do_replace(src, '// {{MEM_INITIALIZER}}', 'var memoryInitializer = "%s";' % os.path.basename(memfile))
  write_file(final_js + '.mem.js', src)
  final_js += '.mem.js'


@ToolchainProfiler.profile_block('final emitting')
def phase_final_emitting(options, state, target, wasm_target, memfile):
  global final_js

  # Remove some trivial whitespace
  # TODO: do not run when compress has already been done on all parts of the code
  # src = read_file(final_js)
  # src = re.sub(r'\n+[ \n]*\n+', '\n', src)
  # write_file(final_js, src)

  if settings.USE_PTHREADS:
    target_dir = os.path.dirname(os.path.abspath(target))
    worker_output = os.path.join(target_dir, settings.PTHREAD_WORKER_FILE)
    with open(worker_output, 'w') as f:
      f.write(shared.read_and_preprocess(shared.path_from_root('src', 'worker.js'), expand_macros=True))

    # Minify the worker.js file in optimized builds
    if (settings.OPT_LEVEL >= 1 or settings.SHRINK_LEVEL >= 1) and not settings.DEBUG_LEVEL:
      minified_worker = building.acorn_optimizer(worker_output, ['minifyWhitespace'], return_output=True)
      write_file(worker_output, minified_worker)

  # track files that will need native eols
  generated_text_files_with_native_eols = []

  if settings.MODULARIZE:
    modularize()

  module_export_name_substitution()

  # Run a final regex pass to clean up items that were not possible to optimize by Closure, or unoptimalities that were left behind
  # by processing steps that occurred after Closure.
  if settings.MINIMAL_RUNTIME == 2 and settings.USE_CLOSURE_COMPILER and settings.DEBUG_LEVEL == 0 and not settings.SINGLE_FILE:
    # Process .js runtime file. Note that we need to handle the license text
    # here, so that it will not confuse the hacky script.
    shared.JS.handle_license(final_js)
    shared.run_process([shared.PYTHON, shared.path_from_root('tools', 'hacky_postprocess_around_closure_limitations.py'), final_js])

  # Unmangle previously mangled `import.meta` references in both main code and libraries.
  # See also: `preprocess` in parseTools.js.
  if settings.EXPORT_ES6 and settings.USE_ES6_IMPORT_META:
    src = read_file(final_js)
    final_js += '.esmeta.js'
    write_file(final_js, src.replace('EMSCRIPTEN$IMPORT$META', 'import.meta'))
    save_intermediate('es6-import-meta')

  # Apply pre and postjs files
  if options.extern_pre_js or options.extern_post_js:
    logger.debug('applying extern pre/postjses')
    src = read_file(final_js)
    final_js += '.epp.js'
    with open(final_js, 'w') as f:
      f.write(fix_windows_newlines(options.extern_pre_js))
      f.write(src)
      f.write(fix_windows_newlines(options.extern_post_js))
    save_intermediate('extern-pre-post')

  shared.JS.handle_license(final_js)

  js_target = state.js_target

  # The JS is now final. Move it to its final location
  move_file(final_js, js_target)

  if not settings.SINGLE_FILE:
    generated_text_files_with_native_eols += [js_target]

  target_basename = unsuffixed_basename(target)

  # If we were asked to also generate HTML, do that
  if options.oformat == OFormat.HTML:
    generate_html(target, options, js_target, target_basename,
                  wasm_target, memfile)
  elif settings.PROXY_TO_WORKER:
    generate_worker_js(target, js_target, target_basename)

  if embed_memfile() and memfile:
    shared.try_delete(memfile)

  if settings.SPLIT_MODULE:
    diagnostics.warning('experimental', 'The SPLIT_MODULE setting is experimental and subject to change')
    do_split_module(wasm_target)

  for f in generated_text_files_with_native_eols:
    tools.line_endings.convert_line_endings_in_file(f, os.linesep, options.output_eol)

  if options.executable:
    make_js_executable(js_target)


def version_string():
  # if the emscripten folder is not a git repo, don't run git show - that can
  # look up and find the revision in a parent directory that is a git repo
  revision_suffix = ''
  if os.path.exists(shared.path_from_root('.git')):
    git_rev = run_process(
      ['git', 'rev-parse', 'HEAD'],
      stdout=PIPE, stderr=PIPE, cwd=shared.path_from_root()).stdout.strip()
    revision_suffix = '-git (%s)' % git_rev
  elif os.path.exists(shared.path_from_root('emscripten-revision.txt')):
    with open(shared.path_from_root('emscripten-revision.txt')) as f:
      git_rev = f.read().strip()
    revision_suffix = ' (%s)' % git_rev
  return f'emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) {shared.EMSCRIPTEN_VERSION}{revision_suffix}'


def parse_args(newargs):
  options = EmccOptions()
  settings_changes = []
  user_js_defines = []
  should_exit = False
  eh_enabled = False
  wasm_eh_enabled = False
  skip = False

  for i in range(len(newargs)):
    if skip:
      skip = False
      continue

    # On Windows Vista (and possibly others), excessive spaces in the command line
    # leak into the items in this array, so trim e.g. 'foo.cpp ' -> 'foo.cpp'
    newargs[i] = newargs[i].strip()
    arg = newargs[i]
    arg_value = None

    def check_flag(value):
      # Check for and consume a flag
      if arg == value:
        newargs[i] = ''
        return True
      return False

    def check_arg(name):
      nonlocal arg_value
      if arg.startswith(name) and '=' in arg:
        arg_value = arg.split('=', 1)[1]
        newargs[i] = ''
        return True
      if arg == name:
        if len(newargs) <= i + 1:
          exit_with_error("option '%s' requires an argument" % arg)
        arg_value = newargs[i + 1]
        newargs[i] = ''
        newargs[i + 1] = ''
        return True
      return False

    def consume_arg():
      nonlocal arg_value
      assert arg_value is not None
      rtn = arg_value
      arg_value = None
      return rtn

    def consume_arg_file():
      name = consume_arg()
      if not os.path.isfile(name):
        exit_with_error("'%s': file not found: '%s'" % (arg, name))
      return name

    if arg.startswith('-O'):
      # Let -O default to -O2, which is what gcc does.
      options.requested_level = strip_prefix(arg, '-O') or '2'
      if options.requested_level == 's':
        options.requested_level = 2
        settings.SHRINK_LEVEL = 1
        settings_changes.append('INLINING_LIMIT=1')
      elif options.requested_level == 'z':
        options.requested_level = 2
        settings.SHRINK_LEVEL = 2
        settings_changes.append('INLINING_LIMIT=1')
      settings.OPT_LEVEL = validate_arg_level(options.requested_level, 3, 'Invalid optimization level: ' + arg, clamp=True)
    elif check_arg('--js-opts'):
      logger.warning('--js-opts ignored when using llvm backend')
      consume_arg()
    elif check_arg('--llvm-opts'):
      diagnostics.warning('deprecated', '--llvm-opts is deprecated.  All non-emcc args are passed through to clang.')
    elif arg.startswith('-flto'):
      if '=' in arg:
        settings.LTO = arg.split('=')[1]
      else:
        settings.LTO = "full"
    elif check_arg('--llvm-lto'):
      logger.warning('--llvm-lto ignored when using llvm backend')
      consume_arg()
    elif check_arg('--closure-args'):
      args = consume_arg()
      options.closure_args += shlex.split(args)
    elif check_arg('--closure'):
      options.use_closure_compiler = int(consume_arg())
    elif check_arg('--js-transform'):
      options.js_transform = consume_arg()
    elif check_arg('--pre-js'):
      options.pre_js += read_file(consume_arg_file()) + '\n'
    elif check_arg('--post-js'):
      options.post_js += read_file(consume_arg_file()) + '\n'
    elif check_arg('--extern-pre-js'):
      options.extern_pre_js += read_file(consume_arg_file()) + '\n'
    elif check_arg('--extern-post-js'):
      options.extern_post_js += read_file(consume_arg_file()) + '\n'
    elif check_arg('--compiler-wrapper'):
      config.COMPILER_WRAPPER = consume_arg()
    elif check_flag('--post-link'):
      options.post_link = True
    elif check_arg('--oformat'):
      formats = [f.lower() for f in OFormat.__members__]
      fmt = consume_arg()
      if fmt not in formats:
        exit_with_error('invalid output format: `%s` (must be one of %s)' % (fmt, formats))
      options.oformat = getattr(OFormat, fmt.upper())
    elif check_arg('--minify'):
      arg = consume_arg()
      if arg != '0':
        exit_with_error('0 is the only supported option for --minify; 1 has been deprecated')
      settings.DEBUG_LEVEL = max(1, settings.DEBUG_LEVEL)
    elif arg.startswith('-g'):
      options.requested_debug = arg
      requested_level = strip_prefix(arg, '-g') or '3'
      if is_int(requested_level):
        # the -gX value is the debug level (-g1, -g2, etc.)
        settings.DEBUG_LEVEL = validate_arg_level(requested_level, 4, 'Invalid debug level: ' + arg)
        # if we don't need to preserve LLVM debug info, do not keep this flag
        # for clang
        if settings.DEBUG_LEVEL < 3:
          newargs[i] = ''
        else:
          # for 3+, report -g to clang as -g4 etc. are not accepted
          newargs[i] = '-g'
          if settings.DEBUG_LEVEL == 4:
            settings.GENERATE_SOURCE_MAP = 1
            diagnostics.warning('deprecated', 'please replace -g4 with -gsource-map')
      else:
        if requested_level.startswith('force_dwarf'):
          exit_with_error('gforce_dwarf was a temporary option and is no longer necessary (use -g)')
        elif requested_level.startswith('separate-dwarf'):
          # emit full DWARF but also emit it in a file on the side
          newargs[i] = '-g'
          # if a file is provided, use that; otherwise use the default location
          # (note that we do not know the default location until all args have
          # been parsed, so just note True for now).
          if requested_level != 'separate-dwarf':
            if not requested_level.startswith('separate-dwarf=') or requested_level.count('=') != 1:
              exit_with_error('invalid -gseparate-dwarf=FILENAME notation')
            settings.SEPARATE_DWARF = requested_level.split('=')[1]
          else:
            settings.SEPARATE_DWARF = True
        elif requested_level == 'source-map':
          settings.GENERATE_SOURCE_MAP = 1
          newargs[i] = '-g'
        # a non-integer level can be something like -gline-tables-only. keep
        # the flag for the clang frontend to emit the appropriate DWARF info.
        # set the emscripten debug level to 3 so that we do not remove that
        # debug info during link (during compile, this does not make a
        # difference).
        settings.DEBUG_LEVEL = 3
    elif check_flag('-profiling') or check_flag('--profiling'):
      settings.DEBUG_LEVEL = max(settings.DEBUG_LEVEL, 2)
    elif check_flag('-profiling-funcs') or check_flag('--profiling-funcs'):
      options.profiling_funcs = True
    elif newargs[i] == '--tracing' or newargs[i] == '--memoryprofiler':
      if newargs[i] == '--memoryprofiler':
        options.memory_profiler = True
      options.tracing = True
      newargs[i] = ''
      settings_changes.append("EMSCRIPTEN_TRACING=1")
      settings.JS_LIBRARIES.append((0, 'library_trace.js'))
    elif check_flag('--emit-symbol-map'):
      options.emit_symbol_map = True
      settings.EMIT_SYMBOL_MAP = 1
    elif check_flag('--bind'):
      settings.EMBIND = 1
      settings.JS_LIBRARIES.append((0, os.path.join('embind', 'emval.js')))
      settings.JS_LIBRARIES.append((0, os.path.join('embind', 'embind.js')))
    elif check_arg('--embed-file'):
      options.embed_files.append(consume_arg())
    elif check_arg('--preload-file'):
      options.preload_files.append(consume_arg())
    elif check_arg('--exclude-file'):
      options.exclude_files.append(consume_arg())
    elif check_flag('--use-preload-cache'):
      options.use_preload_cache = True
    elif check_flag('--no-heap-copy'):
      diagnostics.warning('legacy-settings', 'ignoring legacy flag --no-heap-copy (that is the only mode supported now)')
    elif check_flag('--use-preload-plugins'):
      options.use_preload_plugins = True
    elif check_flag('--ignore-dynamic-linking'):
      options.ignore_dynamic_linking = True
    elif arg == '-v':
      shared.PRINT_STAGES = True
    elif check_arg('--shell-file'):
      options.shell_path = consume_arg_file()
    elif check_arg('--source-map-base'):
      options.source_map_base = consume_arg()
    elif check_flag('--no-entry'):
      options.no_entry = True
    elif check_arg('--js-library'):
      settings.JS_LIBRARIES.append((i + 1, os.path.abspath(consume_arg_file())))
    elif check_flag('--remove-duplicates'):
      diagnostics.warning('legacy-settings', '--remove-duplicates is deprecated as it is no longer needed. If you cannot link without it, file a bug with a testcase')
    elif check_flag('--jcache'):
      logger.error('jcache is no longer supported')
    elif check_arg('--cache'):
      config.CACHE = os.path.normpath(consume_arg())
      shared.reconfigure_cache()
    elif check_flag('--clear-cache'):
      logger.info('clearing cache as requested by --clear-cache: `%s`', shared.Cache.dirname)
      shared.Cache.erase()
      shared.check_sanity(force=True) # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--clear-ports'):
      logger.info('clearing ports and cache as requested by --clear-ports')
      system_libs.Ports.erase()
      shared.Cache.erase()
      shared.check_sanity(force=True) # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--check'):
      print(version_string(), file=sys.stderr)
      shared.check_sanity(force=True)
      should_exit = True
    elif check_flag('--show-ports'):
      system_libs.show_ports()
      should_exit = True
    elif check_arg('--memory-init-file'):
      options.memory_init_file = int(consume_arg())
    elif check_flag('--proxy-to-worker'):
      settings_changes.append('PROXY_TO_WORKER=1')
    elif check_arg('--valid-abspath'):
      options.valid_abspaths.append(consume_arg())
    elif check_flag('--separate-asm'):
      exit_with_error('cannot --separate-asm with the wasm backend, since not emitting asm.js')
    elif arg.startswith(('-I', '-L')):
      path_name = arg[2:]
      if os.path.isabs(path_name) and not is_valid_abspath(options, path_name):
        # Of course an absolute path to a non-system-specific library or header
        # is fine, and you can ignore this warning. The danger are system headers
        # that are e.g. x86 specific and non-portable. The emscripten bundled
        # headers are modified to be portable, local system ones are generally not.
        diagnostics.warning(
            'absolute-paths', f'-I or -L of an absolute path "{arg}" '
            'encountered. If this is to a local system header/library, it may '
            'cause problems (local system files make sense for compiling natively '
            'on your system, but not necessarily to JavaScript).')
    elif check_flag('--emrun'):
      options.emrun = True
    elif check_flag('--cpuprofiler'):
      options.cpu_profiler = True
    elif check_flag('--threadprofiler'):
      options.thread_profiler = True
      settings_changes.append('PTHREADS_PROFILING=1')
    elif arg == '-fno-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
      settings.DISABLE_EXCEPTION_THROWING = 1
      settings.EXCEPTION_HANDLING = 0
    elif arg == '-fexceptions':
      eh_enabled = True
    elif arg == '-fwasm-exceptions':
      wasm_eh_enabled = True
    elif arg == '-fignore-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
    elif check_arg('--default-obj-ext'):
      options.default_object_extension = consume_arg()
      if not options.default_object_extension.startswith('.'):
        options.default_object_extension = '.' + options.default_object_extension
    elif arg == '-fsanitize=cfi':
      options.cfi = True
    elif check_arg('--output_eol'):
      style = consume_arg()
      if style.lower() == 'windows':
        options.output_eol = '\r\n'
      elif style.lower() == 'linux':
        options.output_eol = '\n'
      else:
        exit_with_error(f'Invalid value "{style}" to --output_eol!')
    elif check_arg('--generate-config'):
      optarg = consume_arg()
      path = os.path.expanduser(optarg)
      if os.path.exists(path):
        exit_with_error(f'File {optarg} passed to --generate-config already exists!')
      else:
        config.generate_config(optarg)
      should_exit = True
    # Record USE_PTHREADS setting because it controls whether --shared-memory is passed to lld
    elif arg == '-pthread':
      settings_changes.append('USE_PTHREADS=1')
    elif arg in ('-fno-diagnostics-color', '-fdiagnostics-color=never'):
      colored_logger.disable()
      diagnostics.color_enabled = False
    elif arg == '-fno-rtti':
      settings.USE_RTTI = 0
    elif arg == '-frtti':
      settings.USE_RTTI = 1
    elif arg.startswith('-jsD'):
      key = strip_prefix(arg, '-jsD')
      if '=' in key:
        key, value = key.split('=')
      else:
        value = '1'
      if key in settings.keys():
        exit_with_error(f'{arg}: cannot change built-in settings values with a -jsD directive. Pass -s {key}={value} instead!')
      user_js_defines += [(key, value)]
      newargs[i] = ''
    elif check_flag('-shared'):
      options.shared = True
    elif check_flag('-r'):
      options.relocatable = True
    elif check_arg('-o'):
      options.output_file = consume_arg()
    elif arg.startswith('-o'):
      options.output_file = strip_prefix(arg, '-o')
      newargs[i] = ''
    elif arg == '-mllvm':
      # Ignore the next argument rather than trying to parse it.  This is needed
      # because llvm args could, for example, start with `-o` and we don't want
      # to confuse that with a normal `-o` flag.
      skip = True

  if should_exit:
    sys.exit(0)

  # TODO Currently -fexceptions only means Emscripten EH. Switch to wasm
  # exception handling by default when -fexceptions is given when wasm
  # exception handling becomes stable.
  if wasm_eh_enabled:
    settings.EXCEPTION_HANDLING = 1
    settings.DISABLE_EXCEPTION_THROWING = 1
    settings.DISABLE_EXCEPTION_CATCHING = 1
  elif eh_enabled:
    settings.EXCEPTION_HANDLING = 0
    settings.DISABLE_EXCEPTION_THROWING = 0
    settings.DISABLE_EXCEPTION_CATCHING = 0

  newargs = [a for a in newargs if a]
  return options, settings_changes, user_js_defines, newargs


@ToolchainProfiler.profile_block('binaryen')
def phase_binaryen(target, options, wasm_target):
  global final_js
  logger.debug('using binaryen')
  if settings.GENERATE_SOURCE_MAP and not settings.SOURCE_MAP_BASE:
    logger.warning("Wasm source map won't be usable in a browser without --source-map-base")
  # whether we need to emit -g (function name debug info) in the final wasm
  debug_info = settings.DEBUG_LEVEL >= 2 or options.profiling_funcs
  # whether we need to emit -g in the intermediate binaryen invocations (but not
  # necessarily at the very end). this is necessary if we depend on debug info
  # during compilation, even if we do not emit it at the end.
  # we track the number of causes for needing intermdiate debug info so
  # that we can stop emitting it when possible - in particular, that is
  # important so that we stop emitting it before the end, and it is not in the
  # final binary (if it shouldn't be)
  intermediate_debug_info = 0
  if debug_info:
    intermediate_debug_info += 1
  if options.emit_symbol_map:
    intermediate_debug_info += 1
  if settings.ASYNCIFY:
    intermediate_debug_info += 1
  # note that wasm-ld can strip DWARF info for us too (--strip-debug), but it
  # also strips the Names section. so to emit just the Names section we don't
  # tell wasm-ld to strip anything, and we do it here.
  strip_debug = settings.DEBUG_LEVEL < 3
  strip_producers = not settings.EMIT_PRODUCERS_SECTION
  # run wasm-opt if we have work for it: either passes, or if we are using
  # source maps (which requires some extra processing to keep the source map
  # but remove DWARF)
  passes = get_binaryen_passes()
  if passes or settings.GENERATE_SOURCE_MAP:
    # if we need to strip certain sections, and we have wasm-opt passes
    # to run anyhow, do it with them.
    if strip_debug:
      passes += ['--strip-debug']
    if strip_producers:
      passes += ['--strip-producers']
    building.save_intermediate(wasm_target, 'pre-byn.wasm')
    # if asyncify is used, we will use it in the next stage, and so if it is
    # the only reason we need intermediate debug info, we can stop keeping it
    if settings.ASYNCIFY:
      intermediate_debug_info -= 1
    building.run_wasm_opt(wasm_target,
                          wasm_target,
                          args=passes,
                          debug=intermediate_debug_info)
  elif strip_debug or strip_producers:
    # we are not running wasm-opt. if we need to strip certain sections
    # then do so using llvm-objcopy which is fast and does not rewrite the
    # code (which is better for debug info)
    building.save_intermediate(wasm_target, 'pre-strip.wasm')
    building.strip(wasm_target, wasm_target, debug=strip_debug, producers=strip_producers)

  if settings.EVAL_CTORS:
    building.save_intermediate(wasm_target, 'pre-ctors.wasm')
    building.eval_ctors(final_js, wasm_target, debug_info=intermediate_debug_info)

  # after generating the wasm, do some final operations

  if settings.EMIT_EMSCRIPTEN_METADATA:
    diagnostics.warning('deprecated', 'We hope to remove support for EMIT_EMSCRIPTEN_METADATA. See https://github.com/emscripten-core/emscripten/issues/12231')
    webassembly.add_emscripten_metadata(wasm_target)

  if final_js:
    if settings.SUPPORT_BIG_ENDIAN:
      final_js = building.little_endian_heap(final_js)

    # >=2GB heap support requires pointers in JS to be unsigned. rather than
    # require all pointers to be unsigned by default, which increases code size
    # a little, keep them signed, and just unsign them here if we need that.
    if settings.CAN_ADDRESS_2GB:
      final_js = building.use_unsigned_pointers_in_js(final_js)

    # pthreads memory growth requires some additional JS fixups.
    # note that we must do this after handling of unsigned pointers. unsigning
    # adds some >>> 0 things, while growth will replace a HEAP8 with a call to
    # a method to get the heap, and that call would not be recognized by the
    # unsigning pass
    if settings.USE_PTHREADS and settings.ALLOW_MEMORY_GROWTH:
      final_js = building.apply_wasm_memory_growth(final_js)

    if settings.USE_ASAN:
      final_js = building.instrument_js_for_asan(final_js)

    if settings.SAFE_HEAP:
      final_js = building.instrument_js_for_safe_heap(final_js)

    if settings.OPT_LEVEL >= 2 and settings.DEBUG_LEVEL <= 2:
      # minify the JS. Do not minify whitespace if Closure is used, so that
      # Closure can print out readable error messages (Closure will then
      # minify whitespace afterwards)
      save_intermediate_with_wasm('preclean', wasm_target)
      final_js = building.minify_wasm_js(js_file=final_js,
                                         wasm_file=wasm_target,
                                         expensive_optimizations=will_metadce(),
                                         minify_whitespace=minify_whitespace() and not options.use_closure_compiler,
                                         debug_info=intermediate_debug_info)
      save_intermediate_with_wasm('postclean', wasm_target)

  if settings.ASYNCIFY_LAZY_LOAD_CODE:
    building.asyncify_lazy_load_code(wasm_target, debug=intermediate_debug_info)

  def preprocess_wasm2js_script():
    return read_and_preprocess(shared.path_from_root('src', 'wasm2js.js'), expand_macros=True)

  def run_closure_compiler():
    global final_js
    final_js = building.closure_compiler(final_js, pretty=not minify_whitespace(),
                                         extra_closure_args=options.closure_args)
    save_intermediate_with_wasm('closure', wasm_target)

  if final_js and options.use_closure_compiler:
    run_closure_compiler()

  symbols_file = None
  if options.emit_symbol_map:
    symbols_file = shared.replace_or_append_suffix(target, '.symbols')

  if settings.WASM2JS:
    symbols_file_js = None
    if settings.WASM == 2:
      wasm2js_template = wasm_target + '.js'
      with open(wasm2js_template, 'w') as f:
        f.write(preprocess_wasm2js_script())
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
                               minify_whitespace=minify_whitespace(),
                               use_closure_compiler=options.use_closure_compiler,
                               debug_info=debug_info,
                               symbols_file=symbols_file,
                               symbols_file_js=symbols_file_js)

    shared.configuration.get_temp_files().note(wasm2js)

    if settings.WASM == 2:
      safe_copy(wasm2js, wasm2js_template)

    if settings.WASM != 2:
      final_js = wasm2js
      # if we only target JS, we don't need the wasm any more
      shared.try_delete(wasm_target)

    save_intermediate('wasm2js')

  # emit the final symbols, either in the binary or in a symbol map.
  # this will also remove debug info if we only kept it around in the intermediate invocations.
  # note that if we aren't emitting a binary (like in wasm2js) then we don't
  # have anything to do here.
  if options.emit_symbol_map:
    intermediate_debug_info -= 1
    if os.path.exists(wasm_target):
      building.handle_final_wasm_symbols(wasm_file=wasm_target, symbols_file=symbols_file, debug_info=intermediate_debug_info)
      save_intermediate_with_wasm('symbolmap', wasm_target)

  if settings.DEBUG_LEVEL >= 3 and settings.SEPARATE_DWARF and os.path.exists(wasm_target):
    building.emit_debug_on_side(wasm_target, settings.SEPARATE_DWARF)

  if settings.WASM2C:
    wasm2c.do_wasm2c(wasm_target)

  # we have finished emitting the wasm, and so intermediate debug info will
  # definitely no longer be used tracking it.
  if debug_info:
    intermediate_debug_info -= 1
  assert intermediate_debug_info == 0
  # strip debug info if it was not already stripped by the last command
  if not debug_info and building.binaryen_kept_debug_info and \
     building.os.path.exists(wasm_target):
    building.run_wasm_opt(wasm_target, wasm_target)

  # replace placeholder strings with correct subresource locations
  if final_js and settings.SINGLE_FILE and not settings.WASM2JS:
    js = read_file(final_js)

    if settings.MINIMAL_RUNTIME:
      js = do_replace(js, '<<< WASM_BINARY_DATA >>>', base64_encode(read_binary(wasm_target)))
    else:
      js = do_replace(js, '<<< WASM_BINARY_FILE >>>', shared.JS.get_subresource_location(wasm_target))
    shared.try_delete(wasm_target)
    with open(final_js, 'w') as f:
      f.write(js)


def modularize():
  global final_js
  logger.debug(f'Modularizing, assigning to var {settings.EXPORT_NAME}')
  src = read_file(final_js)

  return_value = settings.EXPORT_NAME
  if settings.WASM_ASYNC_COMPILATION:
    return_value += '.ready'
  if not settings.EXPORT_READY_PROMISE:
    return_value = '{}'

  src = '''
function(%(EXPORT_NAME)s) {
  %(EXPORT_NAME)s = %(EXPORT_NAME)s || {};

%(src)s

  return %(return_value)s
}
''' % {
    'EXPORT_NAME': settings.EXPORT_NAME,
    'src': src,
    'return_value': return_value
  }

  if settings.MINIMAL_RUNTIME and not settings.USE_PTHREADS:
    # Single threaded MINIMAL_RUNTIME programs do not need access to
    # document.currentScript, so a simple export declaration is enough.
    src = 'var %s=%s' % (settings.EXPORT_NAME, src)
  else:
    script_url_node = ""
    # When MODULARIZE this JS may be executed later,
    # after document.currentScript is gone, so we save it.
    # In EXPORT_ES6 + USE_PTHREADS the 'thread' is actually an ES6 module webworker running in strict mode,
    # so doesn't have access to 'document'. In this case use 'import.meta' instead.
    if settings.EXPORT_ES6 and settings.USE_ES6_IMPORT_META:
      script_url = "import.meta.url"
    else:
      script_url = "typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined"
      if shared.target_environment_may_be('node'):
        script_url_node = "if (typeof __filename !== 'undefined') _scriptDir = _scriptDir || __filename;"
    src = '''
var %(EXPORT_NAME)s = (function() {
  var _scriptDir = %(script_url)s;
  %(script_url_node)s
  return (%(src)s);
})();
''' % {
      'EXPORT_NAME': settings.EXPORT_NAME,
      'script_url': script_url,
      'script_url_node': script_url_node,
      'src': src
    }

  final_js += '.modular.js'
  with open(final_js, 'w') as f:
    f.write(src)

    # Export using a UMD style export, or ES6 exports if selected

    if settings.EXPORT_ES6:
      f.write('export default %s;' % settings.EXPORT_NAME)
    elif not settings.MINIMAL_RUNTIME:
      f.write('''\
if (typeof exports === 'object' && typeof module === 'object')
  module.exports = %(EXPORT_NAME)s;
else if (typeof define === 'function' && define['amd'])
  define([], function() { return %(EXPORT_NAME)s; });
else if (typeof exports === 'object')
  exports["%(EXPORT_NAME)s"] = %(EXPORT_NAME)s;
''' % {'EXPORT_NAME': settings.EXPORT_NAME})

  shared.configuration.get_temp_files().note(final_js)
  save_intermediate('modularized')


def module_export_name_substitution():
  global final_js
  logger.debug(f'Private module export name substitution with {settings.EXPORT_NAME}')
  with open(final_js) as f:
    src = f.read()
  final_js += '.module_export_name_substitution.js'
  if settings.MINIMAL_RUNTIME:
    # In MINIMAL_RUNTIME the Module object is always present to provide the .asm.js/.wasm content
    replacement = settings.EXPORT_NAME
  else:
    replacement = "typeof %(EXPORT_NAME)s !== 'undefined' ? %(EXPORT_NAME)s : {}" % {"EXPORT_NAME": settings.EXPORT_NAME}
  src = re.sub(r'{\s*[\'"]?__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__[\'"]?:\s*1\s*}', replacement, src)
  # For Node.js and other shell environments, create an unminified Module object so that
  # loading external .asm.js file that assigns to Module['asm'] works even when Closure is used.
  if settings.MINIMAL_RUNTIME and (shared.target_environment_may_be('node') or shared.target_environment_may_be('shell')):
    src = 'if(typeof Module==="undefined"){var Module={};}\n' + src
  with open(final_js, 'w') as f:
    f.write(src)
  shared.configuration.get_temp_files().note(final_js)
  save_intermediate('module_export_name_substitution')


def generate_traditional_runtime_html(target, options, js_target, target_basename,
                                      wasm_target, memfile):
  script = ScriptSource()

  shell = read_and_preprocess(options.shell_path)
  assert '{{{ SCRIPT }}}' in shell, 'HTML shell must contain  {{{ SCRIPT }}}  , see src/shell.html for an example'
  base_js_target = os.path.basename(js_target)

  if settings.PROXY_TO_WORKER:
    proxy_worker_filename = (settings.PROXY_TO_WORKER_FILENAME or target_basename) + '.js'
    worker_js = worker_js_script(proxy_worker_filename)
    script.inline = ('''
  var filename = '%s';
  if ((',' + window.location.search.substr(1) + ',').indexOf(',noProxy,') < 0) {
    console.log('running code in a web worker');
''' % shared.JS.get_subresource_location(proxy_worker_filename)) + worker_js + '''
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
  else:
    # Normal code generation path
    script.src = base_js_target

  if not settings.SINGLE_FILE:
    if memfile and not settings.MINIMAL_RUNTIME:
      # start to load the memory init file in the HTML, in parallel with the JS
      script.un_src()
      script.inline = ('''
          var memoryInitializer = '%s';
          memoryInitializer = Module['locateFile'] ? Module['locateFile'](memoryInitializer, '') : memoryInitializer;
          Module['memoryInitializerRequestURL'] = memoryInitializer;
          var meminitXHR = Module['memoryInitializerRequest'] = new XMLHttpRequest();
          meminitXHR.open('GET', memoryInitializer, true);
          meminitXHR.responseType = 'arraybuffer';
          meminitXHR.send(null);
''' % shared.JS.get_subresource_location(memfile)) + script.inline

    if not settings.WASM_ASYNC_COMPILATION:
      # We need to load the wasm file before anything else, it has to be synchronously ready TODO: optimize
      script.un_src()
      script.inline = '''
          var wasmURL = '%s';
          var wasmXHR = new XMLHttpRequest();
          wasmXHR.open('GET', wasmURL, true);
          wasmXHR.responseType = 'arraybuffer';
          wasmXHR.onload = function() {
            if (wasmXHR.status === 200 || wasmXHR.status === 0) {
              Module.wasmBinary = wasmXHR.response;
            } else {
              var wasmURLBytes = tryParseAsDataURI(wasmURL);
              if (wasmURLBytes) {
                Module.wasmBinary = wasmURLBytes.buffer;
              }
            }
%s
          };
          wasmXHR.send(null);
''' % (shared.JS.get_subresource_location(wasm_target), script.inline)

    if settings.WASM == 2:
      # If target browser does not support WebAssembly, we need to load the .wasm.js file before the main .js file.
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
''' % (script.inline, shared.JS.get_subresource_location(wasm_target) + '.js')

  # when script.inline isn't empty, add required helper functions such as tryParseAsDataURI
  if script.inline:
    for filename in ('arrayUtils.js', 'base64Utils.js', 'URIUtils.js'):
      content = read_and_preprocess(shared.path_from_root('src', filename))
      script.inline = content + script.inline

    script.inline = 'var ASSERTIONS = %s;\n%s' % (settings.ASSERTIONS, script.inline)

  # inline script for SINGLE_FILE output
  if settings.SINGLE_FILE:
    js_contents = script.inline or ''
    if script.src:
      js_contents += read_file(js_target)
    shared.try_delete(js_target)
    script.src = None
    script.inline = js_contents

  html_contents = do_replace(shell, '{{{ SCRIPT }}}', script.replacement())
  html_contents = tools.line_endings.convert_line_endings(html_contents, '\n', options.output_eol)

  try:
    with open(target, 'wb') as f:
      # Force UTF-8 output for consistency across platforms and with the web.
      f.write(html_contents.encode('utf-8'))
  except OSError as e:
    exit_with_error(f'cannot write output file: {e}')


def minify_html(filename):
  if settings.DEBUG_LEVEL >= 2:
    return

  opts = []
  # -g1 and greater retain whitespace and comments in source
  if settings.DEBUG_LEVEL == 0:
    opts += ['--collapse-whitespace',
             '--collapse-inline-tag-whitespace',
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


def generate_html(target, options, js_target, target_basename,
                  wasm_target, memfile):
  logger.debug('generating HTML')

  if settings.EXPORT_NAME != 'Module' and \
     not settings.MINIMAL_RUNTIME and \
     options.shell_path == shared.path_from_root('src', 'shell.html'):
    # the minimal runtime shell HTML is designed to support changing the export
    # name, but the normal one does not support that currently
    exit_with_error('Customizing EXPORT_NAME requires that the HTML be customized to use that name (see https://github.com/emscripten-core/emscripten/issues/10086)')

  if settings.MINIMAL_RUNTIME:
    generate_minimal_runtime_html(target, options, js_target, target_basename)
  else:
    generate_traditional_runtime_html(target, options, js_target, target_basename,
                                      wasm_target, memfile)

  if settings.MINIFY_HTML and (settings.OPT_LEVEL >= 1 or settings.SHRINK_LEVEL >= 1):
    minify_html(target)


def generate_worker_js(target, js_target, target_basename):
  # compiler output is embedded as base64
  if settings.SINGLE_FILE:
    proxy_worker_filename = shared.JS.get_subresource_location(js_target)

  # compiler output goes in .worker.js file
  else:
    move_file(js_target, unsuffixed(js_target) + '.worker.js')
    worker_target_basename = target_basename + '.worker'
    proxy_worker_filename = (settings.PROXY_TO_WORKER_FILENAME or worker_target_basename) + '.js'

  target_contents = worker_js_script(proxy_worker_filename)
  write_file(target, target_contents)


def worker_js_script(proxy_worker_filename):
  web_gl_client_src = read_file(shared.path_from_root('src', 'webGLClient.js'))
  idb_store_src = read_file(shared.path_from_root('src', 'IDBStore.js'))
  proxy_client_src = read_file(shared.path_from_root('src', 'proxyClient.js'))
  proxy_client_src = do_replace(proxy_client_src, '{{{ filename }}}', proxy_worker_filename)
  proxy_client_src = do_replace(proxy_client_src, '{{{ IDBStore.js }}}', idb_store_src)
  return web_gl_client_src + '\n' + proxy_client_src


def find_library(lib, lib_dirs):
  for lib_dir in lib_dirs:
    path = os.path.join(lib_dir, lib)
    if os.path.isfile(path):
      logger.debug('found library "%s" at %s', lib, path)
      return path
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
    lib = strip_prefix(flag, '-l')

    logger.debug('looking for library "%s"', lib)
    js_libs, native_lib = building.map_to_js_libs(lib)
    if js_libs is not None:
      libraries += [(i, js_lib) for js_lib in js_libs]
      # If native_lib is returned then include it in the link
      # via forced_stdlibs.
      if native_lib:
        state.forced_stdlibs.append(native_lib)
      continue

    # We don't need to resolve system libraries to absolute paths here, we can just
    # let wasm-ld handle that.  However, we do want to map to the correct variant.
    # For example we map `-lc` to `-lc-mt` if we are building with threading support.
    if 'lib' + lib in system_libs_map:
      lib = system_libs_map['lib' + lib]
      new_flags.append((i, '-l' + strip_prefix(lib.get_base_name(), 'lib')))
      continue

    if building.map_and_apply_to_settings(lib):
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
      quoted_src = quote(self.src)
      if settings.EXPORT_ES6:
        return f'''
        <script type="module">
          import initModule from "./{quoted_src}";
          initModule(Module);
        </script>
        '''
      else:
        return f'<script async type="text/javascript" src="{quoted_src}"></script>'
    else:
      return '<script>\n%s\n</script>' % self.inline


def is_valid_abspath(options, path_name):
  # Any path that is underneath the emscripten repository root must be ok.
  if shared.path_from_root().replace('\\', '/') in path_name.replace('\\', '/'):
    return True

  def in_directory(root, child):
    # make both path absolute
    root = os.path.realpath(root)
    child = os.path.realpath(child)

    # return true, if the common prefix of both is equal to directory
    # e.g. /a/b/c/d.rst and directory is /a/b, the common prefix is /a/b
    return os.path.commonprefix([root, child]) == root

  for valid_abspath in options.valid_abspaths:
    if in_directory(valid_abspath, path_name):
      return True
  return False


def parse_symbol_list_file(contents):
  """Parse contents of one-symbol-per-line response file.  This format can by used
  with, for example, -sEXPORTED_FUNCTIONS=@filename and avoids the need for any
  kind of quoting or escaping.
  """
  values = contents.splitlines()
  return [v.strip() for v in values]


def parse_value(text, expect_list):
  # Note that using response files can introduce whitespace, if the file
  # has a newline at the end. For that reason, we rstrip() in relevant
  # places here.
  def parse_string_value(text):
    first = text[0]
    if first == "'" or first == '"':
      text = text.rstrip()
      assert text[-1] == text[0] and len(text) > 1, 'unclosed opened quoted string. expected final character to be "%s" and length to be greater than 1 in "%s"' % (text[0], text)
      return text[1:-1]
    return text

  def parse_string_list_members(text):
    sep = ','
    values = text.split(sep)
    result = []
    index = 0
    while True:
      current = values[index].lstrip() # Cannot safely rstrip for cases like: "HERE-> ,"
      if not len(current):
        exit_with_error('string array should not contain an empty value')
      first = current[0]
      if not(first == "'" or first == '"'):
        result.append(current.rstrip())
      else:
        start = index
        while True: # Continue until closing quote found
          if index >= len(values):
            exit_with_error("unclosed quoted string. expected final character to be '%s' in '%s'" % (first, values[start]))
          new = values[index].rstrip()
          if new and new[-1] == first:
            if start == index:
              result.append(current.rstrip()[1:-1])
            else:
              result.append((current + sep + new)[1:-1])
            break
          else:
            current += sep + values[index]
            index += 1

      index += 1
      if index >= len(values):
        break
    return result

  def parse_string_list(text):
    text = text.rstrip()
    if text and text[0] == '[':
      if text[-1] != ']':
        exit_with_error('unclosed opened string list. expected final character to be "]" in "%s"' % (text))
      text = text[1:-1]
    if text.strip() == "":
      return []
    return parse_string_list_members(text)

  if expect_list or (text and text[0] == '['):
    # if json parsing fails, we fall back to our own parser, which can handle a few
    # simpler syntaxes
    try:
      return json.loads(text)
    except ValueError:
      return parse_string_list(text)

  try:
    return int(text)
  except ValueError:
    return parse_string_value(text)


def validate_arg_level(level_string, max_level, err_msg, clamp=False):
  try:
    level = int(level_string)
  except ValueError:
    raise Exception(err_msg)
  if clamp:
    if level > max_level:
      logger.warning("optimization level '-O" + level_string + "' is not supported; using '-O" + str(max_level) + "' instead")
      level = max_level
  if not 0 <= level <= max_level:
    raise Exception(err_msg)
  return level


def is_int(s):
  try:
    int(s)
    return True
  except ValueError:
    return False


def main(args):
  start_time = time.time()
  ret = run(args)
  logger.debug('total time: %.2f seconds', (time.time() - start_time))
  return ret


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.warning('KeyboardInterrupt')
    sys.exit(1)
