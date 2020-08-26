# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function

import atexit
import json
import logging
import multiprocessing
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from subprocess import STDOUT, PIPE

from . import diagnostics
from . import response_file
from . import shared
from .toolchain_profiler import ToolchainProfiler
from .shared import Settings, CLANG_CC, CLANG_CXX, PYTHON
from .shared import LLVM_NM, EMCC, EMAR, EMXX, EMRANLIB, NODE_JS, WASM_LD, LLVM_AR
from .shared import JS, LLVM_OPT, LLVM_LINK, LLVM_DIS, LLVM_AS, LLVM_OBJCOPY
from .shared import try_delete, run_process, check_call, exit_with_error
from .shared import safe_move, configuration, path_from_root, EXPECTED_BINARYEN_VERSION
from .shared import asmjs_mangle, DEBUG, WINDOWS, JAVA, CLOSURE_COMPILER, EM_CONFIG
from .shared import EM_BUILD_VERBOSE, TEMP_DIR, print_compiler_stage, BINARYEN_ROOT
from .shared import CANONICAL_TEMP_DIR, LLVM_DWARFDUMP, demangle_c_symbol_name, asbytes
from .shared import get_emscripten_temp_dir, exe_suffix, WebAssembly, which, is_c_symbol

logger = logging.getLogger('building')

#  Building
multiprocessing_pool = None
binaryen_checked = False

# internal caches
internal_nm_cache = {}
# cache results of nm - it can be slow to run
uninternal_nm_cache = {}
# Stores the object files contained in different archive files passed as input
ar_contents = {}
_is_ar_cache = {}
# the exports the user requested
user_requested_exports = []


class ObjectFileInfo(object):
  def __init__(self, returncode, output, defs=set(), undefs=set(), commons=set()):
    self.returncode = returncode
    self.output = output
    self.defs = defs
    self.undefs = undefs
    self.commons = commons

  def is_valid_for_nm(self):
    return self.returncode == 0


# llvm-ar appears to just use basenames inside archives. as a result, files
# with the same basename will trample each other when we extract them. to help
# warn of such situations, we warn if there are duplicate entries in the
# archive
def warn_if_duplicate_entries(archive_contents, archive_filename):
  if len(archive_contents) != len(set(archive_contents)):
    msg = '%s: archive file contains duplicate entries. This is not supported by emscripten. Only the last member with a given name will be linked in which can result in undefined symbols. You should either rename your source files, or use `emar` to create you archives which works around this issue.' % archive_filename
    warned = set()
    for i in range(len(archive_contents)):
      curr = archive_contents[i]
      if curr not in warned and curr in archive_contents[i + 1:]:
        msg += '\n   duplicate: %s' % curr
        warned.add(curr)
    diagnostics.warning('emcc', msg)


# This function creates a temporary directory specified by the 'dir' field in
# the returned dictionary. Caller is responsible for cleaning up those files
# after done.
def extract_archive_contents(archive_file):
  lines = run_process([LLVM_AR, 't', archive_file], stdout=PIPE).stdout.splitlines()
  # ignore empty lines
  contents = [l for l in lines if len(l)]
  if len(contents) == 0:
    logger.debug('Archive %s appears to be empty (recommendation: link an .so instead of .a)' % archive_file)
    return {
      'returncode': 0,
      'dir': None,
      'files': []
    }

  # `ar` files can only contains filenames. Just to be sure,  verify that each
  # file has only as filename component and is not absolute
  for f in contents:
    assert not os.path.dirname(f)
    assert not os.path.isabs(f)

  warn_if_duplicate_entries(contents, archive_file)

  # create temp dir
  temp_dir = tempfile.mkdtemp('_archive_contents', 'emscripten_temp_')

  # extract file in temp dir
  proc = run_process([LLVM_AR, 'xo', archive_file], stdout=PIPE, stderr=STDOUT, cwd=temp_dir)
  abs_contents = [os.path.join(temp_dir, c) for c in contents]

  # check that all files were created
  missing_contents = [x for x in abs_contents if not os.path.exists(x)]
  if missing_contents:
    exit_with_error('llvm-ar failed to extract file(s) ' + str(missing_contents) + ' from archive file ' + f + '! Error:' + str(proc.stdout))

  return {
    'returncode': proc.returncode,
    'dir': temp_dir,
    'files': abs_contents
  }


# Due to a python pickling issue, the following two functions must be at top
# level, or multiprocessing pool spawn won't find them.
def g_llvm_nm_uncached(filename):
  return llvm_nm_uncached(filename)


def g_multiprocessing_initializer(*args):
  for item in args:
    (key, value) = item.split('=', 1)
    if key == 'EMCC_POOL_CWD':
      os.chdir(value)
    else:
      os.environ[key] = value


def unique_ordered(values):
  """return a list of unique values in an input list, without changing order
  (list(set(.)) would change order randomly).
  """
  seen = set()

  def check(value):
    if value in seen:
      return False
    seen.add(value)
    return True

  return list(filter(check, values))


# clear internal caches. this is not normally needed, except if the clang/LLVM
# used changes inside this invocation of Building, which can happen in the benchmarker
# when it compares different builds.
def clear():
  internal_nm_cache.clear()
  uninternal_nm_cache.clear()
  ar_contents.clear()
  _is_ar_cache.clear()


def get_num_cores():
  return int(os.environ.get('EMCC_CORES', multiprocessing.cpu_count()))


# Multiprocessing pools are very slow to build up and tear down, and having
# several pools throughout the application has a problem of overallocating
# child processes. Therefore maintain a single centralized pool that is shared
# between all pooled task invocations.
def get_multiprocessing_pool():
  global multiprocessing_pool
  if not multiprocessing_pool:
    cores = get_num_cores()
    if DEBUG:
      # When in EMCC_DEBUG mode, only use a single core in the pool, so that
      # logging is not all jumbled up.
      cores = 1

    # If running with one core only, create a mock instance of a pool that does not
    # actually spawn any new subprocesses. Very useful for internal debugging.
    if cores == 1:
      class FakeMultiprocessor(object):
        def map(self, func, tasks, *args, **kwargs):
          results = []
          for t in tasks:
            results += [func(t)]
          return results

        def map_async(self, func, tasks, *args, **kwargs):
          class Result:
            def __init__(self, func, tasks):
              self.func = func
              self.tasks = tasks

            def get(self, timeout):
              results = []
              for t in tasks:
                results += [func(t)]
              return results

          return Result(func, tasks)

      multiprocessing_pool = FakeMultiprocessor()
    else:
      child_env = [
        # Multiprocessing pool children must have their current working
        # directory set to a safe path that is guaranteed not to die in
        # between of executing commands, or otherwise the pool children will
        # have trouble spawning subprocesses of their own.
        'EMCC_POOL_CWD=' + path_from_root(),
        # Multiprocessing pool children can't spawn their own linear number of
        # children, that could cause a quadratic amount of spawned processes.
        'EMCC_CORES=1'
      ]
      multiprocessing_pool = multiprocessing.Pool(processes=cores, initializer=g_multiprocessing_initializer, initargs=child_env)

      def close_multiprocessing_pool():
        global multiprocessing_pool
        try:
          # Shut down the pool explicitly, because leaving that for Python to do at process shutdown is buggy and can generate
          # noisy "WindowsError: [Error 5] Access is denied" spam which is not fatal.
          multiprocessing_pool.terminate()
          multiprocessing_pool.join()
          multiprocessing_pool = None
        except OSError as e:
          # Mute the "WindowsError: [Error 5] Access is denied" errors, raise all others through
          if not (sys.platform.startswith('win') and isinstance(e, WindowsError) and e.winerror == 5):
            raise
      atexit.register(close_multiprocessing_pool)

  return multiprocessing_pool


# .. but for Popen, we cannot have doublequotes, so provide functionality to
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


def get_building_env(cflags=[]):
  env = os.environ.copy()
  # point CC etc. to the em* tools.
  env['CC'] = EMCC
  env['CXX'] = EMXX
  env['AR'] = EMAR
  env['LD'] = EMCC
  env['NM'] = LLVM_NM
  env['LDSHARED'] = EMCC
  env['RANLIB'] = EMRANLIB
  env['EMSCRIPTEN_TOOLS'] = path_from_root('tools')
  if cflags:
    env['CFLAGS'] = env['EMMAKEN_CFLAGS'] = ' '.join(cflags)
  env['HOST_CC'] = CLANG_CC
  env['HOST_CXX'] = CLANG_CXX
  env['HOST_CFLAGS'] = "-W" # if set to nothing, CFLAGS is used, which we don't want
  env['HOST_CXXFLAGS'] = "-W" # if set to nothing, CXXFLAGS is used, which we don't want
  env['PKG_CONFIG_LIBDIR'] = path_from_root('system', 'local', 'lib', 'pkgconfig') + os.path.pathsep + path_from_root('system', 'lib', 'pkgconfig')
  env['PKG_CONFIG_PATH'] = os.environ.get('EM_PKG_CONFIG_PATH', '')
  env['EMSCRIPTEN'] = path_from_root()
  env['PATH'] = path_from_root('system', 'bin') + os.pathsep + env['PATH']
  env['CROSS_COMPILE'] = path_from_root('em') # produces /path/to/emscripten/em , which then can have 'cc', 'ar', etc appended to it
  return env


# Returns a clone of the given environment with all directories that contain
# sh.exe removed from the PATH.  Used to work around CMake limitation with
# MinGW Makefiles, where sh.exe is not allowed to be present.
def remove_sh_exe_from_path(env):
  env = env.copy()
  if not WINDOWS:
    return env
  path = env['PATH'].split(';')
  path = [p for p in path if not os.path.exists(os.path.join(p, 'sh.exe'))]
  env['PATH'] = ';'.join(path)
  return env


def handle_cmake_toolchain(args, env):
  def has_substr(args, substr):
    return any(substr in s for s in args)

  # Append the Emscripten toolchain file if the user didn't specify one.
  if not has_substr(args, '-DCMAKE_TOOLCHAIN_FILE'):
    args.append('-DCMAKE_TOOLCHAIN_FILE=' + path_from_root('cmake', 'Modules', 'Platform', 'Emscripten.cmake'))
  node_js = NODE_JS

  if not has_substr(args, '-DCMAKE_CROSSCOMPILING_EMULATOR'):
    node_js = NODE_JS[0].replace('"', '\"')
    args.append('-DCMAKE_CROSSCOMPILING_EMULATOR="%s"' % node_js)

  # On Windows specify MinGW Makefiles or ninja if we have them and no other
  # toolchain was specified, to keep CMake from pulling in a native Visual
  # Studio, or Unix Makefiles.
  if WINDOWS and '-G' not in args:
    if which('mingw32-make'):
      args += ['-G', 'MinGW Makefiles']
    elif which('ninja'):
      args += ['-G', 'Ninja']

  # CMake has a requirement that it wants sh.exe off PATH if MinGW Makefiles
  # is being used. This happens quite often, so do this automatically on
  # behalf of the user. See
  # http://www.cmake.org/Wiki/CMake_MinGW_Compiler_Issues
  if WINDOWS and 'MinGW Makefiles' in args:
    env = remove_sh_exe_from_path(env)

  return (args, env)


def configure(args, stdout=None, stderr=None, env=None, cflags=[], **kwargs):
  if env:
    env = env.copy()
  else:
    env = get_building_env(cflags=cflags)
  if 'cmake' in args[0]:
    # Note: EMMAKEN_JUST_CONFIGURE shall not be enabled when configuring with
    #       CMake. This is because CMake does expect to be able to do
    #       config-time builds with emcc.
    args, env = handle_cmake_toolchain(args, env)
  else:
    # When we configure via a ./configure script, don't do config-time
    # compilation with emcc, but instead do builds natively with Clang. This
    # is a heuristic emulation that may or may not work.
    env['EMMAKEN_JUST_CONFIGURE'] = '1'
  if EM_BUILD_VERBOSE >= 2:
    stdout = None
  if EM_BUILD_VERBOSE >= 1:
    stderr = None
  print('configure: ' + ' '.join(args), file=sys.stderr)
  run_process(args, stdout=stdout, stderr=stderr, env=env, **kwargs)


def make(args, stdout=None, stderr=None, env=None, cflags=[], **kwargs):
  if env is None:
    env = get_building_env(cflags=cflags)

  # On Windows prefer building with mingw32-make instead of make, if it exists.
  if WINDOWS:
    if args[0] == 'make':
      mingw32_make = which('mingw32-make')
      if mingw32_make:
        args[0] = mingw32_make

    if 'mingw32-make' in args[0]:
      env = remove_sh_exe_from_path(env)

  # On Windows, run the execution through shell to get PATH expansion and
  # executable extension lookup, e.g. 'sdl2-config' will match with
  # 'sdl2-config.bat' in PATH.
  if EM_BUILD_VERBOSE >= 2:
    stdout = None
  if EM_BUILD_VERBOSE >= 1:
    stderr = None
  print('make: ' + ' '.join(args), file=sys.stderr)
  run_process(args, stdout=stdout, stderr=stderr, env=env, shell=WINDOWS, **kwargs)


def make_paths_absolute(f):
  if f.startswith('-'):  # skip flags
    return f
  else:
    return os.path.abspath(f)


# Runs llvm-nm in parallel for the given list of files.
# The results are populated in uninternal_nm_cache
# multiprocessing_pool: An existing multiprocessing pool to reuse for the operation, or None
# to have the function allocate its own.
def parallel_llvm_nm(files):
  with ToolchainProfiler.profile_block('parallel_llvm_nm'):
    pool = get_multiprocessing_pool()
    object_contents = pool.map(g_llvm_nm_uncached, files)

    for i, file in enumerate(files):
      if object_contents[i].returncode != 0:
        logger.debug('llvm-nm failed on file ' + file + ': return code ' + str(object_contents[i].returncode) + ', error: ' + object_contents[i].output)
      uninternal_nm_cache[file] = object_contents[i]
    return object_contents


def read_link_inputs(files):
  with ToolchainProfiler.profile_block('read_link_inputs'):
    # Before performing the link, we need to look at each input file to determine which symbols
    # each of them provides. Do this in multiple parallel processes.
    archive_names = [] # .a files passed in to the command line to the link
    object_names = [] # .o/.bc files passed in to the command line to the link
    for f in files:
      absolute_path_f = make_paths_absolute(f)

      if absolute_path_f not in ar_contents and is_ar(absolute_path_f):
        archive_names.append(absolute_path_f)
      elif absolute_path_f not in uninternal_nm_cache and is_bitcode(absolute_path_f):
        object_names.append(absolute_path_f)

    # Archives contain objects, so process all archives first in parallel to obtain the object files in them.
    pool = get_multiprocessing_pool()
    object_names_in_archives = pool.map(extract_archive_contents, archive_names)

    def clean_temporary_archive_contents_directory(directory):
      def clean_at_exit():
        try_delete(directory)
      if directory:
        atexit.register(clean_at_exit)

    for n in range(len(archive_names)):
      if object_names_in_archives[n]['returncode'] != 0:
        raise Exception('llvm-ar failed on archive ' + archive_names[n] + '!')
      ar_contents[archive_names[n]] = object_names_in_archives[n]['files']
      clean_temporary_archive_contents_directory(object_names_in_archives[n]['dir'])

    for o in object_names_in_archives:
      for f in o['files']:
        if f not in uninternal_nm_cache:
          object_names.append(f)

    # Next, extract symbols from all object files (either standalone or inside archives we just extracted)
    # The results are not used here directly, but populated to llvm-nm cache structure.
    parallel_llvm_nm(object_names)


def llvm_backend_args():
  # disable slow and relatively unimportant optimization passes
  args = ['-combiner-global-alias-analysis=false']

  # asm.js-style exception handling
  if Settings.DISABLE_EXCEPTION_CATCHING != 1:
    args += ['-enable-emscripten-cxx-exceptions']
  if Settings.DISABLE_EXCEPTION_CATCHING == 2:
    allowed = ','.join(Settings.EXCEPTION_CATCHING_ALLOWED or ['__fake'])
    args += ['-emscripten-cxx-exceptions-allowed=' + allowed]

  # asm.js-style setjmp/longjmp handling
  args += ['-enable-emscripten-sjlj']

  # better (smaller, sometimes faster) codegen, see binaryen#1054
  # and https://bugs.llvm.org/show_bug.cgi?id=39488
  args += ['-disable-lsr']

  return args


def link_to_object(linker_inputs, target):
  # link using lld for the wasm backend with wasm object files,
  # other otherwise for linking of bitcode we must use our python
  # code (necessary for asm.js, for wasm bitcode see
  # https://bugs.llvm.org/show_bug.cgi?id=40654)
  if not Settings.LTO:
    link_lld(linker_inputs + ['--relocatable'], target)
  else:
    link(linker_inputs, target)


def link_llvm(linker_inputs, target):
  # runs llvm-link to link things.
  cmd = [LLVM_LINK] + linker_inputs + ['-o', target]
  cmd = get_command_with_possible_response_file(cmd)
  print_compiler_stage(cmd)
  output = run_process(cmd, stdout=PIPE).stdout
  assert os.path.exists(target) and (output is None or 'Could not open input file' not in output), 'Linking error: ' + output
  return target


def lld_flags_for_executable(external_symbol_list):
  cmd = []
  if external_symbol_list:
    undefs = configuration.get_temp_files().get('.undefined').name
    with open(undefs, 'w') as f:
      f.write('\n'.join(external_symbol_list))
    cmd.append('--allow-undefined-file=%s' % undefs)
  else:
    cmd.append('--allow-undefined')

  # wasi does not import the memory (but for JS it is efficient to do so,
  # as it allows us to set up memory, preload files, etc. even before the
  # wasm module arrives)
  if not Settings.STANDALONE_WASM:
    cmd.append('--import-memory')
    cmd.append('--import-table')
  else:
    cmd.append('--export-table')

  if Settings.USE_PTHREADS:
    cmd.append('--shared-memory')

  if Settings.DEBUG_LEVEL < 2 and (not Settings.EMIT_SYMBOL_MAP and
                                   not Settings.PROFILING_FUNCS and
                                   not Settings.ASYNCIFY):
    cmd.append('--strip-debug')

  if Settings.RELOCATABLE:
    if Settings.MAIN_MODULE == 2 or Settings.SIDE_MODULE == 2:
      cmd.append('--no-export-dynamic')
    else:
      cmd.append('--no-gc-sections')
      cmd.append('--export-dynamic')

  if Settings.LINKABLE:
    cmd.append('--export-all')
  else:
    c_exports = [e for e in Settings.EXPORTED_FUNCTIONS if is_c_symbol(e)]
    # Strip the leading underscores
    c_exports = [demangle_c_symbol_name(e) for e in c_exports]
    if external_symbol_list:
      # Filter out symbols external/JS symbols
      c_exports = [e for e in c_exports if e not in external_symbol_list]
    for export in c_exports:
      cmd += ['--export', export]

  if Settings.RELOCATABLE:
    cmd.append('--experimental-pic')
    if Settings.SIDE_MODULE:
      cmd.append('-shared')
    else:
      cmd.append('-pie')

  if not Settings.SIDE_MODULE:
    cmd += [
      '-z', 'stack-size=%s' % Settings.TOTAL_STACK,
      '--initial-memory=%d' % Settings.INITIAL_MEMORY,
    ]

    if Settings.STANDALONE_WASM:
      # when Settings.EXPECT_MAIN is set we fall back to wasm-ld default of _start
      if not Settings.EXPECT_MAIN:
        cmd += ['--entry=_initialize']
    else:
      if Settings.EXPECT_MAIN and not Settings.IGNORE_MISSING_MAIN:
        cmd += ['--entry=main']
      else:
        cmd += ['--no-entry']
    if not Settings.ALLOW_MEMORY_GROWTH:
      cmd.append('--max-memory=%d' % Settings.INITIAL_MEMORY)
    elif Settings.MAXIMUM_MEMORY != -1:
      cmd.append('--max-memory=%d' % Settings.MAXIMUM_MEMORY)
    if not Settings.RELOCATABLE:
      cmd.append('--global-base=%s' % Settings.GLOBAL_BASE)

  return cmd


def link_lld(args, target, external_symbol_list=None):
  if not os.path.exists(WASM_LD):
    exit_with_error('linker binary not found in LLVM directory: %s', WASM_LD)
  # runs lld to link things.
  # lld doesn't currently support --start-group/--end-group since the
  # semantics are more like the windows linker where there is no need for
  # grouping.
  args = [a for a in args if a not in ('--start-group', '--end-group')]

  # Emscripten currently expects linkable output (SIDE_MODULE/MAIN_MODULE) to
  # include all archive contents.
  if Settings.LINKABLE:
    args.insert(0, '--whole-archive')
    args.append('--no-whole-archive')

  if Settings.STRICT:
    args.append('--fatal-warnings')

  cmd = [WASM_LD, '-o', target] + args
  for a in llvm_backend_args():
    cmd += ['-mllvm', a]

  # For relocatable output (generating an object file) we don't pass any of the
  # normal linker flags that are used when building and exectuable
  if '--relocatable' not in args and '-r' not in args:
    cmd += lld_flags_for_executable(external_symbol_list)

  print_compiler_stage(cmd)
  cmd = get_command_with_possible_response_file(cmd)
  check_call(cmd)
  return target


def link(files, target, force_archive_contents=False, just_calculate=False):
  # "Full-featured" linking: looks into archives (duplicates lld functionality)
  actual_files = []
  # Tracking unresolveds is necessary for .a linking, see below.
  # Specify all possible entry points to seed the linking process.
  # For a simple application, this would just be "main".
  unresolved_symbols = set([func[1:] for func in Settings.EXPORTED_FUNCTIONS])
  resolved_symbols = set()
  # Paths of already included object files from archives.
  added_contents = set()
  has_ar = False
  for f in files:
    if not f.startswith('-'):
      has_ar = has_ar or is_ar(make_paths_absolute(f))

  # If we have only one archive or the force_archive_contents flag is set,
  # then we will add every object file we see, regardless of whether it
  # resolves any undefined symbols.
  force_add_all = len(files) == 1 or force_archive_contents

  # Considers an object file for inclusion in the link. The object is included
  # if force_add=True or if the object provides a currently undefined symbol.
  # If the object is included, the symbol tables are updated and the function
  # returns True.
  def consider_object(f, force_add=False):
    new_symbols = llvm_nm(f)
    # Check if the object was valid according to llvm-nm. It also accepts
    # native object files.
    if not new_symbols.is_valid_for_nm():
      diagnostics.warning('emcc', 'object %s is not valid according to llvm-nm, cannot link', f)
      return False
    # Check the object is valid for us, and not a native object file.
    if not is_bitcode(f):
      exit_with_error('unknown file type: %s', f)
    provided = new_symbols.defs.union(new_symbols.commons)
    do_add = force_add or not unresolved_symbols.isdisjoint(provided)
    if do_add:
      logger.debug('adding object %s to link (forced: %d)' % (f, force_add))
      # Update resolved_symbols table with newly resolved symbols
      resolved_symbols.update(provided)
      # Update unresolved_symbols table by adding newly unresolved symbols and
      # removing newly resolved symbols.
      unresolved_symbols.update(new_symbols.undefs.difference(resolved_symbols))
      unresolved_symbols.difference_update(provided)
      actual_files.append(f)
    return do_add

  # Traverse a single archive. The object files are repeatedly scanned for
  # newly satisfied symbols until no new symbols are found. Returns true if
  # any object files were added to the link.
  def consider_archive(f, force_add):
    added_any_objects = False
    loop_again = True
    logger.debug('considering archive %s' % (f))
    contents = ar_contents[f]
    while loop_again: # repeatedly traverse until we have everything we need
      loop_again = False
      for content in contents:
        if content in added_contents:
          continue
        # Link in the .o if it provides symbols, *or* this is a singleton archive (which is
        # apparently an exception in gcc ld)
        if consider_object(content, force_add=force_add):
          added_contents.add(content)
          loop_again = True
          added_any_objects = True
    logger.debug('done running loop of archive %s' % (f))
    return added_any_objects

  read_link_inputs([x for x in files if not x.startswith('-')])

  # Rescan a group of archives until we don't find any more objects to link.
  def scan_archive_group(group):
    loop_again = True
    logger.debug('starting archive group loop')
    while loop_again:
      loop_again = False
      for archive in group:
        if consider_archive(archive, force_add=False):
          loop_again = True
    logger.debug('done with archive group loop')

  current_archive_group = None
  in_whole_archive = False
  for f in files:
    absolute_path_f = make_paths_absolute(f)
    if f.startswith('-'):
      if f in ['--start-group', '-(']:
        assert current_archive_group is None, 'Nested --start-group, missing --end-group?'
        current_archive_group = []
      elif f in ['--end-group', '-)']:
        assert current_archive_group is not None, '--end-group without --start-group'
        scan_archive_group(current_archive_group)
        current_archive_group = None
      elif f in ['--whole-archive', '-whole-archive']:
        in_whole_archive = True
      elif f in ['--no-whole-archive', '-no-whole-archive']:
        in_whole_archive = False
      else:
        # Command line flags should already be vetted by the time this method
        # is called, so this is an internal error
        assert False, 'unsupported link flag: ' + f
    elif is_ar(absolute_path_f):
      # Extract object files from ar archives, and link according to gnu ld semantics
      # (link in an entire .o from the archive if it supplies symbols still unresolved)
      consider_archive(absolute_path_f, in_whole_archive or force_add_all)
      # If we're inside a --start-group/--end-group section, add to the list
      # so we can loop back around later.
      if current_archive_group is not None:
        current_archive_group.append(absolute_path_f)
    elif is_bitcode(absolute_path_f):
      if has_ar:
        consider_object(f, force_add=True)
      else:
        # If there are no archives then we can simply link all valid object
        # files and skip the symbol table stuff.
        actual_files.append(f)
    else:
      exit_with_error('unknown file type: %s', f)

  # We have to consider the possibility that --start-group was used without a matching
  # --end-group; GNU ld permits this behavior and implicitly treats the end of the
  # command line as having an --end-group.
  if current_archive_group:
    logger.debug('--start-group without matching --end-group, rescanning')
    scan_archive_group(current_archive_group)
    current_archive_group = None

  try_delete(target)

  # Finish link
  # tolerate people trying to link a.so a.so etc.
  actual_files = unique_ordered(actual_files)
  if just_calculate:
    # just calculating; return the link arguments which is the final list of files to link
    return actual_files

  logger.debug('emcc: linking: %s to %s', actual_files, target)
  link_llvm(actual_files, target)
  return target


def get_command_with_possible_response_file(cmd):
  # 8k is a bit of an arbitrary limit, but a reasonable one
  # for max command line size before we use a response file
  if len(' '.join(cmd)) <= 8192:
    return cmd

  logger.debug('using response file for %s' % cmd[0])
  filename = response_file.create_response_file(cmd[1:], TEMP_DIR)
  new_cmd = [cmd[0], "@" + filename]
  return new_cmd


# LLVM optimizations
# @param opt A list of LLVM optimization parameters
def llvm_opt(filename, opts, out=None):
  inputs = filename
  if not isinstance(inputs, list):
    inputs = [inputs]
  else:
    assert out, 'must provide out if llvm_opt on a list of inputs'
  assert len(opts), 'should not call opt with nothing to do'
  opts = opts[:]

  target = out or (filename + '.opt.bc')
  cmd = [LLVM_OPT] + inputs + opts + ['-o', target]
  cmd = get_command_with_possible_response_file(cmd)
  print_compiler_stage(cmd)
  check_call(cmd)
  assert os.path.exists(target), 'llvm optimizer emitted no output.'
  if not out:
    shutil.move(filename + '.opt.bc', filename)
  return target


def llvm_dis(input_filename, output_filename):
  # LLVM binary ==> LLVM assembly
  try_delete(output_filename)
  output = run_process([LLVM_DIS, input_filename, '-o', output_filename], stdout=PIPE).stdout
  assert os.path.exists(output_filename), 'Could not create .ll file: ' + output


def llvm_as(input_filename, output_filename):
  # LLVM assembly ==> LLVM binary
  try_delete(output_filename)
  output = run_process([LLVM_AS, input_filename, '-o', output_filename], stdout=PIPE).stdout
  assert os.path.exists(output_filename), 'Could not create bc file: ' + output


def parse_symbols(output, include_internal=False):
  defs = []
  undefs = []
  commons = []
  for line in output.split('\n'):
    if not line or line[0] == '#':
      continue
    # e.g.  filename.o:  , saying which file it's from
    if ':' in line:
      continue
    parts = [seg for seg in line.split(' ') if len(seg)]
    # pnacl-nm will print zero offsets for bitcode, and newer llvm-nm will print present symbols
    # as  -------- T name
    if len(parts) == 3 and parts[0] == "--------" or re.match(r'^[\da-f]{8}$', parts[0]):
      parts.pop(0)
    if len(parts) == 2:
      # ignore lines with absolute offsets, these are not bitcode anyhow
      # e.g. |00000630 t d_source_name|
      status, symbol = parts
      if status == 'U':
        undefs.append(symbol)
      elif status == 'C':
        commons.append(symbol)
      elif (not include_internal and status == status.upper()) or \
           (include_internal and status in ['W', 't', 'T', 'd', 'D']):
        # FIXME: using WTD in the previous line fails due to llvm-nm behavior on macOS,
        #        so for now we assume all uppercase are normally defined external symbols
        defs.append(symbol)
  return ObjectFileInfo(0, None, set(defs), set(undefs), set(commons))


def llvm_nm_uncached(filename, stdout=PIPE, stderr=PIPE, include_internal=False):
  # LLVM binary ==> list of symbols
  proc = run_process([LLVM_NM, filename], stdout=stdout, stderr=stderr, check=False)
  if proc.returncode == 0:
    return parse_symbols(proc.stdout, include_internal)
  else:
    return ObjectFileInfo(proc.returncode, str(proc.stdout) + str(proc.stderr))


def llvm_nm(filename, stdout=PIPE, stderr=PIPE, include_internal=False):
  # Always use absolute paths to maximize cache usage
  filename = os.path.abspath(filename)

  if include_internal and filename in internal_nm_cache:
    return internal_nm_cache[filename]
  elif not include_internal and filename in uninternal_nm_cache:
    return uninternal_nm_cache[filename]

  ret = llvm_nm_uncached(filename, stdout, stderr, include_internal)

  if ret.returncode != 0:
    logger.debug('llvm-nm failed on file ' + filename + ': return code ' + str(ret.returncode) + ', error: ' + ret.output)

  # Even if we fail, write the results to the NM cache so that we don't keep trying to llvm-nm the failing file again later.
  if include_internal:
    internal_nm_cache[filename] = ret
  else:
    uninternal_nm_cache[filename] = ret

  return ret


def emcc(filename, args=[], output_filename=None, stdout=None, stderr=None, env=None):
  if output_filename is None:
    output_filename = filename + '.o'
  try_delete(output_filename)
  run_process([EMCC, filename] + args + ['-o', output_filename], stdout=stdout, stderr=stderr, env=env)


def emar(action, output_filename, filenames, stdout=None, stderr=None, env=None):
  try_delete(output_filename)
  response_filename = response_file.create_response_file(filenames, TEMP_DIR)
  cmd = [EMAR, action, output_filename] + ['@' + response_filename]
  try:
    run_process(cmd, stdout=stdout, stderr=stderr, env=env)
  finally:
    try_delete(response_filename)

  if 'c' in action:
    assert os.path.exists(output_filename), 'emar could not create output file: ' + output_filename


def can_inline():
  return Settings.INLINING_LIMIT == 0


def is_wasm_only():
  # not even wasm, much less wasm-only
  if not Settings.WASM:
    return False
  # llvm backend can only ever produce wasm
  return True


def get_safe_internalize():
  if Settings.LINKABLE:
    return [] # do not internalize anything

  exps = Settings.EXPORTED_FUNCTIONS
  internalize_public_api = '-internalize-public-api-'
  internalize_list = ','.join([demangle_c_symbol_name(exp) for exp in exps])

  # EXPORTED_FUNCTIONS can potentially be very large.
  # 8k is a bit of an arbitrary limit, but a reasonable one
  # for max command line size before we use a response file
  if len(internalize_list) > 8192:
    logger.debug('using response file for EXPORTED_FUNCTIONS in internalize')
    finalized_exports = '\n'.join([exp[1:] for exp in exps])
    internalize_list_file = configuration.get_temp_files().get('.response').name
    with open(internalize_list_file, 'w') as f:
      f.write(finalized_exports)
    internalize_public_api += 'file=' + internalize_list_file
  else:
    internalize_public_api += 'list=' + internalize_list

  # internalize carefully, llvm 3.2 will remove even main if not told not to
  return ['-internalize', internalize_public_api]


def opt_level_to_str(opt_level, shrink_level=0):
  # convert opt_level/shrink_level pair to a string argument like -O1
  if opt_level == 0:
    return '-O0'
  if shrink_level == 1:
    return '-Os'
  elif shrink_level >= 2:
    return '-Oz'
  else:
    return '-O' + str(min(opt_level, 3))


def js_optimizer(filename, passes, debug=False, extra_info=None, output_filename=None, just_split=False, just_concat=False, extra_closure_args=[], no_license=False):
  from . import js_optimizer
  try:
    ret = js_optimizer.run(filename, passes, debug, extra_info, just_split, just_concat, extra_closure_args)
  except subprocess.CalledProcessError as e:
    exit_with_error("'%s' failed (%d)", ' '.join(e.cmd), e.returncode)
  if output_filename:
    safe_move(ret, output_filename)
    ret = output_filename
  return ret


# run JS optimizer on some JS, ignoring asm.js contents if any - just run on it all
def acorn_optimizer(filename, passes, extra_info=None, return_output=False):
  optimizer = path_from_root('tools', 'acorn-optimizer.js')
  original_filename = filename
  if extra_info is not None:
    temp_files = configuration.get_temp_files()
    temp = temp_files.get('.js').name
    shutil.copyfile(filename, temp)
    with open(temp, 'a') as f:
      f.write('// EXTRA_INFO: ' + extra_info)
    filename = temp
  cmd = NODE_JS + [optimizer, filename] + passes
  # Keep JS code comments intact through the acorn optimization pass so that JSDoc comments
  # will be carried over to a later Closure run.
  if Settings.USE_CLOSURE_COMPILER:
    cmd += ['--closureFriendly']
  if not return_output:
    next = original_filename + '.jso.js'
    configuration.get_temp_files().note(next)
    check_call(cmd, stdout=open(next, 'w'))
    save_intermediate(next, '%s.js' % passes[0])
    return next
  output = check_call(cmd, stdout=PIPE).stdout
  return output


# evals ctors. if binaryen_bin is provided, it is the dir of the binaryen tool
# for this, and we are in wasm mode
def eval_ctors(js_file, binary_file, binaryen_bin='', debug_info=False):
  logger.debug('Ctor evalling in the wasm backend is disabled due to https://github.com/emscripten-core/emscripten/issues/9527')
  return
  # TODO re-enable
  # cmd = [PYTHON, path_from_root('tools', 'ctor_evaller.py'), js_file, binary_file, str(Settings.INITIAL_MEMORY), str(Settings.TOTAL_STACK), str(Settings.GLOBAL_BASE), binaryen_bin, str(int(debug_info))]
  # if binaryen_bin:
  #   cmd += get_binaryen_feature_flags()
  # print_compiler_stage(cmd)
  # check_call(cmd)


def calculate_reachable_functions(infile, initial_list, can_reach=True):
  with ToolchainProfiler.profile_block('calculate_reachable_functions'):
    from . import asm_module
    temp = configuration.get_temp_files().get('.js').name
    js_optimizer(infile, ['dumpCallGraph'], output_filename=temp, just_concat=True)
    asm = asm_module.AsmModule(temp)
    lines = asm.funcs_js.split('\n')
    can_call = {}
    for i in range(len(lines)):
      line = lines[i]
      if line.startswith('// REACHABLE '):
        curr = json.loads(line[len('// REACHABLE '):])
        func = curr[0]
        targets = curr[2]
        can_call[func] = set(targets)
    # function tables too - treat a function all as a function that can call anything in it, which is effectively what it is
    for name, funcs in asm.tables.items():
      can_call[name] = set([x.strip() for x in funcs[1:-1].split(',')])
    # print can_call
    # Note: We ignore calls in from outside the asm module, so you could do emterpreted => outside => emterpreted, and we would
    #       miss the first one there. But this is acceptable to do, because we can't save such a stack anyhow, due to the outside!
    # print 'can call', can_call, '\n!!!\n', asm.tables, '!'
    reachable_from = {}
    for func, targets in can_call.items():
      for target in targets:
        if target not in reachable_from:
          reachable_from[target] = set()
        reachable_from[target].add(func)
    # print 'reachable from', reachable_from
    to_check = initial_list[:]
    advised = set()
    if can_reach:
      # find all functions that can reach the initial list
      while len(to_check):
        curr = to_check.pop()
        if curr in reachable_from:
          for reacher in reachable_from[curr]:
            if reacher not in advised:
              if not JS.is_dyn_call(reacher) and not JS.is_function_table(reacher):
                advised.add(str(reacher))
              to_check.append(reacher)
    else:
      # find all functions that are reachable from the initial list, including it
      # all tables are assumed reachable, as they can be called from dyncall from outside
      for name, funcs in asm.tables.items():
        to_check.append(name)
      while len(to_check):
        curr = to_check.pop()
        if not JS.is_function_table(curr):
          advised.add(curr)
        if curr in can_call:
          for target in can_call[curr]:
            if target not in advised:
              advised.add(str(target))
              to_check.append(target)
    return {'reachable': list(advised), 'total_funcs': len(can_call)}


def check_closure_compiler(args, env):
  if not os.path.exists(CLOSURE_COMPILER[-1]):
    exit_with_error('google-closure-compiler executable (%s) does not exist, check the paths in %s.  To install closure compiler, run "npm install" in emscripten root directory.', CLOSURE_COMPILER[-1], EM_CONFIG)
  try:
    output = run_process(CLOSURE_COMPILER + args + ['--version'], stdout=PIPE, env=env).stdout
  except Exception as e:
    logger.warn(str(e))
    exit_with_error('closure compiler ("%s --version") did not execute properly!' % str(CLOSURE_COMPILER))
  if 'Version:' not in output:
    exit_with_error('unrecognized closure compiler --version output (%s):\n%s' % (str(CLOSURE_COMPILER), output))


def closure_compiler(filename, pretty=True, advanced=True, extra_closure_args=None):
  with ToolchainProfiler.profile_block('closure_compiler'):
    env = shared.env_with_node_in_path()
    user_args = []
    env_args = os.environ.get('EMCC_CLOSURE_ARGS')
    if env_args:
      user_args += shlex.split(env_args)
    if extra_closure_args:
      user_args += extra_closure_args

    # Closure compiler expects JAVA_HOME to be set *and* java.exe to be in the PATH in order
    # to enable use the java backend.  Without this it will only try the native and JavaScript
    # versions of the compiler.
    java_bin = os.path.dirname(JAVA)
    if java_bin:
      def add_to_path(dirname):
        env['PATH'] = env['PATH'] + os.pathsep + dirname
      add_to_path(java_bin)
      java_home = os.path.dirname(java_bin)
      env.setdefault('JAVA_HOME', java_home)

    if WINDOWS and not any(a.startswith('--platform') for a in user_args):
      # Disable native compiler on windows until upstream issue is fixed:
      # https://github.com/google/closure-compiler-npm/issues/147
      user_args.append('--platform=java')

    check_closure_compiler(user_args, env)

    # Closure externs file contains known symbols to be extern to the minification, Closure
    # should not minify these symbol names.
    CLOSURE_EXTERNS = [path_from_root('src', 'closure-externs', 'closure-externs.js')]

    # Closure compiler needs to know about all exports that come from the asm.js/wasm module, because to optimize for small code size,
    # the exported symbols are added to global scope via a foreach loop in a way that evades Closure's static analysis. With an explicit
    # externs file for the exports, Closure is able to reason about the exports.
    if Settings.MODULE_EXPORTS and not Settings.DECLARE_ASM_MODULE_EXPORTS:
      # Generate an exports file that records all the exported symbols from asm.js/wasm module.
      module_exports_suppressions = '\n'.join(['/**\n * @suppress {duplicate, undefinedVars}\n */\nvar %s;\n' % i for i, j in Settings.MODULE_EXPORTS])
      exports_file = configuration.get_temp_files().get('_module_exports.js')
      exports_file.write(module_exports_suppressions.encode())
      exports_file.close()

      CLOSURE_EXTERNS += [exports_file.name]

    # Node.js specific externs
    if Settings.target_environment_may_be('node'):
      NODE_EXTERNS_BASE = path_from_root('third_party', 'closure-compiler', 'node-externs')
      NODE_EXTERNS = os.listdir(NODE_EXTERNS_BASE)
      NODE_EXTERNS = [os.path.join(NODE_EXTERNS_BASE, name) for name in NODE_EXTERNS
                      if name.endswith('.js')]
      CLOSURE_EXTERNS += [path_from_root('src', 'closure-externs', 'node-externs.js')] + NODE_EXTERNS

    # V8/SpiderMonkey shell specific externs
    if Settings.target_environment_may_be('shell'):
      V8_EXTERNS = [path_from_root('src', 'closure-externs', 'v8-externs.js')]
      SPIDERMONKEY_EXTERNS = [path_from_root('src', 'closure-externs', 'spidermonkey-externs.js')]
      CLOSURE_EXTERNS += V8_EXTERNS + SPIDERMONKEY_EXTERNS

    # Web environment specific externs
    if Settings.target_environment_may_be('web') or Settings.target_environment_may_be('worker'):
      BROWSER_EXTERNS_BASE = path_from_root('src', 'closure-externs', 'browser-externs')
      if os.path.isdir(BROWSER_EXTERNS_BASE):
        BROWSER_EXTERNS = os.listdir(BROWSER_EXTERNS_BASE)
        BROWSER_EXTERNS = [os.path.join(BROWSER_EXTERNS_BASE, name) for name in BROWSER_EXTERNS
                           if name.endswith('.js')]
        CLOSURE_EXTERNS += BROWSER_EXTERNS

    if Settings.MINIMAL_RUNTIME and Settings.USE_PTHREADS and not Settings.MODULARIZE:
      CLOSURE_EXTERNS += [path_from_root('src', 'minimal_runtime_worker_externs.js')]
    outfile = filename + '.cc.js'
    configuration.get_temp_files().note(outfile)

    args = ['--compilation_level', 'ADVANCED_OPTIMIZATIONS' if advanced else 'SIMPLE_OPTIMIZATIONS',
            '--language_in', 'ECMASCRIPT5']
    for e in CLOSURE_EXTERNS:
      args += ['--externs', e]
    args += ['--js_output_file', outfile]

    if Settings.IGNORE_CLOSURE_COMPILER_ERRORS:
      args.append('--jscomp_off=*')
    if pretty:
      args += ['--formatting', 'PRETTY_PRINT']
    args += ['--js', filename]
    cmd = CLOSURE_COMPILER + args + user_args
    logger.debug('closure compiler: ' + ' '.join(cmd))

    proc = run_process(cmd, stderr=PIPE, check=False, env=env)

    # XXX Closure bug: if Closure is invoked with --create_source_map, Closure should create a
    # outfile.map source map file (https://github.com/google/closure-compiler/wiki/Source-Maps)
    # But it looks like it creates such files on Linux(?) even without setting that command line
    # flag (and currently we don't), so delete the produced source map file to not leak files in
    # temp directory.
    try_delete(outfile + '.map')

    # Print Closure diagnostics result up front.
    if proc.returncode != 0:
      logger.error('Closure compiler run failed:\n')
    elif len(proc.stderr.strip()) > 0:
      if Settings.CLOSURE_WARNINGS == 'error':
        logger.error('Closure compiler completed with warnings and -s CLOSURE_WARNINGS=error enabled, aborting!\n')
      elif Settings.CLOSURE_WARNINGS == 'warn':
        logger.warn('Closure compiler completed with warnings:\n')

    # Print input file (long wall of text!)
    if DEBUG == 2 and (proc.returncode != 0 or (len(proc.stderr.strip()) > 0 and Settings.CLOSURE_WARNINGS != 'quiet')):
      input_file = open(filename, 'r').read().splitlines()
      for i in range(len(input_file)):
        sys.stderr.write(str(i + 1) + ': ' + input_file[i] + '\n')

    if proc.returncode != 0:
      logger.error(proc.stderr) # print list of errors (possibly long wall of text if input was minified)

      # Exit and print final hint to get clearer output
      msg = 'closure compiler failed (rc: %d): %s' % (proc.returncode, shared.shlex_join(cmd))
      if not pretty:
        msg += ' the error message may be clearer with -g1 and EMCC_DEBUG=2 set'
      exit_with_error(msg)

    if len(proc.stderr.strip()) > 0 and Settings.CLOSURE_WARNINGS != 'quiet':
      # print list of warnings (possibly long wall of text if input was minified)
      if Settings.CLOSURE_WARNINGS == 'error':
        logger.error(proc.stderr)
      else:
        logger.warn(proc.stderr)

      # Exit and/or print final hint to get clearer output
      if not pretty:
        logger.warn('(rerun with -g1 linker flag for an unminified output)')
      elif DEBUG != 2:
        logger.warn('(rerun with EMCC_DEBUG=2 enabled to dump Closure input file)')

      if Settings.CLOSURE_WARNINGS == 'error':
        exit_with_error('closure compiler produced warnings and -s CLOSURE_WARNINGS=error enabled')

    return outfile


# minify the final wasm+JS combination. this is done after all the JS
# and wasm optimizations; here we do the very final optimizations on them
def minify_wasm_js(js_file, wasm_file, expensive_optimizations, minify_whitespace, debug_info):
  # start with JSDCE, to clean up obvious JS garbage. When optimizing for size,
  # use AJSDCE (aggressive JS DCE, performs multiple iterations). Clean up
  # whitespace if necessary too.
  passes = []
  if not Settings.LINKABLE:
    passes.append('JSDCE' if not expensive_optimizations else 'AJSDCE')
  if minify_whitespace:
    passes.append('minifyWhitespace')
  if passes:
    logger.debug('running cleanup on shell code: ' + ' '.join(passes))
    js_file = acorn_optimizer(js_file, passes)
  # if we can optimize this js+wasm combination under the assumption no one else
  # will see the internals, do so
  if not Settings.LINKABLE:
    # if we are optimizing for size, shrink the combined wasm+JS
    # TODO: support this when a symbol map is used
    if expensive_optimizations:
      js_file = metadce(js_file, wasm_file, minify_whitespace=minify_whitespace, debug_info=debug_info)
      # now that we removed unneeded communication between js and wasm, we can clean up
      # the js some more.
      passes = ['AJSDCE']
      if minify_whitespace:
        passes.append('minifyWhitespace')
      logger.debug('running post-meta-DCE cleanup on shell code: ' + ' '.join(passes))
      js_file = acorn_optimizer(js_file, passes)
      if Settings.MINIFY_WASM_IMPORTS_AND_EXPORTS:
        js_file = minify_wasm_imports_and_exports(js_file, wasm_file, minify_whitespace=minify_whitespace, minify_exports=Settings.MINIFY_ASMJS_EXPORT_NAMES, debug_info=debug_info)
  return js_file


# run binaryen's wasm-metadce to dce both js and wasm
def metadce(js_file, wasm_file, minify_whitespace, debug_info):
  logger.debug('running meta-DCE')
  temp_files = configuration.get_temp_files()
  # first, get the JS part of the graph
  extra_info = '{ "exports": [' + ','.join(map(lambda x: '["' + x[0] + '","' + x[1] + '"]', Settings.MODULE_EXPORTS)) + ']}'
  txt = acorn_optimizer(js_file, ['emitDCEGraph', 'noPrint'], return_output=True, extra_info=extra_info)
  graph = json.loads(txt)
  # add exports based on the backend output, that are not present in the JS
  if not Settings.DECLARE_ASM_MODULE_EXPORTS:
    exports = set()
    for item in graph:
      if 'export' in item:
        exports.add(item['export'])
    for export, unminified in Settings.MODULE_EXPORTS:
      if export not in exports:
        graph.append({
          'export': export,
          'name': 'emcc$export$' + export,
          'reaches': []
        })
  # ensure that functions expected to be exported to the outside are roots
  for item in graph:
    if 'export' in item:
      export = item['export']
      # wasm backend's exports are prefixed differently inside the wasm
      export = asmjs_mangle(export)
      if export in user_requested_exports or Settings.EXPORT_ALL:
        item['root'] = True
  # in standalone wasm, always export the memory
  if Settings.STANDALONE_WASM:
    graph.append({
      'export': 'memory',
      'name': 'emcc$export$memory',
      'reaches': [],
      'root': True
    })
    graph.append({
      'export': '__indirect_function_table',
      'name': 'emcc$export$__indirect_function_table',
      'reaches': [],
      'root': True
    })
  # fix wasi imports TODO: support wasm stable with an option?
  WASI_IMPORTS = set([
    'environ_get',
    'environ_sizes_get',
    'args_get',
    'args_sizes_get',
    'fd_write',
    'fd_close',
    'fd_read',
    'fd_seek',
    'fd_fdstat_get',
    'fd_sync',
    'proc_exit',
    'clock_res_get',
    'clock_time_get',
  ])
  for item in graph:
    if 'import' in item and item['import'][1][1:] in WASI_IMPORTS:
      item['import'][0] = Settings.WASI_MODULE_NAME
  # fixup wasm backend prefixing
  for item in graph:
    if 'import' in item:
      if item['import'][1][0] == '_':
        item['import'][1] = item['import'][1][1:]
  # map import names from wasm to JS, using the actual name the wasm uses for the import
  import_name_map = {}
  for item in graph:
    if 'import' in item:
      import_name_map[item['name']] = 'emcc$import$' + item['import'][1]
  temp = temp_files.get('.txt').name
  txt = json.dumps(graph)
  with open(temp, 'w') as f:
    f.write(txt)
  # run wasm-metadce
  out = run_binaryen_command('wasm-metadce',
                             wasm_file,
                             wasm_file,
                             ['--graph-file=' + temp],
                             debug=debug_info,
                             stdout=PIPE)
  # find the unused things in js
  unused = []
  PREFIX = 'unused: '
  for line in out.splitlines():
    if line.startswith(PREFIX):
      name = line.replace(PREFIX, '').strip()
      if name in import_name_map:
        name = import_name_map[name]
      unused.append(name)
  # remove them
  passes = ['applyDCEGraphRemovals']
  if minify_whitespace:
    passes.append('minifyWhitespace')
  extra_info = {'unused': unused}
  return acorn_optimizer(js_file, passes, extra_info=json.dumps(extra_info))


def asyncify_lazy_load_code(wasm_binary_target, debug):
  # create the lazy-loaded wasm. remove the memory segments from it, as memory
  # segments have already been applied by the initial wasm, and apply the knowledge
  # that it will only rewind, after which optimizations can remove some code
  args = ['--remove-memory', '--mod-asyncify-never-unwind']
  if Settings.OPT_LEVEL > 0:
    args.append(opt_level_to_str(Settings.OPT_LEVEL, Settings.SHRINK_LEVEL))
  run_wasm_opt(wasm_binary_target,
               wasm_binary_target + '.lazy.wasm',
               args=args,
               debug=debug)
  # re-optimize the original, by applying the knowledge that imports will
  # definitely unwind, and we never rewind, after which optimizations can remove
  # a lot of code
  # TODO: support other asyncify stuff, imports that don't always unwind?
  # TODO: source maps etc.
  args = ['--mod-asyncify-always-and-only-unwind']
  if Settings.OPT_LEVEL > 0:
    args.append(opt_level_to_str(Settings.OPT_LEVEL, Settings.SHRINK_LEVEL))
  run_wasm_opt(infile=wasm_binary_target,
               outfile=wasm_binary_target,
               args=args,
               debug=debug)


def minify_wasm_imports_and_exports(js_file, wasm_file, minify_whitespace, minify_exports, debug_info):
  logger.debug('minifying wasm imports and exports')
  # run the pass
  if minify_exports:
    # standalone wasm mode means we need to emit a wasi import module.
    # otherwise, minify even the imported module names.
    if Settings.MINIFY_WASM_IMPORTED_MODULES:
      pass_name = '--minify-imports-and-exports-and-modules'
    else:
      pass_name = '--minify-imports-and-exports'
  else:
    pass_name = '--minify-imports'
  out = run_wasm_opt(wasm_file, wasm_file,
                     [pass_name],
                     debug=debug_info,
                     stdout=PIPE)
  # TODO this is the last tool we run, after normal opts and metadce. it
  # might make sense to run Stack IR optimizations here or even -O (as
  # metadce which runs before us might open up new general optimization
  # opportunities). however, the benefit is less than 0.5%.

  # get the mapping
  SEP = ' => '
  mapping = {}
  for line in out.split('\n'):
    if SEP in line:
      old, new = line.strip().split(SEP)
      assert old not in mapping, 'imports must be unique'
      mapping[old] = new
  # apply them
  passes = ['applyImportAndExportNameChanges']
  if minify_whitespace:
    passes.append('minifyWhitespace')
  extra_info = {'mapping': mapping}
  return acorn_optimizer(js_file, passes, extra_info=json.dumps(extra_info))


def wasm2js(js_file, wasm_file, opt_level, minify_whitespace, use_closure_compiler, debug_info, symbols_file=None):
  logger.debug('wasm2js')
  args = ['--emscripten']
  if opt_level > 0:
    args += ['-O']
  if symbols_file:
    args += ['--symbols-file=%s' % symbols_file]
  wasm2js_js = run_binaryen_command('wasm2js', wasm_file,
                                    args=args,
                                    debug=debug_info,
                                    stdout=PIPE)
  if DEBUG:
    with open(os.path.join(get_emscripten_temp_dir(), 'wasm2js-output.js'), 'w') as f:
      f.write(wasm2js_js)
  # JS optimizations
  if opt_level >= 2:
    passes = []
    # it may be useful to also run: simplifyIfs, registerize, asmLastOpts
    # passes += ['simplifyExpressions'] # XXX fails on wasm3js.test_sqlite
    # TODO: enable name minification with pthreads. atm wasm2js emits pthread
    # helper functions outside of the asmFunc(), and they mix up minifyGlobals
    # (which assumes any vars in that area are global, like var HEAP8, but
    # those helpers have internal vars in a scope it doesn't understand yet)
    if not debug_info and not Settings.USE_PTHREADS:
      passes += ['minifyNames']
    if minify_whitespace:
      passes += ['minifyWhitespace']
    passes += ['last']
    if passes:
      # hackish fixups to work around wasm2js style and the js optimizer FIXME
      wasm2js_js = '// EMSCRIPTEN_START_ASM\n' + wasm2js_js + '// EMSCRIPTEN_END_ASM\n'
      wasm2js_js = wasm2js_js.replace('// EMSCRIPTEN_START_FUNCS;\n', '// EMSCRIPTEN_START_FUNCS\n')
      wasm2js_js = wasm2js_js.replace('// EMSCRIPTEN_END_FUNCS;\n', '// EMSCRIPTEN_END_FUNCS\n')
      wasm2js_js = wasm2js_js.replace('\n function $', '\nfunction $')
      wasm2js_js = wasm2js_js.replace('\n }', '\n}')
      wasm2js_js += '\n// EMSCRIPTEN_GENERATED_FUNCTIONS\n'
      temp = configuration.get_temp_files().get('.js').name
      with open(temp, 'w') as f:
        f.write(wasm2js_js)
      temp = js_optimizer(temp, passes)
      with open(temp) as f:
        wasm2js_js = f.read()
  # Closure compiler: in mode 1, we just minify the shell. In mode 2, we
  # minify the wasm2js output as well, which is ok since it isn't
  # validating asm.js.
  # TODO: in the non-closure case, we could run a lightweight general-
  #       purpose JS minifier here.
  if use_closure_compiler == 2:
    temp = configuration.get_temp_files().get('.js').name
    with open(temp, 'a') as f:
      f.write(wasm2js_js)
    temp = closure_compiler(temp, pretty=not minify_whitespace, advanced=False)
    with open(temp) as f:
      wasm2js_js = f.read()
    # closure may leave a trailing `;`, which would be invalid given where we place
    # this code (inside parens)
    wasm2js_js = wasm2js_js.strip()
    if wasm2js_js[-1] == ';':
      wasm2js_js = wasm2js_js[:-1]
  with open(js_file) as f:
    all_js = f.read()
  # quoted notation, something like Module['__wasm2jsInstantiate__']
  finds = re.findall(r'''[\w\d_$]+\[['"]__wasm2jsInstantiate__['"]\]''', all_js)
  if not finds:
    # post-closure notation, something like a.__wasm2jsInstantiate__
    finds = re.findall(r'''[\w\d_$]+\.__wasm2jsInstantiate__''', all_js)
  assert len(finds) == 1
  marker = finds[0]
  all_js = all_js.replace(marker, '(\n' + wasm2js_js + '\n)')
  # replace the placeholder with the actual code
  js_file = js_file + '.wasm2js.js'
  with open(js_file, 'w') as f:
    f.write(all_js)
  return js_file


# extract the DWARF info from the main file, and leave the wasm with
# debug into as a file on the side
# TODO: emit only debug sections in the side file, and not the entire
#       wasm as well
def emit_debug_on_side(wasm_file, wasm_file_with_dwarf):
  # if the dwarf filename wasn't provided, use the default target + a suffix
  wasm_file_with_dwarf = shared.Settings.SEPARATE_DWARF
  if wasm_file_with_dwarf is True:
    wasm_file_with_dwarf = wasm_file + '.debug.wasm'
  embedded_path = shared.Settings.SEPARATE_DWARF_URL or wasm_file_with_dwarf

  shutil.move(wasm_file, wasm_file_with_dwarf)
  run_process([LLVM_OBJCOPY, '--remove-section=.debug*', wasm_file_with_dwarf, wasm_file])

  # embed a section in the main wasm to point to the file with external DWARF,
  # see https://yurydelendik.github.io/webassembly-dwarf/#external-DWARF
  section_name = b'\x13external_debug_info' # section name, including prefixed size
  filename_bytes = asbytes(embedded_path)
  contents = WebAssembly.toLEB(len(filename_bytes)) + filename_bytes
  section_size = len(section_name) + len(contents)
  with open(wasm_file, 'ab') as f:
    f.write(b'\0') # user section is code 0
    f.write(WebAssembly.toLEB(section_size))
    f.write(section_name)
    f.write(contents)


def apply_wasm_memory_growth(js_file):
  logger.debug('supporting wasm memory growth with pthreads')
  fixed = acorn_optimizer(js_file, ['growableHeap'])
  ret = js_file + '.pgrow.js'
  with open(fixed, 'r') as fixed_f:
    with open(ret, 'w') as ret_f:
      with open(path_from_root('src', 'growableHeap.js')) as support_code_f:
        ret_f.write(support_code_f.read() + '\n' + fixed_f.read())
  return ret


def use_unsigned_pointers_in_js(js_file):
  logger.debug('using unsigned pointers in JS')
  return acorn_optimizer(js_file, ['unsignPointers'])


def instrument_js_for_asan(js_file):
  logger.debug('instrumenting JS memory accesses for ASan')
  return acorn_optimizer(js_file, ['asanify'])


def handle_final_wasm_symbols(wasm_file, symbols_file, debug_info):
  logger.debug('handle_final_wasm_symbols')
  args = []
  if symbols_file:
    args += ['--print-function-map']
  if not debug_info:
    # to remove debug info, we just write to that same file, and without -g
    args += ['-o', wasm_file]
  # ignore stderr because if wasm-opt is run without a -o it will warn
  output = run_wasm_opt(wasm_file, args=args, stdout=PIPE)
  if symbols_file:
    with open(symbols_file, 'w') as f:
      f.write(output)


def is_ar(filename):
  try:
    if _is_ar_cache.get(filename):
      return _is_ar_cache[filename]
    header = open(filename, 'rb').read(8)
    sigcheck = header == b'!<arch>\n'
    _is_ar_cache[filename] = sigcheck
    return sigcheck
  except Exception as e:
    logger.debug('is_ar failed to test whether file \'%s\' is a llvm archive file! Failed on exception: %s' % (filename, e))
    return False


def is_bitcode(filename):
  try:
    # look for magic signature
    b = open(filename, 'rb').read(4)
    if b[:2] == b'BC':
      return True
    # on macOS, there is a 20-byte prefix which starts with little endian
    # encoding of 0x0B17C0DE
    elif b == b'\xDE\xC0\x17\x0B':
      b = bytearray(open(filename, 'rb').read(22))
      return b[20:] == b'BC'
  except IndexError:
    # not enough characters in the input
    # note that logging will be done on the caller function
    pass
  return False


def is_wasm(filename):
  magic = open(filename, 'rb').read(4)
  return magic == b'\0asm'


# Given the name of a special Emscripten-implemented system library, returns an
# array of absolute paths to JS library files inside emscripten/src/ that
# corresponds to the library name.
def path_to_system_js_libraries(library_name):
  # Some native libraries are implemented in Emscripten as system side JS libraries
  js_system_libraries = {
    'c': '',
    'dl': '',
    'EGL': 'library_egl.js',
    'GL': ['library_webgl.js', 'library_html5_webgl.js'],
    'webgl.js': ['library_webgl.js', 'library_html5_webgl.js'],
    'GLESv2': 'library_webgl.js',
    # N.b. there is no GLESv3 to link to (note [f] in https://www.khronos.org/registry/implementers_guide.html)
    'GLEW': 'library_glew.js',
    'glfw': 'library_glfw.js',
    'glfw3': 'library_glfw.js',
    'GLU': '',
    'glut': 'library_glut.js',
    'm': '',
    'openal': 'library_openal.js',
    'rt': '',
    'pthread': '',
    'X11': 'library_xlib.js',
    'SDL': 'library_sdl.js',
    'stdc++': '',
    'uuid': 'library_uuid.js',
    'websocket': 'library_websocket.js'
  }
  library_files = []
  if library_name in js_system_libraries:
    if len(js_system_libraries[library_name]):
      lib = js_system_libraries[library_name] if isinstance(js_system_libraries[library_name], list) else [js_system_libraries[library_name]]
      library_files += lib
      logger.debug('Linking in JS library ' + str(lib))

  elif library_name.endswith('.js') and os.path.isfile(path_from_root('src', 'library_' + library_name)):
    library_files += ['library_' + library_name]

  return library_files


def emit_wasm_source_map(wasm_file, map_file):
  # source file paths must be relative to the location of the map (which is
  # emitted alongside the wasm)
  base_path = os.path.dirname(os.path.abspath(Settings.WASM_BINARY_FILE))
  sourcemap_cmd = [PYTHON, path_from_root('tools', 'wasm-sourcemap.py'),
                   wasm_file,
                   '--dwarfdump=' + LLVM_DWARFDUMP,
                   '-o',  map_file,
                   '--basepath=' + base_path]
  check_call(sourcemap_cmd)


def get_binaryen_feature_flags():
  # start with the MVP features, add the rest as needed
  ret = ['--mvp-features']
  if Settings.USE_PTHREADS:
    ret += ['--enable-threads']
  ret += Settings.BINARYEN_FEATURES
  return ret


def check_binaryen(bindir):
  opt = os.path.join(bindir, exe_suffix('wasm-opt'))
  if not os.path.exists(opt):
    exit_with_error('binaryen executable not found (%s). Please check your binaryen installation' % opt)
  try:
    output = run_process([opt, '--version'], stdout=PIPE).stdout
  except subprocess.CalledProcessError:
    exit_with_error('error running binaryen executable (%s). Please check your binaryen installation' % opt)
  if output:
    output = output.splitlines()[0]
  try:
    version = output.split()[2]
    version = int(version)
  except (IndexError, ValueError):
    exit_with_error('error parsing binaryen version (%s). Please check your binaryen installation (%s)' % (output, opt))

  # Allow the expected version or the following one in order avoid needing to update both
  # emscripten and binaryen in lock step in emscripten-releases.
  if version not in (EXPECTED_BINARYEN_VERSION, EXPECTED_BINARYEN_VERSION + 1):
    diagnostics.warning('version-check', 'unexpected binaryen version: %s (expected %s)', version, EXPECTED_BINARYEN_VERSION)


def get_binaryen_bin():
  assert Settings.WASM, 'non wasm builds should not ask for binaryen'
  global binaryen_checked
  rtn = os.path.join(BINARYEN_ROOT, 'bin')
  if not binaryen_checked:
    check_binaryen(rtn)
    binaryen_checked = True
  return rtn


def run_binaryen_command(tool, infile, outfile=None, args=[], debug=False, stdout=None):
  cmd = [os.path.join(get_binaryen_bin(), tool)]
  if outfile and tool == 'wasm-opt' and Settings.DEBUG_LEVEL != 3:
    # remove any dwarf debug info sections, if the debug level is <3, as
    # we don't need them; also remove them if we the level is 4, as then we
    # want a source map, which is implemented separately from dwarf.
    # note that we add this pass first, so that it doesn't interfere with
    # the final set of passes (which may generate stack IR, and nothing
    # should be run after that)
    # TODO: if lld can strip dwarf then we don't need this. atm though it can
    #       only strip all debug info or none, which includes the name section
    #       which we may need
    # TODO: once fastcomp is gone, either remove source maps entirely, or
    #       support them by emitting a source map at the end from the dwarf,
    #       and use llvm-objcpy to remove that final dwarf
    cmd += ['--strip-dwarf']
  cmd += args
  if infile:
    cmd += [infile]
  if outfile:
    cmd += ['-o', outfile]
  if debug:
    cmd += ['-g'] # preserve the debug info
  # if the features are not already handled, handle them
  if '--detect-features' not in cmd:
    cmd += get_binaryen_feature_flags()
  print_compiler_stage(cmd)
  # if we are emitting a source map, every time we load and save the wasm
  # we must tell binaryen to update it
  emit_source_map = Settings.DEBUG_LEVEL == 4 and outfile
  if emit_source_map:
    cmd += ['--input-source-map=' + infile + '.map']
    cmd += ['--output-source-map=' + outfile + '.map']
  ret = check_call(cmd, stdout=stdout).stdout
  if outfile:
    save_intermediate(outfile, '%s.wasm' % tool)
  return ret


def run_wasm_opt(*args, **kwargs):
  return run_binaryen_command('wasm-opt', *args, **kwargs)


save_intermediate_counter = 0


def save_intermediate(src, dst):
  if DEBUG:
    global save_intermediate_counter
    dst = 'emcc-%d-%s' % (save_intermediate_counter, dst)
    save_intermediate_counter += 1
    dst = os.path.join(CANONICAL_TEMP_DIR, dst)
    logger.debug('saving debug copy %s' % dst)
    shutil.copyfile(src, dst)
