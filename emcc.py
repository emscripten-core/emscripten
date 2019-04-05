#!/usr/bin/env python2
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

  EMMAKEN_NO_SDK - Will tell emcc *not* to use the emscripten headers. Instead
                   your system headers will be used.

  EMMAKEN_COMPILER - The compiler to be used, if you don't want the default clang.
"""

from __future__ import print_function

import json
import logging
import os
import re
import shlex
import shutil
import stat
import sys
import time
from subprocess import PIPE

from tools import shared, system_libs, client_mods, js_optimizer, jsrun
from tools.shared import unsuffixed, unsuffixed_basename, WINDOWS, safe_copy, safe_move, run_process, asbytes, read_and_preprocess, exit_with_error, DEBUG
from tools.response_file import substitute_response_files
import tools.line_endings
from tools.toolchain_profiler import ToolchainProfiler
if __name__ == '__main__':
  ToolchainProfiler.record_process_start()

try:
  from urllib.parse import quote
except ImportError:
  # Python 2 compatibility
  from urllib import quote

logger = logging.getLogger('emcc')

# endings = dot + a suffix, safe to test by  filename.endswith(endings)
C_ENDINGS = ('.c', '.C', '.i')
CXX_ENDINGS = ('.cpp', '.cxx', '.cc', '.c++', '.CPP', '.CXX', '.CC', '.C++', '.ii')
OBJC_ENDINGS = ('.m', '.mi')
OBJCXX_ENDINGS = ('.mm', '.mii')
SPECIAL_ENDINGLESS_FILENAMES = ('/dev/null',)

SOURCE_ENDINGS = C_ENDINGS + CXX_ENDINGS + OBJC_ENDINGS + OBJCXX_ENDINGS + SPECIAL_ENDINGLESS_FILENAMES
C_ENDINGS = C_ENDINGS + SPECIAL_ENDINGLESS_FILENAMES # consider the special endingless filenames like /dev/null to be C

JS_CONTAINING_ENDINGS = ('.js', '.mjs', '.html')
BITCODE_ENDINGS = ('.bc', '.o', '.obj', '.lo')
DYNAMICLIB_ENDINGS = ('.dylib', '.so') # Windows .dll suffix is not included in this list, since those are never linked to directly on the command line.
STATICLIB_ENDINGS = ('.a',)
ASSEMBLY_ENDINGS = ('.ll',)
HEADER_ENDINGS = ('.h', '.hxx', '.hpp', '.hh', '.H', '.HXX', '.HPP', '.HH')
WASM_ENDINGS = ('.wasm', '.wast')

SUPPORTED_LINKER_FLAGS = (
    '--start-group', '--end-group',
    '-(', '-)',
    '--whole-archive', '--no-whole-archive',
    '-whole-archive', '-no-whole-archive')

LIB_PREFIXES = ('', 'lib')


DEFERRED_RESPONSE_FILES = ('EMTERPRETIFY_BLACKLIST', 'EMTERPRETIFY_WHITELIST', 'EMTERPRETIFY_SYNCLIST')

# Mapping of emcc opt levels to llvm opt levels. We use llvm opt level 3 in emcc
# opt levels 2 and 3 (emcc 3 is unsafe opts, so unsuitable for the only level to
# get llvm opt level 3, and speed-wise emcc level 2 is already the slowest/most
# optimizing level)
LLVM_OPT_LEVEL = {
  0: ['-O0'],
  1: ['-O1'],
  2: ['-O3'],
  3: ['-O3'],
}

# Do not compile .ll files into .bc, just compile them with emscripten directly
# Not recommended, this is mainly for the test runner, or if you have some other
# specific need.
# One major limitation with this mode is that libc and libc++ cannot be
# added in. Also, LLVM optimizations will not be done, nor dead code elimination
LEAVE_INPUTS_RAW = int(os.environ.get('EMCC_LEAVE_INPUTS_RAW', '0'))

# If emcc is running with LEAVE_INPUTS_RAW and then launches an emcc to build
# something like the struct info, then we don't want LEAVE_INPUTS_RAW to be
# active in that emcc subprocess.
if LEAVE_INPUTS_RAW:
  del os.environ['EMCC_LEAVE_INPUTS_RAW']

# If set to 1, we will run the autodebugger (the automatic debugging tool, see
# tools/autodebugger).  Note that this will disable inclusion of libraries. This
# is useful because including dlmalloc makes it hard to compare native and js
# builds
AUTODEBUG = os.environ.get('EMCC_AUTODEBUG')

# Target options
final = None


class Intermediate(object):
  counter = 0


# this function uses the global 'final' variable, which contains the current
# final output file. if a method alters final, and calls this method, then it
# must modify final globally (i.e. it can't receive final as a param and
# return it)
# TODO: refactor all this, a singleton that abstracts over the final output
#       and saving of intermediates
def save_intermediate(name, suffix='js'):
  if not DEBUG:
    return
  name = os.path.join(shared.get_emscripten_temp_dir(), 'emcc-%d-%s.%s' % (Intermediate.counter, name, suffix))
  if isinstance(final, list):
    logger.debug('(not saving intermediate %s because deferring linking)' % name)
    return
  shutil.copyfile(final, name)
  Intermediate.counter += 1


def save_intermediate_with_wasm(name, wasm_binary):
  if not DEBUG:
    return
  save_intermediate(name) # save the js
  name = os.path.join(shared.get_emscripten_temp_dir(), 'emcc-%d-%s.wasm' % (Intermediate.counter - 1, name))
  shutil.copyfile(wasm_binary, name)


class TimeLogger(object):
  last = time.time()

  @staticmethod
  def update():
    TimeLogger.last = time.time()


def log_time(name):
  """Log out times for emcc stages"""
  if DEBUG:
    now = time.time()
    logger.debug('emcc step "%s" took %.2f seconds', name, now - TimeLogger.last)
    TimeLogger.update()


class EmccOptions(object):
  def __init__(self):
    self.opt_level = 0
    self.debug_level = 0
    self.shrink_level = 0
    self.requested_debug = ''
    self.profiling = False
    self.profiling_funcs = False
    self.tracing = False
    self.emit_symbol_map = False
    self.js_opts = None
    self.force_js_opts = False
    self.llvm_opts = None
    self.llvm_lto = None
    self.default_cxx_std = '-std=c++03' # Enforce a consistent C++ standard when compiling .cpp files, if user does not specify one on the cmdline.
    self.use_closure_compiler = None
    self.js_transform = None
    self.pre_js = '' # before all js
    self.post_js = '' # after all js
    self.preload_files = []
    self.embed_files = []
    self.exclude_files = []
    self.ignore_dynamic_linking = False
    self.shell_path = shared.path_from_root('src', 'shell.html')
    self.source_map_base = ''
    self.js_libraries = []
    self.bind = False
    self.emrun = False
    self.cpu_profiler = False
    self.thread_profiler = False
    self.memory_profiler = False
    self.save_bc = False
    self.memory_init_file = None
    self.use_preload_cache = False
    self.no_heap_copy = False
    self.use_preload_plugins = False
    self.proxy_to_worker = False
    self.default_object_extension = '.o'
    self.valid_abspaths = []
    self.separate_asm = False
    self.cfi = False
    # Specifies the line ending format to use for all generated text files.
    # Defaults to using the native EOL on each platform (\r\n on Windows, \n on
    # Linux & MacOS)
    self.output_eol = os.linesep


def use_source_map(options):
  return options.debug_level >= 4


def will_metadce(options):
  return options.opt_level >= 3 or options.shrink_level >= 1


class JSOptimizer(object):
  def __init__(self, target, options, js_transform_tempfiles, in_temp):
    self.queue = []
    self.extra_info = {}
    self.queue_history = []
    self.blacklist = os.environ.get('EMCC_JSOPT_BLACKLIST', '').split(',')
    self.minify_whitespace = False
    self.cleanup_shell = False

    self.target = target
    self.opt_level = options.opt_level
    self.debug_level = options.debug_level
    self.emit_symbol_map = options.emit_symbol_map
    self.profiling_funcs = options.profiling_funcs
    self.use_closure_compiler = options.use_closure_compiler

    self.js_transform_tempfiles = js_transform_tempfiles
    self.in_temp = in_temp

  def flush(self, title='js_opts'):
    self.queue = [p for p in self.queue if p not in self.blacklist]

    assert not shared.Settings.WASM_BACKEND, 'JSOptimizer should not run with pure wasm output'

    if self.extra_info is not None and len(self.extra_info) == 0:
      self.extra_info = None

    if len(self.queue) and not(not shared.Settings.ASM_JS and len(self.queue) == 1 and self.queue[0] == 'last'):
      passes = self.queue[:]

      if DEBUG != 2 or len(passes) < 2:
        # by assumption, our input is JS, and our output is JS. If a pass is going to run in the native optimizer in C++, then we
        # must give it JSON and receive from it JSON
        chunks = []
        curr = []
        for p in passes:
          if len(curr) == 0:
            curr.append(p)
          else:
            native = js_optimizer.use_native(p, source_map=use_source_map(self))
            last_native = js_optimizer.use_native(curr[-1], source_map=use_source_map(self))
            if native == last_native:
              curr.append(p)
            else:
              curr.append('emitJSON')
              chunks.append(curr)
              curr = ['receiveJSON', p]
        if len(curr):
          chunks.append(curr)
        if len(chunks) == 1:
          self.run_passes(chunks[0], title, just_split=False, just_concat=False)
        else:
          for i, chunk in enumerate(chunks):
            self.run_passes(chunk, 'js_opts_' + str(i),
                            just_split='receiveJSON' in chunk,
                            just_concat='emitJSON' in chunk)
      else:
        # DEBUG 2, run each pass separately
        extra_info = self.extra_info
        for p in passes:
          self.queue = [p]
          self.flush(p)
          self.extra_info = extra_info # flush wipes it
          log_time('part of js opts')
      self.queue_history += self.queue
      self.queue = []
    self.extra_info = {}

  def run_passes(self, passes, title, just_split, just_concat):
    global final
    passes = ['asm'] + passes
    if shared.Settings.PRECISE_F32:
      passes = ['asmPreciseF32'] + passes
    if (self.emit_symbol_map or shared.Settings.CYBERDWARF) and 'minifyNames' in passes:
      passes += ['symbolMap=' + self.target + '.symbols']
    if self.profiling_funcs and 'minifyNames' in passes:
      passes += ['profilingFuncs']
    if self.minify_whitespace and 'last' in passes:
      passes += ['minifyWhitespace']
    if self.cleanup_shell and 'last' in passes:
      passes += ['cleanup']
    logger.debug('applying js optimization passes: %s', ' '.join(passes))
    final = shared.Building.js_optimizer(final, passes, use_source_map(self),
                                         self.extra_info, just_split=just_split,
                                         just_concat=just_concat,
                                         output_filename=self.in_temp(os.path.basename(final) + '.jsopted.js'))
    self.js_transform_tempfiles.append(final)
    save_intermediate(title, suffix='js' if 'emitJSON' not in passes else 'json')

  def do_minify(self):
    """minifies the code.

    this is also when we do certain optimizations that must be done right before or after minification
    """
    if self.opt_level >= 2:
      if self.debug_level < 2 and not self.use_closure_compiler == 2:
        self.queue += ['minifyNames']
      if self.debug_level == 0:
        self.minify_whitespace = True

    if self.use_closure_compiler == 1:
      self.queue += ['closure']
    elif self.debug_level <= 2 and shared.Settings.FINALIZE_ASM_JS and not self.use_closure_compiler:
      self.cleanup_shell = True


def embed_memfile(options):
  return (shared.Settings.SINGLE_FILE or
          (shared.Settings.MEM_INIT_METHOD == 0 and
           (not shared.Settings.MAIN_MODULE and
            not shared.Settings.SIDE_MODULE and
            not use_source_map(options))))


def apply_settings(changes):
  """Take a list of settings in form `NAME=VALUE` and apply them to the global
  Settings object.
  """

  def standardize_setting_change(key, value):
    # Handle aliases in settings flags. These are settings whose name
    # has changed.
    settings_aliases = {
      'BINARYEN': 'WASM',
      'BINARYEN_MEM_MAX': 'WASM_MEM_MAX',
      # TODO: change most (all?) other BINARYEN* names to WASM*
    }
    key = settings_aliases.get(key, key)
    # boolean NO_X settings are aliases for X
    # (note that *non*-boolean setting values have special meanings,
    # and we can't just flip them, so leave them as-is to be
    # handled in a special way later)
    if key.startswith('NO_') and value in ('0', '1'):
      key = key[3:]
      value = str(1 - int(value))
    return key, value

  for change in changes:
    key, value = change.split('=', 1)
    key, value = standardize_setting_change(key, value)

    # In those settings fields that represent amount of memory, translate suffixes to multiples of 1024.
    if key in ('TOTAL_STACK', 'TOTAL_MEMORY', 'GL_MAX_TEMP_BUFFER_SIZE',
               'WASM_MEM_MAX', 'DEFAULT_PTHREAD_STACK_SIZE'):
      value = str(shared.expand_byte_size_suffixes(value))

    if value[0] == '@':
      if key not in DEFERRED_RESPONSE_FILES:
        value = open(value[1:]).read()
    else:
      value = value.replace('\\', '\\\\')
    try:
      setattr(shared.Settings, key, parse_value(value))
    except Exception as e:
      exit_with_error('a problem occured in evaluating the content after a "-s", specifically "%s": %s', change, str(e))

    if key == 'EXPORTED_FUNCTIONS':
      # used for warnings in emscripten.py
      shared.Settings.ORIGINAL_EXPORTED_FUNCTIONS = shared.Settings.EXPORTED_FUNCTIONS[:]


def find_output_arg(args):
  """Find and remove any -o arguments.  The final one takes precedence.
  Return the final -o target along with the remaining (non-o) arguments.
  """
  outargs = []
  specified_target = None
  use_next = False
  for arg in args:
    if use_next:
      specified_target = arg
      use_next = False
      continue
    if arg == '-o':
      use_next = True
    elif arg.startswith('-o'):
      specified_target = arg[2:]
    else:
      outargs.append(arg)
  return specified_target, outargs


#
# Main run() function
#
def run(args):
  global final
  target = None

  # Additional compiler flags that we treat as if they were passed to us on the
  # commandline
  EMCC_CFLAGS = os.environ.get('EMCC_CFLAGS')
  if DEBUG:
    cmd = ' '.join(args)
    if EMCC_CFLAGS:
      cmd += ' + ' + EMCC_CFLAGS
    logger.warning('invocation: ' + cmd + '  (in ' + os.getcwd() + ')')
  if EMCC_CFLAGS:
    args.extend(shlex.split(EMCC_CFLAGS))

  # Strip args[0] (program name)
  args = args[1:]

  if DEBUG and LEAVE_INPUTS_RAW:
    logger.warning('leaving inputs raw')

  if '--emscripten-cxx' in args:
    run_via_emxx = True
    args = [x for x in args if x != '--emscripten-cxx']
  else:
    run_via_emxx = False

  misc_temp_files = shared.configuration.get_temp_files()

  # Handle some global flags

  # read response files very early on
  args = substitute_response_files(args)

  if '--help' in args:
    # Documentation for emcc and its options must be updated in:
    #    site/source/docs/tools_reference/emcc.rst
    # A prebuilt local version of the documentation is available at:
    #    site/build/text/docs/tools_reference/emcc.txt
    #    (it is read from there and printed out when --help is invoked)
    # You can also build docs locally as HTML or other formats in site/
    # An online HTML version (which may be of a different version of Emscripten)
    #    is up at http://kripken.github.io/emscripten-site/docs/tools_reference/emcc.html

    print('''%s

------------------------------------------------------------------

emcc: supported targets: llvm bitcode, javascript, NOT elf
(autoconf likes to see elf above to enable shared object support)
''' % (open(shared.path_from_root('site', 'build', 'text', 'docs', 'tools_reference', 'emcc.txt')).read()))
    return 0

  if '--version' in args:
    revision = '(unknown revision)'
    here = os.getcwd()
    os.chdir(shared.path_from_root())
    try:
      revision = run_process(['git', 'show'], stdout=PIPE, stderr=PIPE).stdout.split('\n')[0]
    except:
      pass
    finally:
      os.chdir(here)
    print('''emcc (Emscripten gcc/clang-like replacement) %s (%s)
Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ''' % (shared.EMSCRIPTEN_VERSION, revision))
    return 0

  if len(args) == 1 and args[0] == '-v': # -v with no inputs
    # autoconf likes to see 'GNU' in the output to enable shared object support
    print('emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) %s' % shared.EMSCRIPTEN_VERSION, file=sys.stderr)
    code = run_process([shared.CLANG_CC, '-v'], check=False).returncode
    shared.check_sanity(force=True)
    return code

  shared.check_sanity(force=DEBUG)

  # This check comes after check_sanity because test_sanity expects this.
  if not args:
    logger.warning('no input files')
    return 1

  if '-dumpmachine' in args:
    print(shared.get_llvm_target())
    return 0

  if '-dumpversion' in args: # gcc's doc states "Print the compiler version [...] and don't do anything else."
    print(shared.EMSCRIPTEN_VERSION)
    return 0

  if '--cflags' in args:
    # fake running the command, to see the full args we pass to clang
    debug_env = os.environ.copy()
    debug_env['EMCC_DEBUG'] = '1'
    args = [x for x in args if x != '--cflags']
    with misc_temp_files.get_file(suffix='.o') as temp_target:
      input_file = 'hello_world.c'
      err = run_process([shared.PYTHON, sys.argv[0]] + args + [shared.path_from_root('tests', input_file), '-c', '-o', temp_target], stderr=PIPE, env=debug_env).stderr
      lines = [x for x in err.split('\n') if shared.CLANG_CC in x and input_file in x]
      line = re.search('running: (.*)', lines[0]).group(1)
      parts = shlex.split(line.replace('\\', '\\\\'))
      parts = [x for x in parts if x != '-c' and x != '-o' and input_file not in x and temp_target not in x and '-emit-llvm' not in x]
      print(' '.join(shared.Building.doublequote_spaces(parts[1:])))
    return 0

  # Default to using C++ even when run as `emcc`.
  # This means that emcc will act as a C++ linker when no source files are
  # specified.  However, when a C source is specified we do default to C.
  # This differs to clang and gcc where the default is always C unless run as
  # clang++/g++.
  use_cxx = True

  def get_language_mode(args):
    return_next = False
    for item in args:
      if return_next:
        return item
      if item == '-x':
        return_next = True
        continue
      if item.startswith('-x'):
        return item[2:]
    return None

  def has_c_source(args):
    for a in args:
      if a[0] != '-' and a.endswith(C_ENDINGS + OBJC_ENDINGS):
        return True
    return False

  language_mode = get_language_mode(args)
  has_fixed_language_mode = language_mode is not None
  if language_mode == 'c':
    use_cxx = False

  if not has_fixed_language_mode:
    if not run_via_emxx and has_c_source(args):
      use_cxx = False

  def is_minus_s_for_emcc(args, i):
    # -s OPT=VALUE or -s OPT are interpreted as emscripten flags.
    # -s by itself is a linker option (alias for --strip-all)
    assert args[i] == '-s'
    if len(args) > i + 1:
      arg = args[i + 1]
      if arg.split('=')[0].isupper():
        return True

    logger.debug('treating -s as linker option and not as -s OPT=VALUE for js compilation')
    return False

  # If this is a configure-type thing, do not compile to JavaScript, instead use clang
  # to compile to a native binary (using our headers, so things make sense later)
  CONFIGURE_CONFIG = (os.environ.get('EMMAKEN_JUST_CONFIGURE') or 'conftest.c' in args) and not os.environ.get('EMMAKEN_JUST_CONFIGURE_RECURSE')
  CMAKE_CONFIG = 'CMakeFiles/cmTryCompileExec.dir' in ' '.join(args)# or 'CMakeCCompilerId' in ' '.join(args)
  if CONFIGURE_CONFIG or CMAKE_CONFIG:
    # XXX use this to debug configure stuff. ./configure's generally hide our
    # normal output including stderr so we write to a file
    debug_configure = 0

    # Whether we fake configure tests using clang - the local, native compiler -
    # or not. if not we generate JS and use node with a shebang
    # Neither approach is perfect, you can try both, but may need to edit
    # configure scripts in some cases
    # By default we configure in js, which can break on local filesystem access,
    # etc., but is otherwise accurate so we
    # disable this if we think we have to. A value of '2' here will force JS
    # checks in all cases. In summary:
    # 0 - use native compilation for configure checks
    # 1 - use js when we think it will work
    # 2 - always use js for configure checks
    use_js = int(os.environ.get('EMCONFIGURE_JS') or 2)

    if debug_configure:
      tempout = '/tmp/emscripten_temp/out'
      if not os.path.exists(tempout):
        open(tempout, 'w').write('//\n')

    src = None
    for arg in args:
      if arg.endswith(SOURCE_ENDINGS):
        try:
          src = open(arg).read()
          if debug_configure:
            open(tempout, 'a').write('============= ' + arg + '\n' + src + '\n=============\n\n')
        except:
          pass
      elif arg.endswith('.s'):
        if debug_configure:
          open(tempout, 'a').write('(compiling .s assembly, must use clang\n')
        if use_js == 1:
          use_js = 0
      elif arg == '-E' or arg == '-M' or arg == '-MM':
        if use_js == 1:
          use_js = 0

    if src:
      if 'fopen' in src and '"w"' in src:
        if use_js == 1:
          use_js = 0 # we cannot write to files from js!
        if debug_configure:
          open(tempout, 'a').write('Forcing clang since uses fopen to write\n')

    # if CONFIGURE_CC is defined, use that. let's you use local gcc etc. if you need that
    compiler = os.environ.get('CONFIGURE_CC')
    if not compiler:
      compiler = shared.EMXX if use_js else shared.CLANG_CPP
    if 'CXXCompiler' not in ' '.join(args) and not use_cxx:
      compiler = shared.to_cc(compiler)

    def filter_emscripten_options(argv):
      skip_next = False
      for idx, arg in enumerate(argv):
        if skip_next:
          skip_next = False
          continue
        if not use_js and arg == '-s' and is_minus_s_for_emcc(argv, idx):
          # skip -s X=Y if not using js for configure
          skip_next = True
          continue
        if use_js or arg != '--tracing':
          yield arg

    if compiler in (shared.EMCC, shared.EMXX):
      compiler = [shared.PYTHON, compiler]
    else:
      compiler = [compiler]
    cmd = compiler + list(filter_emscripten_options(args))
    if not use_js:
      cmd += shared.EMSDK_OPTS + ['-D__EMSCRIPTEN__']
      # The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code
      # in strict mode. Code should use the define __EMSCRIPTEN__ instead.
      if not shared.Settings.STRICT:
        cmd += ['-DEMSCRIPTEN']
    if use_js:
      # configure tests want a more shell-like style, where we emit return codes on exit()
      cmd += ['-s', 'NO_EXIT_RUNTIME=0']
      # use node.js raw filesystem access, to behave just like a native executable
      cmd += ['-s', 'NODERAWFS=1']
      # Disable wasm in configuration checks so that (1) we do not depend on
      # wasm support just for configuration (perhaps the user does not intend
      # to build to wasm; using asm.js only depends on js which we need anyhow),
      # and (2) we don't have issues with a separate .wasm file
      # on the side, async startup, etc..
      if not shared.Settings.WASM_BACKEND:
        cmd += ['-s', 'WASM=0']

    logger.debug('just configuring: ' + ' '.join(cmd))
    if debug_configure:
      open(tempout, 'a').write('emcc, just configuring: ' + ' '.join(cmd) + '\n\n')

    if not use_js:
      return run_process(cmd, check=False).returncode

    only_object = '-c' in cmd
    for i in reversed(range(len(cmd) - 1)): # Last -o directive should take precedence, if multiple are specified
      if cmd[i] == '-o':
        if not only_object:
          cmd[i + 1] += '.js'
        target = cmd[i + 1]
        break
    if not target:
      target = 'a.out.js'
    os.environ['EMMAKEN_JUST_CONFIGURE_RECURSE'] = '1'
    ret = run_process(cmd, check=False).returncode
    os.environ['EMMAKEN_JUST_CONFIGURE_RECURSE'] = ''
    if not os.path.exists(target):
      # note that emcc -c will cause target to have the wrong value here;
      # but then, we don't care about bitcode outputs anyhow, below, so
      # skipping returning early is fine
      return ret
    if target.endswith('.js'):
      shutil.copyfile(target, unsuffixed(target))
      target = unsuffixed(target)
    if not target.endswith(BITCODE_ENDINGS):
      src = open(target).read()
      full_node = ' '.join(shared.NODE_JS)
      if os.path.sep not in full_node:
        full_node = '/usr/bin/' + full_node # TODO: use whereis etc. And how about non-*NIX?
      open(target, 'w').write('#!' + full_node + '\n' + src) # add shebang
      try:
        os.chmod(target, stat.S_IMODE(os.stat(target).st_mode) | stat.S_IXUSR) # make executable
      except:
        pass # can fail if e.g. writing the executable to /dev/null
    return ret

  CXX = os.environ.get('EMMAKEN_COMPILER', shared.CLANG_CPP)
  CC = shared.to_cc(CXX)

  EMMAKEN_CFLAGS = os.environ.get('EMMAKEN_CFLAGS')
  if EMMAKEN_CFLAGS:
    args += shlex.split(EMMAKEN_CFLAGS)

  # ---------------- Utilities ---------------

  def suffix(name):
    """Return the file extension"""
    return os.path.splitext(name)[1]

  seen_names = {}

  def uniquename(name):
    if name not in seen_names:
      seen_names[name] = str(len(seen_names))
    return unsuffixed(name) + '_' + seen_names[name] + suffix(name)

  # ---------------- End configs -------------

  # Check if a target is specified on the command line
  specified_target, args = find_output_arg(args)

  # specified_target is the user-specified one, target is what we will generate
  if specified_target:
    target = specified_target
  else:
    target = 'a.out.js'

  shared.Settings.TARGET_BASENAME = target_basename = unsuffixed_basename(target)

  final_suffix = suffix(target)

  temp_dir = shared.get_emscripten_temp_dir()

  def in_temp(name):
    return os.path.join(temp_dir, os.path.basename(name))

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

  def optimizing(opts):
    return '-O0' not in opts

  with ToolchainProfiler.profile_block('parse arguments and setup'):
    ## Parse args

    newargs = list(args)

    # Scan and strip emscripten specific cmdline warning flags.
    # This needs to run before other cmdline flags have been parsed, so that
    # warnings are properly printed during arg parse.
    newargs = shared.WarningManager.capture_warnings(newargs)

    for i in range(len(newargs)):
      if newargs[i] in ('-l', '-L', '-I'):
        # Scan for individual -l/-L/-I arguments and concatenate the next arg on
        # if there is no suffix
        newargs[i] += newargs[i + 1]
        newargs[i + 1] = ''

    options, settings_changes, newargs = parse_args(newargs)

    if use_cxx:
      clang_compiler = CXX
      # If user did not specify a default -std for C++ code, specify the emscripten default.
      if options.default_cxx_std:
        newargs += [options.default_cxx_std]
    else:
      # Compiling C code with .c files, don't enforce a default C++ std.
      clang_compiler = CC

    if options.emrun:
      options.pre_js += open(shared.path_from_root('src', 'emrun_prejs.js')).read() + '\n'
      options.post_js += open(shared.path_from_root('src', 'emrun_postjs.js')).read() + '\n'
      # emrun mode waits on program exit
      shared.Settings.EXIT_RUNTIME = 1

    if options.cpu_profiler:
      options.post_js += open(shared.path_from_root('src', 'cpuprofiler.js')).read() + '\n'

    if options.memory_profiler:
      options.post_js += open(shared.path_from_root('src', 'memoryprofiler.js')).read() + '\n'

    if options.thread_profiler:
      options.post_js += open(shared.path_from_root('src', 'threadprofiler.js')).read() + '\n'

    if options.js_opts is None:
      options.js_opts = options.opt_level >= 2

    if options.llvm_opts is None:
      options.llvm_opts = LLVM_OPT_LEVEL[options.opt_level]
    elif type(options.llvm_opts) == int:
      options.llvm_opts = ['-O%d' % options.llvm_opts]

    if options.memory_init_file is None:
      options.memory_init_file = options.opt_level >= 2

    # TODO: support source maps with js_transform
    if options.js_transform and use_source_map(options):
      logger.warning('disabling source maps because a js transform is being done')
      options.debug_level = 3

    if DEBUG:
      start_time = time.time() # done after parsing arguments, which might affect debug state

    for i in range(len(newargs)):
      if newargs[i] == '-s':
        if is_minus_s_for_emcc(newargs, i):
          key = newargs[i + 1]
          # If not = is specified default to 1
          if '=' not in key:
            key += '=1'
          settings_changes.append(key)
          newargs[i] = newargs[i + 1] = ''
          if key == 'WASM_BACKEND=1':
            exit_with_error('do not set -s WASM_BACKEND, instead set EMCC_WASM_BACKEND=1 in the environment')
    newargs = [arg for arg in newargs if arg is not '']

    settings_key_changes = set()
    for s in settings_changes:
      key, value = s.split('=', 1)
      settings_key_changes.add(key)

    # Find input files

    # These three arrays are used to store arguments of different types for
    # type-specific processing. In order to shuffle the arguments back together
    # after processing, all of these arrays hold tuples (original_index, value).
    # Note that the index part of the tuple can have a fractional part for input
    # arguments that expand into multiple processed arguments, as in -Wl,-f1,-f2.
    input_files = []
    libs = []
    link_flags = []

    # All of the above arg lists entries contain indexes into the full argument
    # list. In order to add extra implicit args (embind.cc, etc) below, we keep a
    # counter for the next index that should be used.
    next_arg_index = len(newargs)

    has_source_inputs = False
    has_header_inputs = False
    lib_dirs = [shared.path_from_root('system', 'local', 'lib'),
                shared.path_from_root('system', 'lib')]

    # find input files this a simple heuristic. we should really analyze
    # based on a full understanding of gcc params, right now we just assume that
    # what is left contains no more |-x OPT| things
    for i in range(len(newargs)):
      arg = newargs[i]
      if i > 0:
        prev = newargs[i - 1]
        if prev in ('-MT', '-MF', '-MQ', '-D', '-U', '-o', '-x',
                    '-Xpreprocessor', '-include', '-imacros', '-idirafter',
                    '-iprefix', '-iwithprefix', '-iwithprefixbefore',
                    '-isysroot', '-imultilib', '-A', '-isystem', '-iquote',
                    '-install_name', '-compatibility_version',
                    '-current_version', '-I', '-L', '-include-pch'):
          continue # ignore this gcc-style argument

      if os.path.islink(arg) and get_file_suffix(os.path.realpath(arg)) in SOURCE_ENDINGS + BITCODE_ENDINGS + DYNAMICLIB_ENDINGS + ASSEMBLY_ENDINGS + HEADER_ENDINGS:
        arg = os.path.realpath(arg)

      if not arg.startswith('-'):
        if not os.path.exists(arg):
          exit_with_error('%s: No such file or directory ("%s" was expected to be an input file, based on the commandline arguments provided)', arg, arg)

        file_suffix = get_file_suffix(arg)
        if file_suffix in SOURCE_ENDINGS + BITCODE_ENDINGS + DYNAMICLIB_ENDINGS + ASSEMBLY_ENDINGS + HEADER_ENDINGS or shared.Building.is_ar(arg): # we already removed -o <target>, so all these should be inputs
          newargs[i] = ''
          if file_suffix.endswith(SOURCE_ENDINGS):
            input_files.append((i, arg))
            has_source_inputs = True
          elif file_suffix.endswith(HEADER_ENDINGS):
            input_files.append((i, arg))
            has_header_inputs = True
          elif file_suffix.endswith(ASSEMBLY_ENDINGS) or shared.Building.is_bitcode(arg) or shared.Building.is_ar(arg):
            input_files.append((i, arg))
          elif 'WASM_OBJECT_FILES=0' not in settings_changes and shared.Building.is_wasm(arg):
            # this is before libraries, since wasm static libraries (wasm.so that contains wasm) are just
            # object files to be linked
            input_files.append((i, arg))
          elif file_suffix.endswith(STATICLIB_ENDINGS + DYNAMICLIB_ENDINGS):
            # if it's not, and it's a library, just add it to libs to find later
            l = unsuffixed_basename(arg)
            for prefix in LIB_PREFIXES:
              if not prefix:
                continue
              if l.startswith(prefix):
                l = l[len(prefix):]
                break
            libs.append((i, l))
            newargs[i] = ''
          else:
            logger.warning(arg + ' is not a valid input file')
        elif file_suffix.endswith(STATICLIB_ENDINGS):
          if not shared.Building.is_ar(arg):
            if shared.Building.is_bitcode(arg):
              message = arg + ': File has a suffix of a static library ' + str(STATICLIB_ENDINGS) + ', but instead is an LLVM bitcode file! When linking LLVM bitcode files, use one of the suffixes ' + str(BITCODE_ENDINGS)
            else:
              message = arg + ': Unknown format, not a static library!'
            exit_with_error(message)
        else:
          if has_fixed_language_mode:
            newargs[i] = ''
            input_files.append((i, arg))
            has_source_inputs = True
          else:
            exit_with_error(arg + ": Input file has an unknown suffix, don't know what to do with it!")
      elif arg.startswith('-L'):
        lib_dirs.append(arg[2:])
        newargs[i] = ''
      elif arg.startswith('-l'):
        libs.append((i, arg[2:]))
        newargs[i] = ''
      elif arg.startswith('-Wl,'):
        # Multiple comma separated link flags can be specified. Create fake
        # fractional indices for these: -Wl,a,b,c,d at index 4 becomes:
        # (4, a), (4.25, b), (4.5, c), (4.75, d)
        link_flags_to_add = arg.split(',')[1:]
        for flag_index, flag in enumerate(link_flags_to_add):
          link_flags.append((i + float(flag_index) / len(link_flags_to_add), flag))

        newargs[i] = ''
      elif arg == '-s':
        # -s and some other compiler flags are normally passed onto the linker
        # TODO(sbc): Pass this and other flags through when using lld
        # link_flags.append((i, arg))
        newargs[i] = ''

    original_input_files = input_files[:]

    newargs = [a for a in newargs if a is not '']

    # -c means do not link in gcc, and for us, the parallel is to not go all the way to JS, but stop at bitcode
    has_dash_c = '-c' in newargs
    if has_dash_c:
      assert has_source_inputs or has_header_inputs, 'Must have source code or header inputs to use -c'
      target = target_basename + '.o'
      final_suffix = '.o'
    if '-E' in newargs:
      final_suffix = '.eout' # not bitcode, not js; but just result from preprocessing stage of the input file
    if '-M' in newargs or '-MM' in newargs:
      final_suffix = '.mout' # not bitcode, not js; but just dependency rule of the input file

    # target is now finalized, can finalize other _target s
    if final_suffix == '.mjs':
      shared.Settings.EXPORT_ES6 = 1
      shared.Settings.MODULARIZE = 1
      js_target = target
    else:
      js_target = unsuffixed(target) + '.js'

    asm_target = unsuffixed(js_target) + '.asm.js' # might not be used, but if it is, this is the name
    wasm_text_target = asm_target.replace('.asm.js', '.wast') # ditto, might not be used
    wasm_binary_target = asm_target.replace('.asm.js', '.wasm') # ditto, might not be used
    wasm_source_map_target = wasm_binary_target + '.map'

    if final_suffix == '.html' and not options.separate_asm and 'PRECISE_F32=2' in settings_changes:
      options.separate_asm = True
      logger.warning('forcing separate asm output (--separate-asm), because -s PRECISE_F32=2 was passed.')
    if options.separate_asm:
      shared.Settings.SEPARATE_ASM = shared.JS.get_subresource_location(asm_target)

    # Libraries are searched before settings_changes are applied, so apply the
    # value for STRICT and ERROR_ON_MISSING_LIBRARIES from command line already
    # now.

    def get_last_setting_change(setting):
      return ([None] + [x for x in settings_changes if x.startswith(setting + '=')])[-1]

    strict_cmdline = get_last_setting_change('STRICT')
    if strict_cmdline:
      shared.Settings.STRICT = int(strict_cmdline.split('=', 1)[1])

    if shared.Settings.STRICT:
      shared.Settings.ERROR_ON_MISSING_LIBRARIES = 1

    error_on_missing_libraries_cmdline = get_last_setting_change('ERROR_ON_MISSING_LIBRARIES')
    if error_on_missing_libraries_cmdline:
      shared.Settings.ERROR_ON_MISSING_LIBRARIES = int(error_on_missing_libraries_cmdline[len('ERROR_ON_MISSING_LIBRARIES='):])

    settings_changes.append(process_libraries(libs, lib_dirs, settings_changes, input_files))

    # If not compiling to JS, then we are compiling to an intermediate bitcode objects or library, so
    # ignore dynamic linking, since multiple dynamic linkings can interfere with each other
    if get_file_suffix(target) not in JS_CONTAINING_ENDINGS or options.ignore_dynamic_linking:
      def check(input_file):
        if get_file_suffix(input_file) in DYNAMICLIB_ENDINGS:
          if not options.ignore_dynamic_linking:
            logger.warning('ignoring dynamic library %s because not compiling to JS or HTML, remember to link it when compiling to JS or HTML at the end', os.path.basename(input_file))
          return False
        else:
          return True
      input_files = [f for f in input_files if check(f[1])]

    if len(input_files) == 0:
      exit_with_error('no input files\nnote that input files without a known suffix are ignored, make sure your input files end with one of: ' + str(SOURCE_ENDINGS + BITCODE_ENDINGS + DYNAMICLIB_ENDINGS + STATICLIB_ENDINGS + ASSEMBLY_ENDINGS + HEADER_ENDINGS))

    newargs = shared.COMPILER_OPTS + newargs

    if options.separate_asm and final_suffix != '.html':
      shared.WarningManager.warn('SEPARATE_ASM')

    # Apply optimization level settings
    shared.Settings.apply_opt_level(opt_level=options.opt_level, shrink_level=options.shrink_level, noisy=True)

    # For users that opt out of WARN_ON_UNDEFINED_SYMBOLS we assume they also
    # want to opt out of ERROR_ON_UNDEFINED_SYMBOLS.
    if 'WARN_ON_UNDEFINED_SYMBOLS=0' in settings_changes:
      shared.Settings.ERROR_ON_UNDEFINED_SYMBOLS = 0

    # Set ASM_JS default here so that we can override it from the command line.
    shared.Settings.ASM_JS = 1 if options.opt_level > 0 else 2

    # Apply -s settings in newargs here (after optimization levels, so they can override them)
    apply_settings(settings_changes)

    shared.verify_settings()

    # Reconfigure the cache now that settings have been applied (e.g. WASM_OBJECT_FILES)
    # TODO: remove
    shared.reconfigure_cache()

    # Note the exports the user requested
    shared.Building.user_requested_exports = shared.Settings.EXPORTED_FUNCTIONS[:]

    if options.bind:
      # If we are using embind and generating JS, now is the time to link in bind.cpp
      if final_suffix in JS_CONTAINING_ENDINGS:
        input_files.append((next_arg_index, shared.path_from_root('system', 'lib', 'embind', 'bind.cpp')))
        next_arg_index += 1

    # -s ASSERTIONS=1 implies the heaviest stack overflow check mode. Set the implication here explicitly to avoid having to
    # do preprocessor "#if defined(ASSERTIONS) || defined(STACK_OVERFLOW_CHECK)" in .js files, which is not supported.
    if shared.Settings.ASSERTIONS:
      shared.Settings.STACK_OVERFLOW_CHECK = 2

    if shared.Settings.WASM_OBJECT_FILES and not shared.Settings.WASM_BACKEND:
      if 'WASM_OBJECT_FILES=1' in settings_changes:
        logger.error('WASM_OBJECT_FILES can only be used with wasm backend')
        return 1
      shared.Settings.WASM_OBJECT_FILES = 0

    if not shared.Settings.STRICT:
      # The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code
      # in strict mode. Code should use the define __EMSCRIPTEN__ instead.
      shared.COMPILER_OPTS += ['-DEMSCRIPTEN']

    # Use settings

    if options.debug_level > 1 and options.use_closure_compiler:
      logger.warning('disabling closure because debug info was requested')
      options.use_closure_compiler = False

    assert not (shared.Settings.EMTERPRETIFY_FILE and shared.Settings.SINGLE_FILE), 'cannot have both EMTERPRETIFY_FILE and SINGLE_FILE enabled at the same time'

    assert not (not shared.Settings.DYNAMIC_EXECUTION and options.use_closure_compiler), 'cannot have both NO_DYNAMIC_EXECUTION and closure compiler enabled at the same time'

    if options.emrun:
      assert not shared.Settings.MINIMAL_RUNTIME, '--emrun is not compatible with -s MINIMAL_RUNTIME=1'
      shared.Settings.EXPORTED_RUNTIME_METHODS.append('addOnExit')

    if options.use_closure_compiler:
      shared.Settings.USE_CLOSURE_COMPILER = options.use_closure_compiler
      if not shared.check_closure_compiler():
        exit_with_error('fatal: closure compiler is not configured correctly')
      if options.use_closure_compiler == 2 and shared.Settings.ASM_JS == 1:
        shared.WarningManager.warn('ALMOST_ASM', 'not all asm.js optimizations are possible with --closure 2, disabling those - your code will be run more slowly')
        shared.Settings.ASM_JS = 2

    if shared.Settings.MAIN_MODULE:
      assert not shared.Settings.SIDE_MODULE
      if shared.Settings.MAIN_MODULE != 2:
        shared.Settings.INCLUDE_FULL_LIBRARY = 1
    elif shared.Settings.SIDE_MODULE:
      assert not shared.Settings.MAIN_MODULE
      options.memory_init_file = False # memory init file is not supported with asm.js side modules, must be executable synchronously (for dlopen)

    if shared.Settings.MAIN_MODULE or shared.Settings.SIDE_MODULE:
      assert shared.Settings.ASM_JS, 'module linking requires asm.js output (-s ASM_JS=1)'
      if shared.Settings.MAIN_MODULE != 2 and shared.Settings.SIDE_MODULE != 2:
        shared.Settings.LINKABLE = 1
      shared.Settings.RELOCATABLE = 1
      assert not options.use_closure_compiler, 'cannot use closure compiler on shared modules'
      # shared modules need memory utilities to allocate their memory
      shared.Settings.EXPORTED_RUNTIME_METHODS += [
        'allocate',
        'getMemory',
      ]
      shared.Settings.ALLOW_TABLE_GROWTH = 1

    if shared.Settings.USE_PTHREADS:
      # These runtime methods are called from worker.js
      shared.Settings.EXPORTED_RUNTIME_METHODS += ['establishStackSpace', 'dynCall_ii']

    if shared.Settings.MODULARIZE_INSTANCE:
      shared.Settings.MODULARIZE = 1

    if shared.Settings.MODULARIZE:
      assert not options.proxy_to_worker, '-s MODULARIZE=1 and -s MODULARIZE_INSTANCE=1 are not compatible with --proxy-to-worker (if you want to run in a worker with -s MODULARIZE=1, you likely want to do the worker side setup manually)'

    if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS:
      shared.Settings.ALIASING_FUNCTION_POINTERS = 0

    if shared.Settings.LEGACY_VM_SUPPORT:
      # legacy vms don't have wasm
      assert not shared.Settings.WASM, 'LEGACY_VM_SUPPORT is only supported for asm.js, and not wasm. Build with -s WASM=0'
      shared.Settings.POLYFILL_OLD_MATH_FUNCTIONS = 1
      shared.Settings.WORKAROUND_IOS_9_RIGHT_SHIFT_BUG = 1
      shared.Settings.WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG = 1

    # Silently drop any individual backwards compatibility emulation flags that are known never to occur on browsers that support WebAssembly.
    if shared.Settings.WASM:
      shared.Settings.POLYFILL_OLD_MATH_FUNCTIONS = 0
      shared.Settings.WORKAROUND_IOS_9_RIGHT_SHIFT_BUG = 0
      shared.Settings.WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG = 0

    if shared.Settings.STB_IMAGE and final_suffix in JS_CONTAINING_ENDINGS:
      input_files.append((next_arg_index, shared.path_from_root('third_party', 'stb_image.c')))
      next_arg_index += 1
      shared.Settings.EXPORTED_FUNCTIONS += ['_stbi_load', '_stbi_load_from_memory', '_stbi_image_free']
      # stb_image 2.x need to have STB_IMAGE_IMPLEMENTATION defined to include the implementation when compiling
      newargs.append('-DSTB_IMAGE_IMPLEMENTATION')

    if shared.Settings.ASMFS and final_suffix in JS_CONTAINING_ENDINGS:
      input_files.append((next_arg_index, shared.path_from_root('system', 'lib', 'fetch', 'asmfs.cpp')))
      newargs.append('-D__EMSCRIPTEN_ASMFS__=1')
      next_arg_index += 1
      shared.Settings.FILESYSTEM = 0
      shared.Settings.FETCH = 1
      options.js_libraries.append(shared.path_from_root('src', 'library_asmfs.js'))

    if shared.Settings.FETCH and final_suffix in JS_CONTAINING_ENDINGS:
      input_files.append((next_arg_index, shared.path_from_root('system', 'lib', 'fetch', 'emscripten_fetch.cpp')))
      next_arg_index += 1
      options.js_libraries.append(shared.path_from_root('src', 'library_fetch.js'))
      if shared.Settings.USE_PTHREADS:
        shared.Settings.FETCH_WORKER_FILE = unsuffixed(os.path.basename(target)) + '.fetch.js'

    forced_stdlibs = []
    if shared.Settings.DEMANGLE_SUPPORT:
      shared.Settings.EXPORTED_FUNCTIONS += ['___cxa_demangle']
      forced_stdlibs += ['libc++abi']

    if not shared.Settings.ONLY_MY_CODE and not shared.Settings.MINIMAL_RUNTIME:
      # Always need malloc and free to be kept alive and exported, for internal use and other modules
      shared.Settings.EXPORTED_FUNCTIONS += ['_malloc', '_free']
      if shared.Settings.WASM_BACKEND:
        # setjmp/longjmp and exception handling JS code depends on this so we
        # include it by default.  Should be elimiated by meta-DCE if unused.
        shared.Settings.EXPORTED_FUNCTIONS += ['_setThrew']

    if shared.Settings.RELOCATABLE and not shared.Settings.DYNAMIC_EXECUTION:
      exit_with_error('cannot have both DYNAMIC_EXECUTION=0 and RELOCATABLE enabled at the same time, since RELOCATABLE needs to eval()')

    if shared.Settings.RELOCATABLE:
      assert shared.Settings.GLOBAL_BASE < 1
      if 'EMULATED_FUNCTION_POINTERS' not in settings_key_changes and not shared.Settings.WASM_BACKEND:
        shared.Settings.EMULATED_FUNCTION_POINTERS = 2 # by default, use optimized function pointer emulation
      shared.Settings.ERROR_ON_UNDEFINED_SYMBOLS = 0
      shared.Settings.WARN_ON_UNDEFINED_SYMBOLS = 0

    if shared.Settings.EMTERPRETIFY:
      shared.Settings.FINALIZE_ASM_JS = 0
      # shared.Settings.GLOBAL_BASE = 8*256 # keep enough space at the bottom for a full stack frame, for z-interpreter
      shared.Settings.SIMPLIFY_IFS = 0 # this is just harmful for emterpreting
      shared.Settings.EXPORTED_FUNCTIONS += ['emterpret']
      if not options.js_opts:
        logger.debug('enabling js opts for EMTERPRETIFY')
        options.js_opts = True
      options.force_js_opts = True
      if options.use_closure_compiler == 2:
         exit_with_error('EMTERPRETIFY requires valid asm.js, and is incompatible with closure 2 which disables that')
      assert not use_source_map(options), 'EMTERPRETIFY is not compatible with source maps (maps are not useful in emterpreted code, and splitting out non-emterpreted source maps is not yet implemented)'

    if shared.Settings.DEAD_FUNCTIONS:
      if not options.js_opts:
        logger.debug('enabling js opts for DEAD_FUNCTIONS')
        options.js_opts = True
      options.force_js_opts = True

    if options.proxy_to_worker:
      shared.Settings.PROXY_TO_WORKER = 1

    if options.use_preload_plugins or len(options.preload_files) or len(options.embed_files):
      if shared.Settings.NODERAWFS:
        exit_with_error('--preload-file and --embed-file cannot be used with NODERAWFS which disables virtual filesystem')
      # if we include any files, or intend to use preload plugins, then we definitely need filesystem support
      shared.Settings.FORCE_FILESYSTEM = 1

    if options.proxy_to_worker or options.use_preload_plugins:
      shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$Browser']

    if shared.Settings.FILESYSTEM and not shared.Settings.ONLY_MY_CODE:
      if shared.Settings.SUPPORT_ERRNO:
        shared.Settings.EXPORTED_FUNCTIONS += ['___errno_location'] # so FS can report errno back to C
      # to flush streams on FS exit, we need to be able to call fflush
      # we only include it if the runtime is exitable, or when ASSERTIONS
      # (ASSERTIONS will check that streams do not need to be flushed,
      # helping people see when they should have disabled NO_EXIT_RUNTIME)
      if shared.Settings.EXIT_RUNTIME or shared.Settings.ASSERTIONS:
        shared.Settings.EXPORTED_FUNCTIONS += ['_fflush']

    if shared.Settings.USE_PTHREADS:
      if shared.Settings.USE_PTHREADS == 2:
        exit_with_error('USE_PTHREADS=2 is not longer supported')
      if shared.Settings.ALLOW_MEMORY_GROWTH:
        exit_with_error('Memory growth is not yet supported with pthreads')
      # UTF8Decoder.decode doesn't work with a view of a SharedArrayBuffer
      shared.Settings.TEXTDECODER = 0
      options.js_libraries.append(shared.path_from_root('src', 'library_pthread.js'))
      newargs.append('-D__EMSCRIPTEN_PTHREADS__=1')
      # set location of worker.js
      shared.Settings.PTHREAD_WORKER_FILE = unsuffixed(os.path.basename(target)) + '.worker.js'
    else:
      options.js_libraries.append(shared.path_from_root('src', 'library_pthread_stub.js'))

    if shared.Settings.FORCE_FILESYSTEM and not shared.Settings.MINIMAL_RUNTIME:
      # when the filesystem is forced, we export by default methods that filesystem usage
      # may need, including filesystem usage from standalone file packager output (i.e.
      # file packages not built together with emcc, but that are loaded at runtime
      # separately, and they need emcc's output to contain the support they need)
      if not shared.Settings.ASMFS:
        shared.Settings.EXPORTED_RUNTIME_METHODS += [
          'FS_createFolder',
          'FS_createPath',
          'FS_createDataFile',
          'FS_createPreloadedFile',
          'FS_createLazyFile',
          'FS_createLink',
          'FS_createDevice',
          'FS_unlink'
        ]

      shared.Settings.EXPORTED_RUNTIME_METHODS += [
        'getMemory',
        'addRunDependency',
        'removeRunDependency',
      ]

    if shared.Settings.USE_PTHREADS:
      if shared.Settings.MODULARIZE:
        # MODULARIZE+USE_PTHREADS mode requires extra exports out to Module so that worker.js
        # can access them:

        # general threading variables:
        shared.Settings.EXPORTED_RUNTIME_METHODS += ['PThread', 'ExitStatus']

        # pthread stack setup:
        shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['$establishStackSpaceInJsModule']
        shared.Settings.EXPORTED_FUNCTIONS += ['establishStackSpaceInJsModule']

        # stack check:
        if shared.Settings.STACK_OVERFLOW_CHECK:
          shared.Settings.EXPORTED_RUNTIME_METHODS += ['writeStackCookie', 'checkStackCookie']

      if shared.Settings.LINKABLE:
        exit_with_error('-s LINKABLE=1 is not supported with -s USE_PTHREADS>0!')
      if shared.Settings.SIDE_MODULE:
        exit_with_error('-s SIDE_MODULE=1 is not supported with -s USE_PTHREADS>0!')
      if shared.Settings.MAIN_MODULE:
        exit_with_error('-s MAIN_MODULE=1 is not supported with -s USE_PTHREADS>0!')
      if shared.Settings.EMTERPRETIFY:
        exit_with_error('-s EMTERPRETIFY=1 is not supported with -s USE_PTHREADS>0!')
      if shared.Settings.PROXY_TO_WORKER:
        exit_with_error('--proxy-to-worker is not supported with -s USE_PTHREADS>0! Use the option -s PROXY_TO_PTHREAD=1 if you want to run the main thread of a multithreaded application in a web worker.')
    else:
      if shared.Settings.PROXY_TO_PTHREAD:
        exit_with_error('-s PROXY_TO_PTHREAD=1 requires -s USE_PTHREADS to work!')

    if shared.Settings.OUTLINING_LIMIT:
      if shared.Settings.WASM_BACKEND:
        exit_with_error('OUTLINING_LIMIT is not compatible with the LLVM wasm backend')
      if not options.js_opts:
        logger.debug('enabling js opts as optional functionality implemented as a js opt was requested')
        options.js_opts = True
      options.force_js_opts = True

    # Enable minification of asm.js imports on -O1 and higher if -g1 or lower is used.
    if options.opt_level >= 1 and options.debug_level < 2 and not shared.Settings.WASM:
      shared.Settings.MINIFY_ASMJS_IMPORT_NAMES = 1

    if shared.Settings.WASM:
      if not shared.Building.need_asm_js_file():
        asm_target = asm_target.replace('.asm.js', '.temp.asm.js')
        misc_temp_files.note(asm_target)

    if shared.Settings.WASM:
      if shared.Settings.TOTAL_MEMORY % 65536 != 0:
        exit_with_error('For wasm, TOTAL_MEMORY must be a multiple of 64KB, was ' + str(shared.Settings.TOTAL_MEMORY))
    else:
      if shared.Settings.TOTAL_MEMORY < 16 * 1024 * 1024:
        exit_with_error('TOTAL_MEMORY must be at least 16MB, was ' + str(shared.Settings.TOTAL_MEMORY))
      if shared.Settings.TOTAL_MEMORY % (16 * 1024 * 1024) != 0:
        exit_with_error('For asm.js, TOTAL_MEMORY must be a multiple of 16MB, was ' + str(shared.Settings.TOTAL_MEMORY))
    if shared.Settings.TOTAL_MEMORY < shared.Settings.TOTAL_STACK:
      exit_with_error('TOTAL_MEMORY must be larger than TOTAL_STACK, was ' + str(shared.Settings.TOTAL_MEMORY) + ' (TOTAL_STACK=' + str(shared.Settings.TOTAL_STACK) + ')')
    if shared.Settings.WASM_MEM_MAX != -1 and shared.Settings.WASM_MEM_MAX % 65536 != 0:
      exit_with_error('WASM_MEM_MAX must be a multiple of 64KB, was ' + str(shared.Settings.WASM_MEM_MAX))
    if shared.Settings.USE_PTHREADS and shared.Settings.WASM and shared.Settings.ALLOW_MEMORY_GROWTH and shared.Settings.WASM_MEM_MAX == -1:
      exit_with_error('If pthreads and memory growth are enabled, WASM_MEM_MAX must be set')

    if shared.Settings.EXPORT_ES6 and not shared.Settings.MODULARIZE:
      exit_with_error('EXPORT_ES6 requires MODULARIZE to be set')

    # When MODULARIZE option is used, currently declare all module exports
    # individually - TODO: this could be optimized
    if shared.Settings.MODULARIZE and not shared.Settings.DECLARE_ASM_MODULE_EXPORTS:
      shared.Settings.DECLARE_ASM_MODULE_EXPORTS = 1
      logger.warning('Enabling -s DECLARE_ASM_MODULE_EXPORTS=1, since MODULARIZE currently requires declaring asm.js/wasm module exports in full')

    # In MINIMAL_RUNTIME when modularizing, by default output asm.js module under the same name as the JS module. This allows code to share same loading function for both JS and asm.js modules,
    # to save code size. The intent is that loader code captures the function variable from global scope to XHR loader local scope when it finishes loading, to avoid polluting global JS scope with
    # variables. This provides safety via encapsulation. See src/shell_minimal_runtime.html for an example.
    if shared.Settings.MINIMAL_RUNTIME and not shared.Settings.SEPARATE_ASM_MODULE_NAME and not shared.Settings.WASM and shared.Settings.MODULARIZE:
      shared.Settings.SEPARATE_ASM_MODULE_NAME = 'var ' + shared.Settings.EXPORT_NAME

    if shared.Settings.MODULARIZE and shared.Settings.SEPARATE_ASM and not shared.Settings.WASM and not shared.Settings.SEPARATE_ASM_MODULE_NAME:
      exit_with_error('Targeting asm.js with --separate-asm and -s MODULARIZE=1 requires specifying the target variable name to which the asm.js module is loaded into. See https://github.com/emscripten-core/emscripten/pull/7949 for details')
    # Apply default option if no custom name is provided
    if not shared.Settings.SEPARATE_ASM_MODULE_NAME:
      shared.Settings.SEPARATE_ASM_MODULE_NAME = 'Module["asm"]'
    elif shared.Settings.WASM:
      exit_with_error('-s SEPARATE_ASM_MODULE_NAME option only applies to when targeting asm.js, not with WebAssembly!')

    if shared.Settings.MINIMAL_RUNTIME:
      # Minimal runtime uses a different default shell file
      if options.shell_path == shared.path_from_root('src', 'shell.html'):
        options.shell_path = shared.path_from_root('src', 'shell_minimal_runtime.html')

      # Remove the default exported functions 'memcpy', 'memset', 'malloc', 'free', etc. - those should only be linked in if used
      shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = []

      # Always build with STRICT mode enabled
      shared.Settings.STRICT = 1

      # Always use the new HTML5 API event target lookup rules (TODO: enable this when the other PR lands)
      shared.Settings.DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR = 1

      # In asm.js always use memory init file to get the best code size, other modes are not currently supported.
      if not shared.Settings.WASM:
        options.memory_init_file = True

    if shared.Settings.MODULARIZE and not shared.Settings.MODULARIZE_INSTANCE and shared.Settings.EXPORT_NAME == 'Module' and final_suffix == '.html' and \
       (options.shell_path == shared.path_from_root('src', 'shell.html') or options.shell_path == shared.path_from_root('src', 'shell_minimal.html')):
      exit_with_error('Due to collision in variable name "Module", the shell file "' + options.shell_path + '" is not compatible with build options "-s MODULARIZE=1 -s EXPORT_NAME=Module". Either provide your own shell file, change the name of the export to something else to avoid the name collision. (see https://github.com/emscripten-core/emscripten/issues/7950 for details)')

    if shared.Settings.WASM:
      if shared.Settings.SINGLE_FILE:
        # placeholder strings for JS glue, to be replaced with subresource locations in do_binaryen
        shared.Settings.WASM_TEXT_FILE = shared.FilenameReplacementStrings.WASM_TEXT_FILE
        shared.Settings.WASM_BINARY_FILE = shared.FilenameReplacementStrings.WASM_BINARY_FILE
        shared.Settings.ASMJS_CODE_FILE = shared.FilenameReplacementStrings.ASMJS_CODE_FILE
      else:
        # set file locations, so that JS glue can find what it needs
        shared.Settings.WASM_TEXT_FILE = shared.JS.escape_for_js_string(os.path.basename(wasm_text_target))
        shared.Settings.WASM_BINARY_FILE = shared.JS.escape_for_js_string(os.path.basename(wasm_binary_target))
        shared.Settings.ASMJS_CODE_FILE = shared.JS.escape_for_js_string(os.path.basename(asm_target))
      shared.Settings.ASM_JS = 2 # when targeting wasm, we use a wasm Memory, but that is not compatible with asm.js opts
      # a higher global base is useful for optimizing load/store offsets, as it enables the --post-emscripten pass
      shared.Settings.GLOBAL_BASE = 1024
      if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS:
        logger.warning('for wasm there is no need to set ELIMINATE_DUPLICATE_FUNCTIONS, the binaryen optimizer does it automatically')
        shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS = 0
      if shared.Settings.OUTLINING_LIMIT:
        logger.warning('for wasm there is usually no need to set OUTLINING_LIMIT, as VMs can handle large functions well anyhow')
      # default precise-f32 to on, since it works well in wasm
      shared.Settings.PRECISE_F32 = 1
      if options.js_opts and not options.force_js_opts:
        options.js_opts = None
        logger.debug('asm.js opts not forced by user or an option that depends them, and we do not intend to run the asm.js, so disabling and leaving opts to the binaryen optimizer')
      if options.use_closure_compiler == 2:
        exit_with_error('closure compiler mode 2 assumes the code is asm.js, so not meaningful for wasm')
      # for simplicity, we always have a mem init file, which may also be imported into the wasm module.
      #  * if we also supported js mem inits we'd have 4 modes
      #  * and js mem inits are useful for avoiding a side file, but the wasm module avoids that anyhow
      if any(s.startswith('MEM_INIT_METHOD=') for s in settings_changes):
        exit_with_error('MEM_INIT_METHOD is not supported in wasm. Memory will be embedded in the wasm binary if threads are not used, and included in a separate file if threads are used.')
      options.memory_init_file = True

      if not shared.Settings.MINIMAL_RUNTIME: # BINARYEN_ASYNC_COMPILATION and SWAPPABLE_ASM_MODULE do not have a meaning in MINIMAL_RUNTIME (always async)
        if shared.Settings.BINARYEN_ASYNC_COMPILATION == 1:
          # async compilation requires a swappable module - we swap it in when it's ready
          shared.Settings.SWAPPABLE_ASM_MODULE = 1
        else:
          # if not wasm-only, we can't do async compilation as the build can run in other
          # modes than wasm (like asm.js) which may not support an async step
          shared.Settings.BINARYEN_ASYNC_COMPILATION = 0
          warning = 'This will reduce performance and compatibility (some browsers limit synchronous compilation), see http://kripken.github.io/emscripten-site/docs/compiling/WebAssembly.html#codegen-effects'
          if 'BINARYEN_ASYNC_COMPILATION=1' in settings_changes:
            logger.warning('BINARYEN_ASYNC_COMPILATION requested, but disabled because of user options. ' + warning)
          elif 'BINARYEN_ASYNC_COMPILATION=0' not in settings_changes:
            logger.warning('BINARYEN_ASYNC_COMPILATION disabled due to user options. ' + warning)

      if not shared.Settings.DECLARE_ASM_MODULE_EXPORTS:
        # Swappable wasm module/asynchronous wasm compilation requires an indirect stub
        # function generated to each function export from wasm module, so cannot use the
        # concise form of grabbing exports that does not need to refer to each export individually.
        if shared.Settings.SWAPPABLE_ASM_MODULE == 1:
          shared.Settings.DECLARE_ASM_MODULE_EXPORTS = 1
          logger.warning('Enabling -s DECLARE_ASM_MODULE_EXPORTS=1 since -s SWAPPABLE_ASM_MODULE=1 is used')

      # we will include the mem init data in the wasm, when we don't need the
      # mem init file to be loadable by itself
      shared.Settings.MEM_INIT_IN_WASM = not shared.Settings.USE_PTHREADS

      # wasm side modules have suffix .wasm
      if shared.Settings.SIDE_MODULE and target.endswith('.js'):
        logger.warning('output suffix .js requested, but wasm side modules are just wasm files; emitting only a .wasm, no .js')

      if options.separate_asm:
        exit_with_error('cannot --separate-asm when emitting wasm, since not emitting asm.js')

      if shared.Settings.WASM_BACKEND:
        options.js_opts = None

        # wasm backend output can benefit from the binaryen optimizer (in asm2wasm,
        # we run the optimizer during asm2wasm itself). use it, if not overridden
        if 'BINARYEN_PASSES' not in settings_key_changes:
          passes = []
          if not shared.Settings.EXIT_RUNTIME:
            passes += ['--no-exit-runtime']
          if options.opt_level > 0 or options.shrink_level > 0:
            passes += [shared.Building.opt_level_to_str(options.opt_level, options.shrink_level)]
          passes += ['--post-emscripten']
          if shared.Settings.GLOBAL_BASE >= 1024: # hardcoded value in the binaryen pass
            passes += ['--low-memory-unused']
          if options.debug_level < 3:
            passes += ['--strip-debug']
          if not shared.Settings.EMIT_PRODUCERS_SECTION:
            passes += ['--strip-producers']
          if passes:
            shared.Settings.BINARYEN_PASSES = ','.join(passes)

        # to bootstrap struct_info, we need binaryen
        os.environ['EMCC_WASM_BACKEND_BINARYEN'] = '1'

      # run safe-heap as a binaryen pass
      if shared.Settings.SAFE_HEAP and shared.Building.is_wasm_only():
        if shared.Settings.BINARYEN_PASSES:
          shared.Settings.BINARYEN_PASSES += ','
        shared.Settings.BINARYEN_PASSES += 'safe-heap'
      if shared.Settings.EMULATE_FUNCTION_POINTER_CASTS:
        # emulated function pointer casts is emulated in wasm using a binaryen pass
        if shared.Settings.BINARYEN_PASSES:
          shared.Settings.BINARYEN_PASSES += ','
        shared.Settings.BINARYEN_PASSES += 'fpcast-emu'
        if not shared.Settings.WASM_BACKEND:
          # we also need emulated function pointers for that, as we need a single flat
          # table, as is standard in wasm, and not asm.js split ones.
          shared.Settings.EMULATED_FUNCTION_POINTERS = 1

    # wasm outputs are only possible with a side wasm
    if target.endswith(WASM_ENDINGS):
      shared.Settings.EMITTING_JS = 0
      js_target = misc_temp_files.get(suffix='.js').name

    if shared.Settings.EVAL_CTORS:
      if not shared.Settings.WASM:
        # for asm.js: this option is not a js optimizer pass, but does run the js optimizer internally, so
        # we need to generate proper code for that (for wasm, we run a binaryen tool for this)
        shared.Settings.RUNNING_JS_OPTS = 1

    # memory growth does not work in dynamic linking, except for wasm
    if not shared.Settings.WASM and (shared.Settings.MAIN_MODULE or shared.Settings.SIDE_MODULE):
      assert not shared.Settings.ALLOW_MEMORY_GROWTH, 'memory growth is not supported with shared asm.js modules'

    if shared.Settings.MINIMAL_RUNTIME:
      if shared.Settings.ALLOW_MEMORY_GROWTH:
        logging.warning('-s ALLOW_MEMORY_GROWTH=1 is not yet supported with -s MINIMAL_RUNTIME=1')

      if shared.Settings.EMTERPRETIFY:
        exit_with_error('-s EMTERPRETIFY=1 is not supported with -s MINIMAL_RUNTIME=1')

      if shared.Settings.USE_PTHREADS:
        exit_with_error('-s USE_PTHREADS=1 is not yet supported with -s MINIMAL_RUNTIME=1')

      if shared.Settings.PRECISE_F32 == 2:
        exit_with_error('-s PRECISE_F32=2 is not supported with -s MINIMAL_RUNTIME=1')

      if shared.Settings.SINGLE_FILE:
        exit_with_error('-s SINGLE_FILE=1 is not supported with -s MINIMAL_RUNTIME=1')

    if shared.Settings.ALLOW_MEMORY_GROWTH and shared.Settings.ASM_JS == 1:
      # this is an issue in asm.js, but not wasm
      if not shared.Settings.WASM:
        shared.WarningManager.warn('ALMOST_ASM')
        shared.Settings.ASM_JS = 2 # memory growth does not validate as asm.js http://discourse.wicg.io/t/request-for-comments-switching-resizing-heaps-in-asm-js/641/23

    # safe heap in asm.js uses the js optimizer (in wasm-only mode we can use binaryen)
    if shared.Settings.SAFE_HEAP and not shared.Building.is_wasm_only():
      if not options.js_opts:
        logger.debug('enabling js opts for SAFE_HEAP')
        options.js_opts = True
      options.force_js_opts = True

    if options.js_opts:
      shared.Settings.RUNNING_JS_OPTS = 1

    if shared.Settings.CYBERDWARF:
      newargs.append('-g')
      options.debug_level = max(options.debug_level, 2)
      shared.Settings.BUNDLED_CD_DEBUG_FILE = target + ".cd"
      options.js_libraries.append(shared.path_from_root('src', 'library_cyberdwarf.js'))
      options.js_libraries.append(shared.path_from_root('src', 'library_debugger_toolkit.js'))

    if options.tracing:
      if shared.Settings.ALLOW_MEMORY_GROWTH:
        shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE += ['emscripten_trace_report_memory_layout']

    if shared.Settings.ONLY_MY_CODE:
      shared.Settings.DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = []
      options.separate_asm = True
      shared.Settings.FINALIZE_ASM_JS = False

    # MINIMAL_RUNTIME always use separate .asm.js file for best performance and memory usage
    if shared.Settings.MINIMAL_RUNTIME and not shared.Settings.WASM:
      options.separate_asm = True

    if shared.Settings.GLOBAL_BASE < 0:
      shared.Settings.GLOBAL_BASE = 8 # default if nothing else sets it

    if shared.Settings.WASM_BACKEND:
      if shared.Settings.SIMD:
        newargs.append('-msimd128')
    else:
      # We leave the -O option in place so that the clang front-end runs in that
      # optimization mode, but we disable the actual optimization passes, as we'll
      # run them separately.
      if options.opt_level > 0:
        newargs.append('-mllvm')
        newargs.append('-disable-llvm-optzns')

    if not shared.Settings.LEGALIZE_JS_FFI:
      assert shared.Building.is_wasm_only(), 'LEGALIZE_JS_FFI incompatible with RUNNING_JS_OPTS.'

    shared.Settings.EMSCRIPTEN_VERSION = shared.EMSCRIPTEN_VERSION
    shared.Settings.OPT_LEVEL = options.opt_level
    shared.Settings.DEBUG_LEVEL = options.debug_level
    shared.Settings.PROFILING_FUNCS = options.profiling_funcs
    shared.Settings.SOURCE_MAP_BASE = options.source_map_base or ''

    ## Compile source code to bitcode

    logger.debug('compiling to bitcode')

    temp_files = []

  # exit block 'parse arguments and setup'
  log_time('parse arguments and setup')

  if DEBUG:
    # we are about to start using temp dirs. serialize access to the temp dir
    # when using EMCC_DEBUG, since we don't want multiple processes would to
    # use it at once, they might collide if they happen to use the same
    # tempfile names
    shared.Cache.acquire_cache_lock()

  try:
    with ToolchainProfiler.profile_block('bitcodeize inputs'):
      # Precompiled headers support
      if has_header_inputs:
        headers = [header for _, header in input_files]
        for header in headers:
          assert header.endswith(HEADER_ENDINGS), 'if you have one header input, we assume you want to precompile headers, and cannot have source files or other inputs as well: ' + str(headers) + ' : ' + header
        args = newargs + headers
        if specified_target:
          args += ['-o', specified_target]
        args = system_libs.process_args(args, shared.Settings)
        logger.debug("running (for precompiled headers): " + clang_compiler + ' ' + ' '.join(args))
        return run_process([clang_compiler] + args, check=False).returncode

      def get_bitcode_file(input_file):
        if final_suffix not in JS_CONTAINING_ENDINGS:
          # no need for a temp file, just emit to the right place
          if len(input_files) == 1:
            # can just emit directly to the target
            if specified_target:
              if specified_target.endswith('/') or specified_target.endswith('\\') or os.path.isdir(specified_target):
                return os.path.join(specified_target, os.path.basename(unsuffixed(input_file))) + options.default_object_extension
              return specified_target
            return unsuffixed(input_file) + final_suffix
          else:
            if has_dash_c:
              return unsuffixed(input_file) + options.default_object_extension
        return in_temp(unsuffixed(uniquename(input_file)) + options.default_object_extension)

      # Request LLVM debug info if explicitly specified, or building bitcode with -g, or if building a source all the way to JS with -g
      if use_source_map(options) or ((final_suffix not in JS_CONTAINING_ENDINGS or (has_source_inputs and final_suffix in JS_CONTAINING_ENDINGS)) and options.requested_debug == '-g'):
        # do not save llvm debug info if js optimizer will wipe it out anyhow (but if source maps are used, keep it)
        if use_source_map(options) or not (final_suffix in JS_CONTAINING_ENDINGS and options.js_opts):
          newargs.append('-g') # preserve LLVM debug info
          options.debug_level = 4
          shared.Settings.DEBUG_LEVEL = 4

      # For asm.js, the generated JavaScript could preserve LLVM value names, which can be useful for debugging.
      if options.debug_level >= 3 and not shared.Settings.WASM:
        newargs.append('-fno-discard-value-names')

      # Bitcode args generation code
      def get_clang_args(input_files):
        args = [clang_compiler] + newargs + input_files
        if not shared.Building.can_inline():
          args.append('-fno-inline-functions')
        # For fastcomp backend, no LLVM IR functions should ever be annotated
        # 'optnone', because that would skip running the SimplifyCFG pass on
        # them, which is required to always run to clean up LandingPadInst
        # instructions that are not needed.
        if not shared.Settings.WASM_BACKEND:
          args += ['-Xclang', '-disable-O0-optnone']
        args = system_libs.process_args(args, shared.Settings)
        return args

      # -E preprocessor-only support
      if '-E' in newargs or '-M' in newargs or '-MM' in newargs:
        input_files = [x[1] for x in input_files]
        cmd = get_clang_args(input_files)
        if specified_target:
          cmd += ['-o', specified_target]
        # Do not compile, but just output the result from preprocessing stage or
        # output the dependency rule. Warning: clang and gcc behave differently
        # with -MF! (clang seems to not recognize it)
        logger.debug(('just preprocessor ' if '-E' in newargs else 'just dependencies: ') + ' '.join(cmd))
        return run_process(cmd, check=False).returncode

      def compile_source_file(i, input_file):
        logger.debug('compiling source file: ' + input_file)
        output_file = get_bitcode_file(input_file)
        temp_files.append((i, output_file))
        args = get_clang_args([input_file]) + ['-c', '-o', output_file]
        if shared.Settings.WASM_OBJECT_FILES:
          for a in shared.Building.llvm_backend_args():
            args += ['-mllvm', a]
        else:
          args.append('-emit-llvm')
        logger.debug("running: " + ' '.join(shared.Building.doublequote_spaces(args))) # NOTE: Printing this line here in this specific format is important, it is parsed to implement the "emcc --cflags" command
        if run_process(args, check=False).returncode != 0:
          exit_with_error('compiler frontend failed to generate LLVM bitcode, halting')
        assert(os.path.exists(output_file))

      # First, generate LLVM bitcode. For each input file, we get base.o with bitcode
      for i, input_file in input_files:
        file_ending = get_file_suffix(input_file)
        if file_ending.endswith(SOURCE_ENDINGS):
          compile_source_file(i, input_file)
        else: # bitcode
          if file_ending.endswith(BITCODE_ENDINGS):
            logger.debug('using bitcode file: ' + input_file)
            temp_files.append((i, input_file))
          elif file_ending.endswith(DYNAMICLIB_ENDINGS) or shared.Building.is_ar(input_file):
            logger.debug('using library file: ' + input_file)
            temp_files.append((i, input_file))
          elif file_ending.endswith(ASSEMBLY_ENDINGS):
            if not LEAVE_INPUTS_RAW:
              logger.debug('assembling assembly file: ' + input_file)
              temp_file = in_temp(unsuffixed(uniquename(input_file)) + '.o')
              shared.Building.llvm_as(input_file, temp_file)
              temp_files.append((i, temp_file))
          else:
            if has_fixed_language_mode:
              compile_source_file(i, input_file)
            else:
              exit_with_error(input_file + ': Unknown file suffix when compiling to LLVM bitcode!')

    # exit block 'bitcodeize inputs'
    log_time('bitcodeize inputs')

    with ToolchainProfiler.profile_block('process inputs'):
      if not LEAVE_INPUTS_RAW and not shared.Settings.WASM_BACKEND:
        assert len(temp_files) == len(input_files)

        # Optimize source files
        if optimizing(options.llvm_opts):
          for pos, (_, input_file) in enumerate(input_files):
            file_ending = get_file_suffix(input_file)
            if file_ending.endswith(SOURCE_ENDINGS):
              temp_file = temp_files[pos][1]
              logger.debug('optimizing %s', input_file)
              # if DEBUG:
              #   shutil.copyfile(temp_file, os.path.join(shared.configuration.CANONICAL_TEMP_DIR, 'to_opt.bc')) # useful when LLVM opt aborts
              new_temp_file = in_temp(unsuffixed(uniquename(temp_file)) + '.o')
              # after optimizing, lower intrinsics to libc calls so that our linking code
              # will find them (otherwise, llvm.cos.f32() will not link in cosf(), and
              # we end up calling out to JS for Math.cos).
              opts = options.llvm_opts + ['-lower-non-em-intrinsics']
              shared.Building.llvm_opt(temp_file, opts, new_temp_file)
              temp_files[pos] = (temp_files[pos][0], new_temp_file)

      # Decide what we will link
      executable_endings = JS_CONTAINING_ENDINGS + ('.wasm',)
      stop_at_bitcode = final_suffix not in executable_endings

      if stop_at_bitcode or not shared.Settings.WASM_BACKEND:
        # Filter link flags, keeping only those that shared.Building.link knows
        # how to deal with.  We currently can't handle flags with options (like
        # -Wl,-rpath,/bin:/lib, where /bin:/lib is an option for the -rpath
        # flag).
        link_flags = [f for f in link_flags if f[1] in SUPPORTED_LINKER_FLAGS]

      linker_inputs = [val for _, val in sorted(temp_files + link_flags)]

      # If we were just asked to generate bitcode, stop there
      if stop_at_bitcode:
        if not specified_target:
          assert len(temp_files) == len(input_files)
          for tempf, inputf in zip(temp_files, input_files):
            safe_move(tempf[1], unsuffixed_basename(inputf[1]) + final_suffix)
        else:
          if len(input_files) == 1:
            input_file = input_files[0][1]
            temp_file = temp_files[0][1]
            bitcode_target = specified_target if specified_target else unsuffixed_basename(input_file) + final_suffix
            if temp_file != input_file:
              safe_move(temp_file, bitcode_target)
            else:
              shutil.copyfile(temp_file, bitcode_target)
            temp_output_base = unsuffixed(temp_file)
            if os.path.exists(temp_output_base + '.d'):
              # There was a .d file generated, from -MD or -MMD and friends, save a copy of it to where the output resides,
              # adjusting the target name away from the temporary file name to the specified target.
              # It will be deleted with the rest of the temporary directory.
              deps = open(temp_output_base + '.d').read()
              deps = deps.replace(temp_output_base + options.default_object_extension, specified_target)
              with open(os.path.join(os.path.dirname(specified_target), os.path.basename(unsuffixed(input_file) + '.d')), "w") as out_dep:
                out_dep.write(deps)
          else:
            assert len(original_input_files) == 1 or not has_dash_c, 'fatal error: cannot specify -o with -c with multiple files' + str(args) + ':' + str(original_input_files)
            # We have a specified target (-o <target>), which is not JavaScript or HTML, and
            # we have multiple files: Link them
            logger.debug('link: ' + str(linker_inputs) + specified_target)
            shared.Building.link_to_object(linker_inputs, specified_target)
        logger.debug('stopping at bitcode')
        if shared.Settings.SIDE_MODULE:
          exit_with_error('SIDE_MODULE must only be used when compiling to an executable shared library, and not when emitting LLVM bitcode. That is, you should be emitting a .wasm file (for wasm) or a .js file (for asm.js). Note that when compiling to a typical native suffix for a shared library (.so, .dylib, .dll; which many build systems do) then Emscripten emits an LLVM bitcode file, which you should then compile to .wasm or .js with SIDE_MODULE.')
        if final_suffix.lower() in ('.so', '.dylib', '.dll'):
          logger.warning('When Emscripten compiles to a typical native suffix for shared libraries (.so, .dylib, .dll) then it emits an LLVM bitcode file. You should then compile that to an emscripten SIDE_MODULE (using that flag) with suffix .wasm (for wasm) or .js (for asm.js). (You may also want to adapt your build system to emit the more standard suffix for a file with LLVM bitcode, \'.bc\', which would avoid this warning.)')
        return 0

    # exit block 'process inputs'
    log_time('process inputs')

    ## Continue on to create JavaScript

    with ToolchainProfiler.profile_block('calculate system libraries'):
      logger.debug('will generate JavaScript')

      extra_files_to_link = []

      # link in ports and system libraries, if necessary
      if not LEAVE_INPUTS_RAW and \
         not shared.Settings.BOOTSTRAPPING_STRUCT_INFO and \
         not shared.Settings.ONLY_MY_CODE and \
         not shared.Settings.SIDE_MODULE: # shared libraries/side modules link no C libraries, need them in parent
        extra_files_to_link = system_libs.get_ports(shared.Settings)
        extra_files_to_link += system_libs.calculate([f for _, f in sorted(temp_files)] + extra_files_to_link, in_temp, stdout_=None, stderr_=None, forced=forced_stdlibs)

    # exit block 'calculate system libraries'
    log_time('calculate system libraries')

    def dedup_list(lst):
      rtn = []
      for item in lst:
        if item not in rtn:
          rtn.append(item)
      return rtn

    # Make a final pass over shared.Settings.EXPORTED_FUNCTIONS to remove any
    # duplication between functions added by the driver/libraries and function
    # specified by the user
    shared.Settings.EXPORTED_FUNCTIONS = dedup_list(shared.Settings.EXPORTED_FUNCTIONS)

    with ToolchainProfiler.profile_block('link'):
      # final will be an array if linking is deferred, otherwise a normal string.
      if shared.Settings.WASM_BACKEND:
        DEFAULT_FINAL = in_temp(target_basename + '.wasm')
      else:
        DEFAULT_FINAL = in_temp(target_basename + '.bc')

      def get_final():
        global final
        if isinstance(final, list):
          final = DEFAULT_FINAL
        return final

      # First, combine the bitcode files if there are several. We must also link if we have a singleton .a
      linker_inputs += extra_files_to_link
      perform_link = len(linker_inputs) > 1 or shared.Settings.WASM_BACKEND
      if not perform_link and not LEAVE_INPUTS_RAW:
        is_bc = suffix(temp_files[0][1]) in BITCODE_ENDINGS
        is_dylib = suffix(temp_files[0][1]) in DYNAMICLIB_ENDINGS
        is_ar = shared.Building.is_ar(temp_files[0][1])
        perform_link = not (is_bc or is_dylib) and is_ar
      if perform_link:
        logger.debug('linking: ' + str(linker_inputs))
        # force archive contents to all be included, if just archives, or if linking shared modules
        force_archive_contents = all(t.endswith(STATICLIB_ENDINGS) for _, t in temp_files) or shared.Settings.LINKABLE

        # if  EMCC_DEBUG=2  then we must link now, so the temp files are complete.
        # if using the wasm backend, we might be using vanilla LLVM, which does not allow our fastcomp deferred linking opts.
        # TODO: we could check if this is a fastcomp build, and still speed things up here
        just_calculate = DEBUG != 2 and not shared.Settings.WASM_BACKEND
        if shared.Settings.WASM_BACKEND:
          # If LTO is enabled then use the -O opt level as the LTO level
          if options.llvm_lto:
            lto_level = options.opt_level
          else:
            lto_level = 0
          final = shared.Building.link_lld(linker_inputs, DEFAULT_FINAL, lto_level=lto_level)
        else:
          final = shared.Building.link(linker_inputs, DEFAULT_FINAL, force_archive_contents=force_archive_contents, temp_files=misc_temp_files, just_calculate=just_calculate)
      else:
        logger.debug('skipping linking: ' + str(linker_inputs))
        if not LEAVE_INPUTS_RAW:
          _, temp_file = temp_files[0]
          _, input_file = input_files[0]
          final = in_temp(target_basename + '.bc')
          if temp_file != input_file:
            shutil.move(temp_file, final)
          else:
            shutil.copyfile(temp_file, final)
        else:
          _, input_file = input_files[0]
          final = in_temp(input_file)
          shutil.copyfile(input_file, final)

    # exit block 'link'
    log_time('link')

    if not shared.Settings.WASM_BACKEND:
      with ToolchainProfiler.profile_block('post-link'):
        if DEBUG:
          logger.debug('saving intermediate processing steps to %s', shared.get_emscripten_temp_dir())
          if not LEAVE_INPUTS_RAW:
            save_intermediate('basebc', 'bc')

        # Optimize, if asked to
        if not LEAVE_INPUTS_RAW:
          # remove LLVM debug if we are not asked for it
          link_opts = [] if use_source_map(options) or shared.Settings.CYBERDWARF else ['-strip-debug']
          if not shared.Settings.ASSERTIONS:
            link_opts += ['-disable-verify']
          else:
            # when verifying, LLVM debug info has some tricky linking aspects, and llvm-link will
            # disable the type map in that case. we added linking to opt, so we need to do
            # something similar, which we can do with a param to opt
            link_opts += ['-disable-debug-info-type-map']

          if options.llvm_lto is not None and options.llvm_lto >= 2 and optimizing(options.llvm_opts):
            logger.debug('running LLVM opts as pre-LTO')
            final = shared.Building.llvm_opt(final, options.llvm_opts, DEFAULT_FINAL)
            save_intermediate('opt', 'bc')

          # If we can LTO, do it before dce, since it opens up dce opportunities
          if (not shared.Settings.LINKABLE) and options.llvm_lto and options.llvm_lto != 2:
            if not shared.Building.can_inline():
              link_opts.append('-disable-inlining')
            # add a manual internalize with the proper things we need to be kept alive during lto
            link_opts += shared.Building.get_safe_internalize() + ['-std-link-opts']
            # execute it now, so it is done entirely before we get to the stage of legalization etc.
            final = shared.Building.llvm_opt(final, link_opts, DEFAULT_FINAL)
            save_intermediate('lto', 'bc')
            link_opts = []
          else:
            # At minimum remove dead functions etc., this potentially saves a
            # lot in the size of the generated code (and the time to compile it)
            link_opts += shared.Building.get_safe_internalize() + ['-globaldce']

          if options.cfi:
            if use_cxx:
               link_opts.append("-wholeprogramdevirt")
            link_opts.append("-lowertypetests")

          if AUTODEBUG:
            # let llvm opt directly emit ll, to skip writing and reading all the bitcode
            link_opts += ['-S']
            final = shared.Building.llvm_opt(final, link_opts, get_final() + '.link.ll')
            save_intermediate('linktime', 'll')
          else:
            if len(link_opts) > 0:
              final = shared.Building.llvm_opt(final, link_opts, DEFAULT_FINAL)
              save_intermediate('linktime', 'bc')
            if options.save_bc:
              shutil.copyfile(final, options.save_bc)

        # Prepare .ll for Emscripten
        if LEAVE_INPUTS_RAW:
          assert len(input_files) == 1
        if options.save_bc:
          save_intermediate('ll', 'll')

        if AUTODEBUG:
          logger.debug('autodebug')
          next = get_final() + '.ad.ll'
          run_process([shared.PYTHON, shared.AUTODEBUGGER, final, next])
          final = next
          save_intermediate('autodebug', 'll')

        assert not isinstance(final, list), 'we must have linked the final files, if linking was deferred, by this point'

      # exit block 'post-link'
      log_time('post-link')

    with ToolchainProfiler.profile_block('emscript'):
      # Emscripten
      logger.debug('LLVM => JS')
      js_libraries = None
      if options.js_libraries:
        js_libraries = [os.path.abspath(lib) for lib in options.js_libraries]
      if options.memory_init_file:
        shared.Settings.MEM_INIT_METHOD = 1
      else:
        assert shared.Settings.MEM_INIT_METHOD != 1

      if embed_memfile(options):
        shared.Settings.SUPPORT_BASE64_EMBEDDING = 1

      final = shared.Building.emscripten(final, target + '.mem', js_libraries)
      save_intermediate('original')

      if shared.Settings.WASM_BACKEND:
        # we also received wast and wasm at this stage
        temp_basename = unsuffixed(final)
        wasm_temp = temp_basename + '.wasm'
        shutil.move(wasm_temp, wasm_binary_target)
        if use_source_map(options):
          shutil.move(wasm_temp + '.map', wasm_source_map_target)

      if shared.Settings.CYBERDWARF:
        cd_target = final + '.cd'
        shutil.move(cd_target, target + '.cd')

    # exit block 'emscript'
    log_time('emscript (llvm => executable code)')

    with ToolchainProfiler.profile_block('source transforms'):
      # Embed and preload files
      if len(options.preload_files) or len(options.embed_files):

        # Also, MEMFS is not aware of heap resizing feature in wasm, so if MEMFS and memory growth are used together, force
        # no_heap_copy to be enabled.
        if shared.Settings.ALLOW_MEMORY_GROWTH and not options.no_heap_copy:
          logger.info('Enabling --no-heap-copy because -s ALLOW_MEMORY_GROWTH=1 is being used with file_packager.py (pass --no-heap-copy to suppress this notification)')
          options.no_heap_copy = True

        logger.debug('setting up files')
        file_args = ['--from-emcc', '--export-name=' + shared.Settings.EXPORT_NAME]
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
        if options.no_heap_copy:
          file_args.append('--no-heap-copy')
        if shared.Settings.LZ4:
          file_args.append('--lz4')
        if options.use_preload_plugins:
          file_args.append('--use-preload-plugins')
        file_code = run_process([shared.PYTHON, shared.FILE_PACKAGER, unsuffixed(target) + '.data'] + file_args, stdout=PIPE).stdout
        options.pre_js = file_code + options.pre_js

      # Apply pre and postjs files
      if options.pre_js or options.post_js:
        logger.debug('applying pre/postjses')
        src = open(final).read()
        final += '.pp.js'
        if WINDOWS: # Avoid duplicating \r\n to \r\r\n when writing out.
          if options.pre_js:
            options.pre_js = options.pre_js.replace('\r\n', '\n')
          if options.post_js:
            options.post_js = options.post_js.replace('\r\n', '\n')
        with open(final, 'w') as f:
          # pre-js code goes right after the Module integration code (so it
          # can use Module), we have a marker for it
          f.write(src.replace('// {{PRE_JSES}}', options.pre_js))
          f.write(options.post_js)
        options.pre_js = src = options.post_js = None
        save_intermediate('pre-post')

      # Apply a source code transformation, if requested
      if options.js_transform:
        shutil.copyfile(final, final + '.tr.js')
        final += '.tr.js'
        posix = not shared.WINDOWS
        logger.debug('applying transform: %s', options.js_transform)
        shared.check_call(shared.Building.remove_quotes(shlex.split(options.js_transform, posix=posix) + [os.path.abspath(final)]))
        save_intermediate('transformed')

      js_transform_tempfiles = [final]

    # exit block 'source transforms'
    log_time('source transforms')

    with ToolchainProfiler.profile_block('memory initializer'):
      memfile = None
      # for the wasm backend, use a memfile exactly when using pthreads (until
      # we can remove this temporary hack)
      if (not shared.Settings.WASM_BACKEND and (shared.Settings.MEM_INIT_METHOD > 0 or embed_memfile(options))) or \
         (shared.Settings.WASM_BACKEND and shared.Settings.USE_PTHREADS):
        if shared.Settings.MINIMAL_RUNTIME:
          # Independent of whether user is doing -o a.html or -o a.js, generate the mem init file as a.mem (and not as a.html.mem or a.js.mem)
          memfile = target.replace('.html', '.mem').replace('.js', '.mem')
        else:
          memfile = target + '.mem'

      if memfile:
        if shared.Settings.WASM_BACKEND:
          # For the wasm backend, we don't have any memory info in JS. All we need to do
          # is set the memory initializer url.
          src = open(final).read()
          src = src.replace('var memoryInitializer = null;', 'var memoryInitializer = "%s";' % memfile)
          open(final + '.mem.js', 'w').write(src)
          final += '.mem.js'
        else:
          # Non-wasm backend path: Strip the memory initializer out of the asmjs file
          shared.try_delete(memfile)

          def repl(m):
            # handle chunking of the memory initializer
            s = m.group(1)
            if len(s) == 0:
              return '' # don't emit 0-size ones
            membytes = [int(x or '0') for x in s.split(',')]
            while membytes and membytes[-1] == 0:
              membytes.pop()
            if not membytes:
              return ''
            if shared.Settings.MEM_INIT_METHOD == 2:
              # memory initializer in a string literal
              return "memoryInitializer = '%s';" % shared.JS.generate_string_initializer(membytes)
            open(memfile, 'wb').write(bytearray(membytes))
            if DEBUG:
              # Copy into temp dir as well, so can be run there too
              shared.safe_copy(memfile, os.path.join(shared.get_emscripten_temp_dir(), os.path.basename(memfile)))
            if not shared.Settings.WASM or not shared.Settings.MEM_INIT_IN_WASM:
              return 'memoryInitializer = "%s";' % shared.JS.get_subresource_location(memfile, embed_memfile(options))
            else:
              return ''

          src = re.sub(shared.JS.memory_initializer_pattern, repl, open(final).read(), count=1)
          open(final + '.mem.js', 'w').write(src)
          final += '.mem.js'
          src = None
          js_transform_tempfiles[-1] = final # simple text substitution preserves comment line number mappings
          if os.path.exists(memfile):
            save_intermediate('meminit')
            logger.debug('wrote memory initialization to %s', memfile)
          else:
            logger.debug('did not see memory initialization')

      if shared.Settings.USE_PTHREADS:
        target_dir = os.path.dirname(os.path.abspath(target))
        worker_output = os.path.join(target_dir, shared.Settings.PTHREAD_WORKER_FILE)
        with open(worker_output, 'w') as f:
          f.write(shared.read_and_preprocess(shared.path_from_root('src', 'worker.js'), expand_macros=True))

      # Generate the fetch.js worker script for multithreaded emscripten_fetch() support if targeting pthreads.
      if shared.Settings.FETCH and shared.Settings.USE_PTHREADS:
        if shared.Settings.WASM_BACKEND:
          logger.warning('Bug/TODO: Blocking calls to the fetch API do not currently work under WASM backend (https://github.com/emscripten-core/emscripten/issues/7024)')
        else:
          shared.make_fetch_worker(final, shared.Settings.FETCH_WORKER_FILE)

    # exit block 'memory initializer'
    log_time('memory initializer')

    optimizer = JSOptimizer(
      target=target,
      options=options,
      js_transform_tempfiles=js_transform_tempfiles,
      in_temp=in_temp,
    )
    with ToolchainProfiler.profile_block('js opts'):
      # It is useful to run several js optimizer passes together, to save on unneeded unparsing/reparsing
      if shared.Settings.DEAD_FUNCTIONS:
        optimizer.queue += ['eliminateDeadFuncs']
        optimizer.extra_info['dead_functions'] = shared.Settings.DEAD_FUNCTIONS

      if options.opt_level >= 1 and options.js_opts:
        logger.debug('running js post-opts')

        if DEBUG == 2:
          # Clean up the syntax a bit
          optimizer.queue += ['noop']

        def get_eliminate():
          if shared.Settings.ALLOW_MEMORY_GROWTH:
            return 'eliminateMemSafe'
          else:
            return 'eliminate'

        if options.opt_level >= 2:
          optimizer.queue += [get_eliminate()]

          if shared.Settings.AGGRESSIVE_VARIABLE_ELIMINATION:
            # note that this happens before registerize/minification, which can obfuscate the name of 'label', which is tricky
            optimizer.queue += ['aggressiveVariableElimination']

          optimizer.queue += ['simplifyExpressions']

          if shared.Settings.EMTERPRETIFY:
            # emterpreter code will not run through a JS optimizing JIT, do more work ourselves
            optimizer.queue += ['localCSE']

      if shared.Settings.EMTERPRETIFY:
        # add explicit label setting, as we will run aggressiveVariableElimination late, *after* 'label' is no longer notable by name
        optimizer.queue += ['safeLabelSetting']

      if options.opt_level >= 1 and options.js_opts:
        if options.opt_level >= 2:
          # simplify ifs if it is ok to make the code somewhat unreadable, and unless outlining (simplified ifs
          # with commaified code breaks late aggressive variable elimination)
          # do not do this with binaryen, as commaifying confuses binaryen call type detection (FIXME, in theory, but unimportant)
          debugging = options.debug_level == 0 or options.profiling
          if shared.Settings.SIMPLIFY_IFS and debugging and shared.Settings.OUTLINING_LIMIT == 0 and not shared.Settings.WASM:
            optimizer.queue += ['simplifyIfs']

          if shared.Settings.PRECISE_F32:
            optimizer.queue += ['optimizeFrounds']

      if options.js_opts:
        if shared.Settings.SAFE_HEAP and not shared.Building.is_wasm_only():
          optimizer.queue += ['safeHeap']

        if shared.Settings.OUTLINING_LIMIT > 0:
          optimizer.queue += ['outline']
          optimizer.extra_info['sizeToOutline'] = shared.Settings.OUTLINING_LIMIT

        if options.opt_level >= 2 and options.debug_level < 3:
          if options.opt_level >= 3 or options.shrink_level > 0:
            optimizer.queue += ['registerizeHarder']
          else:
            optimizer.queue += ['registerize']

        # NOTE: Important that this comes after registerize/registerizeHarder
        if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS and options.opt_level >= 2:
          optimizer.flush()
          shared.Building.eliminate_duplicate_funcs(final)
          save_intermediate('dfe')

      if shared.Settings.EVAL_CTORS and options.memory_init_file and not use_source_map(options) and not shared.Settings.WASM:
        optimizer.flush()
        shared.Building.eval_ctors(final, memfile)
        save_intermediate('eval-ctors')

      if options.js_opts:
        # some compilation modes require us to minify later or not at all
        if not shared.Settings.EMTERPRETIFY and not shared.Settings.WASM:
          optimizer.do_minify()

        if options.opt_level >= 2:
          optimizer.queue += ['asmLastOpts']

        if shared.Settings.FINALIZE_ASM_JS:
          optimizer.queue += ['last']

        optimizer.flush()

      if options.use_closure_compiler == 2:
        optimizer.flush()

        logger.debug('running closure')
        # no need to add this to js_transform_tempfiles, because closure and
        # debug_level > 0 are never simultaneously true
        final = shared.Building.closure_compiler(final, pretty=options.debug_level >= 1)
        save_intermediate('closure')

    log_time('js opts')

    with ToolchainProfiler.profile_block('final emitting'):
      if shared.Settings.EMTERPRETIFY:
        emterpretify(js_target, optimizer, options)

      # Remove some trivial whitespace
      # TODO: do not run when compress has already been done on all parts of the code
      # src = open(final).read()
      # src = re.sub(r'\n+[ \n]*\n+', '\n', src)
      # open(final, 'w').write(src)

      # Bundle symbol data in with the cyberdwarf file
      if shared.Settings.CYBERDWARF:
        run_process([shared.PYTHON, shared.path_from_root('tools', 'emdebug_cd_merger.py'), target + '.cd', target + '.symbols'])

      if use_source_map(options) and not shared.Settings.WASM:
        emit_js_source_maps(target, optimizer.js_transform_tempfiles)

      # track files that will need native eols
      generated_text_files_with_native_eols = []

      if (options.separate_asm or shared.Settings.WASM) and not shared.Settings.WASM_BACKEND:
        separate_asm_js(final, asm_target)
        generated_text_files_with_native_eols += [asm_target]

      if shared.Settings.WASM:
        do_binaryen(target, asm_target, options, memfile, wasm_binary_target,
                    wasm_text_target, wasm_source_map_target, misc_temp_files,
                    optimizer)

      if shared.Settings.MODULARIZE:
        modularize()

      module_export_name_substitution()

      # Run a final regex pass to clean up items that were not possible to optimize by Closure, or unoptimalities that were left behind
      # by processing steps that occurred after Closure.
      if shared.Settings.MINIMAL_RUNTIME == 2 and shared.Settings.USE_CLOSURE_COMPILER and options.debug_level == 0:
        # Process .js runtime file
        shared.run_process([shared.PYTHON, shared.path_from_root('tools', 'hacky_postprocess_around_closure_limitations.py'), final])
        # Process .asm.js file
        if not shared.Settings.WASM:
          shared.run_process([shared.PYTHON, shared.path_from_root('tools', 'hacky_postprocess_around_closure_limitations.py'), asm_target])

      # The JS is now final. Move it to its final location
      shutil.move(final, js_target)

      generated_text_files_with_native_eols += [js_target]

      # If we were asked to also generate HTML, do that
      if final_suffix == '.html':
        generate_html(target, options, js_target, target_basename,
                      asm_target, wasm_binary_target,
                      memfile, optimizer)
      else:
        if options.proxy_to_worker:
          generate_worker_js(target, js_target, target_basename)

      if embed_memfile(options) and memfile:
        shared.try_delete(memfile)

      for f in generated_text_files_with_native_eols:
        tools.line_endings.convert_line_endings_in_file(f, os.linesep, options.output_eol)
    log_time('final emitting')
    # exit block 'final emitting'

  finally:
    if DEBUG:
      shared.Cache.release_cache_lock()

  if DEBUG:
    logger.debug('total time: %.2f seconds', (time.time() - start_time))

  return 0


def parse_args(newargs):
  options = EmccOptions()
  settings_changes = []
  should_exit = False

  def check_bad_eq(arg):
    if '=' in arg:
      exit_with_error('Invalid parameter (do not use "=" with "--" options)')

  for i in range(len(newargs)):
    # On Windows Vista (and possibly others), excessive spaces in the command line
    # leak into the items in this array, so trim e.g. 'foo.cpp ' -> 'foo.cpp'
    newargs[i] = newargs[i].strip()
    if newargs[i].startswith('-O'):
      # Let -O default to -O2, which is what gcc does.
      options.requested_level = newargs[i][2:] or '2'
      if options.requested_level == 's':
        options.llvm_opts = ['-Os']
        options.requested_level = 2
        options.shrink_level = 1
        settings_changes.append('INLINING_LIMIT=50')
      elif options.requested_level == 'z':
        options.llvm_opts = ['-Oz']
        options.requested_level = 2
        options.shrink_level = 2
        settings_changes.append('INLINING_LIMIT=25')
      options.opt_level = validate_arg_level(options.requested_level, 3, 'Invalid optimization level: ' + newargs[i], clamp=True)
    elif newargs[i].startswith('--js-opts'):
      check_bad_eq(newargs[i])
      options.js_opts = int(newargs[i + 1])
      if options.js_opts:
        options.force_js_opts = True
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--llvm-opts'):
      check_bad_eq(newargs[i])
      options.llvm_opts = parse_value(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--llvm-lto'):
      check_bad_eq(newargs[i])
      options.llvm_lto = int(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--closure'):
      check_bad_eq(newargs[i])
      options.use_closure_compiler = int(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--js-transform'):
      check_bad_eq(newargs[i])
      options.js_transform = newargs[i + 1]
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--pre-js'):
      check_bad_eq(newargs[i])
      options.pre_js += open(newargs[i + 1]).read() + '\n'
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--post-js'):
      check_bad_eq(newargs[i])
      options.post_js += open(newargs[i + 1]).read() + '\n'
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--minify'):
      check_bad_eq(newargs[i])
      assert newargs[i + 1] == '0', '0 is the only supported option for --minify; 1 has been deprecated'
      options.debug_level = max(1, options.debug_level)
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('-g'):
      requested_level = newargs[i][2:] or '3'
      options.debug_level = validate_arg_level(requested_level, 4, 'Invalid debug level: ' + newargs[i])
      options.requested_debug = newargs[i]
      newargs[i] = ''
    elif newargs[i] == '-profiling' or newargs[i] == '--profiling':
      options.debug_level = 2
      options.profiling = True
      newargs[i] = ''
    elif newargs[i] == '-profiling-funcs' or newargs[i] == '--profiling-funcs':
      options.profiling_funcs = True
      newargs[i] = ''
    elif newargs[i] == '--tracing' or newargs[i] == '--memoryprofiler':
      if newargs[i] == '--memoryprofiler':
        options.memory_profiler = True
      options.tracing = True
      newargs[i] = ''
      newargs.append('-D__EMSCRIPTEN_TRACING__=1')
      settings_changes.append("EMSCRIPTEN_TRACING=1")
      options.js_libraries.append(shared.path_from_root('src', 'library_trace.js'))
    elif newargs[i] == '--emit-symbol-map':
      options.emit_symbol_map = True
      newargs[i] = ''
    elif newargs[i] == '--bind':
      options.bind = True
      newargs[i] = ''
      options.js_libraries.append(shared.path_from_root('src', 'embind', 'emval.js'))
      options.js_libraries.append(shared.path_from_root('src', 'embind', 'embind.js'))
      if options.default_cxx_std:
        # Force C++11 for embind code, but only if user has not explicitly overridden a standard.
        options.default_cxx_std = '-std=c++11'
    elif newargs[i].startswith('-std=') or newargs[i].startswith('--std='):
      # User specified a standard to use, clear Emscripten from specifying it.
      options.default_cxx_std = None
    elif newargs[i].startswith('--embed-file'):
      check_bad_eq(newargs[i])
      options.embed_files.append(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--preload-file'):
      check_bad_eq(newargs[i])
      options.preload_files.append(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--exclude-file'):
      check_bad_eq(newargs[i])
      options.exclude_files.append(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--use-preload-cache'):
      options.use_preload_cache = True
      newargs[i] = ''
    elif newargs[i].startswith('--no-heap-copy'):
      options.no_heap_copy = True
      newargs[i] = ''
    elif newargs[i].startswith('--use-preload-plugins'):
      options.use_preload_plugins = True
      newargs[i] = ''
    elif newargs[i] == '--ignore-dynamic-linking':
      options.ignore_dynamic_linking = True
      newargs[i] = ''
    elif newargs[i] == '-v':
      shared.COMPILER_OPTS += ['-v']
      shared.check_sanity(force=True)
      newargs[i] = ''
    elif newargs[i].startswith('--shell-file'):
      check_bad_eq(newargs[i])
      options.shell_path = newargs[i + 1]
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--source-map-base'):
      check_bad_eq(newargs[i])
      options.source_map_base = newargs[i + 1]
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i].startswith('--js-library'):
      check_bad_eq(newargs[i])
      options.js_libraries.append(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--remove-duplicates':
      logger.warning('--remove-duplicates is deprecated as it is no longer needed. If you cannot link without it, file a bug with a testcase')
      newargs[i] = ''
    elif newargs[i] == '--jcache':
      logger.error('jcache is no longer supported')
      newargs[i] = ''
    elif newargs[i] == '--cache':
      check_bad_eq(newargs[i])
      os.environ['EM_CACHE'] = os.path.normpath(newargs[i + 1])
      shared.reconfigure_cache()
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--clear-cache':
      logger.info('clearing cache as requested by --clear-cache')
      shared.Cache.erase()
      shared.check_sanity(force=True) # this is a good time for a sanity check
      should_exit = True
    elif newargs[i] == '--clear-ports':
      logger.info('clearing ports and cache as requested by --clear-ports')
      system_libs.Ports.erase()
      shared.Cache.erase()
      shared.check_sanity(force=True) # this is a good time for a sanity check
      should_exit = True
    elif newargs[i] == '--show-ports':
      system_libs.show_ports()
      should_exit = True
    elif newargs[i] == '--save-bc':
      check_bad_eq(newargs[i])
      options.save_bc = newargs[i + 1]
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--memory-init-file':
      check_bad_eq(newargs[i])
      options.memory_init_file = int(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--proxy-to-worker':
      options.proxy_to_worker = True
      newargs[i] = ''
    elif newargs[i] == '--valid-abspath':
      options.valid_abspaths.append(newargs[i + 1])
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--separate-asm':
      options.separate_asm = True
      newargs[i] = ''
    elif newargs[i].startswith(('-I', '-L')):
      options.path_name = newargs[i][2:]
      if os.path.isabs(options.path_name) and not is_valid_abspath(options, options.path_name):
        # Of course an absolute path to a non-system-specific library or header
        # is fine, and you can ignore this warning. The danger are system headers
        # that are e.g. x86 specific and nonportable. The emscripten bundled
        # headers are modified to be portable, local system ones are generally not.
        shared.WarningManager.warn(
            'ABSOLUTE_PATHS', '-I or -L of an absolute path "' + newargs[i] +
            '" encountered. If this is to a local system header/library, it may '
            'cause problems (local system files make sense for compiling natively '
            'on your system, but not necessarily to JavaScript).')
    elif newargs[i] == '--emrun':
      options.emrun = True
      newargs[i] = ''
    elif newargs[i] == '--cpuprofiler':
      options.cpu_profiler = True
      newargs[i] = ''
    elif newargs[i] == '--threadprofiler':
      options.thread_profiler = True
      settings_changes.append('PTHREADS_PROFILING=1')
      newargs[i] = ''
    elif newargs[i] == '--default-obj-ext':
      newargs[i] = ''
      options.default_object_extension = newargs[i + 1]
      if not options.default_object_extension.startswith('.'):
        options.default_object_extension = '.' + options.default_object_extension
      newargs[i + 1] = ''
    elif newargs[i].startswith("-fsanitize=cfi"):
      options.cfi = True
    elif newargs[i] == "--output_eol":
      if newargs[i + 1].lower() == 'windows':
        options.output_eol = '\r\n'
      elif newargs[i + 1].lower() == 'linux':
        options.output_eol = '\n'
      else:
        exit_with_error('Invalid value "' + newargs[i + 1] + '" to --output_eol!')
      newargs[i] = ''
      newargs[i + 1] = ''
    elif newargs[i] == '--generate-config':
      optarg = newargs[i + 1]
      path = os.path.expanduser(optarg)
      if os.path.exists(path):
        exit_with_error('File ' + optarg + ' passed to --generate-config already exists!')
      else:
        shared.generate_config(optarg)
      should_exit = True

  if should_exit:
    sys.exit(0)

  newargs = [arg for arg in newargs if arg]
  return options, settings_changes, newargs


def emterpretify(js_target, optimizer, options):
  global final
  optimizer.flush('pre-emterpretify')
  logger.debug('emterpretifying')
  blacklist = shared.Settings.EMTERPRETIFY_BLACKLIST
  whitelist = shared.Settings.EMTERPRETIFY_WHITELIST
  synclist = shared.Settings.EMTERPRETIFY_SYNCLIST
  if type(blacklist) == list:
    blacklist = json.dumps(blacklist)
  if type(whitelist) == list:
    whitelist = json.dumps(whitelist)
  if type(synclist) == list:
    synclist = json.dumps(synclist)

  args = [shared.PYTHON,
          shared.path_from_root('tools', 'emterpretify.py'),
          js_target,
          final + '.em.js',
          blacklist,
          whitelist,
          synclist,
          str(shared.Settings.SWAPPABLE_ASM_MODULE)]
  if shared.Settings.EMTERPRETIFY_ASYNC:
    args += ['ASYNC=1']
  if shared.Settings.EMTERPRETIFY_ADVISE:
    args += ['ADVISE=1']
  if options.profiling or options.profiling_funcs:
    args += ['PROFILING=1']
  if shared.Settings.ASSERTIONS:
    args += ['ASSERTIONS=1']
  if shared.Settings.PRECISE_F32:
    args += ['FROUND=1']
  if shared.Settings.ALLOW_MEMORY_GROWTH:
    args += ['MEMORY_SAFE=1']
  if shared.Settings.EMTERPRETIFY_FILE:
    args += ['FILE="' + shared.Settings.EMTERPRETIFY_FILE + '"']

  try:
    # move temp js to final position, alongside its mem init file
    shutil.move(final, js_target)
    shared.check_call(args)
  finally:
    shared.try_delete(js_target)

  final = final + '.em.js'

  if shared.Settings.EMTERPRETIFY_ADVISE:
    logger.warning('halting compilation due to EMTERPRETIFY_ADVISE')
    sys.exit(0)

  # minify (if requested) after emterpreter processing, and finalize output
  logger.debug('finalizing emterpreted code')
  shared.Settings.FINALIZE_ASM_JS = 1
  if not shared.Settings.WASM:
    optimizer.do_minify()
  optimizer.queue += ['last']
  optimizer.flush('finalizing-emterpreted-code')

  # finalize the original as well, if we will be swapping it in (TODO: add specific option for this)
  if shared.Settings.SWAPPABLE_ASM_MODULE:
    real = final
    original = js_target + '.orig.js' # the emterpretify tool saves the original here
    final = original
    logger.debug('finalizing original (non-emterpreted) code at ' + final)
    if not shared.Settings.WASM:
      optimizer.do_minify()
    optimizer.queue += ['last']
    optimizer.flush('finalizing-original-code')
    safe_copy(final, original)
    final = real


def emit_js_source_maps(target, js_transform_tempfiles):
  logger.debug('generating source maps')
  jsrun.run_js_tool(shared.path_from_root('tools', 'source-maps', 'sourcemapper.js'),
                    shared.NODE_JS, js_transform_tempfiles +
                    ['--sourceRoot', os.getcwd(),
                     '--mapFileBaseName', target,
                     '--offset', '0'])


def separate_asm_js(final, asm_target):
  """Separate out the asm.js code, if asked. Or, if necessary for another option"""
  logger.debug('separating asm')
  shared.check_call([shared.PYTHON, shared.path_from_root('tools', 'separate_asm.py'), final, asm_target, final, shared.Settings.SEPARATE_ASM_MODULE_NAME])

  # extra only-my-code logic
  if shared.Settings.ONLY_MY_CODE:
    temp = asm_target + '.only.js'
    jsrun.run_js_tool(shared.path_from_root('tools', 'js-optimizer.js'), shared.NODE_JS, jsargs=[asm_target, 'eliminateDeadGlobals', 'last', 'asm'], stdout=open(temp, 'w'))
    shutil.move(temp, asm_target)


def do_binaryen(target, asm_target, options, memfile, wasm_binary_target,
                wasm_text_target, wasm_source_map_target, misc_temp_files,
                optimizer):
  global final
  logger.debug('using binaryen')
  binaryen_bin = shared.Building.get_binaryen_bin()
  # normally we emit binary, but for debug info, we might emit text first
  debug_info = options.debug_level >= 2 or options.profiling_funcs
  emit_symbol_map = options.emit_symbol_map or shared.Settings.CYBERDWARF
  # finish compiling to WebAssembly, using asm2wasm, if we didn't already emit WebAssembly directly using the wasm backend.
  if not shared.Settings.WASM_BACKEND:
    if DEBUG:
      # save the asm.js input
      shared.safe_copy(asm_target, os.path.join(shared.get_emscripten_temp_dir(), os.path.basename(asm_target)))
    cmd = [os.path.join(binaryen_bin, 'asm2wasm'), asm_target, '--total-memory=' + str(shared.Settings.TOTAL_MEMORY)]
    if shared.Settings.BINARYEN_TRAP_MODE in ('js', 'clamp', 'allow'):
      cmd += ['--trap-mode=' + shared.Settings.BINARYEN_TRAP_MODE]
    else:
      exit_with_error('invalid BINARYEN_TRAP_MODE value: ' + shared.Settings.BINARYEN_TRAP_MODE + ' (should be js/clamp/allow)')
    if shared.Settings.BINARYEN_IGNORE_IMPLICIT_TRAPS:
      cmd += ['--ignore-implicit-traps']
    # pass optimization level to asm2wasm (if not optimizing, or which passes we should run was overridden, do not optimize)
    if options.opt_level > 0:
      cmd.append(shared.Building.opt_level_to_str(options.opt_level, options.shrink_level))
    # import mem init file if it exists, and if we will not be using asm.js as a binaryen method (as it needs the mem init file, of course)
    mem_file_exists = options.memory_init_file and os.path.exists(memfile)
    import_mem_init = mem_file_exists and shared.Settings.MEM_INIT_IN_WASM
    if import_mem_init:
      cmd += ['--mem-init=' + memfile]
      if not shared.Settings.RELOCATABLE:
        cmd += ['--mem-base=' + str(shared.Settings.GLOBAL_BASE)]
    # various options imply that the imported table may not be the exact size as the wasm module's own table segments
    if shared.Settings.RELOCATABLE or shared.Settings.RESERVED_FUNCTION_POINTERS > 0 or shared.Settings.EMULATED_FUNCTION_POINTERS:
      cmd += ['--table-max=-1']
    if shared.Settings.SIDE_MODULE:
      cmd += ['--mem-max=-1']
    elif shared.Settings.WASM_MEM_MAX >= 0:
      cmd += ['--mem-max=' + str(shared.Settings.WASM_MEM_MAX)]
    if shared.Settings.LEGALIZE_JS_FFI != 1:
      cmd += ['--no-legalize-javascript-ffi']
    if shared.Building.is_wasm_only():
      cmd += ['--wasm-only'] # this asm.js is code not intended to run as asm.js, it is only ever going to be wasm, an can contain special fastcomp-wasm support
    if shared.Settings.USE_PTHREADS:
      cmd += ['--enable-threads']
    if debug_info:
      cmd += ['-g']
    if emit_symbol_map:
      cmd += ['--symbolmap=' + target + '.symbols']
    # we prefer to emit a binary, as it is more efficient. however, when we
    # want full debug info support (not just function names), then we must
    # emit text (at least until wasm gains support for debug info in binaries)
    target_binary = options.debug_level < 3
    if target_binary:
      cmd += ['-o', wasm_binary_target]
    else:
      cmd += ['-o', wasm_text_target, '-S']
    cmd += shared.Building.get_binaryen_feature_flags()
    logger.debug('asm2wasm (asm.js => WebAssembly): ' + ' '.join(cmd))
    TimeLogger.update()
    shared.check_call(cmd)

    if not target_binary:
      cmd = [os.path.join(binaryen_bin, 'wasm-as'), wasm_text_target, '-o', wasm_binary_target]
      if debug_info:
        cmd += ['-g']
        if use_source_map(options):
          cmd += ['--source-map=' + wasm_source_map_target]
          cmd += ['--source-map-url=' + options.source_map_base + os.path.basename(wasm_binary_target) + '.map']
      logger.debug('wasm-as (text => binary): ' + ' '.join(cmd))
      shared.check_call(cmd)
    if import_mem_init:
      # remove the mem init file in later processing; it does not need to be prefetched in the html, etc.
      if DEBUG:
        safe_move(memfile, os.path.join(shared.get_emscripten_temp_dir(), os.path.basename(memfile)))
      else:
        os.unlink(memfile)
    log_time('asm2wasm')
  if shared.Settings.BINARYEN_PASSES:
    if DEBUG:
      shared.safe_copy(wasm_binary_target, os.path.join(shared.get_emscripten_temp_dir(), os.path.basename(wasm_binary_target) + '.pre-byn'))
    # BINARYEN_PASSES is comma-separated, and we support both '-'-prefixed and unprefixed pass names
    passes = [('--' + p) if p[0] != '-' else p for p in shared.Settings.BINARYEN_PASSES.split(',')]
    cmd = [os.path.join(binaryen_bin, 'wasm-opt'), wasm_binary_target, '-o', wasm_binary_target] + passes
    cmd += shared.Building.get_binaryen_feature_flags()
    if debug_info:
      cmd += ['-g'] # preserve the debug info
    if use_source_map(options):
      cmd += ['--input-source-map=' + wasm_source_map_target]
      cmd += ['--output-source-map=' + wasm_source_map_target]
      cmd += ['--output-source-map-url=' + options.source_map_base + os.path.basename(wasm_binary_target) + '.map']
      if DEBUG:
        shared.safe_copy(wasm_source_map_target, os.path.join(shared.get_emscripten_temp_dir(), os.path.basename(wasm_source_map_target) + '.pre-byn'))
    logger.debug('wasm-opt on BINARYEN_PASSES: %s', cmd)
    shared.print_compiler_stage(cmd)
    shared.check_call(cmd)
  if shared.Settings.BINARYEN_SCRIPTS:
    binaryen_scripts = os.path.join(shared.BINARYEN_ROOT, 'scripts')
    script_env = os.environ.copy()
    root_dir = os.path.abspath(os.path.dirname(__file__))
    if script_env.get('PYTHONPATH'):
      script_env['PYTHONPATH'] += ':' + root_dir
    else:
      script_env['PYTHONPATH'] = root_dir
    for script in shared.Settings.BINARYEN_SCRIPTS.split(','):
      logger.debug('running binaryen script: ' + script)
      shared.check_call([shared.PYTHON, os.path.join(binaryen_scripts, script), final, wasm_text_target], env=script_env)
  if shared.Settings.EVAL_CTORS:
    if DEBUG:
      save_intermediate_with_wasm('pre-eval-ctors', wasm_binary_target)
    shared.Building.eval_ctors(final, wasm_binary_target, binaryen_bin, debug_info=debug_info)

  # after generating the wasm, do some final operations
  if shared.Settings.SIDE_MODULE and not shared.Settings.WASM_BACKEND:
    wso = shared.WebAssembly.make_shared_library(final, wasm_binary_target, shared.Settings.RUNTIME_LINKED_LIBS)
    # replace the wasm binary output with the dynamic library.
    # TODO: use a specific suffix for such files?
    shutil.move(wso, wasm_binary_target)
    if not DEBUG:
      os.unlink(asm_target) # we don't need the asm.js, it can just confuse

  if shared.Settings.EMIT_EMSCRIPTEN_METADATA:
    wso = shared.WebAssembly.add_emscripten_metadata(final, wasm_binary_target)
    shutil.move(wso, wasm_binary_target)

  if shared.Settings.SIDE_MODULE:
    sys.exit(0) # and we are done.

  if options.opt_level >= 2:
    # minify the JS
    optimizer.do_minify() # calculate how to minify
    if optimizer.cleanup_shell or options.use_closure_compiler:
      save_intermediate_with_wasm('preclean', wasm_binary_target)
      final = shared.Building.minify_wasm_js(js_file=final,
                                             wasm_file=wasm_binary_target,
                                             expensive_optimizations=will_metadce(options),
                                             minify_whitespace=optimizer.minify_whitespace,
                                             use_closure_compiler=options.use_closure_compiler,
                                             debug_info=debug_info,
                                             emit_symbol_map=emit_symbol_map)
      save_intermediate_with_wasm('postclean', wasm_binary_target)
  # replace placeholder strings with correct subresource locations
  if shared.Settings.SINGLE_FILE:
    js = open(final).read()
    for target, replacement_string, should_embed in (
        (wasm_binary_target,
         shared.FilenameReplacementStrings.WASM_BINARY_FILE,
         True),
        (asm_target,
         shared.FilenameReplacementStrings.ASMJS_CODE_FILE,
         False),
      ):
      if should_embed and os.path.isfile(target):
        js = js.replace(replacement_string, shared.JS.get_subresource_location(target))
      else:
        js = js.replace(replacement_string, '')
      shared.try_delete(target)
    with open(final, 'w') as f:
      f.write(js)


def modularize():
  global final
  logger.debug('Modularizing, assigning to var ' + shared.Settings.EXPORT_NAME)
  src = open(final).read()

  # TODO: exports object generation for MINIMAL_RUNTIME
  exports_object = '{}' if shared.Settings.MINIMAL_RUNTIME else shared.Settings.EXPORT_NAME

  src = '''
function(%(EXPORT_NAME)s) {
  %(EXPORT_NAME)s = %(EXPORT_NAME)s || {};

%(src)s

  return %(exports_object)s
}
''' % {
    'EXPORT_NAME': shared.Settings.EXPORT_NAME,
    'src': src,
    'exports_object': exports_object
  }

  if not shared.Settings.MODULARIZE_INSTANCE:
    if shared.Settings.MINIMAL_RUNTIME and not shared.Settings.USE_PTHREADS:
      # Single threaded MINIMAL_RUNTIME programs do not need access to
      # document.currentScript, so a simple export declaration is enough.
      src = 'var %s=%s' % (shared.Settings.EXPORT_NAME, src)
    else:
      # When MODULARIZE this JS may be executed later,
      # after document.currentScript is gone, so we save it.
      # (when MODULARIZE_INSTANCE, an instance is created
      # immediately anyhow, like in non-modularize mode)
      src = '''
var %(EXPORT_NAME)s = (function() {
  var _scriptDir = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;
  return (%(src)s);
})();
''' % {
        'EXPORT_NAME': shared.Settings.EXPORT_NAME,
        'src': src
      }
  else:
    # Create the MODULARIZE_INSTANCE instance
    # Note that we notice the global Module object, just like in normal
    # non-MODULARIZE mode (while MODULARIZE has the user create the instances,
    # and the user can decide whether to use Module there or something
    # else etc.).
    src = '''
var %(EXPORT_NAME)s = (%(src)s)(typeof %(EXPORT_NAME)s === 'object' ? %(EXPORT_NAME)s : {});
''' % {
      'EXPORT_NAME': shared.Settings.EXPORT_NAME,
      'src': src
    }

  final = final + '.modular.js'
  with open(final, 'w') as f:
    f.write(src)

    # Export using a UMD style export, or ES6 exports if selected

    if shared.Settings.EXPORT_ES6:
      f.write('''export default %s;''' % shared.Settings.EXPORT_NAME)
    elif not shared.Settings.MINIMAL_RUNTIME:
      f.write('''if (typeof exports === 'object' && typeof module === 'object')
      module.exports = %(EXPORT_NAME)s;
    else if (typeof define === 'function' && define['amd'])
      define([], function() { return %(EXPORT_NAME)s; });
    else if (typeof exports === 'object')
      exports["%(EXPORT_NAME)s"] = %(EXPORT_NAME)s;
    ''' % {
        'EXPORT_NAME': shared.Settings.EXPORT_NAME
      })

  save_intermediate('modularized')


def module_export_name_substitution():
  global final
  logger.debug('Private module export name substitution with ' + shared.Settings.EXPORT_NAME)
  src = open(final).read()
  final = final + '.module_export_name_substitution.js'
  if shared.Settings.MINIMAL_RUNTIME:
    # In MINIMAL_RUNTIME the Module object is always present to provide the .asm.js/.wasm content
    replacement = shared.Settings.EXPORT_NAME
  else:
    replacement = "typeof %(EXPORT_NAME)s !== 'undefined' ? %(EXPORT_NAME)s : {}" % {"EXPORT_NAME": shared.Settings.EXPORT_NAME}
  with open(final, 'w') as f:
    src = src.replace(shared.JS.module_export_name_substitution_pattern, replacement)
    # For Node.js and other shell environments, create an unminified Module object so that
    # loading external .asm.js file that assigns to Module['asm'] works even when Closure is used.
    if shared.Settings.MINIMAL_RUNTIME and (shared.Settings.target_environment_may_be('node') or
                                            shared.Settings.target_environment_may_be('shell')):
      src = 'if(typeof Module==="undefined"){var Module={};}' + src
    f.write(src)
  save_intermediate('module_export_name_substitution')


def generate_minimal_runtime_html(target, options, js_target, target_basename,
                                  asm_target, wasm_binary_target,
                                  memfile, optimizer):
  logger.debug('generating HTML for minimal runtime')
  shell = read_and_preprocess(options.shell_path)
  if re.search('{{{\s*SCRIPT\s*}}}', shell):
    exit_with_error('--shell-file "' + options.shell_path + '": MINIMAL_RUNTIME uses a different kind of HTML page shell file than the traditional runtime! Please see $EMSCRIPTEN/src/shell_minimal_runtime.html for a template to use as a basis.')

  shell = shell.replace('{{{ TARGET_BASENAME }}}', target_basename)
  shell = shell.replace('{{{ EXPORT_NAME }}}', shared.Settings.EXPORT_NAME)
  shell = tools.line_endings.convert_line_endings(shell, '\n', options.output_eol)
  with open(target, 'wb') as f:
    f.write(asbytes(shell))


def generate_traditional_runtime_html(target, options, js_target, target_basename,
                                      asm_target, wasm_binary_target,
                                      memfile, optimizer):
  script = ScriptSource()

  shell = read_and_preprocess(options.shell_path)
  assert '{{{ SCRIPT }}}' in shell, 'HTML shell must contain  {{{ SCRIPT }}}  , see src/shell.html for an example'
  base_js_target = os.path.basename(js_target)

  asm_mods = []

  if options.proxy_to_worker:
    proxy_worker_filename = (shared.Settings.PROXY_TO_WORKER_FILENAME or target_basename) + '.js'
    worker_js = worker_js_script(proxy_worker_filename)
    script.inline = ('''
  var filename = '%s';
  if ((',' + window.location.search.substr(1) + ',').indexOf(',noProxy,') < 0) {
    console.log('running code in a web worker');
''' % shared.JS.get_subresource_location(proxy_worker_filename)) + worker_js + '''
  } else {
    // note: no support for code mods (PRECISE_F32==2)
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

    asm_mods = client_mods.get_mods(shared.Settings,
                                    minified='minifyNames' in optimizer.queue_history,
                                    separate_asm=options.separate_asm)

  if not shared.Settings.SINGLE_FILE:
    if shared.Settings.EMTERPRETIFY_FILE:
      # We need to load the emterpreter file before anything else, it has to be synchronously ready
      script.un_src()
      script.inline = '''
          var emterpretURL = '%s';
          var emterpretXHR = new XMLHttpRequest();
          emterpretXHR.open('GET', emterpretURL, true);
          emterpretXHR.responseType = 'arraybuffer';
          emterpretXHR.onload = function() {
            if (emterpretXHR.status === 200 || emterpretXHR.status === 0) {
              Module.emterpreterFile = emterpretXHR.response;
            } else {
              var emterpretURLBytes = tryParseAsDataURI(emterpretURL);
              if (emterpretURLBytes) {
                Module.emterpreterFile = emterpretURLBytes.buffer;
              }
            }
%s
          };
          emterpretXHR.send(null);
''' % (shared.JS.get_subresource_location(shared.Settings.EMTERPRETIFY_FILE), script.inline)

    if options.memory_init_file and not shared.Settings.MEM_INIT_IN_WASM:
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

    # Download .asm.js if --separate-asm was passed in an asm.js build, or if 'asmjs' is one
    # of the wasm run methods.
    if not options.separate_asm or shared.Settings.WASM:
      if len(asm_mods):
         exit_with_error('no --separate-asm means no client code mods are possible')
    else:
      script.un_src()
      if len(asm_mods) == 0:
        # just load the asm, then load the rest
        script.inline = '''
    var filename = '%s';
    var fileBytes = tryParseAsDataURI(filename);
    var script = document.createElement('script');
    if (fileBytes) {
      script.innerHTML = intArrayToString(fileBytes);
    } else {
      script.src = filename;
    }
    script.onload = function() {
      setTimeout(function() {
        %s
      }, 1); // delaying even 1ms is enough to allow compilation memory to be reclaimed
    };
    document.body.appendChild(script);
''' % (shared.JS.get_subresource_location(asm_target), script.inline)
      else:
        # may need to modify the asm code, load it as text, modify, and load asynchronously
        script.inline = '''
    var codeURL = '%s';
    var codeXHR = new XMLHttpRequest();
    codeXHR.open('GET', codeURL, true);
    codeXHR.onload = function() {
      var code;
      if (codeXHR.status === 200 || codeXHR.status === 0) {
        code = codeXHR.responseText;
      } else {
        var codeURLBytes = tryParseAsDataURI(codeURL);
        if (codeURLBytes) {
          code = intArrayToString(codeURLBytes);
        }
      }
      %s
      var blob = new Blob([code], { type: 'text/javascript' });
      codeXHR = null;
      var src = URL.createObjectURL(blob);
      var script = document.createElement('script');
      script.src = src;
      script.onload = function() {
        setTimeout(function() {
          %s
        }, 1); // delaying even 1ms is enough to allow compilation memory to be reclaimed
        URL.revokeObjectURL(script.src);
      };
      document.body.appendChild(script);
    };
    codeXHR.send(null);
''' % (shared.JS.get_subresource_location(asm_target), '\n'.join(asm_mods), script.inline)

    if shared.Settings.WASM and not shared.Settings.BINARYEN_ASYNC_COMPILATION:
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
''' % (shared.JS.get_subresource_location(wasm_binary_target), script.inline)

  # when script.inline isn't empty, add required helper functions such as tryParseAsDataURI
  if script.inline:
    for filename in ('arrayUtils.js', 'base64Utils.js', 'URIUtils.js'):
      content = read_and_preprocess(shared.path_from_root('src', filename))
      script.inline = content + script.inline

    script.inline = 'var ASSERTIONS = %s;\n%s' % (shared.Settings.ASSERTIONS, script.inline)

  # inline script for SINGLE_FILE output
  if shared.Settings.SINGLE_FILE:
    js_contents = script.inline or ''
    if script.src:
      js_contents += open(js_target).read()
    shared.try_delete(js_target)
    script.src = None
    script.inline = js_contents

  html_contents = shell.replace('{{{ SCRIPT }}}', script.replacement())
  html_contents = tools.line_endings.convert_line_endings(html_contents, '\n', options.output_eol)
  with open(target, 'wb') as f:
    f.write(asbytes(html_contents))


def minify_html(filename, options):
  opts = []
  # -g1 and greater retain whitespace and comments in source
  if options.debug_level == 0:
    opts += ['--collapse-whitespace',
             '--collapse-inline-tag-whitespace',
             '--remove-comments',
             '--remove-tag-whitespace',
             '--sort-attributes',
             '--sort-class-name']
  # -g2 and greater do not minify HTML at all
  if options.debug_level <= 1:
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

  if options.debug_level >= 2:
    return

  logger.debug('minifying HTML file ' + filename)
  size_before = os.path.getsize(filename)
  start_time = time.time()
  run_process(shared.NODE_JS + [shared.path_from_root('third_party', 'html-minifier', 'cli.js'), filename, '-o', filename] + opts)
  elapsed_time = time.time() - start_time
  size_after = os.path.getsize(filename)
  delta = size_after - size_before
  logger.debug('HTML minification took {:.2f}'.format(elapsed_time) + ' seconds, and shrunk size of ' + filename + ' from ' + str(size_before) + ' to ' + str(size_after) + ' bytes, delta=' + str(delta) + ' ({:+.2f}%)'.format(delta * 100.0 / size_before))


def generate_html(target, options, js_target, target_basename,
                  asm_target, wasm_binary_target,
                  memfile, optimizer):
  logger.debug('generating HTML')

  if shared.Settings.MINIMAL_RUNTIME:
    generate_minimal_runtime_html(target, options, js_target, target_basename, asm_target,
                                  wasm_binary_target, memfile, optimizer)
  else:
    generate_traditional_runtime_html(target, options, js_target, target_basename, asm_target,
                                      wasm_binary_target, memfile, optimizer)

  if shared.Settings.MINIFY_HTML and (options.opt_level >= 1 or options.shrink_level >= 1):
    minify_html(target, options)


def generate_worker_js(target, js_target, target_basename):
  # compiler output is embedded as base64
  if shared.Settings.SINGLE_FILE:
    proxy_worker_filename = shared.JS.get_subresource_location(js_target)

  # compiler output goes in .worker.js file
  else:
    shutil.move(js_target, unsuffixed(js_target) + '.worker.js')
    worker_target_basename = target_basename + '.worker'
    proxy_worker_filename = (shared.Settings.PROXY_TO_WORKER_FILENAME or worker_target_basename) + '.js'

  target_contents = worker_js_script(proxy_worker_filename)
  open(target, 'w').write(target_contents)


def worker_js_script(proxy_worker_filename):
  web_gl_client_src = open(shared.path_from_root('src', 'webGLClient.js')).read()
  idb_store_src = open(shared.path_from_root('src', 'IDBStore.js')).read()
  proxy_client_src = (
    open(shared.path_from_root('src', 'proxyClient.js')).read()
    .replace('{{{ filename }}}', proxy_worker_filename)
    .replace('{{{ IDBStore.js }}}', idb_store_src)
  )

  return web_gl_client_src + '\n' + proxy_client_src


def process_libraries(libs, lib_dirs, settings_changes, input_files):
  libraries = []

  # Find library files
  for i, lib in libs:
    logger.debug('looking for library "%s"', lib)
    found = False
    for prefix in LIB_PREFIXES:
      for suff in STATICLIB_ENDINGS + DYNAMICLIB_ENDINGS:
        name = prefix + lib + suff
        for lib_dir in lib_dirs:
          path = os.path.join(lib_dir, name)
          if os.path.exists(path):
            logger.debug('found library "%s" at %s', lib, path)
            input_files.append((i, path))
            found = True
            break
        if found:
          break
      if found:
        break
    if not found:
      libraries += shared.Building.path_to_system_js_libraries(lib)

  # Certain linker flags imply some link libraries to be pulled in by default.
  libraries += shared.Building.path_to_system_js_libraries_for_settings(settings_changes)
  return 'SYSTEM_JS_LIBRARIES="' + ','.join(libraries) + '"'


class ScriptSource(object):
  def __init__(self):
    self.src = None # if set, we have a script to load with a src attribute
    self.inline = None # if set, we have the contents of a script to write inline in a script

  def un_src(self):
    """Use this if you want to modify the script and need it to be inline."""
    if self.src is None:
      return
    self.inline = '''
          var script = document.createElement('script');
          script.src = "%s";
          document.body.appendChild(script);
''' % self.src
    self.src = None

  def replacement(self):
    """Returns the script tag to replace the {{{ SCRIPT }}} tag in the target"""
    assert (self.src or self.inline) and not (self.src and self.inline)
    if self.src:
      return '<script async type="text/javascript" src="%s"></script>' % quote(self.src)
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


def parse_value(text):
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
    if text[-1] != ']':
      exit_with_error('unclosed opened string list. expected final character to be "]" in "%s"' % (text))
    inner = text[1:-1]
    if inner.strip() == "":
      return []
    return parse_string_list_members(inner)

  if text[0] == '[':
    return parse_string_list(text)

  try:
    return int(text)
  except ValueError:
    return parse_string_value(text)


def validate_arg_level(level_string, max_level, err_msg, clamp=False):
  try:
    level = int(level_string)
    if clamp:
      if level > max_level:
        logger.warning("optimization level '-O" + level_string + "' is not supported; using '-O" + str(max_level) + "' instead")
        level = max_level
    assert 0 <= level <= max_level
  except:
    raise Exception(err_msg)
  return level


if __name__ == '__main__':
  try:
    sys.exit(run(sys.argv))
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
