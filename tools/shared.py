# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function

from subprocess import PIPE
import atexit
import binascii
import base64
import difflib
import json
import logging
import math
import os
import re
import shutil
import subprocess
import time
import sys
import tempfile

if sys.version_info < (3, 5):
  print('error: emscripten requires python 3.5 or above', file=sys.stderr)
  sys.exit(1)

from .toolchain_profiler import ToolchainProfiler
from .tempfiles import try_delete
from . import cache, tempfiles, colored_logger
from . import diagnostics


__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
WINDOWS = sys.platform.startswith('win')
MACOS = sys.platform == 'darwin'
LINUX = sys.platform.startswith('linux')
DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))
EXPECTED_NODE_VERSION = (4, 1, 1)
EXPECTED_BINARYEN_VERSION = 95
EXPECTED_LLVM_VERSION = "12.0"
SIMD_FEATURE_TOWER = ['-msse', '-msse2', '-msse3', '-mssse3', '-msse4.1', '-msse4.2', '-mavx']

# can add  %(asctime)s  to see timestamps
logging.basicConfig(format='%(name)s:%(levelname)s: %(message)s',
                    level=logging.DEBUG if DEBUG else logging.INFO)
colored_logger.enable()
logger = logging.getLogger('shared')

if sys.version_info < (2, 7, 12):
  logger.debug('python versions older than 2.7.12 are known to run into outdated SSL certificate related issues, https://github.com/emscripten-core/emscripten/issues/6275')

# warning about absolute-paths is disabled by default, and not enabled by -Wall
diagnostics.add_warning('absolute-paths', enabled=False, part_of_all=False)
diagnostics.add_warning('separate-asm')
diagnostics.add_warning('almost-asm')
diagnostics.add_warning('invalid-input')
# Don't show legacy settings warnings by default
diagnostics.add_warning('legacy-settings', enabled=False, part_of_all=False)
# Catch-all for other emcc warnings
diagnostics.add_warning('linkflags')
diagnostics.add_warning('emcc')
diagnostics.add_warning('undefined', error=True)
diagnostics.add_warning('deprecated')
diagnostics.add_warning('version-check')
diagnostics.add_warning('export-main')
diagnostics.add_warning('unused-command-line-argument', shared=True)


def exit_with_error(msg, *args):
  diagnostics.error(msg, *args)


# On Windows python suffers from a particularly nasty bug if python is spawning
# new processes while python itself is spawned from some other non-console
# process.
# Use a custom replacement for Popen on Windows to avoid the "WindowsError:
# [Error 6] The handle is invalid" errors when emcc is driven through cmake or
# mingw32-make.
# See http://bugs.python.org/issue3905
class WindowsPopen(object):
  def __init__(self, args, bufsize=0, executable=None, stdin=None, stdout=None, stderr=None, preexec_fn=None, close_fds=False,
               shell=False, cwd=None, env=None, universal_newlines=False, startupinfo=None, creationflags=0):
    self.stdin = stdin
    self.stdout = stdout
    self.stderr = stderr

    # (stdin, stdout, stderr) store what the caller originally wanted to be done with the streams.
    # (stdin_, stdout_, stderr_) will store the fixed set of streams that workaround the bug.
    self.stdin_ = stdin
    self.stdout_ = stdout
    self.stderr_ = stderr

    # If the caller wants one of these PIPEd, we must PIPE them all to avoid the 'handle is invalid' bug.
    if self.stdin_ == PIPE or self.stdout_ == PIPE or self.stderr_ == PIPE:
      if self.stdin_ is None:
        self.stdin_ = PIPE
      if self.stdout_ is None:
        self.stdout_ = PIPE
      if self.stderr_ is None:
        self.stderr_ = PIPE

    try:
      # Call the process with fixed streams.
      self.process = subprocess.Popen(args, bufsize, executable, self.stdin_, self.stdout_, self.stderr_, preexec_fn, close_fds, shell, cwd, env, universal_newlines, startupinfo, creationflags)
      self.pid = self.process.pid
    except Exception as e:
      logger.error('\nsubprocess.Popen(args=%s) failed! Exception %s\n' % (shlex_join(args), str(e)))
      raise

  def communicate(self, input=None):
    output = self.process.communicate(input)
    self.returncode = self.process.returncode

    # If caller never wanted to PIPE stdout or stderr, route the output back to screen to avoid swallowing output.
    if self.stdout is None and self.stdout_ == PIPE and len(output[0].strip()):
      print(output[0], file=sys.stdout)
    if self.stderr is None and self.stderr_ == PIPE and len(output[1].strip()):
      print(output[1], file=sys.stderr)

    # Return a mock object to the caller. This works as long as all emscripten code immediately .communicate()s the result, and doesn't
    # leave the process object around for longer/more exotic uses.
    if self.stdout is None and self.stderr is None:
      return (None, None)
    if self.stdout is None:
      return (None, output[1])
    if self.stderr is None:
      return (output[0], None)
    return (output[0], output[1])

  def poll(self):
    return self.process.poll()

  def kill(self):
    return self.process.kill()


def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)


def root_is_writable():
  return os.access(__rootpath__, os.W_OK)


# Switch to shlex.quote once we can depend on python 3
def shlex_quote(arg):
  if ' ' in arg and (not (arg.startswith('"') and arg.endswith('"'))) and (not (arg.startswith("'") and arg.endswith("'"))):
    return '"' + arg.replace('"', '\\"') + '"'

  return arg


# Switch to shlex.join once we can depend on python 3.8:
# https://docs.python.org/3/library/shlex.html#shlex.join
def shlex_join(cmd):
  return ' '.join(shlex_quote(x) for x in cmd)


# This is a workaround for https://bugs.python.org/issue9400
class Py2CalledProcessError(subprocess.CalledProcessError):
  def __init__(self, returncode, cmd, output=None, stderr=None):
    super(Exception, self).__init__(returncode, cmd, output, stderr)
    self.returncode = returncode
    self.cmd = cmd
    self.output = output
    self.stderr = stderr


# https://docs.python.org/3/library/subprocess.html#subprocess.CompletedProcess
class Py2CompletedProcess:
  def __init__(self, args, returncode, stdout, stderr):
    self.args = args
    self.returncode = returncode
    self.stdout = stdout
    self.stderr = stderr

  def __repr__(self):
    _repr = ['args=%s' % repr(self.args), 'returncode=%s' % self.returncode]
    if self.stdout is not None:
      _repr.append('stdout=' + repr(self.stdout))
    if self.stderr is not None:
      _repr.append('stderr=' + repr(self.stderr))
    return 'CompletedProcess(%s)' % ', '.join(_repr)

  def check_returncode(self):
    if self.returncode != 0:
      raise Py2CalledProcessError(returncode=self.returncode, cmd=self.args, output=self.stdout, stderr=self.stderr)


def run_process(cmd, check=True, input=None, *args, **kw):
  """Runs a subpocess returning the exit code.

  By default this function will raise an exception on failure.  Therefor this should only be
  used if you want to handle such failures.  For most subprocesses, failures are not recoverable
  and should be fatal.  In those cases the `check_call` wrapper should be preferred.
  """

  kw.setdefault('universal_newlines', True)

  debug_text = '%sexecuted %s' % ('successfully ' if check else '', shlex_join(cmd))

  if hasattr(subprocess, "run"):
    ret = subprocess.run(cmd, check=check, input=input, *args, **kw)
    logger.debug(debug_text)
    return ret

  # Python 2 compatibility: Introduce Python 3 subprocess.run-like behavior
  if input is not None:
    kw['stdin'] = subprocess.PIPE
  proc = Popen(cmd, *args, **kw)
  stdout, stderr = proc.communicate(input)
  result = Py2CompletedProcess(cmd, proc.returncode, stdout, stderr)
  if check:
    result.check_returncode()
  logger.debug(debug_text)
  return result


def check_call(cmd, *args, **kw):
  """Like `run_process` above but treat failures as fatal and exit_with_error."""
  try:
    return run_process(cmd, *args, **kw)
  except subprocess.CalledProcessError as e:
    exit_with_error("'%s' failed (%d)", shlex_join(cmd), e.returncode)
  except OSError as e:
    exit_with_error("'%s' failed: %s", shlex_join(cmd), str(e))


def run_js_tool(filename, jsargs=[], *args, **kw):
  """Execute a javascript tool.

  This is used by emcc to run parts of the build process that are written
  implemented in javascript.
  """
  command = NODE_JS + [filename] + jsargs
  print_compiler_stage(command)
  return check_call(command, *args, **kw).stdout


# Finds the given executable 'program' in PATH. Operates like the Unix tool 'which'.
def which(program):
  def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

  if os.path.isabs(program):
    if os.path.isfile(program):
      return program

    if WINDOWS:
      for suffix in ['.exe', '.cmd', '.bat']:
        if is_exe(program + suffix):
          return program + suffix

  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.environ["PATH"].split(os.pathsep):
      path = path.strip('"')
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
      if WINDOWS:
        for suffix in ('.exe', '.cmd', '.bat'):
          if is_exe(exe_file + suffix):
            return exe_file + suffix

  return None


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


def generate_config(path, first_time=False):
  # Note: repr is used to ensure the paths are escaped correctly on Windows.
  # The full string is replaced so that the template stays valid Python.
  config_file = open(path_from_root('tools', 'settings_template.py')).read().splitlines()
  config_file = config_file[3:] # remove the initial comment
  config_file = '\n'.join(config_file)
  # autodetect some default paths
  config_file = config_file.replace('\'{{{ EMSCRIPTEN_ROOT }}}\'', repr(EMSCRIPTEN_ROOT))
  llvm_root = os.path.dirname(which('llvm-dis') or '/usr/bin/llvm-dis')
  config_file = config_file.replace('\'{{{ LLVM_ROOT }}}\'', repr(llvm_root))

  node = which('nodejs') or which('node') or 'node'
  config_file = config_file.replace('\'{{{ NODE }}}\'', repr(node))

  abspath = os.path.abspath(os.path.expanduser(path))
  # write
  with open(abspath, 'w') as f:
    f.write(config_file)

  if first_time:
    print('''
==============================================================================
Welcome to Emscripten!

This is the first time any of the Emscripten tools has been run.

A settings file has been copied to %s, at absolute path: %s

It contains our best guesses for the important paths, which are:

  LLVM_ROOT       = %s
  NODE_JS         = %s
  EMSCRIPTEN_ROOT = %s

Please edit the file if any of those are incorrect.

This command will now exit. When you are done editing those paths, re-run it.
==============================================================================
''' % (path, abspath, llvm_root, node, EMSCRIPTEN_ROOT), file=sys.stderr)


def parse_config_file():
  """Parse the emscripten config file using python's exec.

  Also also EM_<KEY> environment variables to override specific config keys.
  """
  config = {}
  config_text = open(CONFIG_FILE, 'r').read() if CONFIG_FILE else EM_CONFIG
  try:
    exec(config_text, config)
  except Exception as e:
    exit_with_error('Error in evaluating %s (at %s): %s, text: %s', EM_CONFIG, CONFIG_FILE, str(e), config_text)

  CONFIG_KEYS = (
    'NODE_JS',
    'BINARYEN_ROOT',
    'POPEN_WORKAROUND',
    'SPIDERMONKEY_ENGINE',
    'EMSCRIPTEN_NATIVE_OPTIMIZER',
    'V8_ENGINE',
    'LLVM_ROOT',
    'LLVM_ADD_VERSION',
    'CLANG_ADD_VERSION',
    'CLOSURE_COMPILER',
    'JAVA',
    'JS_ENGINE',
    'JS_ENGINES',
    'WASMER',
    'WASMTIME',
    'WASM_ENGINES',
    'FROZEN_CACHE',
    'CACHE',
    'PORTS',
  )

  # Only propagate certain settings from the config file.
  for key in CONFIG_KEYS:
    env_var = 'EM_' + key
    env_value = os.environ.get(env_var)
    if env_value is not None:
      globals()[key] = env_value
    elif key in config:
      globals()[key] = config[key]

  # Certain keys are mandatory
  for key in ('LLVM_ROOT', 'NODE_JS', 'BINARYEN_ROOT'):
    if key not in config:
      exit_with_error('%s is not defined in %s', key, config_file_location())
    if not globals()[key]:
      exit_with_error('%s is set to empty value in %s', key, config_file_location())

  if not NODE_JS:
    exit_with_error('NODE_JS is not defined in %s', config_file_location())


def listify(x):
  if type(x) is not list:
    return [x]
  return x


def fix_js_engine(old, new):
  global JS_ENGINES
  if old is None:
    return
  JS_ENGINES = [new if x == old else x for x in JS_ENGINES]
  return new


def normalize_config_settings():
  global CACHE, PORTS, JAVA, CLOSURE_COMPILER
  global NODE_JS, V8_ENGINE, JS_ENGINE, JS_ENGINES, SPIDERMONKEY_ENGINE, WASM_ENGINES

  # EM_CONFIG stuff
  if not JS_ENGINES:
    JS_ENGINES = [NODE_JS]
  if not JS_ENGINE:
    JS_ENGINE = JS_ENGINES[0]

  # Engine tweaks
  if SPIDERMONKEY_ENGINE:
    new_spidermonkey = SPIDERMONKEY_ENGINE
    if '-w' not in str(new_spidermonkey):
      new_spidermonkey += ['-w']
    SPIDERMONKEY_ENGINE = fix_js_engine(SPIDERMONKEY_ENGINE, new_spidermonkey)
  NODE_JS = fix_js_engine(NODE_JS, listify(NODE_JS))
  V8_ENGINE = fix_js_engine(V8_ENGINE, listify(V8_ENGINE))
  JS_ENGINE = fix_js_engine(JS_ENGINE, listify(JS_ENGINE))
  JS_ENGINES = [listify(engine) for engine in JS_ENGINES]
  WASM_ENGINES = [listify(engine) for engine in WASM_ENGINES]
  if not CACHE:
    if root_is_writable():
      CACHE = path_from_root('cache')
    else:
      # Use the legacy method of putting the cache in the user's home directory
      # if the emscripten root is not writable.
      # This is useful mostly for read-only installation and perhaps could
      # be removed in the future since such installations should probably be
      # setting a specific cache location.
      logger.debug('Using home-directory for emscripten cache due to read-only root')
      CACHE = os.path.expanduser(os.path.join('~', '.emscripten_cache'))
  if not PORTS:
    PORTS = os.path.join(CACHE, 'ports')

  if CLOSURE_COMPILER is None:
    if WINDOWS:
      CLOSURE_COMPILER = [path_from_root('node_modules', '.bin', 'google-closure-compiler.cmd')]
    else:
      # Work around an issue that Closure compiler can take up a lot of memory and crash in an error
     # "FATAL ERROR: Ineffective mark-compacts near heap limit Allocation failed - JavaScript heap out of memory"
      CLOSURE_COMPILER = NODE_JS + ['--max_old_space_size=8192', path_from_root('node_modules', '.bin', 'google-closure-compiler')]

  if JAVA is None:
    logger.debug('JAVA not defined in ' + config_file_location() + ', using "java"')
    JAVA = 'java'


# Returns the location of the emscripten config file.
def config_file_location():
  # Handle the case where there is no config file at all (i.e. If EM_CONFIG is passed as python code
  # direclty on the command line).
  if not CONFIG_FILE:
    return '<inline config>'

  return CONFIG_FILE


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
  diagnostics.warning('version-check', 'LLVM version appears incorrect (seeing "%s", expected "%s")', actual, EXPECTED_LLVM_VERSION)
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
  return os.path.dirname(NODE_JS[0] if type(NODE_JS) is list else NODE_JS)


# When we run some tools from npm (closure, html-minifier-terser), those
# expect that the tools have node.js accessible in PATH. Place our node
# there when invoking those tools.
def env_with_node_in_path():
  env = os.environ.copy()
  env['PATH'] = get_node_directory() + os.pathsep + env['PATH']
  return env


def check_node_version():
  try:
    actual = run_process(NODE_JS + ['--version'], stdout=PIPE).stdout.strip()
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
    EMSCRIPTEN_VERSION = f.read().strip().replace('"', '')
  parts = [int(x) for x in EMSCRIPTEN_VERSION.split('.')]
  EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY = parts


def generate_sanity():
  sanity_file_content = EMSCRIPTEN_VERSION + '|' + LLVM_ROOT + '|' + get_clang_version()
  if CONFIG_FILE:
    config = open(CONFIG_FILE).read()
  else:
    config = EM_CONFIG
  # Convert to unsigned for python2 and python3 compat
  checksum = binascii.crc32(config.encode()) & 0xffffffff
  sanity_file_content += '|%#x\n' % checksum
  return sanity_file_content


def perform_sanify_checks():
  logger.info('(Emscripten: Running sanity checks)')

  with ToolchainProfiler.profile_block('sanity compiler_engine'):
    try:
      run_process(NODE_JS + ['-e', 'console.log("hello")'], stdout=PIPE)
    except Exception as e:
      exit_with_error('The configured node executable (%s) does not seem to work, check the paths in %s (%s)', NODE_JS, config_file_location, str(e))

  with ToolchainProfiler.profile_block('sanity LLVM'):
    for cmd in [CLANG_CC, LLVM_AR, LLVM_AS, LLVM_NM]:
      if not os.path.exists(cmd) and not os.path.exists(cmd + '.exe'):  # .exe extension required for Windows
        exit_with_error('Cannot find %s, check the paths in %s', cmd, EM_CONFIG)


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
  with ToolchainProfiler.profile_block('sanity'):
    check_llvm_version()
    if not CONFIG_FILE:
      return # config stored directly in EM_CONFIG => skip sanity checks
    expected = generate_sanity()

    sanity_file = Cache.get_path('sanity.txt', root=True)
    if os.path.exists(sanity_file):
      sanity_data = open(sanity_file).read()
      if sanity_data != expected:
        logger.debug('old sanity: %s' % sanity_data)
        logger.debug('new sanity: %s' % expected)
        if FROZEN_CACHE:
          logger.info('(Emscripten: config changed, cache may need to be cleared, but FROZEN_CACHE is set)')
        else:
          logger.info('(Emscripten: config changed, clearing cache)')
          Cache.erase()
          # the check actually failed, so definitely write out the sanity file, to
          # avoid others later seeing failures too
          force = False
      elif not force:
        return # all is well

    # some warning, mostly not fatal checks - do them even if EM_IGNORE_SANITY is on
    check_node_version()

    llvm_ok = check_llvm()

    if os.environ.get('EM_IGNORE_SANITY'):
      logger.info('EM_IGNORE_SANITY set, ignoring sanity checks')
      return

    if not llvm_ok:
      exit_with_error('failing sanity checks due to previous llvm failure')

    perform_sanify_checks()

    if not force:
      # Only create/update this file if the sanity check succeeded, i.e., we got here
      Cache.ensure()
      with open(sanity_file, 'w') as f:
        f.write(expected)


# Some distributions ship with multiple llvm versions so they add
# the version to the binaries, cope with that
def build_llvm_tool_path(tool):
  if LLVM_ADD_VERSION:
    return os.path.join(LLVM_ROOT, tool + "-" + LLVM_ADD_VERSION)
  else:
    return os.path.join(LLVM_ROOT, tool)


# Some distributions ship with multiple clang versions so they add
# the version to the binaries, cope with that
def build_clang_tool_path(tool):
  if CLANG_ADD_VERSION:
    return os.path.join(LLVM_ROOT, tool + "-" + CLANG_ADD_VERSION)
  else:
    return os.path.join(LLVM_ROOT, tool)


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
  return replace_suffix(filename, new_suffix) if Settings.MINIMAL_RUNTIME else filename + new_suffix


def safe_ensure_dirs(dirname):
  try:
    os.makedirs(dirname)
  except OSError:
    # Python 2 compatibility: makedirs does not support exist_ok parameter
    # Ignore error for already existing dirname as exist_ok does
    if not os.path.isdir(dirname):
      raise


# Temp dir. Create a random one, unless EMCC_DEBUG is set, in which case use the canonical
# temp directory (TEMP_DIR/emscripten_temp).
def get_emscripten_temp_dir():
  """Returns a path to EMSCRIPTEN_TEMP_DIR, creating one if it didn't exist."""
  global configuration, EMSCRIPTEN_TEMP_DIR
  if not EMSCRIPTEN_TEMP_DIR:
    EMSCRIPTEN_TEMP_DIR = tempfile.mkdtemp(prefix='emscripten_temp_', dir=configuration.TEMP_DIR)

    def prepare_to_clean_temp(d):
      def clean_temp():
        try_delete(d)

      atexit.register(clean_temp)
    prepare_to_clean_temp(EMSCRIPTEN_TEMP_DIR) # this global var might change later
  return EMSCRIPTEN_TEMP_DIR


def get_canonical_temp_dir(temp_dir):
  return os.path.join(temp_dir, 'emscripten_temp')


class Configuration(object):
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
        exit_with_error(str(e) + 'Could not create canonical temp dir. Check definition of TEMP_DIR in ' + config_file_location())

  def get_temp_files(self):
    return tempfiles.TempFiles(
      tmp=self.TEMP_DIR if not DEBUG else get_emscripten_temp_dir(),
      save_debug_files=os.environ.get('EMCC_DEBUG_SAVE'))


def apply_configuration():
  global configuration, EMSCRIPTEN_TEMP_DIR, CANONICAL_TEMP_DIR, TEMP_DIR
  configuration = Configuration()
  EMSCRIPTEN_TEMP_DIR = configuration.EMSCRIPTEN_TEMP_DIR
  CANONICAL_TEMP_DIR = configuration.CANONICAL_TEMP_DIR
  TEMP_DIR = configuration.TEMP_DIR


def get_llvm_target():
  return LLVM_TARGET


def emsdk_ldflags(user_args):
  if os.environ.get('EMMAKEN_NO_SDK'):
    return []

  library_paths = [
      path_from_root('system', 'local', 'lib'),
      path_from_root('system', 'lib'),
      Cache.dirname
  ]
  ldflags = ['-L' + l for l in library_paths]

  if '-nostdlib' in user_args:
    return ldflags

  # TODO(sbc): Add system libraries here rather than conditionally including
  # them via .symbols files.
  libraries = []
  ldflags += ['-l' + l for l in libraries]

  return ldflags


def emsdk_cflags(user_args, cxx):
  # Disable system C and C++ include directories, and add our own (using
  # -isystem so they are last, like system dirs, which allows projects to
  # override them)

  c_opts = ['-Xclang', '-nostdsysteminc']

  c_include_paths = [
    path_from_root('system', 'include', 'compat'),
    path_from_root('system', 'include'),
    path_from_root('system', 'include', 'libc'),
    path_from_root('system', 'lib', 'libc', 'musl', 'arch', 'emscripten'),
    path_from_root('system', 'local', 'include'),
    path_from_root('system', 'include', 'SSE'),
    path_from_root('system', 'lib', 'compiler-rt', 'include'),
    path_from_root('system', 'lib', 'libunwind', 'include'),
    Cache.get_path('include')
  ]

  cxx_include_paths = [
    path_from_root('system', 'include', 'libcxx'),
    path_from_root('system', 'lib', 'libcxxabi', 'include'),
  ]

  def include_directive(paths):
    result = []
    for path in paths:
      result += ['-Xclang', '-isystem' + path]
    return result

  def array_contains_any_of(hay, needles):
    for n in needles:
      if n in hay:
        return True

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER):
    if '-msimd128' not in user_args:
      exit_with_error('Passing any of ' + ', '.join(SIMD_FEATURE_TOWER) + ' flags also requires passing -msimd128!')
    c_opts += ['-D__SSE__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[1:]):
    c_opts += ['-D__SSE2__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[2:]):
    c_opts += ['-D__SSE3__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[3:]):
    c_opts += ['-D__SSSE3__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[4:]):
    c_opts += ['-D__SSE4_1__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[5:]):
    c_opts += ['-D__SSE4_2__=1']

  if array_contains_any_of(user_args, SIMD_FEATURE_TOWER[6:]):
    c_opts += ['-D__AVX__=1']

  # libcxx include paths must be defined before libc's include paths otherwise libcxx will not build
  if cxx:
    c_opts += include_directive(cxx_include_paths)
  return c_opts + include_directive(c_include_paths)


def get_asmflags(user_args):
  return ['-target', get_llvm_target()]


def get_cflags(user_args, cxx):
  # Set the LIBCPP ABI version to at least 2 so that we get nicely aligned string
  # data and other nice fixes.
  c_opts = [# '-fno-threadsafe-statics', # disabled due to issue 1289
            '-target', get_llvm_target(),
            '-D__EMSCRIPTEN_major__=' + str(EMSCRIPTEN_VERSION_MAJOR),
            '-D__EMSCRIPTEN_minor__=' + str(EMSCRIPTEN_VERSION_MINOR),
            '-D__EMSCRIPTEN_tiny__=' + str(EMSCRIPTEN_VERSION_TINY),
            '-D_LIBCPP_ABI_VERSION=2']

  # For compatability with the fastcomp compiler that defined these
  c_opts += ['-Dunix',
             '-D__unix',
             '-D__unix__']

  # Changes to default clang behavior

  # Implicit functions can cause horribly confusing function pointer type errors, see #2175
  # If your codebase really needs them - very unrecommended! - you can disable the error with
  #   -Wno-error=implicit-function-declaration
  # or disable even a warning about it with
  #   -Wno-implicit-function-declaration
  c_opts += ['-Werror=implicit-function-declaration']

  if os.environ.get('EMMAKEN_NO_SDK') or '-nostdinc' in user_args:
    return c_opts

  return c_opts + emsdk_cflags(user_args, cxx)


# Settings. A global singleton. Not pretty, but nicer than passing |, settings| everywhere
class SettingsManager(object):

  class __impl(object):
    attrs = {}
    internal_settings = set()

    def __init__(self):
      self.reset()

    @classmethod
    def reset(cls):
      cls.attrs = {}

      # Load the JS defaults into python.
      settings = open(path_from_root('src', 'settings.js')).read().replace('//', '#')
      settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
      # Variable TARGET_NOT_SUPPORTED is referenced by value settings.js (also beyond declaring it),
      # so must pass it there explicitly.
      exec(settings, {'attrs': cls.attrs})

      settings = open(path_from_root('src', 'settings_internal.js')).read().replace('//', '#')
      settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
      internal_attrs = {}
      exec(settings, {'attrs': internal_attrs})
      cls.attrs.update(internal_attrs)

      if 'EMCC_STRICT' in os.environ:
        cls.attrs['STRICT'] = int(os.environ.get('EMCC_STRICT'))

      # Special handling for LEGACY_SETTINGS.  See src/setting.js for more
      # details
      cls.legacy_settings = {}
      cls.alt_names = {}
      for legacy in cls.attrs['LEGACY_SETTINGS']:
        if len(legacy) == 2:
          name, new_name = legacy
          cls.legacy_settings[name] = (None, 'setting renamed to ' + new_name)
          cls.alt_names[name] = new_name
          cls.alt_names[new_name] = name
          default_value = cls.attrs[new_name]
        else:
          name, fixed_values, err = legacy
          cls.legacy_settings[name] = (fixed_values, err)
          default_value = fixed_values[0]
        assert name not in cls.attrs, 'legacy setting (%s) cannot also be a regular setting' % name
        if not cls.attrs['STRICT']:
          cls.attrs[name] = default_value

      cls.internal_settings = set(internal_attrs.keys())

    # Transforms the Settings information into emcc-compatible args (-s X=Y, etc.). Basically
    # the reverse of load_settings, except for -Ox which is relevant there but not here
    @classmethod
    def serialize(cls):
      ret = []
      for key, value in cls.attrs.items():
        if key == key.upper():  # this is a hack. all of our settings are ALL_CAPS, python internals are not
          jsoned = json.dumps(value, sort_keys=True)
          ret += ['-s', key + '=' + jsoned]
      return ret

    @classmethod
    def to_dict(cls):
      return cls.attrs.copy()

    @classmethod
    def copy(cls, values):
      cls.attrs = values

    @classmethod
    def apply_opt_level(cls, opt_level, shrink_level=0, noisy=False):
      if opt_level >= 1:
        cls.attrs['ASSERTIONS'] = 0
      if shrink_level >= 2:
        cls.attrs['EVAL_CTORS'] = 1

    def keys(self):
      return self.attrs.keys()

    def __getattr__(self, attr):
      if attr in self.attrs:
        return self.attrs[attr]
      else:
        raise AttributeError("Settings object has no attribute '%s'" % attr)

    def __setattr__(self, attr, value):
      if attr == 'STRICT' and value:
        for a in self.legacy_settings:
          self.attrs.pop(a, None)

      if attr in self.legacy_settings:
        # TODO(sbc): Rather then special case this we should have STRICT turn on the
        # legacy-settings warning below
        if self.attrs['STRICT']:
          exit_with_error('legacy setting used in strict mode: %s', attr)
        fixed_values, error_message = self.legacy_settings[attr]
        if fixed_values and value not in fixed_values:
          exit_with_error('Invalid command line option -s ' + attr + '=' + str(value) + ': ' + error_message)
        diagnostics.warning('legacy-settings', 'use of legacy setting: %s (%s)', attr, error_message)

      if attr in self.alt_names:
        alt_name = self.alt_names[attr]
        self.attrs[alt_name] = value

      if attr not in self.attrs:
        msg = "Attempt to set a non-existent setting: '%s'\n" % attr
        suggestions = difflib.get_close_matches(attr, list(self.attrs.keys()))
        suggestions = [s for s in suggestions if s not in self.legacy_settings]
        suggestions = ', '.join(suggestions)
        if suggestions:
          msg += ' - did you mean one of %s?\n' % suggestions
        msg += " - perhaps a typo in emcc's  -s X=Y  notation?\n"
        msg += ' - (see src/settings.js for valid values)'
        exit_with_error(msg)

      self.attrs[attr] = value

    @classmethod
    def get(cls, key):
      return cls.attrs.get(key)

    @classmethod
    def __getitem__(cls, key):
      return cls.attrs[key]

    @classmethod
    def target_environment_may_be(self, environment):
      return self.attrs['ENVIRONMENT'] == '' or environment in self.attrs['ENVIRONMENT'].split(',')

  __instance = None

  @staticmethod
  def instance():
    if SettingsManager.__instance is None:
      SettingsManager.__instance = SettingsManager.__impl()
    return SettingsManager.__instance

  def __getattr__(self, attr):
    return getattr(self.instance(), attr)

  def __setattr__(self, attr, value):
    return setattr(self.instance(), attr, value)

  def get(self, key):
    return self.instance().get(key)

  def __getitem__(self, key):
    return self.instance()[key]


def verify_settings():
  if Settings.SAFE_HEAP not in [0, 1]:
    exit_with_error('emcc: SAFE_HEAP must be 0 or 1 in fastcomp')

  if Settings.WASM and Settings.EXPORT_FUNCTION_TABLES:
      exit_with_error('emcc: EXPORT_FUNCTION_TABLES incompatible with WASM')

  if not Settings.WASM:
    # When the user requests non-wasm output, we enable wasm2js. that is,
    # we still compile to wasm normally, but we compile the final output
    # to js.
    Settings.WASM = 1
    Settings.WASM2JS = 1
  if Settings.WASM == 2:
    # Requesting both Wasm and Wasm2JS support
    Settings.WASM2JS = 1


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
  if name in Settings.WASM_FUNCTIONS_THAT_ARE_NOT_NAME_MANGLED:
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
  Cache = cache.Cache(CACHE)


# Placeholder strings used for SINGLE_FILE
class FilenameReplacementStrings:
  WASM_TEXT_FILE = '{{{ FILENAME_REPLACEMENT_STRINGS_WASM_TEXT_FILE }}}'
  WASM_BINARY_FILE = '{{{ FILENAME_REPLACEMENT_STRINGS_WASM_BINARY_FILE }}}'
  ASMJS_CODE_FILE = '{{{ FILENAME_REPLACEMENT_STRINGS_ASMJS_CODE_FILE }}}'


class JS(object):
  memory_initializer_pattern = r'/\* memory initializer \*/ allocate\(\[([\d, ]*)\], "i8", ALLOC_NONE, ([\d+\.GLOBAL_BASEHgb]+)\);'
  no_memory_initializer_pattern = r'/\* no memory initializer \*/'

  memory_staticbump_pattern = r'STATICTOP = STATIC_BASE \+ (\d+);'

  global_initializers_pattern = r'/\* global initializers \*/ __ATINIT__.push\((.+)\);'

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
  emscripten_license_regex = '''\/\*\*?(\s*\*?\s*@license)?(\s*\*?\s*Copyright \d+ The Emscripten Authors\s*\*?\s*SPDX-License-Identifier: MIT)+\s*\*\/''' # noqa

  @staticmethod
  def handle_license(js_target):
    # ensure we emit the license if and only if we need to, and exactly once
    with open(js_target) as f:
      js = f.read()
    # first, remove the license as there may be more than once
    processed_js = re.sub(JS.emscripten_license_regex, '', js)
    if Settings.EMIT_EMSCRIPTEN_LICENSE:
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
      data_uri = Settings.SINGLE_FILE
    if data_uri:
      # if the path does not exist, then there is no data to encode
      if not os.path.exists(path):
        return ''
      with open(path, 'rb') as f:
        data = base64.b64encode(f.read())
      return 'data:application/octet-stream;base64,' + asstr(data)
    else:
      return os.path.basename(path)

  @staticmethod
  def make_initializer(sig, settings=None):
    settings = settings or Settings
    if sig == 'i':
      return '0'
    elif sig == 'f':
      return 'Math_fround(0)'
    elif sig == 'j':
      if settings:
        assert settings['WASM'], 'j aka i64 only makes sense in wasm-only mode in binaryen'
      return 'i64(0)'
    else:
      return '+0'

  FLOAT_SIGS = ['f', 'd']

  @staticmethod
  def make_coercion(value, sig, settings=None, ffi_arg=False, ffi_result=False, convert_from=None):
    settings = settings or Settings
    if sig == 'i':
      if convert_from in JS.FLOAT_SIGS:
        value = '(~~' + value + ')'
      return value + '|0'
    if sig in JS.FLOAT_SIGS and convert_from == 'i':
      value = '(' + value + '|0)'
    if sig == 'f':
      if ffi_arg:
        return '+Math_fround(' + value + ')'
      elif ffi_result:
        return 'Math_fround(+(' + value + '))'
      else:
        return 'Math_fround(' + value + ')'
    elif sig == 'd' or sig == 'f':
      return '+' + value
    elif sig == 'j':
      if settings:
        assert settings['WASM'], 'j aka i64 only makes sense in wasm-only mode in binaryen'
      return 'i64(' + value + ')'
    else:
      return value

  @staticmethod
  def legalize_sig(sig):
    # with BigInt support all sigs are legal since we can use i64s.
    if Settings.WASM_BIGINT:
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
    if Settings.WASM_BIGINT:
      return True
    return sig == JS.legalize_sig(sig)

  @staticmethod
  def make_jscall(sig):
    fnargs = ','.join('a' + str(i) for i in range(1, len(sig)))
    args = (',' if fnargs else '') + fnargs
    ret = '''\
function jsCall_%s(index%s) {
    %sfunctionPointers[index](%s);
}''' % (sig, args, 'return ' if sig[0] != 'v' else '', fnargs)
    return ret

  @staticmethod
  def make_dynCall(sig, args):
    # wasm2c and asyncify are not yet compatible with direct wasm table calls
    if Settings.ASYNCIFY or Settings.WASM2C or not JS.is_legal_sig(sig):
      args = ','.join(args)
      if not Settings.MAIN_MODULE and not Settings.SIDE_MODULE:
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
    if Settings.SUPPORT_LONGJMP:
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

  @staticmethod
  def align(x, by):
    while x % by != 0:
      x += 1
    return x

  @staticmethod
  def generate_string_initializer(s):
    if Settings.ASSERTIONS:
      # append checksum of length and content
      crcTable = []
      for i in range(256):
        crc = i
        for bit in range(8):
          crc = (crc >> 1) ^ ((crc & 1) * 0xedb88320)
        crcTable.append(crc)
      crc = 0xffffffff
      n = len(s)
      crc = crcTable[(crc ^ n) & 0xff] ^ (crc >> 8)
      crc = crcTable[(crc ^ (n >> 8)) & 0xff] ^ (crc >> 8)
      for i in s:
        crc = crcTable[(crc ^ i) & 0xff] ^ (crc >> 8)
      for i in range(4):
        s.append((crc >> (8 * i)) & 0xff)
    s = ''.join(map(chr, s))
    s = s.replace('\\', '\\\\').replace("'", "\\'")
    s = s.replace('\n', '\\n').replace('\r', '\\r')

    # Escape the ^Z (= 0x1a = substitute) ASCII character and all characters higher than 7-bit ASCII.
    def escape(x):
      return '\\x{:02x}'.format(ord(x.group()))

    return re.sub('[\x1a\x80-\xff]', escape, s)

  @staticmethod
  def is_dyn_call(func):
    return func.startswith('dynCall_')

  @staticmethod
  def is_function_table(name):
    return name.startswith('FUNCTION_TABLE_')


class WebAssembly(object):
  @staticmethod
  def toLEB(x):
    assert x >= 0, 'TODO: signed'
    ret = []
    while 1:
      byte = x & 127
      x >>= 7
      more = x != 0
      if more:
        byte = byte | 128
      ret.append(byte)
      if not more:
        break
    return bytearray(ret)

  @staticmethod
  def readLEB(buf, offset):
    result = 0
    shift = 0
    while True:
      byte = bytearray(buf[offset:offset + 1])[0]
      offset += 1
      result |= (byte & 0x7f) << shift
      if not (byte & 0x80):
        break
      shift += 7
    return (result, offset)

  @staticmethod
  def add_emscripten_metadata(js_file, wasm_file):
    WASM_PAGE_SIZE = 65536

    mem_size = Settings.INITIAL_MEMORY // WASM_PAGE_SIZE
    table_size = Settings.WASM_TABLE_SIZE
    global_base = Settings.GLOBAL_BASE

    js = open(js_file).read()
    m = re.search(r"(^|\s)DYNAMIC_BASE\s+=\s+(\d+)", js)
    dynamic_base = int(m.group(2))
    m = re.search(r"(^|\s)DYNAMICTOP_PTR\s+=\s+(\d+)", js)
    dynamictop_ptr = int(m.group(2))

    logger.debug('creating wasm emscripten metadata section with mem size %d, table size %d' % (mem_size, table_size,))
    name = b'\x13emscripten_metadata' # section name, including prefixed size
    contents = (
      # metadata section version
      WebAssembly.toLEB(EMSCRIPTEN_METADATA_MAJOR) +
      WebAssembly.toLEB(EMSCRIPTEN_METADATA_MINOR) +

      # NB: The structure of the following should only be changed
      #     if EMSCRIPTEN_METADATA_MAJOR is incremented
      # Minimum ABI version
      WebAssembly.toLEB(EMSCRIPTEN_ABI_MAJOR) +
      WebAssembly.toLEB(EMSCRIPTEN_ABI_MINOR) +

      # Wasm backend, always 1 now
      WebAssembly.toLEB(1) +

      WebAssembly.toLEB(mem_size) +
      WebAssembly.toLEB(table_size) +
      WebAssembly.toLEB(global_base) +
      WebAssembly.toLEB(dynamic_base) +
      WebAssembly.toLEB(dynamictop_ptr) +

      # tempDoublePtr, always 0 in wasm backend
      WebAssembly.toLEB(0) +

      WebAssembly.toLEB(int(Settings.STANDALONE_WASM))

      # NB: more data can be appended here as long as you increase
      #     the EMSCRIPTEN_METADATA_MINOR
    )

    orig = open(wasm_file, 'rb').read()
    with open(wasm_file, 'wb') as f:
      f.write(orig[0:8]) # copy magic number and version
      # write the special section
      f.write(b'\0') # user section is code 0
      # need to find the size of this section
      size = len(name) + len(contents)
      f.write(WebAssembly.toLEB(size))
      f.write(name)
      f.write(contents)
      f.write(orig[8:])

  @staticmethod
  def add_dylink_section(wasm_file, needed_dynlibs):
    # a wasm shared library has a special "dylink" section, see tools-conventions repo
    # TODO: use this in the wasm backend
    assert False
    mem_align = Settings.MAX_GLOBAL_ALIGN
    mem_size = Settings.STATIC_BUMP
    table_size = Settings.WASM_TABLE_SIZE
    mem_align = int(math.log(mem_align, 2))
    logger.debug('creating wasm dynamic library with mem size %d, table size %d, align %d' % (mem_size, table_size, mem_align))

    # Write new wasm binary with 'dylink' section
    wasm = open(wasm_file, 'rb').read()
    section_name = b"\06dylink" # section name, including prefixed size
    contents = (WebAssembly.toLEB(mem_size) + WebAssembly.toLEB(mem_align) +
                WebAssembly.toLEB(table_size) + WebAssembly.toLEB(0))

    # we extend "dylink" section with information about which shared libraries
    # our shared library needs. This is similar to DT_NEEDED entries in ELF.
    #
    # In theory we could avoid doing this, since every import in wasm has
    # "module" and "name" attributes, but currently emscripten almost always
    # uses just "env" for "module". This way we have to embed information about
    # required libraries for the dynamic linker somewhere, and "dylink" section
    # seems to be the most relevant place.
    #
    # Binary format of the extension:
    #
    #   needed_dynlibs_count        varuint32       ; number of needed shared libraries
    #   needed_dynlibs_entries      dynlib_entry*   ; repeated dynamic library entries as described below
    #
    # dynlib_entry:
    #
    #   dynlib_name_len             varuint32       ; length of dynlib_name_str in bytes
    #   dynlib_name_str             bytes           ; name of a needed dynamic library: valid UTF-8 byte sequence
    #
    # a proposal has been filed to include the extension into "dylink" specification:
    # https://github.com/WebAssembly/tool-conventions/pull/77
    contents += WebAssembly.toLEB(len(needed_dynlibs))
    for dyn_needed in needed_dynlibs:
      dyn_needed = bytes(asbytes(dyn_needed))
      contents += WebAssembly.toLEB(len(dyn_needed))
      contents += dyn_needed

    section_size = len(section_name) + len(contents)
    with open(wasm_file, 'wb') as f:
      # copy magic number and version
      f.write(wasm[0:8])
      # write the special section
      f.write(b'\0') # user section is code 0
      f.write(WebAssembly.toLEB(section_size))
      f.write(section_name)
      f.write(contents)
      # copy rest of binary
      f.write(wasm[8:])


# Python 2-3 compatibility helper function:
# Converts a string to the native str type.
def asstr(s):
  if str is bytes:
    if isinstance(s, type(u'')):
      return s.encode('utf-8')
  elif isinstance(s, bytes):
    return s.decode('utf-8')
  return s


def asbytes(s):
  if isinstance(s, bytes):
    # Do not attempt to encode bytes
    return s
  return s.encode('utf-8')


def suffix(name):
  """Return the file extension"""
  return os.path.splitext(name)[1]


def unsuffixed(name):
  """Return the filename without the extention.

  If there are multiple extensions this strips only the final one.
  """
  return os.path.splitext(name)[0]


def unsuffixed_basename(name):
  return os.path.basename(unsuffixed(name))


def safe_move(src, dst):
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  if dst == os.devnull:
    return
  logging.debug('move: %s -> %s', src, dst)
  shutil.move(src, dst)


def safe_copy(src, dst):
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  if dst == os.devnull:
    return
  shutil.copyfile(src, dst)


def read_and_preprocess(filename, expand_macros=False):
  temp_dir = get_emscripten_temp_dir()
  # Create a settings file with the current settings to pass to the JS preprocessor
  # Note: Settings.serialize returns an array of -s options i.e. ['-s', '<setting1>', '-s', '<setting2>', ...]
  #       we only want the actual settings, hence the [1::2] slice operation.
  settings_str = "var " + ";\nvar ".join(Settings.serialize()[1::2])
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


# ============================================================================
# End declarations.
# ============================================================================

# Everything below this point is top level code that get run when importing this
# file.  TODO(sbc): We should try to reduce that amount we do here and instead
# have consumers explicitly call initialization functions.

# Emscripten configuration is done through the --em-config command line option
# or the EM_CONFIG environment variable. If the specified string value contains
# newline or semicolon-separated definitions, then these definitions will be
# used to configure Emscripten.  Otherwise, the string is understood to be a
# path to a settings file that contains the required definitions.
# The search order from the config file is as follows:
# 1. Specified on the command line (--em-config)
# 2. Specified via EM_CONFIG environment variable
# 3. Local .emscripten file, if found
# 4. Local .emscripten file, as used by `emsdk --embedded` (two levels above,
#    see below)
# 5. User home directory config (~/.emscripten), if found.

embedded_config = path_from_root('.emscripten')
# For compatibility with `emsdk --embedded` mode also look two levels up.  The
# layout of the emsdk puts emcc two levels below emsdk.  For exmaple:
#  - emsdk/upstream/emscripten/emcc
#  - emsdk/emscipten/1.38.31/emcc
# However `emsdk --embedded` stores the config file in the emsdk root.
# Without this check, when emcc is run from within the emsdk in embedded mode
# and the user forgets to first run `emsdk_env.sh` (which sets EM_CONFIG) emcc
# will not see any config file at all and fall back to creating a new/emtpy
# one.
# We could remove this special case if emsdk were to write its embedded config
# file into the emscripten directory itself.
# See: https://github.com/emscripten-core/emsdk/pull/367
emsdk_root = os.path.dirname(os.path.dirname(__rootpath__))
emsdk_embedded_config = os.path.join(emsdk_root, '.emscripten')
user_home_config = os.path.expanduser('~/.emscripten')

EMSCRIPTEN_ROOT = __rootpath__

if '--em-config' in sys.argv:
  EM_CONFIG = sys.argv[sys.argv.index('--em-config') + 1]
  # And now remove it from sys.argv
  skip = False
  newargs = []
  for arg in sys.argv:
    if not skip and arg != '--em-config':
      newargs += [arg]
    elif arg == '--em-config':
      skip = True
    elif skip:
      skip = False
  sys.argv = newargs
  if not os.path.isfile(EM_CONFIG):
    if EM_CONFIG.startswith('-'):
      exit_with_error('Passed --em-config without an argument. Usage: --em-config /path/to/.emscripten or --em-config LLVM_ROOT=/path;...')
    if '=' not in EM_CONFIG:
      exit_with_error('File ' + EM_CONFIG + ' passed to --em-config does not exist!')
    else:
      EM_CONFIG = EM_CONFIG.replace(';', '\n') + '\n'
elif 'EM_CONFIG' in os.environ:
  EM_CONFIG = os.environ['EM_CONFIG']
elif os.path.exists(embedded_config):
  EM_CONFIG = embedded_config
elif os.path.exists(emsdk_embedded_config):
  EM_CONFIG = emsdk_embedded_config
elif os.path.exists(user_home_config):
  EM_CONFIG = user_home_config
else:
  if root_is_writable():
    generate_config(embedded_config, first_time=True)
  else:
    generate_config(user_home_config, first_time=True)
  sys.exit(0)

PYTHON = sys.executable

# The following globals can be overridden by the config file.
# See parse_config_file below.
NODE_JS = None
BINARYEN_ROOT = None
EM_POPEN_WORKAROUND = None
SPIDERMONKEY_ENGINE = None
V8_ENGINE = None
LLVM_ROOT = None
LLVM_ADD_VERSION = None
CLANG_ADD_VERSION = None
CLOSURE_COMPILER = None
EMSCRIPTEN_NATIVE_OPTIMIZER = None
JAVA = None
JS_ENGINE = None
JS_ENGINES = []
WASMER = None
WASMTIME = None
WASM_ENGINES = []
CACHE = None
PORTS = None
FROZEN_CACHE = False

# Emscripten compiler spawns other processes, which can reimport shared.py, so
# make sure that those child processes get the same configuration file by
# setting it to the currently active environment.
os.environ['EM_CONFIG'] = EM_CONFIG

if '\n' in EM_CONFIG:
  CONFIG_FILE = None
  logger.debug('EM_CONFIG is specified inline without a file')
else:
  CONFIG_FILE = os.path.expanduser(EM_CONFIG)
  logger.debug('EM_CONFIG is located in ' + CONFIG_FILE)
  if not os.path.exists(CONFIG_FILE):
    exit_with_error('emscripten config file not found: ' + CONFIG_FILE)

parse_config_file()
normalize_config_settings()

# Install our replacement Popen handler if we are running on Windows to avoid
# python spawn process function.
# nb. This is by default disabled since it has the adverse effect of buffering
# up all logging messages, which makes builds look unresponsive (messages are
# printed only after the whole build finishes). Whether this workaround is
# needed seems to depend on how the host application that invokes emcc has set
# up its stdout and stderr.
if EM_POPEN_WORKAROUND and os.name == 'nt':
  logger.debug('Installing Popen workaround handler to avoid bug http://bugs.python.org/issue3905')
  Popen = WindowsPopen
else:
  Popen = subprocess.Popen

# Verbosity level control for any intermediate subprocess spawns from the compiler. Useful for internal debugging.
# 0: disabled.
# 1: Log stderr of subprocess spawns.
# 2: Log stdout and stderr of subprocess spawns. Print out subprocess commands that were executed.
# 3: Log stdout and stderr, and pass VERBOSE=1 to CMake configure steps.
EM_BUILD_VERBOSE = int(os.getenv('EM_BUILD_VERBOSE', '0'))
TRACK_PROCESS_SPAWNS = EM_BUILD_VERBOSE >= 3

set_version_globals()

# For the Emscripten-specific WASM metadata section, follows semver, changes
# whenever metadata section changes structure.
# NB: major version 0 implies no compatibility
# NB: when changing the metadata format, we should only append new fields, not
#     reorder, modify, or remove existing ones.
EMSCRIPTEN_METADATA_MAJOR, EMSCRIPTEN_METADATA_MINOR = (0, 3)
# For the JS/WASM ABI, specifies the minimum ABI version required of
# the WASM runtime implementation by the generated WASM binary. It follows
# semver and changes whenever C types change size/signedness or
# syscalls change signature. By semver, the maximum ABI version is
# implied to be less than (EMSCRIPTEN_ABI_MAJOR + 1, 0). On an ABI
# change, increment EMSCRIPTEN_ABI_MINOR if EMSCRIPTEN_ABI_MAJOR == 0
# or the ABI change is backwards compatible, otherwise increment
# EMSCRIPTEN_ABI_MAJOR and set EMSCRIPTEN_ABI_MINOR = 0.
EMSCRIPTEN_ABI_MAJOR, EMSCRIPTEN_ABI_MINOR = (0, 26)

# Tools/paths
if LLVM_ADD_VERSION is None:
  LLVM_ADD_VERSION = os.getenv('LLVM_ADD_VERSION')

if CLANG_ADD_VERSION is None:
  CLANG_ADD_VERSION = os.getenv('CLANG_ADD_VERSION')

CLANG_CC = os.path.expanduser(build_clang_tool_path(exe_suffix('clang')))
CLANG_CXX = os.path.expanduser(build_clang_tool_path(exe_suffix('clang++')))
LLVM_LINK = build_llvm_tool_path(exe_suffix('llvm-link'))
LLVM_AR = build_llvm_tool_path(exe_suffix('llvm-ar'))
LLVM_RANLIB = build_llvm_tool_path(exe_suffix('llvm-ranlib'))
LLVM_OPT = os.path.expanduser(build_llvm_tool_path(exe_suffix('opt')))
LLVM_AS = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-as')))
LLVM_DIS = os.path.expanduser(build_llvm_tool_path(exe_suffix('llvm-dis')))
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
AUTODEBUGGER = path_from_root('tools', 'autodebugger.py')
FILE_PACKAGER = path_from_root('tools', 'file_packager.py')

apply_configuration()

# Target choice.
LLVM_TARGET = 'wasm32-unknown-emscripten'

Settings = SettingsManager()
verify_settings()
Cache = cache.Cache(CACHE)

PRINT_STAGES = int(os.getenv('EMCC_VERBOSE', '0'))

# compatibility with existing emcc, etc. scripts
chunkify = cache.chunkify
