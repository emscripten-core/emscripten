# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from .toolchain_profiler import ToolchainProfiler

from subprocess import PIPE
import atexit
import binascii
import base64
import json
import logging
import os
import re
import shutil
import subprocess
import time
import signal
import sys
import tempfile

# We depend on python 3.6 for fstring support
if sys.version_info < (3, 6):
  print('error: emscripten requires python 3.6 or above', file=sys.stderr)
  sys.exit(1)

from .tempfiles import try_delete
from .utils import path_from_root, exit_with_error, safe_ensure_dirs, WINDOWS
from . import cache, tempfiles, colored_logger
from . import diagnostics
from . import config
from . import filelock
from .settings import settings


DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))
DEBUG_SAVE = DEBUG or int(os.environ.get('EMCC_DEBUG_SAVE', '0'))
EXPECTED_NODE_VERSION = (4, 1, 1)
EXPECTED_LLVM_VERSION = "13.0"
PYTHON = sys.executable

# Used only when EM_PYTHON_MULTIPROCESSING=1 env. var is set.
multiprocessing_pool = None

# can add  %(asctime)s  to see timestamps
logging.basicConfig(format='%(name)s:%(levelname)s: %(message)s',
                    level=logging.DEBUG if DEBUG else logging.INFO)
colored_logger.enable()
logger = logging.getLogger('shared')

# warning about absolute-paths is disabled by default, and not enabled by -Wall
diagnostics.add_warning('absolute-paths', enabled=False, part_of_all=False)
# unused diagnostic flags.  TODO(sbc): remove at some point
diagnostics.add_warning('almost-asm')
diagnostics.add_warning('experimental')
diagnostics.add_warning('invalid-input')
# Don't show legacy settings warnings by default
diagnostics.add_warning('legacy-settings', enabled=False, part_of_all=False)
# Catch-all for other emcc warnings
diagnostics.add_warning('linkflags')
diagnostics.add_warning('emcc')
diagnostics.add_warning('undefined', error=True)
diagnostics.add_warning('deprecated', shared=True)
diagnostics.add_warning('version-check')
diagnostics.add_warning('export-main')
diagnostics.add_warning('unused-command-line-argument', shared=True)
diagnostics.add_warning('pthreads-mem-growth')


# TODO(sbc): Investigate switching to shlex.quote
def shlex_quote(arg):
  arg = os.fspath(arg)
  if ' ' in arg and (not (arg.startswith('"') and arg.endswith('"'))) and (not (arg.startswith("'") and arg.endswith("'"))):
    return '"' + arg.replace('"', '\\"') + '"'

  return arg


# Switch to shlex.join once we can depend on python 3.8:
# https://docs.python.org/3/library/shlex.html#shlex.join
def shlex_join(cmd):
  return ' '.join(shlex_quote(x) for x in cmd)


def run_process(cmd, check=True, input=None, *args, **kw):
  """Runs a subprocess returning the exit code.

  By default this function will raise an exception on failure.  Therefor this should only be
  used if you want to handle such failures.  For most subprocesses, failures are not recoverable
  and should be fatal.  In those cases the `check_call` wrapper should be preferred.
  """

  # Flush standard streams otherwise the output of the subprocess may appear in the
  # output before messages that we have already written.
  sys.stdout.flush()
  sys.stderr.flush()
  kw.setdefault('universal_newlines', True)
  ret = subprocess.run(cmd, check=check, input=input, *args, **kw)
  debug_text = '%sexecuted %s' % ('successfully ' if check else '', shlex_join(cmd))
  logger.debug(debug_text)
  return ret


def get_num_cores():
  return int(os.environ.get('EMCC_CORES', os.cpu_count()))


def mp_run_process(command_tuple):
  temp_files = configuration.get_temp_files()
  cmd, env, route_stdout_to_temp_files_suffix, pipe_stdout, check, cwd = command_tuple
  std_out = temp_files.get(route_stdout_to_temp_files_suffix) if route_stdout_to_temp_files_suffix else (subprocess.PIPE if pipe_stdout else None)
  ret = std_out.name if route_stdout_to_temp_files_suffix else None
  proc = subprocess.Popen(cmd, stdout=std_out, stderr=subprocess.PIPE if pipe_stdout else None, env=env, cwd=cwd)
  out, _ = proc.communicate()
  if pipe_stdout:
    ret = out.decode('UTF-8')
  return ret


def returncode_to_str(code):
  assert code != 0
  if code < 0:
    signal_name = signal.Signals(-code).name
    return f'received {signal_name} ({code})'

  return f'returned {code}'


# Runs multiple subprocess commands.
# bool 'check': If True (default), raises an exception if any of the subprocesses failed with a nonzero exit code.
# string 'route_stdout_to_temp_files_suffix': if not None, all stdouts are instead written to files, and an array of filenames is returned.
# bool 'pipe_stdout': If True, an array of stdouts is returned, for each subprocess.
def run_multiple_processes(commands, env=os.environ.copy(), route_stdout_to_temp_files_suffix=None, pipe_stdout=False, check=True, cwd=None):
  # By default, avoid using Python multiprocessing library due to a large amount of bugs it has on Windows (#8013, #718, #13785, etc.)
  # Use EM_PYTHON_MULTIPROCESSING=1 environment variable to enable it. It can be faster, but may not work on Windows.
  if int(os.getenv('EM_PYTHON_MULTIPROCESSING', '0')):
    import multiprocessing
    global multiprocessing_pool
    if not multiprocessing_pool:
      multiprocessing_pool = multiprocessing.Pool(processes=get_num_cores())
    return multiprocessing_pool.map(mp_run_process, [(cmd, env, route_stdout_to_temp_files_suffix, pipe_stdout, check, cwd) for cmd in commands], chunksize=1)

  std_outs = []

  if route_stdout_to_temp_files_suffix and pipe_stdout:
    raise Exception('Cannot simultaneously pipe stdout to file and a string! Choose one or the other.')

  # TODO: Experiment with registering a signal handler here to see if that helps with Ctrl-C locking up the command prompt
  # when multiple child processes have been spawned.
  # import signal
  # def signal_handler(sig, frame):
  #   sys.exit(1)
  # signal.signal(signal.SIGINT, signal_handler)

  with ToolchainProfiler.profile_block('run_multiple_processes'):
    processes = []
    num_parallel_processes = get_num_cores()
    temp_files = configuration.get_temp_files()
    i = 0
    num_completed = 0

    while num_completed < len(commands):
      if i < len(commands) and len(processes) < num_parallel_processes:
        # Not enough parallel processes running, spawn a new one.
        std_out = temp_files.get(route_stdout_to_temp_files_suffix) if route_stdout_to_temp_files_suffix else (subprocess.PIPE if pipe_stdout else None)
        if DEBUG:
          logger.debug('Running subprocess %d/%d: %s' % (i + 1, len(commands), ' '.join(commands[i])))
        processes += [(i, subprocess.Popen(commands[i], stdout=std_out, stderr=subprocess.PIPE if pipe_stdout else None, env=env, cwd=cwd))]
        if route_stdout_to_temp_files_suffix:
          std_outs += [(i, std_out.name)]
        i += 1
      else:
        # Not spawning a new process (Too many commands running in parallel, or no commands left): find if a process has finished.
        def get_finished_process():
          while True:
            j = 0
            while j < len(processes):
              if processes[j][1].poll() is not None:
                out, err = processes[j][1].communicate()
                return (j, out.decode('UTF-8') if out else '', err.decode('UTF-8') if err else '')
              j += 1
            # All processes still running; wait a short while for the first (oldest) process to finish,
            # then look again if any process has completed.
            try:
              out, err = processes[0][1].communicate(0.2)
              return (0, out.decode('UTF-8') if out else '', err.decode('UTF-8') if err else '')
            except subprocess.TimeoutExpired:
              pass

        j, out, err = get_finished_process()
        idx, finished_process = processes[j]
        del processes[j]
        if pipe_stdout:
          std_outs += [(idx, out)]
        if check and finished_process.returncode != 0:
          if out:
            logger.info(out)
          if err:
            logger.error(err)

          raise Exception('Subprocess %d/%d failed (%s)! (cmdline: %s)' % (idx + 1, len(commands), returncode_to_str(finished_process.returncode), shlex_join(commands[idx])))
        num_completed += 1

  # If processes finished out of order, sort the results to the order of the input.
  std_outs.sort(key=lambda x: x[0])
  return [x[1] for x in std_outs]


def check_call(cmd, *args, **kw):
  """Like `run_process` above but treat failures as fatal and exit_with_error."""
  print_compiler_stage(cmd)
  try:
    return run_process(cmd, *args, **kw)
  except subprocess.CalledProcessError as e:
    exit_with_error("'%s' failed (%s)", shlex_join(cmd), returncode_to_str(e.returncode))
  except OSError as e:
    exit_with_error("'%s' failed: %s", shlex_join(cmd), str(e))


def run_js_tool(filename, jsargs=[], *args, **kw):
  """Execute a javascript tool.

  This is used by emcc to run parts of the build process that are written
  implemented in javascript.
  """
  command = config.NODE_JS + [filename] + jsargs
  return check_call(command, *args, **kw).stdout


# Only used by tests and by ctor_evaller.py.   Once fastcomp is removed
# this can most likely be moved into the tests/jsrun.py.
def timeout_run(proc, timeout=None, full_output=False, check=True):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out")
  stdout, stderr = proc.communicate()
  out = ['' if o is None else o for o in (stdout, stderr)]
  if check and proc.returncode != 0:
    raise subprocess.CalledProcessError(proc.returncode, '', stdout, stderr)
  if TRACK_PROCESS_SPAWNS:
    logging.info('Process ' + str(proc.pid) + ' finished after ' + str(time.time() - start) + ' seconds. Exit code: ' + str(proc.returncode))
  return '\n'.join(out) if full_output else out[0]


def get_npm_cmd(name):
  if WINDOWS:
    cmd = [path_from_root('node_modules', '.bin', name + '.cmd')]
  else:
    cmd = config.NODE_JS + [path_from_root('node_modules', '.bin', name)]
  if not os.path.exists(cmd[-1]):
    exit_with_error('%s was not found! Please run "npm install" in Emscripten root directory to set up npm dependencies' % name)
  return cmd


def get_clang_version():
  if not hasattr(get_clang_version, 'found_version'):
    if not os.path.exists(CLANG_CC):
      exit_with_error('clang executable not found at `%s`' % CLANG_CC)
    proc = check_call([CLANG_CC, '--version'], stdout=PIPE)
    m = re.search(r'[Vv]ersion\s+(\d+\.\d+)', proc.stdout)
    get_clang_version.found_version = m and m.group(1)
  return get_clang_version.found_version


def check_llvm_version():
  actual = get_clang_version()
  if EXPECTED_LLVM_VERSION in actual:
    return True
  diagnostics.warning('version-check', 'LLVM version for clang executable "%s" appears incorrect (seeing "%s", expected "%s")', CLANG_CC, actual, EXPECTED_LLVM_VERSION)
  return False


def get_llc_targets():
  if not os.path.exists(LLVM_COMPILER):
    exit_with_error('llc executable not found at `%s`' % LLVM_COMPILER)
  try:
    llc_version_info = run_process([LLVM_COMPILER, '--version'], stdout=PIPE).stdout
  except subprocess.CalledProcessError:
    exit_with_error('error running `llc --version`.  Check your llvm installation (%s)' % LLVM_COMPILER)
  if 'Registered Targets:' not in llc_version_info:
    exit_with_error('error parsing output of `llc --version`.  Check your llvm installation (%s)' % LLVM_COMPILER)
  pre, targets = llc_version_info.split('Registered Targets:')
  return targets


def check_llvm():
  targets = get_llc_targets()
  if 'wasm32' not in targets and 'WebAssembly 32-bit' not in targets:
    logger.critical('LLVM has not been built with the WebAssembly backend, llc reports:')
    print('===========================================================================', file=sys.stderr)
    print(targets, file=sys.stderr)
    print('===========================================================================', file=sys.stderr)
    return False

  return True


def get_node_directory():
  return os.path.dirname(config.NODE_JS[0] if type(config.NODE_JS) is list else config.NODE_JS)


# When we run some tools from npm (closure, html-minifier-terser), those
# expect that the tools have node.js accessible in PATH. Place our node
# there when invoking those tools.
def env_with_node_in_path():
  env = os.environ.copy()
  env['PATH'] = get_node_directory() + os.pathsep + env['PATH']
  return env


def check_node_version():
  try:
    actual = run_process(config.NODE_JS + ['--version'], stdout=PIPE).stdout.strip()
    version = tuple(map(int, actual.replace('v', '').replace('-pre', '').split('.')))
  except Exception as e:
    diagnostics.warning('version-check', 'cannot check node version: %s', e)
    return False

  if version < EXPECTED_NODE_VERSION:
    diagnostics.warning('version-check', 'node version appears too old (seeing "%s", expected "%s")', actual, 'v' + ('.'.join(map(str, EXPECTED_NODE_VERSION))))
    return False

  return True


def set_version_globals():
  global EMSCRIPTEN_VERSION, EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY
  filename = path_from_root('emscripten-version.txt')
  with open(filename) as f:
    EMSCRIPTEN_VERSION = f.read().strip().strip('"')
  parts = [int(x) for x in EMSCRIPTEN_VERSION.split('.')]
  EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY = parts


def generate_sanity():
  sanity_file_content = EMSCRIPTEN_VERSION + '|' + config.LLVM_ROOT + '|' + get_clang_version()
  config_data = open(config.EM_CONFIG).read()
  checksum = binascii.crc32(config_data.encode())
  sanity_file_content += '|%#x\n' % checksum
  return sanity_file_content


def perform_sanity_checks():
  # some warning, mostly not fatal checks - do them even if EM_IGNORE_SANITY is on
  check_node_version()
  check_llvm_version()

  llvm_ok = check_llvm()

  if os.environ.get('EM_IGNORE_SANITY'):
    logger.info('EM_IGNORE_SANITY set, ignoring sanity checks')
    return

  logger.info('(Emscripten: Running sanity checks)')

  if not llvm_ok:
    exit_with_error('failing sanity checks due to previous llvm failure')

  with ToolchainProfiler.profile_block('sanity compiler_engine'):
    try:
      run_process(config.NODE_JS + ['-e', 'console.log("hello")'], stdout=PIPE)
    except Exception as e:
      exit_with_error('The configured node executable (%s) does not seem to work, check the paths in %s (%s)', config.NODE_JS, config.EM_CONFIG, str(e))

  with ToolchainProfiler.profile_block('sanity LLVM'):
    for cmd in [CLANG_CC, LLVM_AR, LLVM_NM]:
      if not os.path.exists(cmd) and not os.path.exists(cmd + '.exe'):  # .exe extension required for Windows
        exit_with_error('Cannot find %s, check the paths in %s', cmd, config.EM_CONFIG)


@ToolchainProfiler.profile_block('sanity')
def check_sanity(force=False):
  """Check that basic stuff we need (a JS engine to compile, Node.js, and Clang
  and LLVM) exists.

  The test runner always does this check (through |force|). emcc does this less
  frequently, only when ${EM_CONFIG}_sanity does not exist or is older than
  EM_CONFIG (so, we re-check sanity when the settings are changed).  We also
  re-check sanity and clear the cache when the version changes.
  """
  if not force and os.environ.get('EMCC_SKIP_SANITY_CHECK') == '1':
    return

  # We set EMCC_SKIP_SANITY_CHECK so that any subprocesses that we launch will
  # not re-run the tests.
  os.environ['EMCC_SKIP_SANITY_CHECK'] = '1'

  if DEBUG:
    force = True

  if config.FROZEN_CACHE:
    if force:
      perform_sanity_checks()
    return

  if os.environ.get('EM_IGNORE_SANITY'):
    perform_sanity_checks()
    return

  expected = generate_sanity()

  sanity_file = Cache.get_path('sanity.txt')
  with Cache.lock():
    if os.path.exists(sanity_file):
      sanity_data = open(sanity_file).read()
      if sanity_data != expected:
        logger.debug('old sanity: %s' % sanity_data)
        logger.debug('new sanity: %s' % expected)
        logger.info('(Emscripten: config changed, clearing cache)')
        Cache.erase()
        # the check actually failed, so definitely write out the sanity file, to
        # avoid others later seeing failures too
        force = False
      else:
        if force:
          logger.debug(f'sanity file up-to-date but check forced: {sanity_file}')
        else:
          logger.debug(f'sanity file up-to-date: {sanity_file}')
          return # all is well
    else:
      logger.debug(f'sanity file not found: {sanity_file}')

    perform_sanity_checks()

    if not force:
      # Only create/update this file if the sanity check succeeded, i.e., we got here
      with open(sanity_file, 'w') as f:
        f.write(expected)


# Some distributions ship with multiple llvm versions so they add
# the version to the binaries, cope with that
def build_llvm_tool_path(tool):
  if config.LLVM_ADD_VERSION:
    return os.path.join(config.LLVM_ROOT, tool + "-" + config.LLVM_ADD_VERSION)
  else:
    return os.path.join(config.LLVM_ROOT, tool)


# Some distributions ship with multiple clang versions so they add
# the version to the binaries, cope with that
def build_clang_tool_path(tool):
  if config.CLANG_ADD_VERSION:
    return os.path.join(config.LLVM_ROOT, tool + "-" + config.CLANG_ADD_VERSION)
  else:
    return os.path.join(config.LLVM_ROOT, tool)


def exe_suffix(cmd):
  return cmd + '.exe' if WINDOWS else cmd


def bat_suffix(cmd):
  return cmd + '.bat' if WINDOWS else cmd


def replace_suffix(filename, new_suffix):
  assert new_suffix[0] == '.'
  return os.path.splitext(filename)[0] + new_suffix


# In MINIMAL_RUNTIME mode, keep suffixes of generated files simple
# ('.mem' instead of '.js.mem'; .'symbols' instead of '.js.symbols' etc)
# Retain the original naming scheme in traditional runtime.
def replace_or_append_suffix(filename, new_suffix):
  assert new_suffix[0] == '.'
  return replace_suffix(filename, new_suffix) if settings.MINIMAL_RUNTIME else filename + new_suffix


# Temp dir. Create a random one, unless EMCC_DEBUG is set, in which case use the canonical
# temp directory (TEMP_DIR/emscripten_temp).
def get_emscripten_temp_dir():
  """Returns a path to EMSCRIPTEN_TEMP_DIR, creating one if it didn't exist."""
  global configuration, EMSCRIPTEN_TEMP_DIR
  if not EMSCRIPTEN_TEMP_DIR:
    EMSCRIPTEN_TEMP_DIR = tempfile.mkdtemp(prefix='emscripten_temp_', dir=configuration.TEMP_DIR)

    if not DEBUG_SAVE:
      def prepare_to_clean_temp(d):
        def clean_temp():
          try_delete(d)

        atexit.register(clean_temp)
      # this global var might change later
      prepare_to_clean_temp(EMSCRIPTEN_TEMP_DIR)
  return EMSCRIPTEN_TEMP_DIR


def get_canonical_temp_dir(temp_dir):
  return os.path.join(temp_dir, 'emscripten_temp')


class Configuration:
  def __init__(self):
    self.EMSCRIPTEN_TEMP_DIR = None

    self.TEMP_DIR = os.environ.get("EMCC_TEMP_DIR", tempfile.gettempdir())
    if not os.path.isdir(self.TEMP_DIR):
      exit_with_error("The temporary directory `" + self.TEMP_DIR + "` does not exist! Please make sure that the path is correct.")

    self.CANONICAL_TEMP_DIR = get_canonical_temp_dir(self.TEMP_DIR)

    if DEBUG:
      self.EMSCRIPTEN_TEMP_DIR = self.CANONICAL_TEMP_DIR
      try:
        safe_ensure_dirs(self.EMSCRIPTEN_TEMP_DIR)
      except Exception as e:
        exit_with_error(str(e) + 'Could not create canonical temp dir. Check definition of TEMP_DIR in ' + config.EM_CONFIG)

      # Since the canonical temp directory is, by definition, the same
      # between all processes that run in DEBUG mode we need to use a multi
      # process lock to prevent more than one process from writing to it.
      # This is because emcc assumes that it can use non-unique names inside
      # the temp directory.
      # Sadly we need to allow child processes to access this directory
      # though, since emcc can recursively call itself when building
      # libraries and ports.
      if 'EM_HAVE_TEMP_DIR_LOCK' not in os.environ:
        filelock_name = os.path.join(self.EMSCRIPTEN_TEMP_DIR, 'emscripten.lock')
        lock = filelock.FileLock(filelock_name)
        os.environ['EM_HAVE_TEMP_DIR_LOCK'] = '1'
        lock.acquire()
        atexit.register(lock.release)

  def get_temp_files(self):
    if DEBUG_SAVE:
      # In debug mode store all temp files in the emscripten-specific temp dir
      # and don't worry about cleaning them up.
      return tempfiles.TempFiles(get_emscripten_temp_dir(), save_debug_files=True)
    else:
      # Otherwise use the system tempdir and try to clean up after ourselves.
      return tempfiles.TempFiles(self.TEMP_DIR, save_debug_files=False)


def apply_configuration():
  global configuration, EMSCRIPTEN_TEMP_DIR, CANONICAL_TEMP_DIR, TEMP_DIR
  configuration = Configuration()
  EMSCRIPTEN_TEMP_DIR = configuration.EMSCRIPTEN_TEMP_DIR
  CANONICAL_TEMP_DIR = configuration.CANONICAL_TEMP_DIR
  TEMP_DIR = configuration.TEMP_DIR


def target_environment_may_be(environment):
  return settings.ENVIRONMENT == '' or environment in settings.ENVIRONMENT.split(',')


def print_compiler_stage(cmd):
  """Emulate the '-v' of clang/gcc by printing the name of the sub-command
  before executing it."""
  if PRINT_STAGES:
    print(' "%s" %s' % (cmd[0], shlex_join(cmd[1:])), file=sys.stderr)
    sys.stderr.flush()


def mangle_c_symbol_name(name):
  return '_' + name if not name.startswith('$') else name[1:]


def demangle_c_symbol_name(name):
  return name[1:] if name.startswith('_') else '$' + name


def is_c_symbol(name):
  return name.startswith('_')


def treat_as_user_function(name):
  if name.startswith('dynCall_'):
    return False
  if name in settings.WASM_SYSTEM_EXPORTS:
    return False
  return True


def asmjs_mangle(name):
  """Mangle a name the way asm.js/JSBackend globals are mangled.

  Prepends '_' and replaces non-alphanumerics with '_'.
  Used by wasm backend for JS library consistency with asm.js.
  """
  if treat_as_user_function(name):
    return '_' + name
  else:
    return name


def reconfigure_cache():
  global Cache
  Cache = cache.Cache(config.CACHE)


class JS:
  emscripten_license = '''\
/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */
'''

  # handle the above form, and also what closure can emit which is stuff like
  #  /*
  #
  #   Copyright 2019 The Emscripten Authors
  #   SPDX-License-Identifier: MIT
  #
  #   Copyright 2017 The Emscripten Authors
  #   SPDX-License-Identifier: MIT
  #  */
  emscripten_license_regex = r'\/\*\*?(\s*\*?\s*@license)?(\s*\*?\s*Copyright \d+ The Emscripten Authors\s*\*?\s*SPDX-License-Identifier: MIT)+\s*\*\/'

  @staticmethod
  def handle_license(js_target):
    # ensure we emit the license if and only if we need to, and exactly once
    with open(js_target) as f:
      js = f.read()
    # first, remove the license as there may be more than once
    processed_js = re.sub(JS.emscripten_license_regex, '', js)
    if settings.EMIT_EMSCRIPTEN_LICENSE:
      processed_js = JS.emscripten_license + processed_js
    if processed_js != js:
      with open(js_target, 'w') as f:
        f.write(processed_js)

  @staticmethod
  def to_nice_ident(ident): # limited version of the JS function toNiceIdent
    return ident.replace('%', '$').replace('@', '_').replace('.', '_')

  # Returns the given string with escapes added so that it can safely be placed inside a string in JS code.
  @staticmethod
  def escape_for_js_string(s):
    s = s.replace('\\', '/').replace("'", "\\'").replace('"', '\\"')
    return s

  # Returns the subresource location for run-time access
  @staticmethod
  def get_subresource_location(path, data_uri=None):
    if data_uri is None:
      data_uri = settings.SINGLE_FILE
    if data_uri:
      # if the path does not exist, then there is no data to encode
      if not os.path.exists(path):
        return ''
      with open(path, 'rb') as f:
        data = base64.b64encode(f.read())
      return 'data:application/octet-stream;base64,' + data.decode('ascii')
    else:
      return os.path.basename(path)

  @staticmethod
  def legalize_sig(sig):
    # with BigInt support all sigs are legal since we can use i64s.
    if settings.WASM_BIGINT:
      return sig
    legal = [sig[0]]
    # a return of i64 is legalized into an i32 (and the high bits are
    # accessible on the side through getTempRet0).
    if legal[0] == 'j':
      legal[0] = 'i'
    # a parameter of i64 is legalized into i32, i32
    for s in sig[1:]:
      if s != 'j':
        legal.append(s)
      else:
        legal.append('i')
        legal.append('i')
    return ''.join(legal)

  @staticmethod
  def is_legal_sig(sig):
    # with BigInt support all sigs are legal since we can use i64s.
    if settings.WASM_BIGINT:
      return True
    return sig == JS.legalize_sig(sig)

  @staticmethod
  def isidentifier(name):
    # https://stackoverflow.com/questions/43244604/check-that-a-string-is-a-valid-javascript-identifier-name-using-python-3
    return name.replace('$', '_').isidentifier()

  @staticmethod
  def make_dynCall(sig, args):
    # wasm2c and asyncify are not yet compatible with direct wasm table calls
    if settings.DYNCALLS or not JS.is_legal_sig(sig):
      args = ','.join(args)
      if not settings.MAIN_MODULE and not settings.SIDE_MODULE:
        # Optimize dynCall accesses in the case when not building with dynamic
        # linking enabled.
        return 'dynCall_%s(%s)' % (sig, args)
      else:
        return 'Module["dynCall_%s"](%s)' % (sig, args)
    else:
      return 'wasmTable.get(%s)(%s)' % (args[0], ','.join(args[1:]))

  @staticmethod
  def make_invoke(sig, named=True):
    legal_sig = JS.legalize_sig(sig) # TODO: do this in extcall, jscall?
    args = ['index'] + ['a' + str(i) for i in range(1, len(legal_sig))]
    ret = 'return ' if sig[0] != 'v' else ''
    body = '%s%s;' % (ret, JS.make_dynCall(sig, args))
    # C++ exceptions are numbers, and longjmp is a string 'longjmp'
    if settings.SUPPORT_LONGJMP:
      rethrow = "if (e !== e+0 && e !== 'longjmp') throw e;"
    else:
      rethrow = "if (e !== e+0) throw e;"

    name = (' invoke_' + sig) if named else ''
    ret = '''\
function%s(%s) {
  var sp = stackSave();
  try {
    %s
  } catch(e) {
    stackRestore(sp);
    %s
    _setThrew(1, 0);
  }
}''' % (name, ','.join(args), body, rethrow)

    return ret


def suffix(name):
  """Return the file extension"""
  return os.path.splitext(name)[1]


def unsuffixed(name):
  """Return the filename without the extension.

  If there are multiple extensions this strips only the final one.
  """
  return os.path.splitext(name)[0]


def unsuffixed_basename(name):
  return os.path.basename(unsuffixed(name))


def strip_prefix(string, prefix):
  assert string.startswith(prefix)
  return string[len(prefix):]


def safe_copy(src, dst):
  logging.debug('copy: %s -> %s', src, dst)
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  if dst == os.devnull:
    return
  # Copies data and permission bits, but not other metadata such as timestamp
  shutil.copy(src, dst)


def read_and_preprocess(filename, expand_macros=False):
  temp_dir = get_emscripten_temp_dir()
  # Create a settings file with the current settings to pass to the JS preprocessor

  settings_str = ''
  for key, value in settings.dict().items():
    assert key == key.upper()  # should only ever be uppercase keys in settings
    jsoned = json.dumps(value, sort_keys=True)
    settings_str += f'var {key} = {jsoned};\n'

  settings_file = os.path.join(temp_dir, 'settings.js')
  with open(settings_file, 'w') as f:
    f.write(settings_str)

  # Run the JS preprocessor
  # N.B. We can't use the default stdout=PIPE here as it only allows 64K of output before it hangs
  # and shell.html is bigger than that!
  # See https://thraxil.org/users/anders/posts/2008/03/13/Subprocess-Hanging-PIPE-is-your-enemy/
  dirname, filename = os.path.split(filename)
  if not dirname:
    dirname = None
  stdout = os.path.join(temp_dir, 'stdout')
  args = [settings_file, filename]
  if expand_macros:
    args += ['--expandMacros']

  run_js_tool(path_from_root('tools/preprocessor.js'), args, True, stdout=open(stdout, 'w'), cwd=dirname)
  out = open(stdout, 'r').read()

  return out


def do_replace(input_, pattern, replacement):
  if pattern not in input_:
    exit_with_error('expected to find pattern in input JS: %s' % pattern)
  return input_.replace(pattern, replacement)


# ============================================================================
# End declarations.
# ============================================================================

# Everything below this point is top level code that get run when importing this
# file.  TODO(sbc): We should try to reduce that amount we do here and instead
# have consumers explicitly call initialization functions.

# Verbosity level control for any intermediate subprocess spawns from the compiler. Useful for internal debugging.
# 0: disabled.
# 1: Log stderr of subprocess spawns.
# 2: Log stdout and stderr of subprocess spawns. Print out subprocess commands that were executed.
# 3: Log stdout and stderr, and pass VERBOSE=1 to CMake configure steps.
EM_BUILD_VERBOSE = int(os.getenv('EM_BUILD_VERBOSE', '0'))
TRACK_PROCESS_SPAWNS = EM_BUILD_VERBOSE >= 3

set_version_globals()

CLANG_CC = os.path.expanduser(build_clang_tool_path(exe_suffix('clang')))
CLANG_CXX = os.path.expanduser(build_clang_tool_path(exe_suffix('clang++')))
LLVM_LINK = build_llvm_tool_path(exe_suffix('llvm-link'))
LLVM_AR = build_llvm_tool_path(exe_suffix('llvm-ar'))
LLVM_RANLIB = build_llvm_tool_path(exe_suffix('llvm-ranlib'))
LLVM_OPT = os.path.expanduser(build_llvm_tool_path(exe_suffix('opt')))
LLVM_NM = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-nm')))
LLVM_INTERPRETER = os.path.expanduser(build_llvm_tool_path(exe_suffix('lli')))
LLVM_COMPILER = os.path.expanduser(build_llvm_tool_path(exe_suffix('llc')))
LLVM_DWARFDUMP = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-dwarfdump')))
LLVM_OBJCOPY = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-objcopy')))
WASM_LD = os.path.expanduser(build_llvm_tool_path(exe_suffix('wasm-ld')))

EMCC = bat_suffix(path_from_root('emcc'))
EMXX = bat_suffix(path_from_root('em++'))
EMAR = bat_suffix(path_from_root('emar'))
EMRANLIB = bat_suffix(path_from_root('emranlib'))
EMCMAKE = bat_suffix(path_from_root('emcmake'))
EMCONFIGURE = bat_suffix(path_from_root('emconfigure'))
FILE_PACKAGER = bat_suffix(path_from_root('tools', 'file_packager'))

apply_configuration()

Cache = cache.Cache(config.CACHE)

PRINT_STAGES = int(os.getenv('EMCC_VERBOSE', '0'))
