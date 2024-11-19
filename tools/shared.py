# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from .toolchain_profiler import ToolchainProfiler

from enum import Enum, unique, auto
from functools import wraps
from subprocess import PIPE
import atexit
import json
import logging
import os
import re
import shutil
import subprocess
import signal
import stat
import sys
import tempfile

# We depend on python 3.6 for fstring support
if sys.version_info < (3, 6):
  print('error: emscripten requires python 3.6 or above', file=sys.stderr)
  sys.exit(1)

from . import colored_logger

# Configure logging before importing any other local modules so even
# log message during import are shown as expected.
DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))
EMCC_LOGGING = int(os.environ.get('EMCC_LOGGING', '1'))
log_level = logging.ERROR
if DEBUG:
  log_level = logging.DEBUG
elif EMCC_LOGGING:
  log_level = logging.INFO
# can add  %(asctime)s  to see timestamps
logging.basicConfig(format='%(name)s:%(levelname)s: %(message)s', level=log_level)
colored_logger.enable()

from .utils import path_from_root, exit_with_error, safe_ensure_dirs, WINDOWS, set_version_globals
from . import cache, tempfiles
from . import diagnostics
from . import config
from . import filelock
from . import utils
from .settings import settings


DEBUG_SAVE = DEBUG or int(os.environ.get('EMCC_DEBUG_SAVE', '0'))
PRINT_SUBPROCS = int(os.getenv('EMCC_VERBOSE', '0'))
SKIP_SUBPROCS = False

# Minimum node version required to run the emscripten compiler.  This is
# distinct from the minimum version required to execute the generated code
# (settings.MIN_NODE_VERSION).
# This version currently matches the node version that we ship with emsdk
# which means that we can say for sure that this version is well supported.
MINIMUM_NODE_VERSION = (16, 20, 0)
EXPECTED_LLVM_VERSION = 20

# These get set by setup_temp_dirs
TEMP_DIR = None
EMSCRIPTEN_TEMP_DIR = None

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
diagnostics.add_warning('map-unrecognized-libraries')
diagnostics.add_warning('unused-command-line-argument', shared=True)
diagnostics.add_warning('pthreads-mem-growth')
diagnostics.add_warning('transpile')
diagnostics.add_warning('limited-postlink-optimizations')
diagnostics.add_warning('em-js-i64')
diagnostics.add_warning('js-compiler')
diagnostics.add_warning('compatibility')
diagnostics.add_warning('unsupported')
diagnostics.add_warning('unused-main')
# Closure warning are not (yet) enabled by default
diagnostics.add_warning('closure', enabled=False)


# TODO(sbc): Investigate switching to shlex.quote
def shlex_quote(arg):
  arg = os.fspath(arg)
  if ' ' in arg and (not (arg.startswith('"') and arg.endswith('"'))) and (not (arg.startswith("'") and arg.endswith("'"))):
    return '"' + arg.replace('"', '\\"') + '"'

  return arg


# Switch to shlex.join once we can depend on python 3.8:
# https://docs.python.org/3/library/shlex.html#shlex.join
def shlex_join(cmd):
  if type(cmd) is str:
    return cmd
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
  kw.setdefault('encoding', 'utf-8')
  ret = subprocess.run(cmd, check=check, input=input, *args, **kw)
  debug_text = '%sexecuted %s' % ('successfully ' if check else '', shlex_join(cmd))
  logger.debug(debug_text)
  return ret


def get_num_cores():
  return int(os.environ.get('EMCC_CORES', os.cpu_count()))


def returncode_to_str(code):
  assert code != 0
  if code < 0:
    signal_name = signal.Signals(-code).name
    return f'received {signal_name} ({code})'

  return f'returned {code}'


def cap_max_workers_in_pool(max_workers):
  # Python has an issue that it can only use max 61 cores on Windows: https://github.com/python/cpython/issues/89240
  if WINDOWS:
    return min(max_workers, 61)
  return max_workers


def run_multiple_processes(commands,
                           env=None,
                           route_stdout_to_temp_files_suffix=None,
                           cwd=None):
  """Runs multiple subprocess commands.

  route_stdout_to_temp_files_suffix : string
    if not None, all stdouts are instead written to files, and an array
    of filenames is returned.
  """

  if env is None:
    env = os.environ.copy()

  std_outs = []

  # TODO: Experiment with registering a signal handler here to see if that helps with Ctrl-C locking up the command prompt
  # when multiple child processes have been spawned.
  # import signal
  # def signal_handler(sig, frame):
  #   sys.exit(1)
  # signal.signal(signal.SIGINT, signal_handler)

  # Map containing all currently running processes.
  # command index -> proc/Popen object
  processes = {}

  def get_finished_process():
    while True:
      for idx, proc in processes.items():
        if proc.poll() is not None:
          return idx
      # All processes still running; wait a short while for the first
      # (oldest) process to finish, then look again if any process has completed.
      idx, proc = next(iter(processes.items()))
      try:
        proc.communicate(timeout=0.2)
        return idx
      except subprocess.TimeoutExpired:
        pass

  num_parallel_processes = get_num_cores()
  temp_files = get_temp_files()
  i = 0
  num_completed = 0
  while num_completed < len(commands):
    if i < len(commands) and len(processes) < num_parallel_processes:
      # Not enough parallel processes running, spawn a new one.
      if route_stdout_to_temp_files_suffix:
        stdout = temp_files.get(route_stdout_to_temp_files_suffix)
      else:
        stdout = None
      if DEBUG:
        logger.debug('Running subprocess %d/%d: %s' % (i + 1, len(commands), ' '.join(commands[i])))
      print_compiler_stage(commands[i])
      proc = subprocess.Popen(commands[i], stdout=stdout, stderr=None, env=env, cwd=cwd)
      processes[i] = proc
      if route_stdout_to_temp_files_suffix:
        std_outs.append((i, stdout.name))
      i += 1
    else:
      # Not spawning a new process (Too many commands running in parallel, or
      # no commands left): find if a process has finished.
      idx = get_finished_process()
      finished_process = processes.pop(idx)
      if finished_process.returncode != 0:
        exit_with_error('subprocess %d/%d failed (%s)! (cmdline: %s)' % (idx + 1, len(commands), returncode_to_str(finished_process.returncode), shlex_join(commands[idx])))
      num_completed += 1

  if route_stdout_to_temp_files_suffix:
    # If processes finished out of order, sort the results to the order of the input.
    std_outs.sort(key=lambda x: x[0])
    return [x[1] for x in std_outs]


def check_call(cmd, *args, **kw):
  """Like `run_process` above but treat failures as fatal and exit_with_error."""
  print_compiler_stage(cmd)
  if SKIP_SUBPROCS:
    return 0
  try:
    return run_process(cmd, *args, **kw)
  except subprocess.CalledProcessError as e:
    exit_with_error("'%s' failed (%s)", shlex_join(cmd), returncode_to_str(e.returncode))
  except OSError as e:
    exit_with_error("'%s' failed: %s", shlex_join(cmd), str(e))


def exec_process(cmd):
  print_compiler_stage(cmd)
  if utils.WINDOWS:
    rtn = run_process(cmd, stdin=sys.stdin, check=False).returncode
    sys.exit(rtn)
  else:
    sys.stdout.flush()
    sys.stderr.flush()
    os.execvp(cmd[0], cmd)


def run_js_tool(filename, jsargs=[], node_args=[], **kw):  # noqa: mutable default args
  """Execute a javascript tool.

  This is used by emcc to run parts of the build process that are written
  implemented in javascript.
  """
  command = config.NODE_JS + node_args + [filename] + jsargs
  return check_call(command, **kw).stdout


def get_npm_cmd(name):
  if WINDOWS:
    cmd = [path_from_root('node_modules/.bin', name + '.cmd')]
  else:
    cmd = config.NODE_JS + [path_from_root('node_modules/.bin', name)]
  if not os.path.exists(cmd[-1]):
    exit_with_error(f'{name} was not found! Please run "npm install" in Emscripten root directory to set up npm dependencies')
  return cmd


# TODO(sbc): Replace with functools.cache, once we update to python 3.7
def memoize(func):
  called = False
  result = None

  @wraps(func)
  def helper():
    nonlocal called, result
    if not called:
      result = func()
      called = True
    return result

  return helper


@memoize
def get_clang_version():
  if not os.path.exists(CLANG_CC):
    exit_with_error('clang executable not found at `%s`' % CLANG_CC)
  proc = check_call([CLANG_CC, '--version'], stdout=PIPE)
  m = re.search(r'[Vv]ersion\s+(\d+\.\d+)', proc.stdout)
  return m and m.group(1)


def check_llvm_version():
  actual = get_clang_version()
  if actual.startswith('%d.' % EXPECTED_LLVM_VERSION):
    return True
  # When running in CI environment we also silently allow the next major
  # version of LLVM here so that new versions of LLVM can be rolled in
  # without disruption.
  if 'BUILDBOT_BUILDNUMBER' in os.environ:
    if actual.startswith('%d.' % (EXPECTED_LLVM_VERSION + 1)):
      return True
  diagnostics.warning('version-check', 'LLVM version for clang executable "%s" appears incorrect (seeing "%s", expected "%s")', CLANG_CC, actual, EXPECTED_LLVM_VERSION)
  return False


def get_clang_targets():
  if not os.path.exists(CLANG_CC):
    exit_with_error('clang executable not found at `%s`' % CLANG_CC)
  try:
    target_info = run_process([CLANG_CC, '-print-targets'], stdout=PIPE).stdout
  except subprocess.CalledProcessError:
    exit_with_error('error running `clang -print-targets`.  Check your llvm installation (%s)' % CLANG_CC)
  if 'Registered Targets:' not in target_info:
    exit_with_error('error parsing output of `clang -print-targets`.  Check your llvm installation (%s)' % CLANG_CC)
  return target_info.split('Registered Targets:')[1]


def check_llvm():
  targets = get_clang_targets()
  if 'wasm32' not in targets:
    logger.critical('LLVM has not been built with the WebAssembly backend, clang reports:')
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


def _get_node_version_pair(nodejs):
  actual = run_process(nodejs + ['--version'], stdout=PIPE).stdout.strip()
  version = actual.replace('v', '')
  version = version.split('-')[0].split('.')
  version = tuple(int(v) for v in version)
  return actual, version


def get_node_version(nodejs):
  return _get_node_version_pair(nodejs)[1]


@memoize
def check_node_version():
  try:
    actual, version = _get_node_version_pair(config.NODE_JS)
  except Exception as e:
    diagnostics.warning('version-check', 'cannot check node version: %s', e)
    return

  if version < MINIMUM_NODE_VERSION:
    expected = '.'.join(str(v) for v in MINIMUM_NODE_VERSION)
    diagnostics.warning('version-check', f'node version appears too old (seeing "{actual}", expected "v{expected}")')

  return version


def node_bigint_flags(nodejs):
  node_version = get_node_version(nodejs)
  # wasm bigint was enabled by default in node v16.
  if node_version and node_version < (16, 0, 0):
    return ['--experimental-wasm-bigint']
  else:
    return []


def node_reference_types_flags(nodejs):
  node_version = get_node_version(nodejs)
  # reference types were enabled by default in node v18.
  if node_version and node_version < (18, 0, 0):
    return ['--experimental-wasm-reftypes']
  else:
    return []


def node_memory64_flags():
  return ['--experimental-wasm-memory64']


def node_exception_flags(nodejs):
  node_version = get_node_version(nodejs)
  # Exception handling was enabled by default in node v17.
  if node_version and node_version < (17, 0, 0):
    return ['--experimental-wasm-eh']
  else:
    return []


def node_pthread_flags(nodejs):
  node_version = get_node_version(nodejs)
  # bulk memory and wasm threads were enabled by default in node v16.
  if node_version and node_version < (16, 0, 0):
    return ['--experimental-wasm-bulk-memory', '--experimental-wasm-threads']
  else:
    return []


@memoize
@ToolchainProfiler.profile()
def check_node():
  try:
    run_process(config.NODE_JS + ['-e', 'console.log("hello")'], stdout=PIPE)
  except Exception as e:
    exit_with_error('the configured node executable (%s) does not seem to work, check the paths in %s (%s)', config.NODE_JS, config.EM_CONFIG, str(e))


def generate_sanity():
  return f'{utils.EMSCRIPTEN_VERSION}|{config.LLVM_ROOT}\n'


@memoize
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

  check_node()

  with ToolchainProfiler.profile_block('sanity LLVM'):
    for cmd in (CLANG_CC, LLVM_AR):
      if not os.path.exists(cmd) and not os.path.exists(cmd + '.exe'):  # .exe extension required for Windows
        exit_with_error('cannot find %s, check the paths in %s', cmd, config.EM_CONFIG)


@ToolchainProfiler.profile()
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

  # In DEBUG mode we perform the sanity checks even when
  # early return due to the file being up-to-date.
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

  sanity_file = cache.get_path('sanity.txt')

  def sanity_is_correct():
    sanity_data = None
    # We can't simply check for the existence of sanity_file and then read from
    # it here because we don't hold the cache lock yet and some other process
    # could clear the cache between checking for, and reading from, the file.
    try:
      sanity_data = utils.read_file(sanity_file)
    except Exception:
      pass
    if sanity_data == expected:
      logger.debug(f'sanity file up-to-date: {sanity_file}')
      # Even if the sanity file is up-to-date we still run the checks
      # when force is set.
      if force:
        perform_sanity_checks()
      return True # all is well
    return False

  if sanity_is_correct():
    # Early return without taking the cache lock
    return

  with cache.lock('sanity'):
    # Check again once the cache lock as aquired
    if sanity_is_correct():
      return

    if os.path.exists(sanity_file):
      sanity_data = utils.read_file(sanity_file)
      logger.info('old sanity: %s', sanity_data.strip())
      logger.info('new sanity: %s', expected.strip())
      logger.info('(Emscripten: config changed, clearing cache)')
      cache.erase()
    else:
      logger.debug(f'sanity file not found: {sanity_file}')

    perform_sanity_checks()

    # Only create/update this file if the sanity check succeeded, i.e., we got here
    utils.write_file(sanity_file, expected)


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
@memoize
def get_emscripten_temp_dir():
  """Returns a path to EMSCRIPTEN_TEMP_DIR, creating one if it didn't exist."""
  global EMSCRIPTEN_TEMP_DIR
  if not EMSCRIPTEN_TEMP_DIR:
    EMSCRIPTEN_TEMP_DIR = tempfile.mkdtemp(prefix='emscripten_temp_', dir=TEMP_DIR)

    if not DEBUG_SAVE:
      def prepare_to_clean_temp(d):
        def clean_temp():
          utils.delete_dir(d)

        atexit.register(clean_temp)
      # this global var might change later
      prepare_to_clean_temp(EMSCRIPTEN_TEMP_DIR)
  return EMSCRIPTEN_TEMP_DIR


def in_temp(name):
  return os.path.join(get_emscripten_temp_dir(), os.path.basename(name))


def get_canonical_temp_dir(temp_dir):
  return os.path.join(temp_dir, 'emscripten_temp')


def setup_temp_dirs():
  global EMSCRIPTEN_TEMP_DIR, CANONICAL_TEMP_DIR, TEMP_DIR
  EMSCRIPTEN_TEMP_DIR = None

  TEMP_DIR = os.environ.get("EMCC_TEMP_DIR", tempfile.gettempdir())
  if not os.path.isdir(TEMP_DIR):
    exit_with_error(f'The temporary directory `{TEMP_DIR}` does not exist! Please make sure that the path is correct.')

  CANONICAL_TEMP_DIR = get_canonical_temp_dir(TEMP_DIR)

  if DEBUG:
    EMSCRIPTEN_TEMP_DIR = CANONICAL_TEMP_DIR
    try:
      safe_ensure_dirs(EMSCRIPTEN_TEMP_DIR)
    except Exception as e:
      exit_with_error(str(e) + f'Could not create canonical temp dir. Check definition of TEMP_DIR in {config.EM_CONFIG}')

    # Since the canonical temp directory is, by definition, the same
    # between all processes that run in DEBUG mode we need to use a multi
    # process lock to prevent more than one process from writing to it.
    # This is because emcc assumes that it can use non-unique names inside
    # the temp directory.
    # Sadly we need to allow child processes to access this directory
    # though, since emcc can recursively call itself when building
    # libraries and ports.
    if 'EM_HAVE_TEMP_DIR_LOCK' not in os.environ:
      filelock_name = os.path.join(EMSCRIPTEN_TEMP_DIR, 'emscripten.lock')
      lock = filelock.FileLock(filelock_name)
      os.environ['EM_HAVE_TEMP_DIR_LOCK'] = '1'
      lock.acquire()
      atexit.register(lock.release)


@memoize
def get_temp_files():
  if DEBUG_SAVE:
    # In debug mode store all temp files in the emscripten-specific temp dir
    # and don't worry about cleaning them up.
    return tempfiles.TempFiles(get_emscripten_temp_dir(), save_debug_files=True)
  else:
    # Otherwise use the system tempdir and try to clean up after ourselves.
    return tempfiles.TempFiles(TEMP_DIR, save_debug_files=False)


def target_environment_may_be(environment):
  return not settings.ENVIRONMENT or environment in settings.ENVIRONMENT.split(',')


def print_compiler_stage(cmd):
  """Emulate the '-v/-###' flags of clang/gcc by printing the sub-commands
  that we run."""

  def maybe_quote(arg):
    if all(c.isalnum() or c in './-_' for c in arg):
      return arg
    else:
      return f'"{arg}"'

  if SKIP_SUBPROCS:
    print(' ' + ' '.join([maybe_quote(a) for a in cmd]), file=sys.stderr)
    sys.stderr.flush()
  elif PRINT_SUBPROCS:
    print(' %s %s' % (maybe_quote(cmd[0]), shlex_join(cmd[1:])), file=sys.stderr)
    sys.stderr.flush()


def demangle_c_symbol_name(name):
  if not is_c_symbol(name):
    return '$' + name
  return name[1:] if name.startswith('_') else name


def is_c_symbol(name):
  return name.startswith('_')


def treat_as_user_export(name):
  return not name.startswith('dynCall_')


def asmjs_mangle(name):
  """Mangle a name the way asm.js/JSBackend globals are mangled.

  Prepends '_' and replaces non-alphanumerics with '_'.
  Used by wasm backend for JS library consistency with asm.js.
  """
  # We also use this function to convert the clang-mangled `__main_argc_argv`
  # to simply `main` which is expected by the emscripten JS glue code.
  if name == '__main_argc_argv':
    name = 'main'
  if treat_as_user_export(name):
    return '_' + name
  return name


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


def get_file_suffix(filename):
  """Parses the essential suffix of a filename, discarding Unix-style version
  numbers in the name. For example for 'libz.so.1.2.8' returns '.so'"""
  while filename:
    filename, suffix = os.path.splitext(filename)
    if not suffix[1:].isdigit():
      return suffix
  return ''


def make_writable(filename):
  assert os.path.isfile(filename)
  old_mode = stat.S_IMODE(os.stat(filename).st_mode)
  os.chmod(filename, old_mode | stat.S_IWUSR)


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
  # We always want the target file to be writable even when copying from
  # read-only source. (e.g. a read-only install of emscripten).
  make_writable(dst)


def read_and_preprocess(filename, expand_macros=False):
  temp_dir = get_emscripten_temp_dir()
  # Create a settings file with the current settings to pass to the JS preprocessor

  settings_str = ''
  for key, value in settings.external_dict().items():
    assert key == key.upper()  # should only ever be uppercase keys in settings
    jsoned = json.dumps(value, sort_keys=True)
    settings_str += f'var {key} = {jsoned};\n'

  settings_file = os.path.join(temp_dir, 'settings.js')
  utils.write_file(settings_file, settings_str)

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

  run_js_tool(path_from_root('tools/preprocessor.mjs'), args, stdout=open(stdout, 'w'), cwd=dirname)
  out = utils.read_file(stdout)

  return out


def do_replace(input_, pattern, replacement):
  if pattern not in input_:
    exit_with_error('expected to find pattern in input JS: %s' % pattern)
  return input_.replace(pattern, replacement)


def get_llvm_target():
  if settings.MEMORY64:
    return 'wasm64-unknown-emscripten'
  else:
    return 'wasm32-unknown-emscripten'


def init():
  set_version_globals()
  setup_temp_dirs()


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


# ============================================================================
# End declarations.
# ============================================================================

# Everything below this point is top level code that get run when importing this
# file.  TODO(sbc): We should try to reduce that amount we do here and instead
# have consumers explicitly call initialization functions.

CLANG_CC = os.path.expanduser(build_clang_tool_path(exe_suffix('clang')))
CLANG_CXX = os.path.expanduser(build_clang_tool_path(exe_suffix('clang++')))
LLVM_AR = build_llvm_tool_path(exe_suffix('llvm-ar'))
LLVM_DWP = build_llvm_tool_path(exe_suffix('llvm-dwp'))
LLVM_RANLIB = build_llvm_tool_path(exe_suffix('llvm-ranlib'))
LLVM_NM = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-nm')))
LLVM_DWARFDUMP = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-dwarfdump')))
LLVM_OBJCOPY = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-objcopy')))
LLVM_STRIP = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-strip')))
WASM_LD = os.path.expanduser(build_llvm_tool_path(exe_suffix('wasm-ld')))

EMCC = bat_suffix(path_from_root('emcc'))
EMXX = bat_suffix(path_from_root('em++'))
EMAR = bat_suffix(path_from_root('emar'))
EMRANLIB = bat_suffix(path_from_root('emranlib'))
EMCMAKE = bat_suffix(path_from_root('emcmake'))
EMCONFIGURE = bat_suffix(path_from_root('emconfigure'))
EM_NM = bat_suffix(path_from_root('emnm'))
FILE_PACKAGER = bat_suffix(path_from_root('tools/file_packager'))
WASM_SOURCEMAP = bat_suffix(path_from_root('tools/wasm-sourcemap'))
# Windows .dll suffix is not included in this list, since those are never
# linked to directly on the command line.
DYNAMICLIB_ENDINGS = ['.dylib', '.so']
STATICLIB_ENDINGS = ['.a']

run_via_emxx = False

init()
