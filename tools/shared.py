# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from subprocess import PIPE
import atexit
import binascii
import base64
import difflib
import json
import logging
import os
import re
import shutil
import subprocess
import time
import sys
import tempfile

# We depend on python 3.6 for fstring support
if sys.version_info < (3, 6):
  print('error: emscripten requires python 3.6 or above', file=sys.stderr)
  sys.exit(1)

from .toolchain_profiler import ToolchainProfiler
from .tempfiles import try_delete
from .utils import path_from_root, exit_with_error, safe_ensure_dirs, WINDOWS
from . import cache, tempfiles, colored_logger
from . import diagnostics
from . import config


DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))
EXPECTED_NODE_VERSION = (4, 1, 1)
EXPECTED_BINARYEN_VERSION = 98
EXPECTED_LLVM_VERSION = "12.0"
SIMD_INTEL_FEATURE_TOWER = ['-msse', '-msse2', '-msse3', '-mssse3', '-msse4.1', '-msse4.2', '-mavx']
SIMD_NEON_FLAGS = ['-mfpu=neon']
PYTHON = sys.executable

# can add  %(asctime)s  to see timestamps
logging.basicConfig(format='%(name)s:%(levelname)s: %(message)s',
                    level=logging.DEBUG if DEBUG else logging.INFO)
colored_logger.enable()
logger = logging.getLogger('shared')

# warning about absolute-paths is disabled by default, and not enabled by -Wall
diagnostics.add_warning('absolute-paths', enabled=False, part_of_all=False)
# unused diagnositic flags.  TODO(sbc): remove at some point
diagnostics.add_warning('almost-asm')
diagnostics.add_warning('experimental')
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
diagnostics.add_warning('pthreads-mem-growth')


# TODO(sbc): Investigate switching to shlex.quote
def shlex_quote(arg):
  if ' ' in arg and (not (arg.startswith('"') and arg.endswith('"'))) and (not (arg.startswith("'") and arg.endswith("'"))):
    return '"' + arg.replace('"', '\\"') + '"'

  return arg


# Switch to shlex.join once we can depend on python 3.8:
# https://docs.python.org/3/library/shlex.html#shlex.join
def shlex_join(cmd):
  return ' '.join(shlex_quote(x) for x in cmd)


def run_process(cmd, check=True, input=None, *args, **kw):
  """Runs a subpocess returning the exit code.

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


def check_call(cmd, *args, **kw):
  """Like `run_process` above but treat failures as fatal and exit_with_error."""
  print_compiler_stage(cmd)
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
    EMSCRIPTEN_VERSION = f.read().strip().replace('"', '')
  parts = [int(x) for x in EMSCRIPTEN_VERSION.split('.')]
  EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY = parts


def generate_sanity():
  sanity_file_content = EMSCRIPTEN_VERSION + '|' + config.LLVM_ROOT + '|' + get_clang_version()
  if config.config_file:
    config_data = open(config.config_file).read()
  else:
    config_data = config.EM_CONFIG
  checksum = binascii.crc32(config_data.encode())
  sanity_file_content += '|%#x\n' % checksum
  return sanity_file_content


def perform_sanity_checks():
  logger.info('(Emscripten: Running sanity checks)')

  with ToolchainProfiler.profile_block('sanity compiler_engine'):
    try:
      run_process(config.NODE_JS + ['-e', 'console.log("hello")'], stdout=PIPE)
    except Exception as e:
      exit_with_error('The configured node executable (%s) does not seem to work, check the paths in %s (%s)', config.NODE_JS, config.config_file_location(), str(e))

  with ToolchainProfiler.profile_block('sanity LLVM'):
    for cmd in [CLANG_CC, LLVM_AR, LLVM_NM]:
      if not os.path.exists(cmd) and not os.path.exists(cmd + '.exe'):  # .exe extension required for Windows
        exit_with_error('Cannot find %s, check the paths in %s', cmd, config.EM_CONFIG)


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
    if not config.config_file:
      return # config stored directly in EM_CONFIG => skip sanity checks
    expected = generate_sanity()

    sanity_file = Cache.get_path('sanity.txt')
    if os.path.exists(sanity_file):
      sanity_data = open(sanity_file).read()
      if sanity_data != expected:
        logger.debug('old sanity: %s' % sanity_data)
        logger.debug('new sanity: %s' % expected)
        if config.FROZEN_CACHE:
          logger.info('(Emscripten: config changed, cache may need to be cleared, but FROZEN_CACHE is set)')
        else:
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

    # some warning, mostly not fatal checks - do them even if EM_IGNORE_SANITY is on
    check_node_version()

    llvm_ok = check_llvm()

    if os.environ.get('EM_IGNORE_SANITY'):
      logger.info('EM_IGNORE_SANITY set, ignoring sanity checks')
      return

    if not llvm_ok:
      exit_with_error('failing sanity checks due to previous llvm failure')

    perform_sanity_checks()

    if not force:
      # Only create/update this file if the sanity check succeeded, i.e., we got here
      Cache.ensure()
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
  return replace_suffix(filename, new_suffix) if Settings.MINIMAL_RUNTIME else filename + new_suffix


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
        exit_with_error(str(e) + 'Could not create canonical temp dir. Check definition of TEMP_DIR in ' + config.config_file_location())

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
  if Settings.MEMORY64:
    return 'wasm64-unknown-emscripten'
  else:
    return 'wasm32-unknown-emscripten'


def emsdk_ldflags(user_args):
  if os.environ.get('EMMAKEN_NO_SDK'):
    return []

  library_paths = [
      path_from_root('system', 'local', 'lib'),
      path_from_root('system', 'lib'),
      Cache.get_path(Cache.get_lib_dir())
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

  c_opts = ['--sysroot=' + path_from_root('system')]

  def array_contains_any_of(hay, needles):
    for n in needles:
      if n in hay:
        return True

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER) or array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    if '-msimd128' not in user_args:
      exit_with_error('Passing any of ' + ', '.join(SIMD_INTEL_FEATURE_TOWER + SIMD_NEON_FLAGS) + ' flags also requires passing -msimd128!')
    c_opts += ['-D__SSE__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[1:]):
    c_opts += ['-D__SSE2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[2:]):
    c_opts += ['-D__SSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[3:]):
    c_opts += ['-D__SSSE3__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[4:]):
    c_opts += ['-D__SSE4_1__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[5:]):
    c_opts += ['-D__SSE4_2__=1']

  if array_contains_any_of(user_args, SIMD_INTEL_FEATURE_TOWER[6:]):
    c_opts += ['-D__AVX__=1']

  if array_contains_any_of(user_args, SIMD_NEON_FLAGS):
    c_opts += ['-D__ARM_NEON__=1']

  sysroot_include_paths = []

  if cxx:
    sysroot_include_paths += [
      os.path.join('/include', 'libcxx'),
      os.path.join('/lib', 'libcxxabi', 'include'),
    ]

  # TODO: Merge the cache into the sysroot.
  c_opts += ['-Xclang', '-isystem' + Cache.get_path('include')]

  sysroot_include_paths += [
    os.path.join('/include', 'compat'),
    os.path.join('/include', 'libc'),
    os.path.join('/lib', 'libc', 'musl', 'arch', 'emscripten'),
    os.path.join('/local', 'include'),
    os.path.join('/include', 'SSE'),
    os.path.join('/include', 'neon'),
    os.path.join('/lib', 'compiler-rt', 'include'),
    os.path.join('/lib', 'libunwind', 'include'),
  ]

  def include_directive(paths):
    result = []
    for path in paths:
      result += ['-Xclang', '-iwithsysroot' + path]
    return result

  # libcxx include paths must be defined before libc's include paths otherwise libcxx will not build
  return c_opts + include_directive(sysroot_include_paths)


def get_clang_flags():
  return ['-target', get_llvm_target()]


def get_cflags(user_args, cxx):
  c_opts = get_clang_flags()

  # Set the LIBCPP ABI version to at least 2 so that we get nicely aligned string
  # data and other nice fixes.
  c_opts += [# '-fno-threadsafe-statics', # disabled due to issue 1289
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
  if name in Settings.WASM_SYSTEM_EXPORTS:
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


class JS(object):
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
  def make_dynCall(sig, args):
    # wasm2c and asyncify are not yet compatible with direct wasm table calls
    if Settings.USE_LEGACY_DYNCALLS or not JS.is_legal_sig(sig):
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
  logging.debug('move: %s -> %s', src, dst)
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  if dst == os.devnull:
    return
  shutil.move(src, dst)


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
FILE_PACKAGER = bat_suffix(path_from_root('tools', 'file_packager'))

apply_configuration()

Settings = SettingsManager()
verify_settings()
Cache = cache.Cache(config.CACHE)

PRINT_STAGES = int(os.getenv('EMCC_VERBOSE', '0'))
