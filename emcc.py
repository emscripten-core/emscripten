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

import json
import logging
import os
import re
import shlex
import shutil
import sys
import time
import tarfile
from enum import Enum, auto, unique
from subprocess import PIPE


from tools import shared, system_libs, utils, ports
from tools import colored_logger, diagnostics, building
from tools.shared import unsuffixed, unsuffixed_basename, get_file_suffix
from tools.shared import run_process, exit_with_error, DEBUG
from tools.shared import in_temp, OFormat
from tools.shared import DYNAMICLIB_ENDINGS
from tools.response_file import substitute_response_files
from tools import config
from tools import cache
from tools.settings import default_setting, user_settings, settings, MEM_SIZE_SETTINGS, COMPILE_TIME_SETTINGS
from tools.utils import read_file, removeprefix

logger = logging.getLogger('emcc')

# In git checkouts of emscripten `bootstrap.py` exists to run post-checkout
# steps.  In packaged versions (e.g. emsdk) this file does not exist (because
# it is excluded in tools/install.py) and these steps are assumed to have been
# run already.
if os.path.exists(utils.path_from_root('.git')) and os.path.exists(utils.path_from_root('bootstrap.py')):
  import bootstrap
  bootstrap.check()

# endings = dot + a suffix, compare against result of shared.suffix()
C_ENDINGS = ['.c', '.i']
CXX_ENDINGS = ['.cppm', '.pcm', '.cpp', '.cxx', '.cc', '.c++', '.CPP', '.CXX', '.C', '.CC', '.C++', '.ii']
OBJC_ENDINGS = ['.m', '.mi']
PREPROCESSED_ENDINGS = ['.i', '.ii']
OBJCXX_ENDINGS = ['.mm', '.mii']
SPECIAL_ENDINGLESS_FILENAMES = [os.devnull]
C_ENDINGS += SPECIAL_ENDINGLESS_FILENAMES # consider the special endingless filenames like /dev/null to be C

SOURCE_ENDINGS = C_ENDINGS + CXX_ENDINGS + OBJC_ENDINGS + OBJCXX_ENDINGS + ['.bc', '.ll', '.S']
ASSEMBLY_ENDINGS = ['.s']
HEADER_ENDINGS = ['.h', '.hxx', '.hpp', '.hh', '.H', '.HXX', '.HPP', '.HH']

# These symbol names are allowed in INCOMING_MODULE_JS_API but are not part of the
# default set.
EXTRA_INCOMING_JS_API = [
  'fetchSettings'
]

SIMD_INTEL_FEATURE_TOWER = ['-msse', '-msse2', '-msse3', '-mssse3', '-msse4.1', '-msse4.2', '-msse4', '-mavx']
SIMD_NEON_FLAGS = ['-mfpu=neon']
LINK_ONLY_FLAGS = {
    '--bind', '--closure', '--cpuprofiler', '--embed-file',
    '--emit-symbol-map', '--emrun', '--exclude-file', '--extern-post-js',
    '--extern-pre-js', '--ignore-dynamic-linking', '--js-library',
    '--js-transform', '--oformat', '--output_eol',
    '--post-js', '--pre-js', '--preload-file', '--profiling-funcs',
    '--proxy-to-worker', '--shell-file', '--source-map-base',
    '--threadprofiler', '--use-preload-plugins'
}


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
    # Using tuple here to prevent accidental mutation
    self.orig_args = tuple(args)
    self.has_dash_c = False
    self.has_dash_E = False
    self.has_dash_S = False
    # List of link options paired with their position on the command line [(i, option), ...].
    self.link_flags = []
    self.lib_dirs = []

  def has_link_flag(self, f):
    return f in [x for _, x in self.link_flags]

  def add_link_flag(self, i, flag):
    if flag.startswith('-L'):
      self.lib_dirs.append(flag[2:])
    # Link flags should be adding in strictly ascending order
    assert not self.link_flags or i > self.link_flags[-1][0], self.link_flags
    self.link_flags.append((i, flag))

  def append_link_flag(self, flag):
    if self.link_flags:
      index = self.link_flags[-1][0] + 1
    else:
      index = 1
    self.add_link_flag(index, flag)


class EmccOptions:
  def __init__(self):
    self.target = ''
    self.output_file = None
    self.no_minify = False
    self.post_link = False
    self.save_temps = False
    self.executable = False
    self.compiler_wrapper = None
    self.oformat = None
    self.requested_debug = ''
    self.emit_symbol_map = False
    self.use_closure_compiler = None
    self.closure_args = []
    self.js_transform = None
    self.pre_js = [] # before all js
    self.post_js = [] # after all js
    self.extern_pre_js = [] # before all js, external to optimized code
    self.extern_post_js = [] # after all js, external to optimized code
    self.preload_files = []
    self.embed_files = []
    self.exclude_files = []
    self.ignore_dynamic_linking = False
    self.shell_path = None
    self.source_map_base = ''
    self.emit_tsd = ''
    self.embind_emit_tsd = ''
    self.emrun = False
    self.cpu_profiler = False
    self.memory_profiler = False
    self.use_preload_cache = False
    self.use_preload_plugins = False
    self.valid_abspaths = []
    # Specifies the line ending format to use for all generated text files.
    # Defaults to using the native EOL on each platform (\r\n on Windows, \n on
    # Linux & MacOS)
    self.output_eol = os.linesep
    self.no_entry = False
    self.shared = False
    self.relocatable = False
    self.reproduce = None


def create_reproduce_file(name, args):
  def make_relative(filename):
    filename = os.path.normpath(os.path.abspath(filename))
    filename = os.path.splitdrive(filename)[1]
    filename = filename[1:]
    return filename

  root = unsuffixed_basename(name)
  with tarfile.open(name, 'w') as reproduce_file:
    reproduce_file.add(shared.path_from_root('emscripten-version.txt'), os.path.join(root, 'version.txt'))

    with shared.get_temp_files().get_file(suffix='.tar') as rsp_name:
      with open(rsp_name, 'w') as rsp:
        ignore_next = False
        output_arg = None

        for arg in args:
          ignore = ignore_next
          ignore_next = False
          if arg.startswith('--reproduce='):
            continue

          if arg.startswith('-o='):
            rsp.write('-o\n')
            arg = arg[3:]
            output_arg = True
            ignore = True

          if output_arg:
            # If -o path contains directories, "emcc @response.txt" will likely
            # fail because the archive we are creating doesn't contain empty
            # directories for the output path (-o doesn't create directories).
            # Strip directories to prevent the issue.
            arg = os.path.basename(arg)
            output_arg = False

          if not arg.startswith('-') and not ignore:
            relpath = make_relative(arg)
            rsp.write(relpath + '\n')
            reproduce_file.add(arg, os.path.join(root, relpath))
          else:
            rsp.write(arg + '\n')

          if ignore:
            continue

          if arg in ('-MT', '-MF', '-MJ', '-MQ', '-D', '-U', '-o', '-x',
                     '-Xpreprocessor', '-include', '-imacros', '-idirafter',
                     '-iprefix', '-iwithprefix', '-iwithprefixbefore',
                     '-isysroot', '-imultilib', '-A', '-isystem', '-iquote',
                     '-install_name', '-compatibility_version',
                     '-current_version', '-I', '-L', '-include-pch',
                     '-Xlinker', '-Xclang'):
            ignore_next = True

          if arg == '-o':
            output_arg = True

      reproduce_file.add(rsp_name, os.path.join(root, 'response.txt'))


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


def apply_user_settings():
  """Take a map of users settings {NAME: VALUE} and apply them to the global
  settings object.
  """

  # Stash a copy of all available incoming APIs before the user can potentially override it
  settings.ALL_INCOMING_MODULE_JS_API = settings.INCOMING_MODULE_JS_API + EXTRA_INCOMING_JS_API

  for key, value in user_settings.items():
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
      filename = removeprefix(value, '@')
      if not os.path.isfile(filename):
        exit_with_error('%s: file not found parsing argument: %s=%s' % (filename, key, value))
      value = read_file(filename).strip()
    else:
      value = value.replace('\\', '\\\\')

    expected_type = settings.types.get(key)

    if filename and expected_type == list and value.strip()[0] != '[':
      # Prefer simpler one-line-per value parser
      value = parse_symbol_list_file(value)
    else:
      try:
        value = parse_value(value, expected_type)
      except Exception as e:
        exit_with_error(f'error parsing "-s" setting "{key}={value}": {e}')

    setattr(settings, user_key, value)

    if key == 'EXPORTED_FUNCTIONS':
      # used for warnings in emscripten.py
      settings.USER_EXPORTS = settings.EXPORTED_FUNCTIONS.copy()

    # TODO(sbc): Remove this legacy way.
    if key == 'WASM_OBJECT_FILES':
      settings.LTO = 0 if value else 'full'

    if key == 'JSPI':
      settings.ASYNCIFY = 2
    if key == 'JSPI_IMPORTS':
      settings.ASYNCIFY_IMPORTS = value
    if key == 'JSPI_EXPORTS':
      settings.ASYNCIFY_EXPORTS = value


def cxx_to_c_compiler(cxx):
  # Convert C++ compiler name into C compiler name
  dirname, basename = os.path.split(cxx)
  basename = basename.replace('clang++', 'clang').replace('g++', 'gcc').replace('em++', 'emcc')
  return os.path.join(dirname, basename)


def is_dash_s_for_emcc(args, i):
  # -s OPT=VALUE or -s OPT or -sOPT are all interpreted as emscripten flags.
  # -s by itself is a linker option (alias for --strip-all)
  if args[i] == '-s':
    if len(args) <= i + 1:
      return False
    arg = args[i + 1]
  else:
    arg = removeprefix(args[i], '-s')
  arg = arg.split('=')[0]
  return arg.isidentifier() and arg.isupper()


def parse_s_args(args):
  settings_changes = []
  for i in range(len(args)):
    if args[i].startswith('-s'):
      if is_dash_s_for_emcc(args, i):
        if args[i] == '-s':
          key = args[i + 1]
          args[i + 1] = ''
        else:
          key = removeprefix(args[i], '-s')
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


def get_target_flags():
  return ['-target', shared.get_llvm_target()]


def get_clang_flags(user_args):
  flags = get_target_flags()

  # if exception catching is disabled, we can prevent that code from being
  # generated in the frontend
  if settings.DISABLE_EXCEPTION_CATCHING and not settings.WASM_EXCEPTIONS:
    flags.append('-fignore-exceptions')

  if settings.INLINING_LIMIT:
    flags.append('-fno-inline-functions')

  if settings.RELOCATABLE and '-fPIC' not in user_args:
    flags.append('-fPIC')

  if settings.RELOCATABLE or settings.LINKABLE or '-fPIC' in user_args:
    if not any(a.startswith('-fvisibility') for a in user_args):
      # For relocatable code we default to visibility=default in emscripten even
      # though the upstream backend defaults visibility=hidden.  This matches the
      # expectations of C/C++ code in the wild which expects undecorated symbols
      # to be exported to other DSO's by default.
      flags.append('-fvisibility=default')

  if settings.LTO:
    if not any(a.startswith('-flto') for a in user_args):
      flags.append('-flto=' + settings.LTO)
    # setjmp/longjmp handling using Wasm EH
    # For non-LTO, '-mllvm -wasm-enable-eh' added in
    # building.llvm_backend_args() sets this feature in clang. But in LTO, the
    # argument is added to wasm-ld instead, so clang needs to know that EH is
    # enabled so that it can be added to the attributes in LLVM IR.
    if settings.SUPPORT_LONGJMP == 'wasm':
      flags.append('-mexception-handling')

  else:
    # In LTO mode these args get passed instead at link time when the backend runs.
    for a in building.llvm_backend_args():
      flags += ['-mllvm', a]

  return flags


cflags = None


def get_cflags(user_args, is_cxx):
  global cflags
  if cflags:
    return cflags

  # Flags we pass to the compiler when building C/C++ code
  # We add these to the user's flags (newargs), but not when building .s or .S assembly files
  cflags = get_clang_flags(user_args)
  cflags.append('--sysroot=' + cache.get_sysroot(absolute=True))

  if settings.EMSCRIPTEN_TRACING:
    cflags.append('-D__EMSCRIPTEN_TRACING__=1')

  if settings.SHARED_MEMORY:
    cflags.append('-D__EMSCRIPTEN_SHARED_MEMORY__=1')

  if settings.WASM_WORKERS:
    cflags.append('-D__EMSCRIPTEN_WASM_WORKERS__=1')

  if not settings.STRICT:
    # The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code
    # in strict mode. Code should use the define __EMSCRIPTEN__ instead.
    cflags.append('-DEMSCRIPTEN')

  # Changes to default clang behavior

  # Implicit functions can cause horribly confusing function pointer type errors, see #2175
  # If your codebase really needs them - very unrecommended! - you can disable the error with
  #   -Wno-error=implicit-function-declaration
  # or disable even a warning about it with
  #   -Wno-implicit-function-declaration
  # This is already an error in C++ so we don't need to inject extra flags.
  if not is_cxx:
    cflags += ['-Werror=implicit-function-declaration']

  ports.add_cflags(cflags, settings)

  def array_contains_any_of(hay, needles):
    for n in needles:
      if n in hay:
        return True

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER) or array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    if '-msimd128' not in user_args and '-mrelaxed-simd' not in user_args:
      exit_with_error('passing any of ' + ', '.join(SIMD_INTEL_FEATURE_TOWER + SIMD_NEON_FLAGS) + ' flags also requires passing -msimd128 (or -mrelaxed-simd)!')
    cflags += ['-D__SSE__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[1:]):
    cflags += ['-D__SSE2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[2:]):
    cflags += ['-D__SSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[3:]):
    cflags += ['-D__SSSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[4:]):
    cflags += ['-D__SSE4_1__=1']

  # Handle both -msse4.2 and its alias -msse4.
  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[5:]):
    cflags += ['-D__SSE4_2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[7:]):
    cflags += ['-D__AVX__=1']

  if array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    cflags += ['-D__ARM_NEON__=1']

  if '-nostdinc' not in user_args:
    if not settings.USE_SDL:
      cflags += ['-Xclang', '-iwithsysroot' + os.path.join('/include', 'fakesdl')]
    cflags += ['-Xclang', '-iwithsysroot' + os.path.join('/include', 'compat')]

  return cflags


def get_library_basename(filename):
  """Similar to get_file_suffix this strips off all numeric suffixes and then
  then final non-numeric one.  For example for 'libz.so.1.2.8' returns 'libz'"""
  filename = os.path.basename(filename)
  while filename:
    filename, suffix = os.path.splitext(filename)
    # Keep stipping suffixes until we strip a non-numeric one.
    if not suffix[1:].isdigit():
      return filename


#
# Main run() function
#
def run(args):
  if shared.run_via_emxx:
    clang = shared.CLANG_CXX
  else:
    clang = shared.CLANG_CC

  # Special case the handling of `-v` because it has a special/different meaning
  # when used with no other arguments.  In particular, we must handle this early
  # on, before we inject EMCC_CFLAGS.  This is because tools like cmake and
  # autoconf will run `emcc -v` to determine the compiler version and we don't
  # want that to break for users of EMCC_CFLAGS.
  if len(args) == 2 and args[1] == '-v':
    # autoconf likes to see 'GNU' in the output to enable shared object support
    print(version_string(), file=sys.stderr)
    return shared.check_call([clang, '-v'] + get_target_flags(), check=False).returncode

  # Additional compiler flags that we treat as if they were passed to us on the
  # commandline
  EMCC_CFLAGS = os.environ.get('EMCC_CFLAGS')
  if EMCC_CFLAGS:
    args += shlex.split(EMCC_CFLAGS)

  if DEBUG:
    logger.warning(f'invocation: {shared.shlex_join(args)} (in {os.getcwd()})')

  # Strip args[0] (program name)
  args = args[1:]

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
    print(read_file(utils.path_from_root('docs/emcc.txt')))

    print('''
------------------------------------------------------------------

emcc: supported targets: llvm bitcode, WebAssembly, NOT elf
(autoconf likes to see elf above to enable shared object support)
''')
    return 0

  ## Process argument and setup the compiler
  state = EmccState(args)
  options, newargs = phase_parse_arguments(state)

  if not shared.SKIP_SUBPROCS:
    shared.check_sanity()

  if '--version' in args:
    print(version_string())
    print('''\
Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''')
    return 0

  if '-dumpversion' in args: # gcc's doc states "Print the compiler version [...] and don't do anything else."
    print(utils.EMSCRIPTEN_VERSION)
    return 0

  if '--cflags' in args:
    # fake running the command, to see the full args we pass to clang
    args = [x for x in args if x != '--cflags']
    with shared.get_temp_files().get_file(suffix='.o') as temp_target:
      input_file = 'hello_world.c'
      compiler = shared.EMCC
      if shared.run_via_emxx:
        compiler = shared.EMXX
      cmd = [compiler, utils.path_from_root('test', input_file), '-v', '-c', '-o', temp_target] + args
      proc = run_process(cmd, stderr=PIPE, check=False)
      if proc.returncode != 0:
        print(proc.stderr)
        exit_with_error('error getting cflags')
      lines = [x for x in proc.stderr.splitlines() if clang in x and input_file in x]
      if not lines:
        exit_with_error(f'unable to parse output of `{cmd}`:\n{proc.stderr}')
      parts = shlex.split(lines[0].replace('\\', '\\\\'))
      parts = [x for x in parts if x not in ['-c', '-o', '-v', '-emit-llvm'] and input_file not in x and temp_target not in x]
      print(shared.shlex_join(parts[1:]))
    return 0

  if 'EMMAKEN_NO_SDK' in os.environ:
    exit_with_error('EMMAKEN_NO_SDK is no longer supported.  The standard -nostdlib and -nostdinc flags should be used instead')

  if 'EMMAKEN_COMPILER' in os.environ:
    exit_with_error('`EMMAKEN_COMPILER` is no longer supported.\n' +
                    'Please use the `LLVM_ROOT` and/or `COMPILER_WRAPPER` config settings instead')

  if 'EMMAKEN_CFLAGS' in os.environ:
    exit_with_error('`EMMAKEN_CFLAGS` is no longer supported, please use `EMCC_CFLAGS` instead')

  if 'EMCC_REPRODUCE' in os.environ:
    options.reproduce = os.environ['EMCC_REPRODUCE']

  # For internal consistency, ensure we don't attempt or read or write any link time
  # settings until we reach the linking phase.
  settings.limit_settings(COMPILE_TIME_SETTINGS)

  newargs, input_files = phase_setup(options, state, newargs)

  if '-dumpmachine' in newargs or '-print-target-triple' in newargs or '--print-target-triple' in newargs:
    print(shared.get_llvm_target())
    return 0

  if '-print-search-dirs' in newargs or '--print-search-dirs' in newargs:
    print(f'programs: ={config.LLVM_ROOT}')
    print(f'libraries: ={cache.get_lib_dir(absolute=True)}')
    return 0

  if '-print-resource-dir' in newargs:
    shared.check_call([clang] + newargs)
    return 0

  if '-print-libgcc-file-name' in newargs or '--print-libgcc-file-name' in newargs:
    settings.limit_settings(None)
    compiler_rt = system_libs.Library.get_usable_variations()['libcompiler_rt']
    print(compiler_rt.get_path(absolute=True))
    return 0

  print_file_name = [a for a in newargs if a.startswith('-print-file-name=') or a.startswith('--print-file-name=')]
  if print_file_name:
    libname = print_file_name[-1].split('=')[1]
    system_libpath = cache.get_lib_dir(absolute=True)
    fullpath = os.path.join(system_libpath, libname)
    if os.path.isfile(fullpath):
      print(fullpath)
    else:
      print(libname)
    return 0

  if options.reproduce:
    create_reproduce_file(options.reproduce, args)

  if state.mode == Mode.POST_LINK_ONLY:
    if len(input_files) != 1:
      exit_with_error('--post-link requires a single input file')
    # Delay import of link.py to avoid processing this file when only compiling
    from tools import link
    link.run_post_link(input_files[0][1], options, state, newargs)
    return 0

  ## Compile source code to object files
  linker_inputs = phase_compile_inputs(options, state, newargs, input_files)

  if state.mode == Mode.COMPILE_AND_LINK:
    # Delay import of link.py to avoid processing this file when only compiling
    from tools import link
    return link.run(linker_inputs, options, state, newargs)
  else:
    logger.debug('stopping after compile phase')
    return 0


def normalize_boolean_setting(name, value):
  # boolean NO_X settings are aliases for X
  # (note that *non*-boolean setting values have special meanings,
  # and we can't just flip them, so leave them as-is to be
  # handled in a special way later)
  if name.startswith('NO_') and value in ('0', '1'):
    name = removeprefix(name, 'NO_')
    value = str(1 - int(value))
  return name, value


@ToolchainProfiler.profile_block('parse arguments')
def phase_parse_arguments(state):
  """The first phase of the compiler.  Parse command line argument and
  populate settings.
  """
  newargs = list(state.orig_args)

  # Scan and strip emscripten specific cmdline warning flags.
  # This needs to run before other cmdline flags have been parsed, so that
  # warnings are properly printed during arg parse.
  newargs = diagnostics.capture_warnings(newargs)

  if not diagnostics.is_enabled('deprecated'):
    settings.WARN_DEPRECATED = 0

  for i in range(len(newargs)):
    if newargs[i] in ('-l', '-L', '-I', '-z'):
      # Scan for flags that can be written as either one or two arguments
      # and normalize them to the single argument form.
      newargs[i] += newargs[i + 1]
      newargs[i + 1] = ''

  options, settings_changes, user_js_defines, newargs = parse_args(newargs)

  if options.post_link or options.oformat == OFormat.BARE:
    diagnostics.warning('experimental', '--oformat=bare/--post-link are experimental and subject to change.')

  explicit_settings_changes, newargs = parse_s_args(newargs)
  settings_changes += explicit_settings_changes

  for s in settings_changes:
    key, value = s.split('=', 1)
    key, value = normalize_boolean_setting(key, value)
    user_settings[key] = value

  # STRICT is used when applying settings so it needs to be applied first before
  # calling `apply_user_settings`.
  strict_cmdline = user_settings.get('STRICT')
  if strict_cmdline:
    settings.STRICT = int(strict_cmdline)

  # Apply user -jsD settings
  for s in user_js_defines:
    settings[s[0]] = s[1]

  # Apply -s settings in newargs here (after optimization levels, so they can override them)
  apply_user_settings()

  return options, newargs


@ToolchainProfiler.profile_block('setup')
def phase_setup(options, state, newargs):
  """Second phase: configure and setup the compiler based on the specified settings and arguments.
  """

  if settings.RUNTIME_LINKED_LIBS:
    newargs += settings.RUNTIME_LINKED_LIBS

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
    if arg in {'-MT', '-MF', '-MJ', '-MQ', '-D', '-U', '-o', '-x',
               '-Xpreprocessor', '-include', '-imacros', '-idirafter',
               '-iprefix', '-iwithprefix', '-iwithprefixbefore',
               '-isysroot', '-imultilib', '-A', '-isystem', '-iquote',
               '-install_name', '-compatibility_version',
               '-current_version', '-I', '-L', '-include-pch',
               '-undefined', '-target',
               '-Xlinker', '-Xclang', '-z'}:
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
      input_files.append((i, arg))
    elif arg.startswith('-L'):
      state.add_link_flag(i, arg)
    elif arg.startswith('-l'):
      state.add_link_flag(i, arg)
    elif arg == '-z':
      state.add_link_flag(i, newargs[i])
      state.add_link_flag(i + 1, newargs[i + 1])
    elif arg.startswith('-z'):
      state.add_link_flag(i, newargs[i])
    elif arg.startswith('-Wl,'):
      # Multiple comma separated link flags can be specified. Create fake
      # fractional indices for these: -Wl,a,b,c,d at index 4 becomes:
      # (4, a), (4.25, b), (4.5, c), (4.75, d)
      link_flags_to_add = arg.split(',')[1:]
      for flag_index, flag in enumerate(link_flags_to_add):
        state.add_link_flag(i + float(flag_index) / len(link_flags_to_add), flag)
    elif arg == '-Xlinker':
      state.add_link_flag(i + 1, newargs[i + 1])
    elif arg == '-s':
      state.add_link_flag(i, newargs[i])
    elif arg == '-':
      input_files.append((i, arg))
      newargs[i] = ''

  newargs = [a for a in newargs if a]

  # SSEx is implemented on top of SIMD128 instruction set, but do not pass SSE flags to LLVM
  # so it won't think about generating native x86 SSE code.
  newargs = [x for x in newargs if x not in SIMD_INTEL_FEATURE_TOWER and x not in SIMD_NEON_FLAGS]

  state.has_dash_c = '-c' in newargs or '--precompile' in newargs
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
    for key in user_settings:
      if key not in COMPILE_TIME_SETTINGS:
        diagnostics.warning(
            'unused-command-line-argument',
            "linker setting ignored during compilation: '%s'" % key)
    for arg in state.orig_args:
      if arg in LINK_ONLY_FLAGS:
        diagnostics.warning(
            'unused-command-line-argument',
            "linker flag ignored during compilation: '%s'" % arg)

  if settings.MAIN_MODULE or settings.SIDE_MODULE:
    settings.RELOCATABLE = 1

  if 'USE_PTHREADS' in user_settings:
    settings.PTHREADS = settings.USE_PTHREADS

  # Pthreads and Wasm Workers require targeting shared Wasm memory (SAB).
  if settings.PTHREADS or settings.WASM_WORKERS:
    settings.SHARED_MEMORY = 1

  if settings.PTHREADS and '-pthread' not in newargs:
    newargs += ['-pthread']
  elif settings.SHARED_MEMORY:
    if '-matomics' not in newargs:
      newargs += ['-matomics']
    if '-mbulk-memory' not in newargs:
      newargs += ['-mbulk-memory']

  if settings.SHARED_MEMORY:
    settings.BULK_MEMORY = 1

  if 'DISABLE_EXCEPTION_CATCHING' in user_settings and 'EXCEPTION_CATCHING_ALLOWED' in user_settings:
    # If we get here then the user specified both DISABLE_EXCEPTION_CATCHING and EXCEPTION_CATCHING_ALLOWED
    # on the command line.  This is no longer valid so report either an error or a warning (for
    # backwards compat with the old `DISABLE_EXCEPTION_CATCHING=2`
    if user_settings['DISABLE_EXCEPTION_CATCHING'] in ('0', '2'):
      diagnostics.warning('deprecated', 'DISABLE_EXCEPTION_CATCHING=X is no longer needed when specifying EXCEPTION_CATCHING_ALLOWED')
    else:
      exit_with_error('DISABLE_EXCEPTION_CATCHING and EXCEPTION_CATCHING_ALLOWED are mutually exclusive')

  if settings.EXCEPTION_CATCHING_ALLOWED:
    settings.DISABLE_EXCEPTION_CATCHING = 0

  if settings.WASM_EXCEPTIONS:
    if user_settings.get('DISABLE_EXCEPTION_CATCHING') == '0':
      exit_with_error('DISABLE_EXCEPTION_CATCHING=0 is not compatible with -fwasm-exceptions')
    if user_settings.get('DISABLE_EXCEPTION_THROWING') == '0':
      exit_with_error('DISABLE_EXCEPTION_THROWING=0 is not compatible with -fwasm-exceptions')
    # -fwasm-exceptions takes care of enabling them, so users aren't supposed to
    # pass them explicitly, regardless of their values
    if 'DISABLE_EXCEPTION_CATCHING' in user_settings or 'DISABLE_EXCEPTION_THROWING' in user_settings:
      diagnostics.warning('emcc', 'you no longer need to pass DISABLE_EXCEPTION_CATCHING or DISABLE_EXCEPTION_THROWING when using Wasm exceptions')
    settings.DISABLE_EXCEPTION_CATCHING = 1
    settings.DISABLE_EXCEPTION_THROWING = 1

    if user_settings.get('ASYNCIFY') == '1':
      diagnostics.warning('emcc', 'ASYNCIFY=1 is not compatible with -fwasm-exceptions. Parts of the program that mix ASYNCIFY and exceptions will not compile.')

    if user_settings.get('SUPPORT_LONGJMP') == 'emscripten':
      exit_with_error('SUPPORT_LONGJMP=emscripten is not compatible with -fwasm-exceptions')

  if settings.DISABLE_EXCEPTION_THROWING and not settings.DISABLE_EXCEPTION_CATCHING:
    exit_with_error("DISABLE_EXCEPTION_THROWING was set (probably from -fno-exceptions) but is not compatible with enabling exception catching (DISABLE_EXCEPTION_CATCHING=0). If you don't want exceptions, set DISABLE_EXCEPTION_CATCHING to 1; if you do want exceptions, don't link with -fno-exceptions")

  if options.target.startswith('wasm64'):
    default_setting('MEMORY64', 1)

  if settings.MEMORY64:
    if options.target.startswith('wasm32'):
      exit_with_error('wasm32 target is not compatible with -sMEMORY64')
    diagnostics.warning('experimental', '-sMEMORY64 is still experimental. Many features may not work.')

  # Wasm SjLj cannot be used with Emscripten EH
  if settings.SUPPORT_LONGJMP == 'wasm':
    # DISABLE_EXCEPTION_THROWING is 0 by default for Emscripten EH throwing, but
    # Wasm SjLj cannot be used with Emscripten EH. We error out if
    # DISABLE_EXCEPTION_THROWING=0 is explicitly requested by the user;
    # otherwise we disable it here.
    if user_settings.get('DISABLE_EXCEPTION_THROWING') == '0':
      exit_with_error('SUPPORT_LONGJMP=wasm cannot be used with DISABLE_EXCEPTION_THROWING=0')
    # We error out for DISABLE_EXCEPTION_CATCHING=0, because it is 1 by default
    # and this can be 0 only if the user specifies so.
    if user_settings.get('DISABLE_EXCEPTION_CATCHING') == '0':
      exit_with_error('SUPPORT_LONGJMP=wasm cannot be used with DISABLE_EXCEPTION_CATCHING=0')
    default_setting('DISABLE_EXCEPTION_THROWING', 1)

  # SUPPORT_LONGJMP=1 means the default SjLj handling mechanism, which is 'wasm'
  # if Wasm EH is used and 'emscripten' otherwise.
  if settings.SUPPORT_LONGJMP == 1:
    if settings.WASM_EXCEPTIONS:
      settings.SUPPORT_LONGJMP = 'wasm'
    else:
      settings.SUPPORT_LONGJMP = 'emscripten'

  # SDL2 requires eglGetProcAddress() to work.
  # NOTE: if SDL2 is updated to not rely on eglGetProcAddress(), this can be removed
  if settings.USE_SDL == 2 or settings.USE_SDL_MIXER == 2 or settings.USE_SDL_GFX == 2:
    default_setting('GL_ENABLE_GET_PROC_ADDRESS', 1)

  return (newargs, input_files)


def get_clang_output_extension(state):
  if '-emit-llvm' in state.orig_args:
    if state.has_dash_S:
      return '.ll'
    else:
      return '.bc'

  if state.has_dash_S:
    return '.s'
  else:
    return '.o'


def filter_out_link_flags(args):
  rtn = []

  def is_link_flag(flag):
    if flag in ('-nostdlib', '-nostartfiles', '-nolibc', '-nodefaultlibs', '-s'):
      return True
    return flag.startswith(('-l', '-L', '-Wl,', '-z'))

  skip = False
  for arg in args:
    if skip:
      skip = False
      continue
    if is_link_flag(arg):
      continue
    if arg == '-Xlinker':
      skip = True
      continue
    rtn.append(arg)
  return rtn


@ToolchainProfiler.profile_block('compile inputs')
def phase_compile_inputs(options, state, newargs, input_files):
  if shared.run_via_emxx:
    compiler = [shared.CLANG_CXX]
  else:
    compiler = [shared.CLANG_CC]

  if config.COMPILER_WRAPPER:
    logger.debug('using compiler wrapper: %s', config.COMPILER_WRAPPER)
    compiler.insert(0, config.COMPILER_WRAPPER)

  compile_args = newargs
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
        return removeprefix(item, '-x')
    return ''

  language_mode = get_language_mode(newargs)
  use_cxx = 'c++' in language_mode or shared.run_via_emxx

  def get_clang_command():
    return compiler + get_cflags(state.orig_args, use_cxx) + compile_args

  def get_clang_command_preprocessed():
    return compiler + get_clang_flags(state.orig_args) + compile_args

  def get_clang_command_asm():
    return compiler + get_target_flags() + compile_args

  # preprocessor-only (-E) support
  if state.mode == Mode.PREPROCESS_ONLY:
    inputs = [i[1] for i in input_files]
    cmd = get_clang_command() + inputs
    if options.output_file:
      cmd += ['-o', options.output_file]
    # Do not compile, but just output the result from preprocessing stage or
    # output the dependency rule. Warning: clang and gcc behave differently
    # with -MF! (clang seems to not recognize it)
    logger.debug(('just preprocessor ' if state.has_dash_E else 'just dependencies: ') + ' '.join(cmd))
    shared.exec_process(cmd)
    assert False, 'exec_process does not return'

  # Precompiled headers support
  if state.mode == Mode.PCH:
    inputs = [i[1] for i in input_files]
    for header in inputs:
      if not shared.suffix(header) in HEADER_ENDINGS:
        exit_with_error(f'cannot mix precompiled headers with non-header inputs: {inputs} : {header}')
    cmd = get_clang_command() + inputs
    if options.output_file:
      cmd += ['-o', options.output_file]
    logger.debug(f"running (for precompiled headers): {cmd[0]} {' '.join(cmd[1:])}")
    shared.exec_process(cmd)
    assert False, 'exec_process does not return'

  if state.mode == Mode.COMPILE_ONLY:
    inputs = [i[1] for i in input_files]
    if all(get_file_suffix(i) in ASSEMBLY_ENDINGS for i in inputs):
      cmd = get_clang_command_asm() + inputs
    else:
      cmd = get_clang_command() + inputs
    if options.output_file:
      cmd += ['-o', options.output_file]
      if get_file_suffix(options.output_file) == '.bc' and not settings.LTO and '-emit-llvm' not in state.orig_args:
        diagnostics.warning('emcc', '.bc output file suffix used without -flto or -emit-llvm.  Consider using .o extension since emcc will output an object file, not a bitcode file')
    shared.exec_process(cmd)
    assert False, 'exec_process does not return'

  # In COMPILE_AND_LINK we need to compile source files too, but we also need to
  # filter out the link flags
  compile_args = filter_out_link_flags(compile_args)
  linker_inputs = []
  seen_names = {}

  def uniquename(name):
    if name not in seen_names:
      seen_names[name] = str(len(seen_names))
    return unsuffixed(name) + '_' + seen_names[name] + shared.suffix(name)

  def get_object_filename(input_file):
    return in_temp(shared.replace_suffix(uniquename(input_file), '.o'))

  def compile_source_file(i, input_file):
    logger.debug(f'compiling source file: {input_file}')
    output_file = get_object_filename(input_file)
    linker_inputs.append((i, output_file))
    if get_file_suffix(input_file) in ASSEMBLY_ENDINGS:
      cmd = get_clang_command_asm()
    elif get_file_suffix(input_file) in PREPROCESSED_ENDINGS:
      cmd = get_clang_command_preprocessed()
    else:
      cmd = get_clang_command()
      if get_file_suffix(input_file) in ['.pcm']:
        cmd = [c for c in cmd if not c.startswith('-fprebuilt-module-path=')]
    cmd += [input_file]
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
    if not shared.SKIP_SUBPROCS:
      assert os.path.exists(output_file)
      if options.save_temps:
        shutil.copyfile(output_file, shared.unsuffixed_basename(input_file) + '.o')

  # First, generate LLVM bitcode. For each input file, we get base.o with bitcode
  for i, input_file in input_files:
    file_suffix = get_file_suffix(input_file)
    if file_suffix in SOURCE_ENDINGS + ASSEMBLY_ENDINGS or (state.has_dash_c and file_suffix == '.bc'):
      compile_source_file(i, input_file)
    elif file_suffix in DYNAMICLIB_ENDINGS:
      logger.debug(f'using shared library: {input_file}')
      linker_inputs.append((i, input_file))
    elif building.is_ar(input_file):
      logger.debug(f'using static library: {input_file}')
      linker_inputs.append((i, input_file))
    elif language_mode:
      compile_source_file(i, input_file)
    elif input_file == '-':
      exit_with_error('-E or -x required when input is from standard input')
    else:
      # Default to assuming the inputs are object files and pass them to the linker
      logger.debug(f'using object file: {input_file}')
      linker_inputs.append((i, input_file))

  return linker_inputs


def version_string():
  # if the emscripten folder is not a git repo, don't run git show - that can
  # look up and find the revision in a parent directory that is a git repo
  revision_suffix = ''
  if os.path.exists(utils.path_from_root('.git')):
    git_rev = run_process(
      ['git', 'rev-parse', 'HEAD'],
      stdout=PIPE, stderr=PIPE, cwd=utils.path_from_root()).stdout.strip()
    revision_suffix = ' (%s)' % git_rev
  elif os.path.exists(utils.path_from_root('emscripten-revision.txt')):
    rev = read_file(utils.path_from_root('emscripten-revision.txt')).strip()
    revision_suffix = ' (%s)' % rev
  return f'emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) {utils.EMSCRIPTEN_VERSION}{revision_suffix}'


def parse_args(newargs):
  options = EmccOptions()
  settings_changes = []
  user_js_defines = []
  should_exit = False
  skip = False

  for i in range(len(newargs)):
    if skip:
      skip = False
      continue

    # Support legacy '--bind' flag, by mapping to `-lembind` which now
    # has the same effect
    if newargs[i] == '--bind':
      newargs[i] = '-lembind'

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
      requested_level = removeprefix(arg, '-O') or '2'
      if requested_level == 's':
        requested_level = 2
        settings.SHRINK_LEVEL = 1
      elif requested_level == 'z':
        requested_level = 2
        settings.SHRINK_LEVEL = 2
      elif requested_level == 'g':
        requested_level = 1
        settings.SHRINK_LEVEL = 0
        settings.DEBUG_LEVEL = max(settings.DEBUG_LEVEL, 1)
      elif requested_level == 'fast':
        # TODO(https://github.com/emscripten-core/emscripten/issues/21497):
        # If we ever map `-ffast-math` to `wasm-opt --fast-math` then
        # then we should enable that too here.
        requested_level = 3
        settings.SHRINK_LEVEL = 0
      else:
        settings.SHRINK_LEVEL = 0
      settings.OPT_LEVEL = validate_arg_level(requested_level, 3, 'invalid optimization level: ' + arg, clamp=True)
    elif check_arg('--js-opts'):
      logger.warning('--js-opts ignored when using llvm backend')
      consume_arg()
    elif check_arg('--llvm-opts'):
      diagnostics.warning('deprecated', '--llvm-opts is deprecated.  All non-emcc args are passed through to clang.')
    elif arg.startswith('-flto'):
      if '=' in arg:
        settings.LTO = arg.split('=')[1]
      else:
        settings.LTO = 'full'
    elif arg == "-fno-lto":
      settings.LTO = 0
    elif arg == "--save-temps":
      options.save_temps = True
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
    elif check_arg('--reproduce'):
      options.reproduce = consume_arg()
    elif check_arg('--pre-js'):
      options.pre_js.append(consume_arg_file())
    elif check_arg('--post-js'):
      options.post_js.append(consume_arg_file())
    elif check_arg('--extern-pre-js'):
      options.extern_pre_js.append(consume_arg_file())
    elif check_arg('--extern-post-js'):
      options.extern_post_js.append(consume_arg_file())
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
      options.no_minify = True
    elif arg.startswith('-g'):
      options.requested_debug = arg
      requested_level = removeprefix(arg, '-g') or '3'
      if is_int(requested_level):
        # the -gX value is the debug level (-g1, -g2, etc.)
        settings.DEBUG_LEVEL = validate_arg_level(requested_level, 4, 'invalid debug level: ' + arg)
        # if we don't need to preserve LLVM debug info, do not keep this flag
        # for clang
        if settings.DEBUG_LEVEL < 3:
          newargs[i] = '-g0'
        else:
          # for 3+, report -g3 to clang as -g4 etc. are not accepted
          newargs[i] = '-g3'
          if settings.DEBUG_LEVEL == 3:
            settings.GENERATE_DWARF = 1
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
          settings.GENERATE_DWARF = 1
        elif requested_level == 'source-map':
          settings.GENERATE_SOURCE_MAP = 1
          newargs[i] = '-g'
        else:
          # Other non-integer levels (e.g. -gline-tables-only or -gdwarf-5) are
          # usually clang flags that emit DWARF. So we pass them through to
          # clang and make the emscripten code treat it like any other DWARF.
          settings.GENERATE_DWARF = 1
        # In all cases set the emscripten debug level to 3 so that we do not
        # strip during link (during compile, this does not make a difference).
        settings.DEBUG_LEVEL = 3
    elif check_flag('-profiling') or check_flag('--profiling'):
      settings.DEBUG_LEVEL = max(settings.DEBUG_LEVEL, 2)
    elif check_flag('-profiling-funcs') or check_flag('--profiling-funcs'):
      settings.EMIT_NAME_SECTION = 1
    elif newargs[i] == '--tracing' or newargs[i] == '--memoryprofiler':
      if newargs[i] == '--memoryprofiler':
        options.memory_profiler = True
      newargs[i] = ''
      settings_changes.append('EMSCRIPTEN_TRACING=1')
      settings.JS_LIBRARIES.append((0, 'library_trace.js'))
    elif check_flag('--emit-symbol-map'):
      options.emit_symbol_map = True
      settings.EMIT_SYMBOL_MAP = 1
    elif check_arg('--emit-minification-map'):
      settings.MINIFICATION_MAP = consume_arg()
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
      shared.PRINT_SUBPROCS = True
    elif arg == '-###':
      shared.SKIP_SUBPROCS = True
    elif check_arg('--shell-file'):
      options.shell_path = consume_arg_file()
    elif check_arg('--source-map-base'):
      options.source_map_base = consume_arg()
    elif check_arg('--embind-emit-tsd'):
      diagnostics.warning('deprecated', '--embind-emit-tsd is deprecated.  Use --emit-tsd instead.')
      options.emit_tsd = consume_arg()
    elif check_arg('--emit-tsd'):
      options.emit_tsd = consume_arg()
    elif check_flag('--no-entry'):
      options.no_entry = True
    elif check_arg('--js-library'):
      settings.JS_LIBRARIES.append((i + 1, os.path.abspath(consume_arg_file())))
    elif check_flag('--remove-duplicates'):
      diagnostics.warning('legacy-settings', '--remove-duplicates is deprecated as it is no longer needed. If you cannot link without it, file a bug with a testcase')
    elif check_flag('--jcache'):
      logger.error('jcache is no longer supported')
    elif check_arg('--cache'):
      config.CACHE = os.path.abspath(consume_arg())
      cache.setup()
      # Ensure child processes share the same cache (e.g. when using emcc to compiler system
      # libraries)
      os.environ['EM_CACHE'] = config.CACHE
    elif check_flag('--clear-cache'):
      logger.info('clearing cache as requested by --clear-cache: `%s`', cache.cachedir)
      cache.erase()
      shared.perform_sanity_checks() # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--clear-ports'):
      logger.info('clearing ports and cache as requested by --clear-ports')
      ports.clear()
      cache.erase()
      shared.perform_sanity_checks() # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--check'):
      print(version_string(), file=sys.stderr)
      shared.check_sanity(force=True)
      should_exit = True
    elif check_flag('--show-ports'):
      ports.show_ports()
      should_exit = True
    elif check_arg('--memory-init-file'):
      exit_with_error('--memory-init-file is no longer supported')
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
      settings_changes.append('PTHREADS_PROFILING=1')
    elif arg == '-fno-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
      settings.DISABLE_EXCEPTION_THROWING = 1
      settings.WASM_EXCEPTIONS = 0
    elif arg == '-mbulk-memory':
      settings.BULK_MEMORY = 1
    elif arg == '-mno-bulk-memory':
      settings.BULK_MEMORY = 0
    elif arg == '-fexceptions':
      # TODO Currently -fexceptions only means Emscripten EH. Switch to wasm
      # exception handling by default when -fexceptions is given when wasm
      # exception handling becomes stable.
      settings.DISABLE_EXCEPTION_THROWING = 0
      settings.DISABLE_EXCEPTION_CATCHING = 0
    elif arg == '-fwasm-exceptions':
      settings.WASM_EXCEPTIONS = 1
    elif arg == '-fignore-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
    elif check_arg('--default-obj-ext'):
      exit_with_error('--default-obj-ext is no longer supported by emcc')
    elif arg.startswith('-fsanitize=cfi'):
      exit_with_error('emscripten does not currently support -fsanitize=cfi')
    elif check_arg('--output_eol'):
      style = consume_arg()
      if style.lower() == 'windows':
        options.output_eol = '\r\n'
      elif style.lower() == 'linux':
        options.output_eol = '\n'
      else:
        exit_with_error(f'Invalid value "{style}" to --output_eol!')
    # Record PTHREADS setting because it controls whether --shared-memory is passed to lld
    elif arg == '-pthread':
      settings.PTHREADS = 1
      # Also set the legacy setting name, in case use JS code depends on it.
      settings.USE_PTHREADS = 1
    elif arg == '-no-pthread':
      settings.PTHREADS = 0
      # Also set the legacy setting name, in case use JS code depends on it.
      settings.USE_PTHREADS = 0
    elif arg == '-pthreads':
      exit_with_error('unrecognized command-line option `-pthreads`; did you mean `-pthread`?')
    elif arg in ('-fno-diagnostics-color', '-fdiagnostics-color=never'):
      colored_logger.disable()
      diagnostics.color_enabled = False
    elif arg == '-fno-rtti':
      settings.USE_RTTI = 0
    elif arg == '-frtti':
      settings.USE_RTTI = 1
    elif arg.startswith('-jsD'):
      key = removeprefix(arg, '-jsD')
      if '=' in key:
        key, value = key.split('=')
      else:
        value = '1'
      if key in settings.keys():
        exit_with_error(f'{arg}: cannot change built-in settings values with a -jsD directive. Pass -s{key}={value} instead!')
      user_js_defines += [(key, value)]
      newargs[i] = ''
    elif check_flag('-shared'):
      options.shared = True
    elif check_flag('-r'):
      options.relocatable = True
    elif check_arg('-o'):
      options.output_file = consume_arg()
    elif arg.startswith('-o'):
      options.output_file = removeprefix(arg, '-o')
      newargs[i] = ''
    elif check_arg('-target') or check_arg('--target'):
      options.target = consume_arg()
      if options.target not in ('wasm32', 'wasm64', 'wasm64-unknown-emscripten', 'wasm32-unknown-emscripten'):
        exit_with_error(f'unsupported target: {options.target} (emcc only supports wasm64-unknown-emscripten and wasm32-unknown-emscripten)')
    elif check_arg('--use-port'):
      ports.handle_use_port_arg(settings, consume_arg())
    elif arg == '-mllvm':
      # Ignore the next argument rather than trying to parse it.  This is needed
      # because llvm args could, for example, start with `-o` and we don't want
      # to confuse that with a normal `-o` flag.
      skip = True

  if should_exit:
    sys.exit(0)

  newargs = [a for a in newargs if a]
  return options, settings_changes, user_js_defines, newargs


def is_valid_abspath(options, path_name):
  # Any path that is underneath the emscripten repository root must be ok.
  if utils.normalize_path(path_name).startswith(utils.normalize_path(utils.path_from_root())):
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
  return [v.strip() for v in values if not v.startswith('#')]


def parse_value(text, expected_type):
  # Note that using response files can introduce whitespace, if the file
  # has a newline at the end. For that reason, we rstrip() in relevant
  # places here.
  def parse_string_value(text):
    first = text[0]
    if first == "'" or first == '"':
      text = text.rstrip()
      if text[-1] != text[0] or len(text) < 2:
         raise ValueError(f'unclosed quoted string. expected final character to be "{text[0]}" and length to be greater than 1 in "{text[0]}"')
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
        raise ValueError('empty value in string list')
      first = current[0]
      if not (first == "'" or first == '"'):
        result.append(current.rstrip())
      else:
        start = index
        while True: # Continue until closing quote found
          if index >= len(values):
            raise ValueError(f"unclosed quoted string. expected final character to be '{first}' in '{values[start]}'")
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
        raise ValueError('unterminated string list. expected final character to be "]"')
      text = text[1:-1]
    if text.strip() == "":
      return []
    return parse_string_list_members(text)

  if expected_type == list or (text and text[0] == '['):
    # if json parsing fails, we fall back to our own parser, which can handle a few
    # simpler syntaxes
    try:
      parsed = json.loads(text)
    except ValueError:
      return parse_string_list(text)

    # if we succeeded in parsing as json, check some properties of it before returning
    if type(parsed) not in (str, list):
      raise ValueError(f'settings must be strings or lists (not ${type(parsed)})')
    if type(parsed) is list:
      for elem in parsed:
        if type(elem) is not str:
          raise ValueError(f'list members in settings must be strings (not ${type(elem)})')

    return parsed

  if expected_type == float:
    try:
      return float(text)
    except ValueError:
      pass

  try:
    if text.startswith('0x'):
      base = 16
    else:
      base = 10
    return int(text, base)
  except ValueError:
    return parse_string_value(text)


def validate_arg_level(level_string, max_level, err_msg, clamp=False):
  try:
    level = int(level_string)
  except ValueError:
    exit_with_error(err_msg)
  if clamp:
    if level > max_level:
      logger.warning("optimization level '-O" + level_string + "' is not supported; using '-O" + str(max_level) + "' instead")
      level = max_level
  if not 0 <= level <= max_level:
    exit_with_error(err_msg)
  return level


def is_int(s):
  try:
    int(s)
    return True
  except ValueError:
    return False


@ToolchainProfiler.profile()
def main(args):
  start_time = time.time()
  ret = run(args)
  logger.debug('total time: %.2f seconds', (time.time() - start_time))
  return ret


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.debug('KeyboardInterrupt')
    sys.exit(1)
