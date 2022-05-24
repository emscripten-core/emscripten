# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from .toolchain_profiler import ToolchainProfiler

import json
import logging
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from subprocess import PIPE

from . import diagnostics
from . import response_file
from . import shared
from . import webassembly
from . import config
from . import utils
from .shared import CLANG_CC, CLANG_CXX, PYTHON
from .shared import LLVM_NM, EMCC, EMAR, EMXX, EMRANLIB, WASM_LD, LLVM_AR
from .shared import LLVM_LINK, LLVM_OBJCOPY
from .shared import try_delete, run_process, check_call, exit_with_error
from .shared import path_from_root
from .shared import asmjs_mangle, DEBUG
from .shared import TEMP_DIR
from .shared import CANONICAL_TEMP_DIR, LLVM_DWARFDUMP, demangle_c_symbol_name
from .shared import get_emscripten_temp_dir, exe_suffix, is_c_symbol
from .utils import WINDOWS
from .settings import settings

logger = logging.getLogger('building')

#  Building
binaryen_checked = False

EXPECTED_BINARYEN_VERSION = 108
# cache results of nm - it can be slow to run
nm_cache = {}
_is_ar_cache = {}
# the exports the user requested
user_requested_exports = set()


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


# Extracts the given list of archive files and outputs their contents
def extract_archive_contents(archive_files):
  archive_results = shared.run_multiple_processes([[LLVM_AR, 't', a] for a in archive_files], pipe_stdout=True)

  unpack_temp_dir = tempfile.mkdtemp('_archive_contents', 'emscripten_temp_')

  def clean_at_exit():
    try_delete(unpack_temp_dir)
  shared.atexit.register(clean_at_exit)

  archive_contents = []

  for i in range(len(archive_results)):
    a = archive_results[i]
    contents = [l for l in a.splitlines() if len(l)]
    if len(contents) == 0:
      logger.debug('Archive %s appears to be empty (recommendation: link an .so instead of .a)' % a)

    # `ar` files can only contains filenames. Just to be sure, verify that each
    # file has only as filename component and is not absolute
    for f in contents:
      assert not os.path.dirname(f)
      assert not os.path.isabs(f)

    warn_if_duplicate_entries(contents, a)

    archive_contents += [{
      'archive_name': archive_files[i],
      'o_files': [os.path.join(unpack_temp_dir, c) for c in contents]
    }]

  shared.run_multiple_processes([[LLVM_AR, 'xo', a] for a in archive_files], cwd=unpack_temp_dir)

  # check that all files were created
  for a in archive_contents:
    missing_contents = [x for x in a['o_files'] if not os.path.exists(x)]
    if missing_contents:
      exit_with_error(f'llvm-ar failed to extract file(s) {missing_contents} from archive file {f}!')

  return archive_contents


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

  return [v for v in values if check(v)]


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


def get_building_env():
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
  env['HOST_CC'] = CLANG_CC
  env['HOST_CXX'] = CLANG_CXX
  env['HOST_CFLAGS'] = '-W' # if set to nothing, CFLAGS is used, which we don't want
  env['HOST_CXXFLAGS'] = '-W' # if set to nothing, CXXFLAGS is used, which we don't want
  env['PKG_CONFIG_LIBDIR'] = shared.Cache.get_sysroot_dir('local', 'lib', 'pkgconfig') + os.path.pathsep + shared.Cache.get_sysroot_dir('lib', 'pkgconfig')
  env['PKG_CONFIG_PATH'] = os.environ.get('EM_PKG_CONFIG_PATH', '')
  env['EMSCRIPTEN'] = path_from_root()
  env['PATH'] = shared.Cache.get_sysroot_dir('bin') + os.pathsep + env['PATH']
  env['CROSS_COMPILE'] = path_from_root('em') # produces /path/to/emscripten/em , which then can have 'cc', 'ar', etc appended to it
  return env


def make_paths_absolute(f):
  if f.startswith('-'):  # skip flags
    return f
  else:
    return os.path.abspath(f)


# Runs llvm-nm for the given list of files.
# The results are populated in nm_cache, and also returned as an array with order corresponding with the input files.
# If a given file cannot be processed, None will be present in its place.
@ToolchainProfiler.profile()
def llvm_nm_multiple(files):
  if len(files) == 0:
    return []
  # Run llvm-nm on files that we haven't cached yet
  cmd = [LLVM_NM, '--print-file-name'] + [f for f in files if f not in nm_cache]
  cmd = get_command_with_possible_response_file(cmd)
  results = run_process(cmd, stdout=PIPE, stderr=PIPE, check=False)

  # If one or more of the input files cannot be processed, llvm-nm will return a non-zero error code, but it will still process and print
  # out all the other files in order. So even if process return code is non zero, we should always look at what we got to stdout.
  if results.returncode != 0:
    logger.debug(f'Subcommand {" ".join(cmd)} failed with return code {results.returncode}! (An input file was corrupt?)')

  for key, value in parse_llvm_nm_symbols(results.stdout).items():
    nm_cache[key] = value

  return [nm_cache[f] if f in nm_cache else {'defs': set(), 'undefs': set(), 'parse_error': True} for f in files]


def llvm_nm(file):
  return llvm_nm_multiple([file])[0]


@ToolchainProfiler.profile()
def read_link_inputs(files):
  # Before performing the link, we need to look at each input file to determine which symbols
  # each of them provides. Do this in multiple parallel processes.
  archive_names = [] # .a files passed in to the command line to the link
  object_names = [] # .o/.bc files passed in to the command line to the link
  # Stores the object files contained in different archive files passed as input
  ar_contents = {}
  for f in files:
    absolute_path_f = make_paths_absolute(f)

    if absolute_path_f not in ar_contents and is_ar(absolute_path_f):
      archive_names.append(absolute_path_f)
    elif absolute_path_f not in nm_cache and is_bitcode(absolute_path_f):
      object_names.append(absolute_path_f)

  # Archives contain objects, so process all archives first in parallel to obtain the object files in them.
  archive_contents = extract_archive_contents(archive_names)

  for a in archive_contents:
    ar_contents[os.path.abspath(a['archive_name'])] = a['o_files']
    for o in a['o_files']:
      if o not in nm_cache:
        object_names.append(o)

  # Next, extract symbols from all object files (either standalone or inside archives we just extracted)
  # The results are not used here directly, but populated to llvm-nm cache structure.
  llvm_nm_multiple(object_names)
  return ar_contents


def llvm_backend_args():
  # disable slow and relatively unimportant optimization passes
  args = ['-combiner-global-alias-analysis=false']

  # asm.js-style exception handling
  if not settings.DISABLE_EXCEPTION_CATCHING:
    args += ['-enable-emscripten-cxx-exceptions']
  if settings.EXCEPTION_CATCHING_ALLOWED:
    # When 'main' has a non-standard signature, LLVM outlines its content out to
    # '__original_main'. So we add it to the allowed list as well.
    if 'main' in settings.EXCEPTION_CATCHING_ALLOWED:
      settings.EXCEPTION_CATCHING_ALLOWED += ['__original_main']
    allowed = ','.join(settings.EXCEPTION_CATCHING_ALLOWED)
    args += ['-emscripten-cxx-exceptions-allowed=' + allowed]

  # asm.js-style setjmp/longjmp handling
  if settings.SUPPORT_LONGJMP == 'emscripten':
    args += ['-enable-emscripten-sjlj']
  # setjmp/longjmp handling using Wasm EH
  elif settings.SUPPORT_LONGJMP == 'wasm':
    args += ['-wasm-enable-sjlj']

  # better (smaller, sometimes faster) codegen, see binaryen#1054
  # and https://bugs.llvm.org/show_bug.cgi?id=39488
  args += ['-disable-lsr']

  return args


@ToolchainProfiler.profile()
def link_to_object(args, target):
  # link using lld unless LTO is requested (lld can't output LTO/bitcode object files).
  if not settings.LTO:
    link_lld(args + ['--relocatable'], target)
  else:
    link_bitcode(args, target)


def link_llvm(linker_inputs, target):
  # runs llvm-link to link things.
  cmd = [LLVM_LINK] + linker_inputs + ['-o', target]
  cmd = get_command_with_possible_response_file(cmd)
  check_call(cmd)


def lld_flags_for_executable(external_symbols):
  cmd = []
  if external_symbols:
    undefs = shared.get_temp_files().get('.undefined').name
    utils.write_file(undefs, '\n'.join(external_symbols))
    cmd.append('--allow-undefined-file=%s' % undefs)
  else:
    cmd.append('--import-undefined')

  if settings.IMPORTED_MEMORY:
    cmd.append('--import-memory')

  if settings.SHARED_MEMORY:
    cmd.append('--shared-memory')

  # wasm-ld can strip debug info for us. this strips both the Names
  # section and DWARF, so we can only use it when we don't need any of
  # those things.
  if settings.DEBUG_LEVEL < 2 and (not settings.EMIT_SYMBOL_MAP and
                                   not settings.EMIT_NAME_SECTION and
                                   not settings.ASYNCIFY):
    cmd.append('--strip-debug')

  if settings.LINKABLE:
    cmd.append('--export-dynamic')
    cmd.append('--no-gc-sections')

  c_exports = [e for e in settings.EXPORTED_FUNCTIONS if is_c_symbol(e)]
  # Strip the leading underscores
  c_exports = [demangle_c_symbol_name(e) for e in c_exports]
  c_exports += settings.EXPORT_IF_DEFINED
  if external_symbols:
    # Filter out symbols external/JS symbols
    c_exports = [e for e in c_exports if e not in external_symbols]
  for export in c_exports:
    cmd.append('--export-if-defined=' + export)

  for export in settings.REQUIRED_EXPORTS:
    if settings.ERROR_ON_UNDEFINED_SYMBOLS:
      cmd.append('--export=' + export)
    else:
      cmd.append('--export-if-defined=' + export)

  if settings.RELOCATABLE:
    cmd.append('--experimental-pic')
    if settings.SIDE_MODULE:
      cmd.append('-shared')
    else:
      cmd.append('-pie')
    if not settings.LINKABLE:
      cmd.append('--no-export-dynamic')
  else:
    cmd.append('--export-table')
    if settings.ALLOW_TABLE_GROWTH:
      cmd.append('--growable-table')

  if not settings.SIDE_MODULE:
    # Export these two section start symbols so that we can extact the string
    # data that they contain.
    cmd += [
      '-z', 'stack-size=%s' % settings.TOTAL_STACK,
      '--initial-memory=%d' % settings.INITIAL_MEMORY,
    ]

    if settings.STANDALONE_WASM:
      # when settings.EXPECT_MAIN is set we fall back to wasm-ld default of _start
      if not settings.EXPECT_MAIN:
        cmd += ['--entry=_initialize']
    else:
      if settings.EXPECT_MAIN and not settings.IGNORE_MISSING_MAIN:
        cmd += ['--entry=main']
      else:
        cmd += ['--no-entry']
    if not settings.ALLOW_MEMORY_GROWTH:
      cmd.append('--max-memory=%d' % settings.INITIAL_MEMORY)
    elif settings.MAXIMUM_MEMORY != -1:
      cmd.append('--max-memory=%d' % settings.MAXIMUM_MEMORY)
    if not settings.RELOCATABLE:
      cmd.append('--global-base=%s' % settings.GLOBAL_BASE)

  return cmd


def link_lld(args, target, external_symbols=None):
  if not os.path.exists(WASM_LD):
    exit_with_error('linker binary not found in LLVM directory: %s', WASM_LD)
  # runs lld to link things.
  # lld doesn't currently support --start-group/--end-group since the
  # semantics are more like the windows linker where there is no need for
  # grouping.
  args = [a for a in args if a not in ('--start-group', '--end-group')]

  # Emscripten currently expects linkable output (SIDE_MODULE/MAIN_MODULE) to
  # include all archive contents.
  if settings.LINKABLE:
    args.insert(0, '--whole-archive')
    args.append('--no-whole-archive')

  if settings.STRICT:
    args.append('--fatal-warnings')

  cmd = [WASM_LD, '-o', target] + args
  for a in llvm_backend_args():
    cmd += ['-mllvm', a]

  if settings.EXCEPTION_HANDLING:
    cmd += ['-mllvm', '-wasm-enable-eh']
  if settings.EXCEPTION_HANDLING or settings.SUPPORT_LONGJMP == 'wasm':
    cmd += ['-mllvm', '-exception-model=wasm']

  if settings.MEMORY64:
    cmd.append('-mwasm64')

  # For relocatable output (generating an object file) we don't pass any of the
  # normal linker flags that are used when building and exectuable
  if '--relocatable' not in args and '-r' not in args:
    cmd += lld_flags_for_executable(external_symbols)

  cmd = get_command_with_possible_response_file(cmd)
  check_call(cmd)


def link_bitcode(args, target, force_archive_contents=False):
  diagnostics.warning('deprecated', 'bitcode linking with llvm-link is deprecated.  Please use emar archives instead.  See https://github.com/emscripten-core/emscripten/issues/13492')
  # "Full-featured" linking: looks into archives (duplicates lld functionality)
  input_files = [a for a in args if not a.startswith('-')]
  files_to_link = []
  # Tracking unresolveds is necessary for .a linking, see below.
  # Specify all possible entry points to seed the linking process.
  # For a simple application, this would just be "main".
  unresolved_symbols = {func[1:] for func in settings.EXPORTED_FUNCTIONS}
  resolved_symbols = set()
  # Paths of already included object files from archives.
  added_contents = set()
  has_ar = any(is_ar(make_paths_absolute(f)) for f in input_files)

  # If we have only one archive or the force_archive_contents flag is set,
  # then we will add every object file we see, regardless of whether it
  # resolves any undefined symbols.
  force_add_all = len(input_files) == 1 or force_archive_contents

  # Considers an object file for inclusion in the link. The object is included
  # if force_add=True or if the object provides a currently undefined symbol.
  # If the object is included, the symbol tables are updated and the function
  # returns True.
  def consider_object(f, force_add=False):
    new_symbols = llvm_nm(f)
    # Check if the object was valid according to llvm-nm. It also accepts
    # native object files.
    if new_symbols['parse_error']:
      diagnostics.warning('emcc', 'object %s is not valid according to llvm-nm, cannot link', f)
      return False
    # Check the object is valid for us, and not a native object file.
    if not is_bitcode(f):
      exit_with_error('unknown file type: %s', f)
    provided = new_symbols['defs'].union(new_symbols['commons'])
    do_add = force_add or not unresolved_symbols.isdisjoint(provided)
    if do_add:
      logger.debug('adding object %s to link (forced: %d)' % (f, force_add))
      # Update resolved_symbols table with newly resolved symbols
      resolved_symbols.update(provided)
      # Update unresolved_symbols table by adding newly unresolved symbols and
      # removing newly resolved symbols.
      unresolved_symbols.update(new_symbols['undefs'].difference(resolved_symbols))
      unresolved_symbols.difference_update(provided)
      files_to_link.append(f)
    return do_add

  ar_contents = read_link_inputs(input_files)

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
  for a in args:
    if a.startswith('-'):
      if a in ['--start-group', '-(']:
        assert current_archive_group is None, 'Nested --start-group, missing --end-group?'
        current_archive_group = []
      elif a in ['--end-group', '-)']:
        assert current_archive_group is not None, '--end-group without --start-group'
        scan_archive_group(current_archive_group)
        current_archive_group = None
      elif a in ['--whole-archive', '-whole-archive']:
        in_whole_archive = True
      elif a in ['--no-whole-archive', '-no-whole-archive']:
        in_whole_archive = False
      else:
        # Command line flags should already be vetted by the time this method
        # is called, so this is an internal error
        exit_with_error('unsupported link flag: %s', a)
    else:
      lib_path = make_paths_absolute(a)
      if is_ar(lib_path):
        # Extract object files from ar archives, and link according to gnu ld semantics
        # (link in an entire .o from the archive if it supplies symbols still unresolved)
        consider_archive(lib_path, in_whole_archive or force_add_all)
        # If we're inside a --start-group/--end-group section, add to the list
        # so we can loop back around later.
        if current_archive_group is not None:
          current_archive_group.append(lib_path)
      elif is_bitcode(lib_path):
        if has_ar:
          consider_object(a, force_add=True)
        else:
          # If there are no archives then we can simply link all valid object
          # files and skip the symbol table stuff.
          files_to_link.append(a)
      else:
        exit_with_error('unknown file type: %s', a)

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
  files_to_link = unique_ordered(files_to_link)

  logger.debug('emcc: linking: %s to %s', files_to_link, target)
  link_llvm(files_to_link, target)


def get_command_with_possible_response_file(cmd):
  # One of None, 0 or 1. (None: do default decision, 0: force disable, 1: force enable)
  force_response_files = os.getenv('EM_FORCE_RESPONSE_FILES')

  # 8k is a bit of an arbitrary limit, but a reasonable one
  # for max command line size before we use a response file
  if (len(shared.shlex_join(cmd)) <= 8192 and force_response_files != '1') or force_response_files == '0':
    return cmd

  logger.debug('using response file for %s' % cmd[0])
  filename = response_file.create_response_file(cmd[1:], TEMP_DIR)
  new_cmd = [cmd[0], "@" + filename]
  return new_cmd


# Parses the output of llnm-nm and returns a dictionary of symbols for each file in the output.
# This function can be called either for a single file output listing ("llvm-nm a.o", or for
# multiple files listing ("llvm-nm a.o b.o").
def parse_llvm_nm_symbols(output):
  # If response files are used in a call to llvm-nm, the output printed by llvm-nm has a
  # quirk that it will contain double backslashes as directory separators on Windows. (it will
  # not remove the escape characters when printing)
  # Therefore canonicalize the output to single backslashes.
  output = output.replace('\\\\', '\\')

  # a dictionary from 'filename' -> { 'defs': set(), 'undefs': set(), 'commons': set() }
  symbols = {}

  for line in output.split('\n'):
    # Line format: "[archive filename:]object filename: address status name"
    entry_pos = line.rfind(':')
    if entry_pos < 0:
      continue

    filename_pos = line.rfind(':', 0, entry_pos)
    # Disambiguate between
    #   C:\bar.o: T main
    # and
    #   /foo.a:bar.o: T main
    # (both of these have same number of ':'s, but in the first, the file name on disk is "C:\bar.o", in second, it is "/foo.a")
    if filename_pos < 0 or line[filename_pos + 1] in '/\\':
      filename_pos = entry_pos

    filename = line[:filename_pos]
    status = line[entry_pos + 11] # Skip address, which is always fixed-length 8 chars.
    symbol = line[entry_pos + 13:]

    if filename not in symbols:
      record = symbols.setdefault(filename, {
        'defs': set(),
        'undefs': set(),
        'commons': set(),
        'parse_error': False
      })
    if status == 'U':
      record['undefs'] |= {symbol}
    elif status == 'C':
      record['commons'] |= {symbol}
    elif status == status.upper():
      record['defs'] |= {symbol}
  return symbols


def emar(action, output_filename, filenames, stdout=None, stderr=None, env=None):
  try_delete(output_filename)
  cmd = [EMAR, action, output_filename] + filenames
  cmd = get_command_with_possible_response_file(cmd)
  run_process(cmd, stdout=stdout, stderr=stderr, env=env)

  if 'c' in action:
    assert os.path.exists(output_filename), 'emar could not create output file: ' + output_filename


def opt_level_to_str(opt_level, shrink_level=0):
  # convert opt_level/shrink_level pair to a string argument like -O1
  if opt_level == 0:
    return '-O0'
  if shrink_level == 1:
    return '-Os'
  elif shrink_level >= 2:
    return '-Oz'
  else:
    return f'-O{min(opt_level, 3)}'


def js_optimizer(filename, passes):
  from . import js_optimizer
  try:
    return js_optimizer.run(filename, passes)
  except subprocess.CalledProcessError as e:
    exit_with_error("'%s' failed (%d)", ' '.join(e.cmd), e.returncode)


# run JS optimizer on some JS, ignoring asm.js contents if any - just run on it all
def acorn_optimizer(filename, passes, extra_info=None, return_output=False):
  optimizer = path_from_root('tools/acorn-optimizer.js')
  original_filename = filename
  if extra_info is not None:
    temp_files = shared.get_temp_files()
    temp = temp_files.get('.js').name
    shutil.copyfile(filename, temp)
    with open(temp, 'a') as f:
      f.write('// EXTRA_INFO: ' + extra_info)
    filename = temp
  cmd = config.NODE_JS + [optimizer, filename] + passes
  # Keep JS code comments intact through the acorn optimization pass so that JSDoc comments
  # will be carried over to a later Closure run.
  if settings.USE_CLOSURE_COMPILER:
    cmd += ['--closureFriendly']
  if settings.EXPORT_ES6:
    cmd += ['--exportES6']
  if settings.VERBOSE:
    cmd += ['verbose']
  if not return_output:
    next = original_filename + '.jso.js'
    shared.get_temp_files().note(next)
    check_call(cmd, stdout=open(next, 'w'))
    save_intermediate(next, '%s.js' % passes[0])
    return next
  output = check_call(cmd, stdout=PIPE).stdout
  return output


WASM_CALL_CTORS = '__wasm_call_ctors'


# evals ctors. if binaryen_bin is provided, it is the dir of the binaryen tool
# for this, and we are in wasm mode
def eval_ctors(js_file, wasm_file, debug_info):
  if settings.MINIMAL_RUNTIME:
    CTOR_ADD_PATTERN = f"asm['{WASM_CALL_CTORS}']();" # TODO test
  else:
    CTOR_ADD_PATTERN = f"addOnInit(Module['asm']['{WASM_CALL_CTORS}']);"

  js = utils.read_file(js_file)

  has_wasm_call_ctors = False

  # eval the ctor caller as well as main, or, in standalone mode, the proper
  # entry/init function
  if not settings.STANDALONE_WASM:
    ctors = []
    kept_ctors = []
    has_wasm_call_ctors = CTOR_ADD_PATTERN in js
    if has_wasm_call_ctors:
      ctors += [WASM_CALL_CTORS]
    if settings.HAS_MAIN:
      ctors += ['main']
      # TODO perhaps remove the call to main from the JS? or is this an abi
      #      we want to preserve?
      kept_ctors += ['main']
    if not ctors:
      logger.info('ctor_evaller: no ctors')
      return
    args = ['--ctors=' + ','.join(ctors)]
    if kept_ctors:
      args += ['--kept-exports=' + ','.join(kept_ctors)]
  else:
    if settings.EXPECT_MAIN:
      ctor = '_start'
    else:
      ctor = '_initialize'
    args = ['--ctors=' + ctor, '--kept-exports=' + ctor]
  if settings.EVAL_CTORS == 2:
    args += ['--ignore-external-input']
  logger.info('ctor_evaller: trying to eval global ctors (' + ' '.join(args) + ')')
  out = run_binaryen_command('wasm-ctor-eval', wasm_file, wasm_file, args=args, stdout=PIPE, debug=debug_info)
  logger.info('\n\n' + out)
  num_successful = out.count('success on')
  if num_successful and has_wasm_call_ctors:
    js = js.replace(CTOR_ADD_PATTERN, '')
  utils.write_file(js_file, js)


def get_closure_compiler():
  # First check if the user configured a specific CLOSURE_COMPILER in thier settings
  if config.CLOSURE_COMPILER:
    return config.CLOSURE_COMPILER

  # Otherwise use the one installed vai npm
  cmd = shared.get_npm_cmd('google-closure-compiler')
  if not WINDOWS:
    # Work around an issue that Closure compiler can take up a lot of memory and crash in an error
    # "FATAL ERROR: Ineffective mark-compacts near heap limit Allocation failed - JavaScript heap
    # out of memory"
    cmd.insert(-1, '--max_old_space_size=8192')
  return cmd


def check_closure_compiler(cmd, args, env, allowed_to_fail):
  cmd = cmd + args + ['--version']
  try:
    output = run_process(cmd, stdout=PIPE, env=env).stdout
  except Exception as e:
    if allowed_to_fail:
      return False
    if isinstance(e, subprocess.CalledProcessError):
      sys.stderr.write(e.stdout)
    sys.stderr.write(str(e) + '\n')
    exit_with_error('closure compiler (%s) did not execute properly!' % shared.shlex_join(cmd))

  if 'Version:' not in output:
    if allowed_to_fail:
      return False
    exit_with_error('unrecognized closure compiler --version output (%s):\n%s' % (shared.shlex_join(cmd), output))

  return True


# Remove this once we require python3.7 and can use std.isascii.
# See: https://docs.python.org/3/library/stdtypes.html#str.isascii
def isascii(s):
  try:
    s.encode('ascii')
  except UnicodeEncodeError:
    return False
  else:
    return True


def get_closure_compiler_and_env(user_args):
  env = shared.env_with_node_in_path()
  closure_cmd = get_closure_compiler()

  native_closure_compiler_works = check_closure_compiler(closure_cmd, user_args, env, allowed_to_fail=True)
  if not native_closure_compiler_works and not any(a.startswith('--platform') for a in user_args):
    # Run with Java Closure compiler as a fallback if the native version does not work
    user_args.append('--platform=java')
    check_closure_compiler(closure_cmd, user_args, env, allowed_to_fail=False)

  if config.JAVA and '--platform=java' in user_args:
    # Closure compiler expects JAVA_HOME to be set *and* java.exe to be in the PATH in order
    # to enable use the java backend.  Without this it will only try the native and JavaScript
    # versions of the compiler.
    java_bin = os.path.dirname(config.JAVA)
    if java_bin:
      def add_to_path(dirname):
        env['PATH'] = env['PATH'] + os.pathsep + dirname
      add_to_path(java_bin)
      java_home = os.path.dirname(java_bin)
      env.setdefault('JAVA_HOME', java_home)

  return closure_cmd, env


@ToolchainProfiler.profile()
def closure_transpile(filename, pretty):
  user_args = []
  closure_cmd, env = get_closure_compiler_and_env(user_args)
  closure_cmd += ['--language_out', 'ES5']
  closure_cmd += ['--compilation_level', 'WHITESPACE_ONLY']
  return run_closure_cmd(closure_cmd, filename, env, pretty)


@ToolchainProfiler.profile()
def closure_compiler(filename, pretty, advanced=True, extra_closure_args=None):
  user_args = []
  env_args = os.environ.get('EMCC_CLOSURE_ARGS')
  if env_args:
    user_args += shlex.split(env_args)
  if extra_closure_args:
    user_args += extra_closure_args

  closure_cmd, env = get_closure_compiler_and_env(user_args)

  # Closure externs file contains known symbols to be extern to the minification, Closure
  # should not minify these symbol names.
  CLOSURE_EXTERNS = [path_from_root('src/closure-externs/closure-externs.js')]

  # Closure compiler needs to know about all exports that come from the wasm module, because to optimize for small code size,
  # the exported symbols are added to global scope via a foreach loop in a way that evades Closure's static analysis. With an explicit
  # externs file for the exports, Closure is able to reason about the exports.
  if settings.WASM_FUNCTION_EXPORTS and not settings.DECLARE_ASM_MODULE_EXPORTS:
    # Generate an exports file that records all the exported symbols from the wasm module.
    module_exports_suppressions = '\n'.join(['/**\n * @suppress {duplicate, undefinedVars}\n */\nvar %s;\n' % asmjs_mangle(i) for i in settings.WASM_FUNCTION_EXPORTS])
    exports_file = shared.get_temp_files().get('_module_exports.js')
    exports_file.write(module_exports_suppressions.encode())
    exports_file.close()

    CLOSURE_EXTERNS += [exports_file.name]

  # Node.js specific externs
  if shared.target_environment_may_be('node'):
    NODE_EXTERNS_BASE = path_from_root('third_party/closure-compiler/node-externs')
    NODE_EXTERNS = os.listdir(NODE_EXTERNS_BASE)
    NODE_EXTERNS = [os.path.join(NODE_EXTERNS_BASE, name) for name in NODE_EXTERNS
                    if name.endswith('.js')]
    CLOSURE_EXTERNS += [path_from_root('src/closure-externs/node-externs.js')] + NODE_EXTERNS

  # V8/SpiderMonkey shell specific externs
  if shared.target_environment_may_be('shell'):
    V8_EXTERNS = [path_from_root('src/closure-externs/v8-externs.js')]
    SPIDERMONKEY_EXTERNS = [path_from_root('src/closure-externs/spidermonkey-externs.js')]
    CLOSURE_EXTERNS += V8_EXTERNS + SPIDERMONKEY_EXTERNS

  # Web environment specific externs
  if shared.target_environment_may_be('web') or shared.target_environment_may_be('worker'):
    BROWSER_EXTERNS_BASE = path_from_root('src/closure-externs/browser-externs')
    if os.path.isdir(BROWSER_EXTERNS_BASE):
      BROWSER_EXTERNS = os.listdir(BROWSER_EXTERNS_BASE)
      BROWSER_EXTERNS = [os.path.join(BROWSER_EXTERNS_BASE, name) for name in BROWSER_EXTERNS
                         if name.endswith('.js')]
      CLOSURE_EXTERNS += BROWSER_EXTERNS

  if settings.DYNCALLS:
    CLOSURE_EXTERNS += [path_from_root('src/closure-externs/dyncall-externs.js')]

  if settings.MINIMAL_RUNTIME and settings.USE_PTHREADS:
    CLOSURE_EXTERNS += [path_from_root('src/closure-externs/minimal_runtime_worker_externs.js')]

  args = ['--compilation_level', 'ADVANCED_OPTIMIZATIONS' if advanced else 'SIMPLE_OPTIMIZATIONS']
  # Keep in sync with ecmaVersion in tools/acorn-optimizer.js
  args += ['--language_in', 'ECMASCRIPT_2020']
  # Tell closure not to do any transpiling or inject any polyfills.
  # At some point we may want to look into using this as way to convert to ES5 but
  # babel is perhaps a better tool for that.
  if settings.TRANSPILE_TO_ES5:
    args += ['--language_out', 'ES5']
  else:
    args += ['--language_out', 'NO_TRANSPILE']
  # Tell closure never to inject the 'use strict' directive.
  args += ['--emit_use_strict=false']

  if settings.IGNORE_CLOSURE_COMPILER_ERRORS:
    args.append('--jscomp_off=*')
  # Specify input file relative to the temp directory to avoid specifying non-7-bit-ASCII path names.
  for e in CLOSURE_EXTERNS:
    args += ['--externs', e]
  args += user_args

  cmd = closure_cmd + args
  return run_closure_cmd(cmd, filename, env, pretty=pretty)


def run_closure_cmd(cmd, filename, env, pretty):
  cmd += ['--js', filename]

  # Closure compiler is unable to deal with path names that are not 7-bit ASCII:
  # https://github.com/google/closure-compiler/issues/3784
  tempfiles = shared.get_temp_files()

  def move_to_safe_7bit_ascii_filename(filename):
    if isascii(filename):
      return os.path.abspath(filename)
    safe_filename = tempfiles.get('.js').name  # Safe 7-bit filename
    shutil.copyfile(filename, safe_filename)
    return os.path.relpath(safe_filename, tempfiles.tmpdir)

  for i in range(len(cmd)):
    for prefix in ('--externs', '--js'):
      # Handle the case where the the flag and the value are two separate arguments.
      if cmd[i] == prefix:
        cmd[i + 1] = move_to_safe_7bit_ascii_filename(cmd[i + 1])
      # and the case where they are one argument, e.g. --externs=foo.js
      elif cmd[i].startswith(prefix + '='):
        # Replace the argument with a version that has a safe filename.
        filename = cmd[i].split('=', 1)[1]
        cmd[i] = '='.join([prefix, move_to_safe_7bit_ascii_filename(filename)])

  outfile = tempfiles.get('.cc.js').name  # Safe 7-bit filename

  # Specify output file relative to the temp directory to avoid specifying non-7-bit-ASCII path names.
  cmd += ['--js_output_file', os.path.relpath(outfile, tempfiles.tmpdir)]
  if pretty:
    cmd += ['--formatting', 'PRETTY_PRINT']

  shared.print_compiler_stage(cmd)

  # Closure compiler does not work if any of the input files contain characters outside the
  # 7-bit ASCII range. Therefore make sure the command line we pass does not contain any such
  # input files by passing all input filenames relative to the cwd. (user temp directory might
  # be in user's home directory, and user's profile name might contain unicode characters)
  proc = run_process(cmd, stderr=PIPE, check=False, env=env, cwd=tempfiles.tmpdir)

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
    if settings.CLOSURE_WARNINGS == 'error':
      logger.error('Closure compiler completed with warnings and -sCLOSURE_WARNINGS=error enabled, aborting!\n')
    elif settings.CLOSURE_WARNINGS == 'warn':
      logger.warn('Closure compiler completed with warnings:\n')

  # Print input file (long wall of text!)
  if DEBUG == 2 and (proc.returncode != 0 or (len(proc.stderr.strip()) > 0 and settings.CLOSURE_WARNINGS != 'quiet')):
    input_file = open(filename, 'r').read().splitlines()
    for i in range(len(input_file)):
      sys.stderr.write(f'{i + 1}: {input_file[i]}\n')

  if proc.returncode != 0:
    logger.error(proc.stderr) # print list of errors (possibly long wall of text if input was minified)

    # Exit and print final hint to get clearer output
    msg = f'closure compiler failed (rc: {proc.returncode}): {shared.shlex_join(cmd)}'
    if not pretty:
      msg += ' the error message may be clearer with -g1 and EMCC_DEBUG=2 set'
    exit_with_error(msg)

  if len(proc.stderr.strip()) > 0 and settings.CLOSURE_WARNINGS != 'quiet':
    # print list of warnings (possibly long wall of text if input was minified)
    if settings.CLOSURE_WARNINGS == 'error':
      logger.error(proc.stderr)
    else:
      logger.warn(proc.stderr)

    # Exit and/or print final hint to get clearer output
    if not pretty:
      logger.warn('(rerun with -g1 linker flag for an unminified output)')
    elif DEBUG != 2:
      logger.warn('(rerun with EMCC_DEBUG=2 enabled to dump Closure input file)')

    if settings.CLOSURE_WARNINGS == 'error':
      exit_with_error('closure compiler produced warnings and -sCLOSURE_WARNINGS=error enabled')

  return outfile


# minify the final wasm+JS combination. this is done after all the JS
# and wasm optimizations; here we do the very final optimizations on them
def minify_wasm_js(js_file, wasm_file, expensive_optimizations, minify_whitespace, debug_info):
  # start with JSDCE, to clean up obvious JS garbage. When optimizing for size,
  # use AJSDCE (aggressive JS DCE, performs multiple iterations). Clean up
  # whitespace if necessary too.
  passes = []
  if not settings.LINKABLE:
    passes.append('JSDCE' if not expensive_optimizations else 'AJSDCE')
  if minify_whitespace:
    passes.append('minifyWhitespace')
  if passes:
    logger.debug('running cleanup on shell code: ' + ' '.join(passes))
    js_file = acorn_optimizer(js_file, passes)
  # if we can optimize this js+wasm combination under the assumption no one else
  # will see the internals, do so
  if not settings.LINKABLE:
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
      if settings.MINIFY_WASM_IMPORTS_AND_EXPORTS:
        js_file = minify_wasm_imports_and_exports(js_file, wasm_file, minify_whitespace=minify_whitespace, minify_exports=settings.MINIFY_ASMJS_EXPORT_NAMES, debug_info=debug_info)
  return js_file


# run binaryen's wasm-metadce to dce both js and wasm
def metadce(js_file, wasm_file, minify_whitespace, debug_info):
  logger.debug('running meta-DCE')
  temp_files = shared.get_temp_files()
  # first, get the JS part of the graph
  if settings.MAIN_MODULE:
    # For the main module we include all exports as possible roots, not just function exports.
    # This means that any usages of data symbols within the JS or in the side modules can/will keep
    # these exports alive on the wasm module.
    # This is important today for weak data symbols that are defined by the main and the side module
    # (i.e.  RTTI info).  We want to make sure the main module's symbols get added to asmLibraryArg
    # when the main module is loaded.  If this doesn't happen then the symbols in the side module
    # will take precedence.
    exports = settings.WASM_EXPORTS
  else:
    exports = settings.WASM_FUNCTION_EXPORTS
  extra_info = '{ "exports": [' + ','.join(f'["{asmjs_mangle(x)}", "{x}"]' for x in exports) + ']}'

  txt = acorn_optimizer(js_file, ['emitDCEGraph', 'noPrint'], return_output=True, extra_info=extra_info)
  graph = json.loads(txt)
  # ensure that functions expected to be exported to the outside are roots
  required_symbols = user_requested_exports.union(set(settings.SIDE_MODULE_IMPORTS))
  for item in graph:
    if 'export' in item:
      export = asmjs_mangle(item['export'])
      if settings.EXPORT_ALL or export in required_symbols:
        item['root'] = True
  # in standalone wasm, always export the memory
  if not settings.IMPORTED_MEMORY:
    graph.append({
      'export': 'memory',
      'name': 'emcc$export$memory',
      'reaches': [],
      'root': True
    })
  if not settings.RELOCATABLE:
    graph.append({
      'export': '__indirect_function_table',
      'name': 'emcc$export$__indirect_function_table',
      'reaches': [],
      'root': True
    })
  # fix wasi imports TODO: support wasm stable with an option?
  WASI_IMPORTS = {
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
    'fd_pread',
    'fd_pwrite',
    'proc_exit',
    'clock_res_get',
    'clock_time_get',
  }
  for item in graph:
    if 'import' in item and item['import'][1][1:] in WASI_IMPORTS:
      item['import'][0] = settings.WASI_MODULE_NAME
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
  utils.write_file(temp, json.dumps(graph))
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


def asyncify_lazy_load_code(wasm_target, debug):
  # create the lazy-loaded wasm. remove the memory segments from it, as memory
  # segments have already been applied by the initial wasm, and apply the knowledge
  # that it will only rewind, after which optimizations can remove some code
  args = ['--remove-memory', '--mod-asyncify-never-unwind']
  if settings.OPT_LEVEL > 0:
    args.append(opt_level_to_str(settings.OPT_LEVEL, settings.SHRINK_LEVEL))
  run_wasm_opt(wasm_target,
               wasm_target + '.lazy.wasm',
               args=args,
               debug=debug)
  # re-optimize the original, by applying the knowledge that imports will
  # definitely unwind, and we never rewind, after which optimizations can remove
  # a lot of code
  # TODO: support other asyncify stuff, imports that don't always unwind?
  # TODO: source maps etc.
  args = ['--mod-asyncify-always-and-only-unwind']
  if settings.OPT_LEVEL > 0:
    args.append(opt_level_to_str(settings.OPT_LEVEL, settings.SHRINK_LEVEL))
  run_wasm_opt(infile=wasm_target,
               outfile=wasm_target,
               args=args,
               debug=debug)


def minify_wasm_imports_and_exports(js_file, wasm_file, minify_whitespace, minify_exports, debug_info):
  logger.debug('minifying wasm imports and exports')
  # run the pass
  if minify_exports:
    # standalone wasm mode means we need to emit a wasi import module.
    # otherwise, minify even the imported module names.
    if settings.MINIFY_WASM_IMPORTED_MODULES:
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


def wasm2js(js_file, wasm_file, opt_level, minify_whitespace, use_closure_compiler, debug_info, symbols_file=None, symbols_file_js=None):
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
    utils.write_file(os.path.join(get_emscripten_temp_dir(), 'wasm2js-output.js'), wasm2js_js)
  # JS optimizations
  if opt_level >= 2:
    passes = []
    if not debug_info and not settings.USE_PTHREADS:
      passes += ['minifyNames']
      if symbols_file_js:
        passes += ['symbolMap=%s' % symbols_file_js]
    if minify_whitespace:
      passes += ['minifyWhitespace']
    passes += ['last']
    if passes:
      # hackish fixups to work around wasm2js style and the js optimizer FIXME
      wasm2js_js = f'// EMSCRIPTEN_START_ASM\n{wasm2js_js}// EMSCRIPTEN_END_ASM\n'
      wasm2js_js = wasm2js_js.replace('// EMSCRIPTEN_START_FUNCS;\n', '// EMSCRIPTEN_START_FUNCS\n')
      wasm2js_js = wasm2js_js.replace('// EMSCRIPTEN_END_FUNCS;\n', '// EMSCRIPTEN_END_FUNCS\n')
      wasm2js_js = wasm2js_js.replace('\n function $', '\nfunction $')
      wasm2js_js = wasm2js_js.replace('\n }', '\n}')
      wasm2js_js += '\n// EMSCRIPTEN_GENERATED_FUNCTIONS\n'
      temp = shared.get_temp_files().get('.js').name
      utils.write_file(temp, wasm2js_js)
      temp = js_optimizer(temp, passes)
      wasm2js_js = utils.read_file(temp)
  # Closure compiler: in mode 1, we just minify the shell. In mode 2, we
  # minify the wasm2js output as well, which is ok since it isn't
  # validating asm.js.
  # TODO: in the non-closure case, we could run a lightweight general-
  #       purpose JS minifier here.
  if use_closure_compiler == 2:
    temp = shared.get_temp_files().get('.js').name
    with open(temp, 'a') as f:
      f.write(wasm2js_js)
    temp = closure_compiler(temp, pretty=not minify_whitespace, advanced=False)
    wasm2js_js = utils.read_file(temp)
    # closure may leave a trailing `;`, which would be invalid given where we place
    # this code (inside parens)
    wasm2js_js = wasm2js_js.strip()
    if wasm2js_js[-1] == ';':
      wasm2js_js = wasm2js_js[:-1]
  all_js = utils.read_file(js_file)
  # quoted notation, something like Module['__wasm2jsInstantiate__']
  finds = re.findall(r'''[\w\d_$]+\[['"]__wasm2jsInstantiate__['"]\]''', all_js)
  if not finds:
    # post-closure notation, something like a.__wasm2jsInstantiate__
    finds = re.findall(r'''[\w\d_$]+\.__wasm2jsInstantiate__''', all_js)
  assert len(finds) == 1
  marker = finds[0]
  all_js = all_js.replace(marker, f'(\n{wasm2js_js}\n)')
  # replace the placeholder with the actual code
  js_file = js_file + '.wasm2js.js'
  utils.write_file(js_file, all_js)
  return js_file


def strip(infile, outfile, debug=False, producers=False):
  cmd = [LLVM_OBJCOPY, infile, outfile]
  if debug:
    cmd += ['--remove-section=.debug*']
  if producers:
    cmd += ['--remove-section=producers']
  check_call(cmd)


# extract the DWARF info from the main file, and leave the wasm with
# debug into as a file on the side
# TODO: emit only debug sections in the side file, and not the entire
#       wasm as well
def emit_debug_on_side(wasm_file):
  # if the dwarf filename wasn't provided, use the default target + a suffix
  wasm_file_with_dwarf = settings.SEPARATE_DWARF
  if wasm_file_with_dwarf is True:
    wasm_file_with_dwarf = wasm_file + '.debug.wasm'
  embedded_path = settings.SEPARATE_DWARF_URL
  if not embedded_path:
    # a path was provided - make it relative to the wasm.
    embedded_path = os.path.relpath(wasm_file_with_dwarf,
                                    os.path.dirname(wasm_file))
    # normalize the path to use URL-style separators, per the spec
    embedded_path = embedded_path.replace('\\', '/').replace('//', '/')

  shutil.move(wasm_file, wasm_file_with_dwarf)
  strip(wasm_file_with_dwarf, wasm_file, debug=True)

  # embed a section in the main wasm to point to the file with external DWARF,
  # see https://yurydelendik.github.io/webassembly-dwarf/#external-DWARF
  section_name = b'\x13external_debug_info' # section name, including prefixed size
  filename_bytes = embedded_path.encode('utf-8')
  contents = webassembly.toLEB(len(filename_bytes)) + filename_bytes
  section_size = len(section_name) + len(contents)
  with open(wasm_file, 'ab') as f:
    f.write(b'\0') # user section is code 0
    f.write(webassembly.toLEB(section_size))
    f.write(section_name)
    f.write(contents)


def little_endian_heap(js_file):
  logger.debug('enforcing little endian heap byte order')
  return acorn_optimizer(js_file, ['littleEndianHeap'])


def apply_wasm_memory_growth(js_file):
  logger.debug('supporting wasm memory growth with pthreads')
  fixed = acorn_optimizer(js_file, ['growableHeap'])
  ret = js_file + '.pgrow.js'
  fixed = utils.read_file(fixed)
  support_code = utils.read_file(path_from_root('src/growableHeap.js'))
  utils.write_file(ret, support_code + '\n' + fixed)
  return ret


def use_unsigned_pointers_in_js(js_file):
  logger.debug('using unsigned pointers in JS')
  return acorn_optimizer(js_file, ['unsignPointers'])


def instrument_js_for_asan(js_file):
  logger.debug('instrumenting JS memory accesses for ASan')
  return acorn_optimizer(js_file, ['asanify'])


def instrument_js_for_safe_heap(js_file):
  logger.debug('instrumenting JS memory accesses for SAFE_HEAP')
  return acorn_optimizer(js_file, ['safeHeap'])


def handle_final_wasm_symbols(wasm_file, symbols_file, debug_info):
  logger.debug('handle_final_wasm_symbols')
  args = []
  if symbols_file:
    args += ['--print-function-map']
  if not debug_info:
    # to remove debug info, we just write to that same file, and without -g
    args += ['-o', wasm_file]
  else:
    # suppress the wasm-opt warning regarding "no output file specified"
    args += ['--quiet']
  output = run_wasm_opt(wasm_file, args=args, stdout=PIPE)
  if symbols_file:
    utils.write_file(symbols_file, output)


def is_ar(filename):
  try:
    if _is_ar_cache.get(filename):
      return _is_ar_cache[filename]
    header = open(filename, 'rb').read(8)
    sigcheck = header in (b'!<arch>\n', b'!<thin>\n')
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
  if not os.path.isfile(filename):
    return False
  header = open(filename, 'rb').read(webassembly.HEADER_SIZE)
  return header == webassembly.MAGIC + webassembly.VERSION


def is_wasm_dylib(filename):
  """Detect wasm dynamic libraries by the presence of the "dylink" custom section."""
  if not is_wasm(filename):
    return False
  module = webassembly.Module(filename)
  section = next(module.sections())
  if section.type == webassembly.SecType.CUSTOM:
    module.seek(section.offset)
    if module.readString() in ('dylink', 'dylink.0'):
      return True
  return False


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
    'websocket': ['library_websocket.js'],
    # These 4 libraries are seperate under glibc but are all rolled into
    # libc with musl.  For compatibility with glibc we just ignore them
    # completely.
    'dl': [],
    'm': [],
    'rt': [],
    'pthread': [],
    # This is the name of GNU's C++ standard library. We ignore it here
    # for compatability with GNU toolchains.
    'stdc++': [],
  }
  # And some are hybrid and require JS and native libraries to be included
  native_library_map = {
    'embind': 'libembind',
    'GL': 'libGL',
  }

  if library_name in library_map:
    libs = library_map[library_name]
    logger.debug('Mapping library `%s` to JS libraries: %s' % (library_name, libs))
    return (libs, native_library_map.get(library_name))

  if library_name.endswith('.js') and os.path.isfile(path_from_root('src', f'library_{library_name}')):
    return ([f'library_{library_name}'], None)

  return (None, None)


# Map a linker flag to a settings. This lets a user write -lSDL2 and it will
# have the same effect as -sUSE_SDL=2.
def map_and_apply_to_settings(library_name):
  # most libraries just work, because the -l name matches the name of the
  # library we build. however, if a library has variations, which cause us to
  # build multiple versions with multiple names, then we need this mechanism.
  library_map = {
    # SDL2_mixer's built library name contains the specific codecs built in.
    'SDL2_mixer': [('USE_SDL_MIXER', 2)],
  }

  if library_name in library_map:
    for key, value in library_map[library_name]:
      logger.debug('Mapping library `%s` to settings changes: %s = %s' % (library_name, key, value))
      setattr(settings, key, value)
    return True

  return False


def emit_wasm_source_map(wasm_file, map_file, final_wasm):
  # source file paths must be relative to the location of the map (which is
  # emitted alongside the wasm)
  base_path = os.path.dirname(os.path.abspath(final_wasm))
  sourcemap_cmd = [PYTHON, path_from_root('tools/wasm-sourcemap.py'),
                   wasm_file,
                   '--dwarfdump=' + LLVM_DWARFDUMP,
                   '-o',  map_file,
                   '--basepath=' + base_path]
  check_call(sourcemap_cmd)


def get_binaryen_feature_flags():
  # settings.BINARYEN_FEATURES is empty unless features have been extracted by
  # wasm-emscripten-finalize already.
  if settings.BINARYEN_FEATURES:
    return settings.BINARYEN_FEATURES
  else:
    return ['--detect-features']


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
  global binaryen_checked
  rtn = os.path.join(config.BINARYEN_ROOT, 'bin')
  if not binaryen_checked:
    check_binaryen(rtn)
    binaryen_checked = True
  return rtn


# track whether the last binaryen command kept debug info around. this is used
# to see whether we need to do an extra step at the end to strip it.
binaryen_kept_debug_info = False


def run_binaryen_command(tool, infile, outfile=None, args=[], debug=False, stdout=None):
  cmd = [os.path.join(get_binaryen_bin(), tool)]
  cmd += args
  if infile:
    cmd += [infile]
  if outfile:
    cmd += ['-o', outfile]
    if settings.ERROR_ON_WASM_CHANGES_AFTER_LINK:
      # emit some extra helpful text for common issues
      extra = ''
      # a plain -O0 build *almost* doesn't need post-link changes, except for
      # legalization. show a clear error for those (as the flags the user passed
      # in are not enough to see what went wrong)
      if settings.LEGALIZE_JS_FFI:
        extra += '\nnote: to disable int64 legalization (which requires changes after link) use -sWASM_BIGINT'
      if settings.OPT_LEVEL > 0:
        extra += '\nnote: -O2+ optimizations always require changes, build with -O0 or -O1 instead'
      exit_with_error(f'changes to the wasm are required after link, but disallowed by ERROR_ON_WASM_CHANGES_AFTER_LINK: {cmd}{extra}')
  if debug:
    cmd += ['-g'] # preserve the debug info
  # if the features are not already handled, handle them
  cmd += get_binaryen_feature_flags()
  # if we are emitting a source map, every time we load and save the wasm
  # we must tell binaryen to update it
  # TODO: all tools should support source maps; wasm-ctor-eval does not atm,
  #       for example
  if settings.GENERATE_SOURCE_MAP and outfile and tool in ['wasm-opt', 'wasm-emscripten-finalize']:
    cmd += [f'--input-source-map={infile}.map']
    cmd += [f'--output-source-map={outfile}.map']
  ret = check_call(cmd, stdout=stdout).stdout
  if outfile:
    save_intermediate(outfile, '%s.wasm' % tool)
    global binaryen_kept_debug_info
    binaryen_kept_debug_info = '-g' in cmd
  return ret


def run_wasm_opt(infile, outfile=None, args=[], **kwargs):
  if outfile and (settings.DEBUG_LEVEL < 3 or settings.GENERATE_SOURCE_MAP):
    # remove any dwarf debug info sections, if the debug level is <3, as
    # we don't need them; also remove them if we use source maps (which are
    # implemented separately from dwarf).
    # note that we add this pass first, so that it doesn't interfere with
    # the final set of passes (which may generate stack IR, and nothing
    # should be run after that)
    # TODO: if lld can strip dwarf then we don't need this. atm though it can
    #       only strip all debug info or none, which includes the name section
    #       which we may need
    # TODO: once fastcomp is gone, either remove source maps entirely, or
    #       support them by emitting a source map at the end from the dwarf,
    #       and use llvm-objcopy to remove that final dwarf
    args.insert(0, '--strip-dwarf')
  return run_binaryen_command('wasm-opt', infile, outfile, args=args, **kwargs)


save_intermediate_counter = 0


def save_intermediate(src, dst):
  if DEBUG:
    global save_intermediate_counter
    dst = 'emcc-%d-%s' % (save_intermediate_counter, dst)
    save_intermediate_counter += 1
    dst = os.path.join(CANONICAL_TEMP_DIR, dst)
    logger.debug('saving debug copy %s' % dst)
    shutil.copyfile(src, dst)
