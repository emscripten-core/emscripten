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

import logging
import os
import shlex
import shutil
import sys
import time
import tarfile
from enum import Enum, auto, unique
from subprocess import PIPE


from tools import shared, system_libs, utils, ports, cmdline
from tools import diagnostics, building
from tools.shared import unsuffixed, unsuffixed_basename, get_file_suffix
from tools.shared import run_process, exit_with_error, DEBUG
from tools.shared import in_temp
from tools.shared import DYLIB_EXTENSIONS
from tools.cmdline import SIMD_INTEL_FEATURE_TOWER, SIMD_NEON_FLAGS, CLANG_FLAGS_WITH_ARGS
from tools.response_file import substitute_response_files
from tools import config
from tools import cache
from tools.settings import default_setting, user_settings, settings, COMPILE_TIME_SETTINGS
from tools.utils import read_file, memoize

logger = logging.getLogger('emcc')

# In git checkouts of emscripten `bootstrap.py` exists to run post-checkout
# steps.  In packaged versions (e.g. emsdk) this file does not exist (because
# it is excluded in tools/install.py) and these steps are assumed to have been
# run already.
if os.path.exists(utils.path_from_root('.git')) and os.path.exists(utils.path_from_root('bootstrap.py')):
  import bootstrap
  bootstrap.check()

PREPROCESSED_EXTENSIONS = {'.i', '.ii'}
ASSEMBLY_EXTENSIONS = {'.s'}
HEADER_EXTENSIONS = {'.h', '.hxx', '.hpp', '.hh', '.H', '.HXX', '.HPP', '.HH'}
SOURCE_EXTENSIONS = {
  '.c', '.i', # C
  '.cppm', '.pcm', '.cpp', '.cxx', '.cc', '.c++', '.CPP', '.CXX', '.C', '.CC', '.C++', '.ii', # C++
  '.m', '.mi', '.mm', '.mii', # ObjC/ObjC++
  '.bc', '.ll', # LLVM IR
  '.S', # asm with preprocessor
  os.devnull, # consider the special endingless filenames like /dev/null to be C
} | PREPROCESSED_EXTENSIONS

LINK_ONLY_FLAGS = {
    '--bind', '--closure', '--cpuprofiler', '--embed-file',
    '--emit-symbol-map', '--emrun', '--exclude-file', '--extern-post-js',
    '--extern-pre-js', '--ignore-dynamic-linking', '--js-library',
    '--js-transform', '--oformat', '--output_eol', '--output-eol',
    '--post-js', '--pre-js', '--preload-file', '--profiling-funcs',
    '--proxy-to-worker', '--shell-file', '--source-map-base',
    '--threadprofiler', '--use-preload-plugins',
}


@unique
class Mode(Enum):
  # Used any time we are not linking, including PCH, pre-processing, etc
  COMPILE_ONLY = auto()
  # Only when --post-link is specified
  POST_LINK_ONLY = auto()
  # This is the default mode, in the absence of any flags such as -c, -E, etc
  COMPILE_AND_LINK = auto()


class LinkFlag:
  """Used to represent a linker flag.

  The flag value is stored along with a bool that distingingishes input
  files from non-files.

  A list of these is return by separate_linker_flags.
  """
  def __init__(self, value, is_file):
    self.value = value
    self.is_file = is_file


class EmccState:
  def __init__(self, args):
    self.mode = Mode.COMPILE_AND_LINK
    # Using tuple here to prevent accidental mutation
    self.orig_args = tuple(args)


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

          if len(arg) > 2 and arg.startswith('-o'):
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

          if arg in CLANG_FLAGS_WITH_ARGS:
            ignore_next = True

          if arg == '-o':
            output_arg = True

      reproduce_file.add(rsp_name, os.path.join(root, 'response.txt'))


def cxx_to_c_compiler(cxx):
  # Convert C++ compiler name into C compiler name
  dirname, basename = os.path.split(cxx)
  basename = basename.replace('clang++', 'clang').replace('g++', 'gcc').replace('em++', 'emcc')
  return os.path.join(dirname, basename)


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

  if settings.PTHREADS:
    if '-pthread' not in user_args:
      flags.append('-pthread')
  elif settings.SHARED_MEMORY:
    if '-matomics' not in user_args:
      flags.append('-matomics')
    if '-mbulk-memory' not in user_args:
      flags.append('-mbulk-memory')

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


@memoize
def get_cflags(user_args):
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

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[8:]):
    cflags += ['-D__AVX2__=1']

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
    print(cmdline.version_string(), file=sys.stderr)
    return shared.check_call([clang, '-v'] + get_target_flags(), check=False).returncode

  # Additional compiler flags that we treat as if they were passed to us on the
  # commandline
  if EMCC_CFLAGS := os.environ.get('EMCC_CFLAGS'):
    args += shlex.split(EMCC_CFLAGS)

  if DEBUG:
    logger.warning(f'invocation: {shlex.join(args)} (in {os.getcwd()})')

  # Strip args[0] (program name)
  args = args[1:]

  # Handle some global flags

  # read response files very early on
  try:
    args = substitute_response_files(args)
  except OSError as e:
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
  options, newargs = cmdline.parse_arguments(state.orig_args)

  if not shared.SKIP_SUBPROCS:
    shared.check_sanity()

  # Begin early-exit flag handling.

  if '--version' in args:
    print(cmdline.version_string())
    print('''\
Copyright (C) 2025 the Emscripten authors (see AUTHORS.txt)
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
      print(shlex.join(parts[1:]))
    return 0

  if '-dumpmachine' in args or '-print-target-triple' in args or '--print-target-triple' in args:
    print(shared.get_llvm_target())
    return 0

  if '-print-search-dirs' in args or '--print-search-dirs' in args:
    print(f'programs: ={config.LLVM_ROOT}')
    print(f'libraries: ={cache.get_lib_dir(absolute=True)}')
    return 0

  if '-print-resource-dir' in args:
    shared.check_call([clang] + args)
    return 0

  if '-print-libgcc-file-name' in args or '--print-libgcc-file-name' in args:
    settings.limit_settings(None)
    compiler_rt = system_libs.Library.get_usable_variations()['libcompiler_rt']
    print(compiler_rt.get_path(absolute=True))
    return 0

  print_file_name = [a for a in args if a.startswith(('-print-file-name=', '--print-file-name='))]
  if print_file_name:
    libname = print_file_name[-1].split('=')[1]
    system_libpath = cache.get_lib_dir(absolute=True)
    fullpath = os.path.join(system_libpath, libname)
    if os.path.isfile(fullpath):
      print(fullpath)
    else:
      print(libname)
    return 0

  # End early-exit flag handling

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

  phase_setup(options, state)

  if options.reproduce:
    create_reproduce_file(options.reproduce, args)

  if state.mode == Mode.POST_LINK_ONLY:
    if len(options.input_files) != 1:
      exit_with_error('--post-link requires a single input file')
    linker_args = separate_linker_flags(newargs)[1]
    linker_args = [f.value for f in linker_args]
    # Delay import of link.py to avoid processing this file when only compiling
    from tools import link
    link.run_post_link(options.input_files[0], options, linker_args)
    return 0

  # Compile source code to object files
  # When only compiling this function never returns.
  linker_args = phase_compile_inputs(options, state, newargs)

  if state.mode == Mode.COMPILE_AND_LINK:
    # Delay import of link.py to avoid processing this file when only compiling
    from tools import link
    return link.run(options, linker_args)
  else:
    logger.debug('stopping after compile phase')
    return 0


def separate_linker_flags(newargs):
  """Process argument list separating out compiler args and linker args.

  - Linker flags include input files and are returned a list of LinkFlag objects.
  - Compiler flags are those to be passed to `clang -c`.
  """

  if settings.RUNTIME_LINKED_LIBS:
    newargs += settings.RUNTIME_LINKED_LIBS

  compiler_args = []
  linker_args = []

  def add_link_arg(flag, is_file=False):
    linker_args.append(LinkFlag(flag, is_file))

  skip = False
  for i in range(len(newargs)):
    if skip:
      skip = False
      continue

    arg = newargs[i]
    if arg in CLANG_FLAGS_WITH_ARGS:
      skip = True

    def get_next_arg():
      if len(newargs) <= i + 1:
        exit_with_error(f"option '{arg}' requires an argument")
      return newargs[i + 1]

    if not arg.startswith('-') or arg == '-':
      # os.devnul should always be reported as existing but there is bug in windows
      # python before 3.8:
      # https://bugs.python.org/issue1311
      if not os.path.exists(arg) and arg not in (os.devnull, '-'):
        exit_with_error('%s: No such file or directory ("%s" was expected to be an input file, based on the commandline arguments provided)', arg, arg)
      add_link_arg(arg, True)
    elif arg == '-z':
      add_link_arg(arg)
      add_link_arg(get_next_arg())
    elif arg.startswith('-Wl,'):
      for flag in arg.split(',')[1:]:
        add_link_arg(flag)
    elif arg == '-Xlinker':
      add_link_arg(get_next_arg())
    elif arg == '-s' or arg.startswith(('-l', '-L', '--js-library=', '-z', '-u')):
      add_link_arg(arg)
    elif not arg.startswith('-o') and arg not in ('-nostdlib', '-nostartfiles', '-nolibc', '-nodefaultlibs', '-s'):
      # All other flags are for the compiler
      compiler_args.append(arg)
      if skip:
        compiler_args.append(get_next_arg())

  return compiler_args, linker_args


@ToolchainProfiler.profile_block('setup')
def phase_setup(options, state):
  """Second phase: configure and setup the compiler based on the specified settings and arguments.
  """

  has_header_inputs = any(get_file_suffix(f) in HEADER_EXTENSIONS for f in options.input_files)

  if options.post_link:
    state.mode = Mode.POST_LINK_ONLY
  elif has_header_inputs or options.dash_c or options.dash_S or options.syntax_only or options.dash_E or options.dash_M:
    state.mode = Mode.COMPILE_ONLY

  if state.mode == Mode.COMPILE_ONLY:
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

  if settings.MEMORY64 and options.target.startswith('wasm32'):
    exit_with_error('wasm32 target is not compatible with -sMEMORY64')

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


@ToolchainProfiler.profile_block('compile inputs')
def phase_compile_inputs(options, state, newargs):
  if shared.run_via_emxx:
    compiler = [shared.CLANG_CXX]
  else:
    compiler = [shared.CLANG_CC]

  if config.COMPILER_WRAPPER:
    logger.debug('using compiler wrapper: %s', config.COMPILER_WRAPPER)
    compiler.insert(0, config.COMPILER_WRAPPER)

  system_libs.ensure_sysroot()

  def get_clang_command():
    return compiler + get_cflags(state.orig_args)

  def get_clang_command_preprocessed():
    return compiler + get_clang_flags(state.orig_args)

  def get_clang_command_asm():
    return compiler + get_target_flags()

  if state.mode == Mode.COMPILE_ONLY:
    if options.output_file and get_file_suffix(options.output_file) == '.bc' and not settings.LTO and '-emit-llvm' not in state.orig_args:
      diagnostics.warning('emcc', '.bc output file suffix used without -flto or -emit-llvm.  Consider using .o extension since emcc will output an object file, not a bitcode file')
    if all(get_file_suffix(i) in ASSEMBLY_EXTENSIONS for i in options.input_files):
      cmd = get_clang_command_asm() + newargs
    else:
      cmd = get_clang_command() + newargs
    shared.exec_process(cmd)
    assert False, 'exec_process does not return'

  # In COMPILE_AND_LINK we need to compile source files too, but we also need to
  # filter out the link flags
  assert state.mode == Mode.COMPILE_AND_LINK
  assert not options.dash_c
  compile_args, linker_args = separate_linker_flags(newargs)
  seen_names = {}

  def uniquename(name):
    if name not in seen_names:
      seen_names[name] = str(len(seen_names))
    return unsuffixed(name) + '_' + seen_names[name] + shared.suffix(name)

  def get_object_filename(input_file):
    return in_temp(shared.replace_suffix(uniquename(input_file), '.o'))

  def compile_source_file(input_file):
    logger.debug(f'compiling source file: {input_file}')
    output_file = get_object_filename(input_file)
    if get_file_suffix(input_file) in ASSEMBLY_EXTENSIONS:
      cmd = get_clang_command_asm()
    elif get_file_suffix(input_file) in PREPROCESSED_EXTENSIONS:
      cmd = get_clang_command_preprocessed()
    else:
      cmd = get_clang_command()
      if get_file_suffix(input_file) in ['.pcm']:
        cmd = [c for c in cmd if not c.startswith('-fprebuilt-module-path=')]
    cmd += compile_args + ['-c', input_file, '-o', output_file]
    if options.requested_debug == '-gsplit-dwarf':
      # When running in COMPILE_AND_LINK mode we compile objects to a temporary location
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
    return output_file

  # Compile input files individually to temporary locations.
  for arg in linker_args:
    if not arg.is_file:
      continue
    input_file = arg.value
    file_suffix = get_file_suffix(input_file)
    if file_suffix in SOURCE_EXTENSIONS | ASSEMBLY_EXTENSIONS or (options.dash_c and file_suffix == '.bc'):
      arg.value = compile_source_file(input_file)
    elif file_suffix in DYLIB_EXTENSIONS:
      logger.debug(f'using shared library: {input_file}')
    elif building.is_ar(input_file):
      logger.debug(f'using static library: {input_file}')
    elif options.input_language:
      arg.value = compile_source_file(input_file)
    elif input_file == '-':
      exit_with_error('-E or -x required when input is from standard input')
    else:
      # Default to assuming the inputs are object files and pass them to the linker
      pass

  return [f.value for f in linker_args]


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
