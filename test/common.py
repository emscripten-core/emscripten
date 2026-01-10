# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import difflib
import hashlib
import io
import json
import logging
import os
import re
import shlex
import shutil
import stat
import string
import subprocess
import sys
import tempfile
import textwrap
from functools import wraps
from pathlib import Path
from subprocess import PIPE, STDOUT

import clang_native
import jsrun
import line_endings
from retryable_unittest import RetryableTestCase

from tools import building, config, feature_matrix, shared, utils
from tools.feature_matrix import Feature
from tools.settings import COMPILE_TIME_SETTINGS
from tools.shared import DEBUG, EMCC, EMXX, get_canonical_temp_dir
from tools.utils import (
  WINDOWS,
  exe_path_from_root,
  path_from_root,
  read_binary,
  read_file,
  write_binary,
)

logger = logging.getLogger('common')

# If we are drawing a parallel swimlane graph of test output, we need to use a temp
# file to track which tests were flaky so they can be graphed in orange color to
# visually stand out.
flaky_tests_log_filename = os.path.join(path_from_root('out/flaky_tests.txt'))

EMTEST_DETECT_TEMPFILE_LEAKS = None
EMTEST_SAVE_DIR = None
# generally js engines are equivalent, testing 1 is enough. set this
# to force testing on all js engines, good to find js engine bugs
EMTEST_ALL_ENGINES = None
EMTEST_SKIP_SLOW = None
EMTEST_SKIP_FLAKY = None
EMTEST_RETRY_FLAKY = None
EMTEST_LACKS_NATIVE_CLANG = None
EMTEST_VERBOSE = None
EMTEST_REBASELINE = None

# Special value for passing to assert_returncode which means we expect that program
# to fail with non-zero return code, but we don't care about specifically which one.
NON_ZERO = -1

TEST_ROOT = path_from_root('test')
LAST_TEST = path_from_root('out/last_test.txt')
PREVIOUS_TEST_RUN_RESULTS_FILE = path_from_root('out/previous_test_run_results.json')

WEBIDL_BINDER = exe_path_from_root('tools/webidl_binder')

EMBUILDER = exe_path_from_root('embuilder')
EMMAKE = exe_path_from_root('emmake')
EMCMAKE = exe_path_from_root('emcmake')
EMCONFIGURE = exe_path_from_root('emconfigure')
EMRUN = exe_path_from_root('emrun')
WASM_DIS = os.path.join(building.get_binaryen_bin(), 'wasm-dis')
LLVM_OBJDUMP = shared.llvm_tool_path('llvm-objdump')
PYTHON = sys.executable

assert config.NODE_JS # assert for mypy's benefit
# By default we run the tests in the same version of node as emscripten itself used.
if not config.NODE_JS_TEST:
  config.NODE_JS_TEST = config.NODE_JS
# The default set of JS_ENGINES contains just node.
if not config.JS_ENGINES:
  config.JS_ENGINES = [config.NODE_JS_TEST]


def errlog(*args):
  """Shorthand for print with file=sys.stderr

  Use this for all internal test framework logging..
  """
  print(*args, file=sys.stderr)


def load_previous_test_run_results():
  try:
    return json.load(open(PREVIOUS_TEST_RUN_RESULTS_FILE))
  except FileNotFoundError:
    return {}


def test_file(*path_components):
  """Construct a path relative to the emscripten "tests" directory."""
  return str(Path(TEST_ROOT, *path_components))


def maybe_test_file(filename):
  if not os.path.exists(filename) and os.path.exists(test_file(filename)):
    filename = test_file(filename)
  return filename


def copytree(src, dest):
  shutil.copytree(src, dest, dirs_exist_ok=True)


def exe_suffix(cmd):
  return cmd + '.exe' if WINDOWS else cmd


def compiler_for(filename, force_c=False):
  if utils.suffix(filename) in ('.cc', '.cxx', '.cpp') and not force_c:
    return EMXX
  else:
    return EMCC


def record_flaky_test(test_name, attempt_count, max_attempts, exception_msg):
  logger.info(f'Retrying flaky test "{test_name}" (attempt {attempt_count}/{max_attempts} failed):\n{exception_msg}')
  open(flaky_tests_log_filename, 'a').write(f'{test_name}\n')


def node_bigint_flags(node_version):
  # The --experimental-wasm-bigint flag was added in v12, and then removed (enabled by default)
  # in v16.
  if node_version and node_version < (16, 0, 0):
    return ['--experimental-wasm-bigint']
  else:
    return []


@contextlib.contextmanager
def env_modify(updates):
  """A context manager that updates os.environ."""
  # This could also be done with mock.patch.dict() but taking a dependency
  # on the mock library is probably not worth the benefit.
  old_env = os.environ.copy()
  print("env_modify: " + str(updates))
  # Setting a value to None means clear the environment variable
  clears = [key for key, value in updates.items() if value is None]
  updates = {key: value for key, value in updates.items() if value is not None}
  os.environ.update(updates)
  for key in clears:
    if key in os.environ:
      del os.environ[key]
  try:
    yield
  finally:
    os.environ.clear()
    os.environ.update(old_env)


def ensure_dir(dirname):
  dirname = Path(dirname)
  dirname.mkdir(parents=True, exist_ok=True)


def limit_size(string):
  maxbytes = 800000 * 20
  if sys.stdout.isatty():
    maxlines = 500
    max_line = 500
  else:
    max_line = 5000
    maxlines = 1000
  lines = string.splitlines()
  for i, line in enumerate(lines):
    if len(line) > max_line:
      lines[i] = line[:max_line] + '[..]'
  if len(lines) > maxlines:
    lines = lines[0:maxlines // 2] + ['[..]'] + lines[-maxlines // 2:]
    lines.append('(not all output shown. See `limit_size`)')
  string = '\n'.join(lines) + '\n'
  if len(string) > maxbytes:
    string = string[0:maxbytes // 2] + '\n[..]\n' + string[-maxbytes // 2:]
  return string


def create_file(name, contents, binary=False, absolute=False):
  name = Path(name)
  assert absolute or not name.is_absolute(), name
  if binary:
    name.write_bytes(contents)
  else:
    # Dedent the contents of text files so that the files on disc all
    # start in column 1, even if they are indented when embedded in the
    # python test code.
    contents = textwrap.dedent(contents)
    name.write_text(contents, encoding='utf-8')


@contextlib.contextmanager
def chdir(dir):
  """A context manager that performs actions in the given directory."""
  orig_cwd = os.getcwd()
  os.chdir(dir)
  try:
    yield
  finally:
    os.chdir(orig_cwd)


def make_executable(name):
  Path(name).chmod(stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_dir_writeable(dirname):
  # Some tests make files and subdirectories read-only, so rmtree/unlink will not delete
  # them. Force-make everything writable in the subdirectory to make it
  # removable and re-attempt.
  os.chmod(dirname, 0o777)

  for directory, subdirs, files in os.walk(dirname):
    for item in files + subdirs:
      i = os.path.join(directory, item)
      if not os.path.islink(i):
        os.chmod(i, 0o777)


def force_delete_dir(dirname):
  """Deletes a directory. Returns whether deletion succeeded."""
  if not os.path.exists(dirname):
    return True
  assert not os.path.isfile(dirname)

  try:
    make_dir_writeable(dirname)
    utils.delete_dir(dirname)
  except PermissionError as e:
    # This issue currently occurs on Windows when running browser tests e.g.
    # on Firefox browser. Killing Firefox browser is not 100% watertight, and
    # occasionally a Firefox browser process can be left behind, holding on
    # to a file handle, preventing the deletion from succeeding.
    # We expect this issue to only occur on Windows.
    if not WINDOWS:
      raise e
    print(f'Warning: Failed to delete directory "{dirname}"\n{e}')
    return False
  return True


def force_delete_contents(dirname):
  make_dir_writeable(dirname)
  utils.delete_contents(dirname)


def get_output_suffix(args):
  if any(a in args for a in ('-sEXPORT_ES6', '-sWASM_ESM_INTEGRATION', '-sMODULARIZE=instance')):
    return '.mjs'
  else:
    return '.js'


class RunnerMeta(type):
  @classmethod
  def make_test(mcs, name, func, suffix, args):
    """
    This is a helper function to create new test functions for each parameterized form.

    :param name: the original name of the function
    :param func: the original function that we are parameterizing
    :param suffix: the suffix to append to the name of the function for this parameterization
    :param args: the positional arguments to pass to the original function for this parameterization
    :returns: a tuple of (new_function_name, new_function_object)
    """

    # Create the new test function. It calls the original function with the specified args.
    # We use @functools.wraps to copy over all the function attributes.
    @wraps(func)
    def resulting_test(self):
      return func(self, *args)

    # Add suffix to the function name so that it displays correctly.
    if suffix:
      resulting_test.__name__ = f'{name}_{suffix}'
    else:
      resulting_test.__name__ = name

    # On python 3, functions have __qualname__ as well. This is a full dot-separated path to the
    # function.  We add the suffix to it as well.
    resulting_test.__qualname__ = f'{func.__qualname__}_{suffix}'

    return resulting_test.__name__, resulting_test

  def __new__(mcs, name, bases, attrs):
    # This metaclass expands parameterized methods from `attrs` into separate ones in `new_attrs`.
    new_attrs = {}

    for attr_name, value in attrs.items():
      # Check if a member of the new class has _parameterize, the tag inserted by @parameterized.
      if hasattr(value, '_parameterize'):
        # If it does, we extract the parameterization information, build new test functions.
        for suffix, args in value._parameterize.items():
          new_name, func = mcs.make_test(attr_name, value, suffix, args)
          assert new_name not in new_attrs, 'Duplicate attribute name generated when parameterizing %s' % attr_name
          new_attrs[new_name] = func
      else:
        # If not, we just copy it over to new_attrs verbatim.
        assert attr_name not in new_attrs, '%s collided with an attribute from parameterization' % attr_name
        new_attrs[attr_name] = value

    # We invoke type, the default metaclass, to actually create the new class, with new_attrs.
    return type.__new__(mcs, name, bases, new_attrs)


class RunnerCore(RetryableTestCase, metaclass=RunnerMeta):
  # default temporary directory settings. set_temp_dir may be called later to
  # override these
  temp_dir = shared.TEMP_DIR
  canonical_temp_dir = get_canonical_temp_dir(shared.TEMP_DIR)

  # This avoids cluttering the test runner output, which is stderr too, with compiler warnings etc.
  # Change this to None to get stderr reporting, for debugging purposes
  stderr_redirect = STDOUT

  library_cache: dict[str, tuple[str, object]] = {}

  def is_wasm(self):
    return self.get_setting('WASM') != 0

  def is_wasm2js(self):
    return not self.is_wasm()

  def is_browser_test(self):
    return False

  def is_wasm64(self):
    return self.get_setting('MEMORY64')

  def is_4gb(self):
    return self.get_setting('INITIAL_MEMORY') == '4200mb'

  def is_2gb(self):
    return self.get_setting('INITIAL_MEMORY') == '2200mb'

  def check_dylink(self):
    if self.get_setting('WASM_ESM_INTEGRATION'):
      self.skipTest('dynamic linking not supported with WASM_ESM_INTEGRATION')
    if '-lllvmlibc' in self.cflags:
      self.skipTest('dynamic linking not supported with llvm-libc')
    if self.is_wasm2js():
      self.skipTest('dynamic linking not supported with wasm2js')
    # MEMORY64=2 mode doesn't currently support dynamic linking because
    # The side modules are lowered to wasm32 when they are built, making
    # them unlinkable with wasm64 binaries.
    if self.get_setting('MEMORY64') == 2:
      self.skipTest('dynamic linking not supported with MEMORY64=2')

  def get_v8(self):
    """Return v8 engine, if one is configured, otherwise None"""
    if not config.V8_ENGINE or config.V8_ENGINE not in config.JS_ENGINES:
      return None
    return config.V8_ENGINE

  def require_v8(self):
    if 'EMTEST_SKIP_V8' in os.environ:
      self.skipTest('test requires v8 and EMTEST_SKIP_V8 is set')
    v8 = self.get_v8()
    if not v8:
      self.fail('d8 required to run this test.  Use EMTEST_SKIP_V8 to skip')
    self.require_engine(v8)
    self.cflags.append('-sENVIRONMENT=shell')

  def get_nodejs(self):
    """Return nodejs engine, if one is configured, otherwise None"""
    if config.NODE_JS_TEST not in config.JS_ENGINES:
      return None
    return config.NODE_JS_TEST

  def require_node(self):
    if 'EMTEST_SKIP_NODE' in os.environ:
      self.skipTest('test requires node and EMTEST_SKIP_NODE is set')
    nodejs = self.get_nodejs()
    if not nodejs:
      self.fail('node required to run this test.  Use EMTEST_SKIP_NODE to skip')
    self.require_engine(nodejs)
    return nodejs

  def node_is_canary(self, nodejs):
    return nodejs and nodejs[0] and ('canary' in nodejs[0] or 'nightly' in nodejs[0])

  def require_node_canary(self):
    if 'EMTEST_SKIP_NODE_CANARY' in os.environ:
      self.skipTest('test requires node canary and EMTEST_SKIP_NODE_CANARY is set')
    nodejs = self.get_nodejs()
    if self.node_is_canary(nodejs):
      self.require_engine(nodejs)
      return

    self.fail('node canary required to run this test.  Use EMTEST_SKIP_NODE_CANARY to skip')

  def require_engine(self, engine):
    logger.debug(f'require_engine: {engine}')
    if self.required_engine and self.required_engine != engine:
      self.skipTest(f'Skipping test that requires `{engine}` when `{self.required_engine}` was previously required')
    self.required_engine = engine
    self.js_engines = [engine]
    self.wasm_engines = []

  def require_wasm64(self):
    if 'EMTEST_SKIP_WASM64' in os.environ:
      self.skipTest('test requires node >= 24 or d8 (and EMTEST_SKIP_WASM64 is set)')
    if self.is_browser_test():
      return

    if self.try_require_node_version(24):
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      return

    self.fail('either d8 or node >= 24 required to run wasm64 tests.  Use EMTEST_SKIP_WASM64 to skip')

  def try_require_node_version(self, major, minor = 0, revision = 0):
    nodejs = self.get_nodejs()
    if not nodejs:
      self.skipTest('Test requires nodejs to run')
    version = shared.get_node_version(nodejs)
    if version < (major, minor, revision):
      return False

    self.js_engines = [nodejs]
    return True

  def require_simd(self):
    if 'EMTEST_SKIP_SIMD' in os.environ:
      self.skipTest('test requires node >= 16 or d8 (and EMTEST_SKIP_SIMD is set)')
    if self.is_browser_test():
      return

    if self.try_require_node_version(16):
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      return

    self.fail('either d8 or node >= 16 required to run wasm64 tests.  Use EMTEST_SKIP_SIMD to skip')

  def require_wasm_legacy_eh(self):
    if 'EMTEST_SKIP_WASM_LEGACY_EH' in os.environ:
      self.skipTest('test requires node >= 17 or d8 (and EMTEST_SKIP_WASM_LEGACY_EH is set)')
    self.set_setting('WASM_LEGACY_EXCEPTIONS')

    if self.is_browser_test():
      self.check_browser_feature('EMTEST_SKIP_WASM_LEGACY_EH', Feature.WASM_LEGACY_EXCEPTIONS, 'test requires Wasm Legacy EH')
      return

    if self.try_require_node_version(17):
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      return

    self.fail('either d8 or node >= 17 required to run legacy wasm-eh tests.  Use EMTEST_SKIP_WASM_LEGACY_EH to skip')

  def require_wasm_eh(self):
    if 'EMTEST_SKIP_WASM_EH' in os.environ:
      self.skipTest('test requires node v24 or d8 (and EMTEST_SKIP_WASM_EH is set)')
    self.set_setting('WASM_LEGACY_EXCEPTIONS', 0)

    if self.is_browser_test():
      self.check_browser_feature('EMTEST_SKIP_WASM_EH', Feature.WASM_EXCEPTIONS, 'test requires Wasm EH')
      return

    if self.try_require_node_version(22):
      self.node_args.append('--experimental-wasm-exnref')
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      self.v8_args.append('--experimental-wasm-exnref')
      return

    self.fail('either d8 or node v24 required to run wasm-eh tests.  Use EMTEST_SKIP_WASM_EH to skip')

  def require_jspi(self):
    if 'EMTEST_SKIP_JSPI' in os.environ:
      self.skipTest('skipping JSPI (EMTEST_SKIP_JSPI is set)')
    # emcc warns about stack switching being experimental, and we build with
    # warnings-as-errors, so disable that warning
    self.cflags += ['-Wno-experimental']
    self.set_setting('JSPI')
    if self.is_wasm2js():
      self.skipTest('JSPI is not currently supported for WASM2JS')
    if self.get_setting('WASM_ESM_INTEGRATION'):
      self.skipTest('WASM_ESM_INTEGRATION is not compatible with JSPI')

    if self.is_browser_test():
      return

    # Support for JSPI came earlier than 22, but the new API changes require v24
    if self.try_require_node_version(24):
      self.node_args += ['--experimental-wasm-stack-switching']
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      return

    self.fail('either d8 or node v24 required to run JSPI tests.  Use EMTEST_SKIP_JSPI to skip')

  def require_wasm2js(self):
    if self.is_wasm64():
      self.skipTest('wasm2js is not compatible with MEMORY64')
    if self.is_2gb() or self.is_4gb():
      self.skipTest('wasm2js does not support over 2gb of memory')
    if self.get_setting('WASM_ESM_INTEGRATION'):
      self.skipTest('wasm2js is not compatible with WASM_ESM_INTEGRATION')

  def setup_nodefs_test(self):
    self.require_node()
    if self.get_setting('WASMFS'):
      # without this the JS setup code in setup_nodefs.js doesn't work
      self.set_setting('FORCE_FILESYSTEM')
    self.cflags += ['-DNODEFS', '-lnodefs.js', '--pre-js', test_file('setup_nodefs.js'), '-sINCOMING_MODULE_JS_API=onRuntimeInitialized']

  def setup_noderawfs_test(self):
    self.require_node()
    self.cflags += ['-DNODERAWFS']
    self.set_setting('NODERAWFS')

  def setup_wasmfs_test(self):
    self.set_setting('WASMFS')
    self.cflags += ['-DWASMFS']

  def setup_node_pthreads(self):
    self.require_node()
    self.cflags += ['-Wno-pthreads-mem-growth', '-pthread']
    if self.get_setting('MINIMAL_RUNTIME'):
      self.skipTest('node pthreads not yet supported with MINIMAL_RUNTIME')
    nodejs = self.get_nodejs()
    self.js_engines = [nodejs]
    self.node_args += shared.node_pthread_flags(nodejs)

  def set_temp_dir(self, temp_dir):
    self.temp_dir = temp_dir
    self.canonical_temp_dir = get_canonical_temp_dir(self.temp_dir)
    # Explicitly set dedicated temporary directory for parallel tests
    os.environ['EMCC_TEMP_DIR'] = self.temp_dir

  def parse_wasm(self, filename):
    wat = self.get_wasm_text(filename)
    imports = []
    exports = []
    funcs = []
    for line in wat.splitlines():
      line = line.strip()
      if line.startswith('(import '):
        line = line.strip('()')
        parts = line.split()
        module = parts[1].strip('"')
        name = parts[2].strip('"')
        imports.append('%s.%s' % (module, name))
      if line.startswith('(export '):
        line = line.strip('()')
        name = line.split()[1].strip('"')
        exports.append(name)
      if line.startswith('(func '):
        line = line.strip('()')
        name = line.split()[1].strip('"')
        funcs.append(name)
    return imports, exports, funcs

  def output_name(self, basename):
    suffix = get_output_suffix(self.get_cflags())
    return basename + suffix

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    shared.check_sanity(force=True, quiet=True)

  def setUp(self):
    super().setUp()
    self.js_engines = config.JS_ENGINES.copy()
    self.settings_mods = {}
    self.skip_exec = None
    self.flaky = False
    self.cflags = ['-Wclosure', '-Werror', '-Wno-limited-postlink-optimizations']
    # TODO(https://github.com/emscripten-core/emscripten/issues/11121)
    # For historical reasons emcc compiles and links as C++ by default.
    # However we want to run our tests in a more strict manner.  We can
    # remove this if the issue above is ever fixed.
    self.set_setting('NO_DEFAULT_TO_CXX')
    self.ldflags = []
    # Increase the stack trace limit to maximise usefulness of test failure reports.
    # Also, include backtrace for all uncaught exceptions (not just Error).
    self.node_args = ['--stack-trace-limit=50', '--trace-uncaught']
    self.spidermonkey_args = ['-w']

    nodejs = self.get_nodejs()
    if nodejs:
      node_version = shared.get_node_version(nodejs)
      if node_version < (13, 0, 0):
        self.node_args.append('--unhandled-rejections=strict')
      elif node_version < (15, 0, 0):
        # Opt in to node v15 default behaviour:
        # https://nodejs.org/api/cli.html#cli_unhandled_rejections_mode
        self.node_args.append('--unhandled-rejections=throw')
      self.node_args += node_bigint_flags(node_version)

      # If the version we are running tests in is lower than the version that
      # emcc targets then we need to tell emcc to target that older version.
      emcc_min_node_version_str = str(shared.settings.MIN_NODE_VERSION)
      emcc_min_node_version = (
        int(emcc_min_node_version_str[0:2]),
        int(emcc_min_node_version_str[2:4]),
        int(emcc_min_node_version_str[4:6]),
      )
      if node_version < emcc_min_node_version:
        self.cflags += building.get_emcc_node_flags(node_version)
        self.cflags.append('-Wno-transpile')

      # This allows much of the test suite to be run on older versions of node that don't
      # support wasm bigint integration
      if node_version[0] < feature_matrix.min_browser_versions[feature_matrix.Feature.JS_BIGINT_INTEGRATION]['node'] / 10000:
        self.cflags.append('-sWASM_BIGINT=0')

    self.v8_args = ['--wasm-staging']
    self.env = {}
    self.temp_files_before_run = []
    self.required_engine = None
    self.wasm_engines = config.WASM_ENGINES.copy()
    self.use_all_engines = EMTEST_ALL_ENGINES
    if self.get_current_js_engine() != config.NODE_JS_TEST:
      # If our primary JS engine is something other than node then enable
      # shell support.
      default_envs = 'web,webview,worker,node'
      self.set_setting('ENVIRONMENT', default_envs + ',shell')

    if EMTEST_DETECT_TEMPFILE_LEAKS:
      for root, dirnames, filenames in os.walk(self.temp_dir):
        for dirname in dirnames:
          self.temp_files_before_run.append(os.path.normpath(os.path.join(root, dirname)))
        for filename in filenames:
          self.temp_files_before_run.append(os.path.normpath(os.path.join(root, filename)))

    if self.runningInParallel():
      self.working_dir = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=self.temp_dir)
    else:
      self.working_dir = path_from_root('out/test')
      if os.path.exists(self.working_dir):
        if EMTEST_SAVE_DIR == 2:
          print('Not clearing existing test directory')
        else:
          logger.debug('Clearing existing test directory: %s', self.working_dir)
          # Even when --save-dir is used we still try to start with an empty directory as many tests
          # expect this.  --no-clean can be used to keep the old contents for the new test
          # run. This can be useful when iterating on a given test with extra files you want to keep
          # around in the output directory.
          force_delete_contents(self.working_dir)
      else:
        logger.debug('Creating new test output directory: %s', self.working_dir)
        ensure_dir(self.working_dir)
      utils.write_file(LAST_TEST, self.id() + '\n')
    os.chdir(self.working_dir)

  def runningInParallel(self):
    return getattr(self, 'is_parallel', False)

  def tearDown(self):
    if self.runningInParallel() and not EMTEST_SAVE_DIR:
      # rmtree() fails on Windows if the current working directory is inside the tree.
      os.chdir(os.path.dirname(self.get_dir()))
      force_delete_dir(self.get_dir())

    if EMTEST_DETECT_TEMPFILE_LEAKS:
      temp_files_after_run = []
      for root, dirnames, filenames in os.walk(self.temp_dir):
        for dirname in dirnames:
          temp_files_after_run.append(os.path.normpath(os.path.join(root, dirname)))
        for filename in filenames:
          temp_files_after_run.append(os.path.normpath(os.path.join(root, filename)))

      # Our leak detection will pick up *any* new temp files in the temp dir.
      # They may not be due to us, but e.g. the browser when running browser
      # tests. Until we figure out a proper solution, ignore some temp file
      # names that we see on our CI infrastructure.
      ignorable_file_prefixes = [
        '/tmp/tmpaddon',
        '/tmp/circleci-no-output-timeout',
        '/tmp/wasmer',
      ]

      left_over_files = set(temp_files_after_run) - set(self.temp_files_before_run)
      left_over_files = [f for f in left_over_files if not any(f.startswith(p) for p in ignorable_file_prefixes)]
      if left_over_files:
        errlog(f'ERROR: After running test, there are {len(left_over_files)} new temporary files/directories left behind:')
        for f in left_over_files:
          errlog('leaked file: ', f)
        self.fail(f'Test leaked {len(left_over_files)} temporary files!')

  def get_setting(self, key, default=None):
    return self.settings_mods.get(key, default)

  def set_setting(self, key, value=1):
    if value is None:
      self.clear_setting(key)
    if type(value) is bool:
      value = int(value)
    self.settings_mods[key] = value

  def has_changed_setting(self, key):
    return key in self.settings_mods

  def clear_setting(self, key):
    self.settings_mods.pop(key, None)

  def serialize_settings(self, compile_only=False):
    ret = []
    for key, value in self.settings_mods.items():
      if compile_only and key not in COMPILE_TIME_SETTINGS:
        continue
      if value == 1:
        ret.append(f'-s{key}')
      elif type(value) is list:
        ret.append(f'-s{key}={",".join(value)}')
      else:
        ret.append(f'-s{key}={value}')
    return ret

  def get_dir(self):
    return self.working_dir

  def in_dir(self, *pathelems):
    return os.path.join(self.get_dir(), *pathelems)

  def add_pre_run(self, code):
    assert not self.get_setting('MINIMAL_RUNTIME')
    create_file('prerun.js', 'Module.preRun = function() { %s }\n' % code)
    self.cflags += ['--pre-js', 'prerun.js', '-sINCOMING_MODULE_JS_API=preRun']

  def add_post_run(self, code):
    assert not self.get_setting('MINIMAL_RUNTIME')
    create_file('postrun.js', 'Module.postRun = function() { %s }\n' % code)
    self.cflags += ['--pre-js', 'postrun.js', '-sINCOMING_MODULE_JS_API=postRun']

  def add_on_exit(self, code):
    assert not self.get_setting('MINIMAL_RUNTIME')
    create_file('onexit.js', 'Module.onExit = function() { %s }\n' % code)
    self.cflags += ['--pre-js', 'onexit.js', '-sINCOMING_MODULE_JS_API=onExit']

  # returns the full list of arguments to pass to emcc
  # param @main_file whether this is the main file of the test. some arguments
  #                  (like --pre-js) do not need to be passed when building
  #                  libraries, for example
  def get_cflags(self, main_file=False, compile_only=False, asm_only=False):
    def is_ldflag(f):
      return f.startswith('-l') or any(f.startswith(s) for s in ['-sEXPORT_ES6', '-sGL_TESTING', '-sPROXY_TO_PTHREAD', '-sENVIRONMENT=', '--pre-js=', '--post-js=', '-sPTHREAD_POOL_SIZE='])

    args = self.serialize_settings(compile_only or asm_only) + self.cflags
    if asm_only:
      args = [a for a in args if not a.startswith('-O')]
    if compile_only or asm_only:
      args = [a for a in args if not is_ldflag(a)]
    else:
      args += self.ldflags
    if not main_file:
      for i, arg in enumerate(args):
        if arg in ('--pre-js', '--post-js'):
          args[i] = None
          args[i + 1] = None
      args = [arg for arg in args if arg is not None]
    return args

  def verify_es5(self, filename):
    es_check = shared.get_npm_cmd('es-check')
    # use --quiet once its available
    # See: https://github.com/dollarshaveclub/es-check/pull/126/
    es_check_env = os.environ.copy()
    # Use NODE_JS here (the version of node that the compiler uses) rather then NODE_JS_TEST (the
    # version of node being used to run the tests) since we only care about having something that
    # can run the es-check tool.
    es_check_env['PATH'] = os.path.dirname(config.NODE_JS[0]) + os.pathsep + es_check_env['PATH']
    inputfile = os.path.abspath(filename)
    # For some reason es-check requires unix paths, even on windows
    if WINDOWS:
      inputfile = utils.normalize_path(inputfile)
    try:
      # es-check prints the details of the errors to stdout, but it also prints
      # stuff in the case there are no errors:
      #  ES-Check: there were no ES version matching errors!
      # pipe stdout and stderr so that we can choose if/when to print this
      # output and avoid spamming stdout when tests are successful.
      utils.run_process(es_check + ['es5', inputfile], stdout=PIPE, stderr=STDOUT, env=es_check_env)
    except subprocess.CalledProcessError as e:
      print(e.stdout)
      self.fail('es-check failed to verify ES5 output compliance')

  # Build JavaScript code from source code
  def build(self, filename, libraries=None, includes=None, force_c=False, cflags=None, output_basename=None, output_suffix=None):
    filename = maybe_test_file(filename)
    compiler = [compiler_for(filename, force_c)]

    if force_c:
      assert utils.suffix(filename) != '.c', 'force_c is not needed for source files ending in .c'
      compiler.append('-xc')

    all_cflags = self.get_cflags(main_file=True)
    if cflags:
      all_cflags += cflags
    if not output_suffix:
      output_suffix = get_output_suffix(all_cflags)

    if output_basename:
      output = output_basename + output_suffix
    else:
      output = utils.unsuffixed_basename(filename) + output_suffix
    cmd = compiler + [str(filename), '-o', output] + all_cflags
    if libraries:
      cmd += libraries
    if includes:
      cmd += ['-I' + str(include) for include in includes]

    self.run_process(cmd, stderr=self.stderr_redirect if not DEBUG else None)
    self.assertExists(output)

    if output_suffix in ('.js', '.mjs'):
      # Make sure we produced correct line endings
      self.assertEqual(line_endings.check_line_endings(output), 0)

    return output

  def get_func(self, src, name):
    start = src.index('function ' + name + '(')
    t = start
    n = 0
    while True:
      if src[t] == '{':
        n += 1
      elif src[t] == '}':
        n -= 1
        if n == 0:
          return src[start:t + 1]
      t += 1
      assert t < len(src)

  def get_wasm_text(self, wasm_binary):
    return self.run_process([WASM_DIS, wasm_binary], stdout=PIPE).stdout

  def is_exported_in_wasm(self, name, wasm):
    wat = self.get_wasm_text(wasm)
    return ('(export "%s"' % name) in wat

  def measure_wasm_code_lines(self, wasm):
    wat_lines = self.get_wasm_text(wasm).splitlines()
    non_data_lines = [line for line in wat_lines if '(data ' not in line]
    return len(non_data_lines)

  def clean_js_output(self, output):
    """Cleanup the JS output prior to running verification steps on it.

    Due to minification, when we get a crash report from JS it can sometimes
    contains the entire program in the output (since the entire program is
    on a single line).  In this case we can sometimes get false positives
    when checking for strings in the output.  To avoid these false positives
    and the make the output easier to read in such cases we attempt to remove
    such lines from the JS output.
    """
    lines = output.splitlines()
    long_lines = []

    def cleanup(line):
      if len(line) > 2048 and line.startswith('var Module=typeof Module!="undefined"'):
        long_lines.append(line)
        line = '<REPLACED ENTIRE PROGRAM ON SINGLE LINE>'
      return line

    lines = [cleanup(l) for l in lines]
    if not long_lines:
      # No long lines found just return the unmodified output
      return output

    # Sanity check that we only a single long line.
    assert len(long_lines) == 1
    return '\n'.join(lines)

  def get_current_js_engine(self):
    """Return the default JS engine to run tests under"""
    return self.js_engines[0]

  def get_engine_with_args(self, engine=None):
    if not engine:
      engine = self.get_current_js_engine()
    # Make a copy of the engine command before we modify/extend it.
    engine = list(engine)
    if engine == config.NODE_JS_TEST:
      engine += self.node_args
    elif engine == config.V8_ENGINE:
      engine += self.v8_args
    elif engine == config.SPIDERMONKEY_ENGINE:
      engine += self.spidermonkey_args
    return engine

  def run_js(self, filename, engine=None, args=None,
             assert_returncode=0,
             interleaved_output=True,
             input=None):
    # use files, as PIPE can get too full and hang us
    stdout_file = self.in_dir('stdout')
    stderr_file = None
    if interleaved_output:
      stderr = STDOUT
    else:
      stderr_file = self.in_dir('stderr')
      stderr = open(stderr_file, 'w')
    stdout = open(stdout_file, 'w')
    error = None
    timeout_error = None
    engine = self.get_engine_with_args(engine)
    try:
      jsrun.run_js(filename, engine, args,
                   stdout=stdout,
                   stderr=stderr,
                   assert_returncode=assert_returncode,
                   input=input)
    except subprocess.TimeoutExpired as e:
      timeout_error = e
    except subprocess.CalledProcessError as e:
      error = e
    finally:
      stdout.close()
      if stderr != STDOUT:
        stderr.close()

    ret = read_file(stdout_file)
    if not interleaved_output:
      ret += read_file(stderr_file)
    if assert_returncode != 0:
      ret = self.clean_js_output(ret)
    if error or timeout_error or EMTEST_VERBOSE:
      print('-- begin program output --')
      print(limit_size(read_file(stdout_file)), end='')
      print('-- end program output --')
      if not interleaved_output:
        print('-- begin program stderr --')
        print(limit_size(read_file(stderr_file)), end='')
        print('-- end program stderr --')
    if timeout_error:
      raise timeout_error
    if error:
      ret = limit_size(ret)
      if assert_returncode == NON_ZERO:
        self.fail('JS subprocess unexpectedly succeeded (%s):  Output:\n%s' % (error.cmd, ret))
      else:
        self.fail('JS subprocess failed (%s): %s (expected=%s).  Output:\n%s' % (error.cmd, error.returncode, assert_returncode, ret))

    return ret

  def assertExists(self, filename, msg=None):
    if not msg:
      msg = f'Expected file not found: {filename}'
    self.assertTrue(os.path.exists(filename), msg)

  def assertNotExists(self, filename, msg=None):
    if not msg:
      msg = 'Unexpected file exists: ' + filename
    self.assertFalse(os.path.exists(filename), msg)

  # Tests that the given two paths are identical, modulo path delimiters. E.g. "C:/foo" is equal to "C:\foo".
  def assertPathsIdentical(self, path1, path2):
    path1 = utils.normalize_path(path1)
    path2 = utils.normalize_path(path2)
    return self.assertIdentical(path1, path2)

  # Tests that the given two multiline text content are identical, modulo line
  # ending differences (\r\n on Windows, \n on Unix).
  def assertTextDataIdentical(self, text1, text2, msg=None,
                              fromfile='expected', tofile='actual'):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertIdentical(text1, text2, msg, fromfile, tofile)

  def assertIdentical(self, values, y, msg=None,
                      fromfile='expected', tofile='actual'):
    if type(values) not in (list, tuple):
      values = [values]
    for x in values:
      if x == y:
        return # success
    diff_lines = difflib.unified_diff(x.splitlines(), y.splitlines(),
                                      fromfile=fromfile, tofile=tofile)
    diff = ''.join([a.rstrip() + '\n' for a in diff_lines])
    if EMTEST_VERBOSE:
      print("Expected to have '%s' == '%s'" % (values[0], y))
    else:
      diff = limit_size(diff)
      diff += '\nFor full output run with --verbose.'
    fail_message = 'Unexpected difference:\n' + diff
    if msg:
      fail_message += '\n' + msg
    self.fail(fail_message)

  def assertTextDataContained(self, text1, text2):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertContained(text1, text2)

  def assertFileContents(self, filename, contents):
    if EMTEST_VERBOSE:
      print(f'Comparing results contents of file: {filename}')

    contents = contents.replace('\r', '')

    if EMTEST_REBASELINE:
      with open(filename, 'w') as f:
        f.write(contents)
      return

    if not os.path.exists(filename):
      self.fail('Test expectation file not found: ' + filename + '.\n' +
                'Run with --rebaseline to generate.')
    expected_content = read_file(filename)
    message = "Run with --rebaseline to automatically update expectations"
    self.assertTextDataIdentical(expected_content, contents, message,
                                 filename, filename + '.new')

  def assertContained(self, values, string, additional_info='', regex=False):
    if callable(string):
      string = string()

    if regex:
      if type(values) is str:
        self.assertTrue(re.search(values, string, re.DOTALL), 'Expected regex "%s" to match on:\n%s' % (values, limit_size(string)))
      else:
        match_any = any(re.search(o, string, re.DOTALL) for o in values)
        self.assertTrue(match_any, 'Expected at least one of "%s" to match on:\n%s' % (values, limit_size(string)))
      return

    if type(values) not in [list, tuple]:
      values = [values]

    if not any(v in string for v in values):
      diff = difflib.unified_diff(values[0].split('\n'), string.split('\n'), fromfile='expected', tofile='actual')
      diff = ''.join(a.rstrip() + '\n' for a in diff)
      self.fail("Expected to find '%s' in '%s', diff:\n\n%s\n%s" % (
        limit_size(values[0]), limit_size(string), limit_size(diff),
        additional_info,
      ))

  def assertNotContained(self, value, string):
    if callable(value):
      value = value() # lazy loading
    if callable(string):
      string = string()
    if value in string:
      self.fail("Expected to NOT find '%s' in '%s'" % (limit_size(value), limit_size(string)))

  def assertContainedIf(self, value, string, condition):
    if condition:
      self.assertContained(value, string)
    else:
      self.assertNotContained(value, string)

  def assertBinaryEqual(self, file1, file2):
    self.assertEqual(os.path.getsize(file1),
                     os.path.getsize(file2))
    self.assertEqual(read_binary(file1),
                     read_binary(file2))

  def get_build_dir(self):
    ret = self.in_dir('building')
    ensure_dir(ret)
    return ret

  def get_library(self, name, generated_libs, configure=['sh', './configure'],  # noqa
                  configure_args=None, make=None, make_args=None,
                  env_init=None, cache_name_extra='', native=False,
                  force_rebuild=False):
    if make is None:
      make = ['make']
    if env_init is None:
      env_init = {}
    if make_args is None:
      make_args = ['-j', str(utils.get_num_cores())]

    build_dir = self.get_build_dir()

    cflags = []
    if not native:
      # get_library() is used to compile libraries, and not link executables,
      # so we don't want to pass linker flags here (emscripten warns if you
      # try to pass linker settings when compiling).
      cflags = self.get_cflags(compile_only=True)

    hash_input = (str(cflags) + ' $ ' + str(env_init)).encode('utf-8')
    cache_name = name + ','.join([opt for opt in cflags if len(opt) < 7]) + '_' + hashlib.md5(hash_input).hexdigest() + cache_name_extra

    valid_chars = "_%s%s" % (string.ascii_letters, string.digits)
    cache_name = ''.join([(c if c in valid_chars else '_') for c in cache_name])

    if not force_rebuild and self.library_cache.get(cache_name):
      errlog('<load %s from cache> ' % cache_name)
      generated_libs = []
      for basename, contents in self.library_cache[cache_name]:
        bc_file = os.path.join(build_dir, cache_name + '_' + basename)
        write_binary(bc_file, contents)
        generated_libs.append(bc_file)
      return generated_libs

    errlog(f'<building and saving {cache_name} into cache>')
    if configure and configure_args:
      # Make to copy to avoid mutating default param
      configure = list(configure)
      configure += configure_args

    cflags = ' '.join(cflags)
    env_init.setdefault('CFLAGS', cflags)
    env_init.setdefault('CXXFLAGS', cflags)
    return self.build_library(name, build_dir, generated_libs, configure,
                              make, make_args, cache_name, env_init=env_init, native=native)

  def clear(self):
    force_delete_contents(self.get_dir())
    if shared.EMSCRIPTEN_TEMP_DIR:
      utils.delete_contents(shared.EMSCRIPTEN_TEMP_DIR)

  def run_process(self, cmd, check=True, **kwargs):
    # Wrapper around utils.run_process.  This is desirable so that the tests
    # can fail (in the unittest sense) rather than error'ing.
    # In the long run it would nice to completely remove the dependency on
    # core emscripten code (shared.py) here.

    # Handle buffering for subprocesses.  The python unittest buffering mechanism
    # will only buffer output from the current process (by overriding sys.stdout
    # and sys.stderr), not from sub-processes.
    stdout_buffering = 'stdout' not in kwargs and isinstance(sys.stdout, io.StringIO)
    stderr_buffering = 'stderr' not in kwargs and isinstance(sys.stderr, io.StringIO)
    if stdout_buffering:
      kwargs['stdout'] = PIPE
    if stderr_buffering:
      kwargs['stderr'] = PIPE

    try:
      rtn = utils.run_process(cmd, check=check, **kwargs)
    except subprocess.CalledProcessError as e:
      if check and e.returncode != 0:
        print(e.stdout)
        print(e.stderr)
        self.fail(f'subprocess exited with non-zero return code({e.returncode}): `{shlex.join(cmd)}`')

    if stdout_buffering:
      sys.stdout.write(rtn.stdout)
    if stderr_buffering:
      sys.stderr.write(rtn.stderr)
    return rtn

  def emcc(self, filename, args=[], **kwargs):  # noqa
    filename = maybe_test_file(filename)
    compile_only = '-c' in args or '-sSIDE_MODULE' in args
    cmd = [compiler_for(filename), filename] + self.get_cflags(compile_only=compile_only) + args
    self.run_process(cmd, **kwargs)

  # Shared test code between main suite and others

  def expect_fail(self, cmd, expect_traceback=False, **kwargs):
    """Run a subprocess and assert that it returns non-zero.

    Return the stderr of the subprocess.
    """
    proc = self.run_process(cmd, check=False, stderr=PIPE, **kwargs)
    self.assertNotEqual(proc.returncode, 0, 'subprocess unexpectedly succeeded. stderr:\n' + proc.stderr)
    # When we check for failure we expect a user-visible error, not a traceback.
    # However, on windows a python traceback can happen randomly sometimes,
    # due to "Access is denied" https://github.com/emscripten-core/emscripten/issues/718
    if expect_traceback:
      self.assertContained('Traceback', proc.stderr)
    elif not WINDOWS or 'Access is denied' not in proc.stderr:
      self.assertNotContained('Traceback', proc.stderr)
    if EMTEST_VERBOSE:
      sys.stderr.write(proc.stderr)
    return proc.stderr

  def assert_fail(self, cmd, expected, **kwargs):
    """Just like expect_fail, but also check for expected message in stderr.
    """
    err = self.expect_fail(cmd, **kwargs)
    self.assertContained(expected, err)
    return err

  # exercise dynamic linker.
  #
  # test that linking to shared library B, which is linked to A, loads A as well.
  # main is also linked to C, which is also linked to A. A is loaded/initialized only once.
  #
  #          B
  #   main <   > A
  #          C
  #
  # this test is used by both test_core and test_browser.
  # when run under browser it exercises how dynamic linker handles concurrency
  # - because B and C are loaded in parallel.
  def _test_dylink_dso_needed(self, do_run):
    create_file('liba.cpp', r'''
        #include <stdio.h>
        #include <emscripten.h>

        static const char *afunc_prev;

        extern "C" {
        EMSCRIPTEN_KEEPALIVE void afunc(const char *s);
        }

        void afunc(const char *s) {
          printf("a: %s (prev: %s)\n", s, afunc_prev);
          afunc_prev = s;
        }

        struct ainit {
          ainit() {
            puts("a: loaded");
          }
        };

        static ainit _;
      ''')

    create_file('libb.c', r'''
        #include <emscripten.h>

        void afunc(const char *s);

        EMSCRIPTEN_KEEPALIVE void bfunc() {
          afunc("b");
        }
      ''')

    create_file('libc.c', r'''
        #include <emscripten.h>

        void afunc(const char *s);

        EMSCRIPTEN_KEEPALIVE void cfunc() {
          afunc("c");
        }
      ''')

    def ccshared(src, linkto=None):
      cmdv = [EMCC, src, '-o', utils.unsuffixed(src) + '.wasm', '-sSIDE_MODULE'] + self.get_cflags()
      if linkto:
        cmdv += linkto
      self.run_process(cmdv)

    ccshared('liba.cpp')
    ccshared('libb.c', ['liba.wasm'])
    ccshared('libc.c', ['liba.wasm'])

    self.set_setting('MAIN_MODULE')
    extra_args = ['-L.', 'libb.wasm', 'libc.wasm']
    do_run(r'''
      #ifdef __cplusplus
      extern "C" {
      #endif
      void bfunc();
      void cfunc();
      #ifdef __cplusplus
      }
      #endif

      int test_main() {
        bfunc();
        cfunc();
        return 0;
      }
      ''',
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n', cflags=extra_args)

    extra_args = []
    for libname in ('liba.wasm', 'libb.wasm', 'libc.wasm'):
      extra_args += ['--embed-file', libname]
    do_run(r'''
      #include <assert.h>
      #include <dlfcn.h>
      #include <stddef.h>

      int test_main() {
        void *bdso, *cdso;
        void (*bfunc_ptr)(), (*cfunc_ptr)();

        // FIXME for RTLD_LOCAL binding symbols to loaded lib is not currently working
        bdso = dlopen("libb.wasm", RTLD_NOW|RTLD_GLOBAL);
        assert(bdso != NULL);
        cdso = dlopen("libc.wasm", RTLD_NOW|RTLD_GLOBAL);
        assert(cdso != NULL);

        bfunc_ptr = (void (*)())dlsym(bdso, "bfunc");
        assert(bfunc_ptr != NULL);
        cfunc_ptr = (void (*)())dlsym(cdso, "cfunc");
        assert(cfunc_ptr != NULL);

        bfunc_ptr();
        cfunc_ptr();
        return 0;
      }
    ''' % locals(),
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n', cflags=extra_args)

  def do_run(self, src, expected_output=None, force_c=False, **kwargs):
    if 'no_build' in kwargs:
      filename = src
    else:
      if force_c:
        filename = 'src.c'
      else:
        filename = 'src.cpp'
      create_file(filename, src)
    return self._build_and_run(filename, expected_output, **kwargs)

  def do_runf(self, filename, expected_output=None, **kwargs):
    return self._build_and_run(filename, expected_output, **kwargs)

  def do_run_in_out_file_test(self, srcfile, **kwargs):
    srcfile = maybe_test_file(srcfile)
    out_suffix = kwargs.pop('out_suffix', '')
    outfile = utils.unsuffixed(srcfile) + out_suffix + '.out'
    if EMTEST_REBASELINE:
      expected = None
    else:
      expected = read_file(outfile)
    output = self._build_and_run(srcfile, expected, **kwargs)
    if EMTEST_REBASELINE:
      utils.write_file(outfile, output)
    return output

  ## Does a complete test - builds, runs, checks output, etc.
  def _build_and_run(self, filename, expected_output, args=None,
                     no_build=False,
                     assert_returncode=0, assert_identical=False, assert_all=False,
                     check_for_error=True,
                     interleaved_output=True,
                     regex=False,
                     input=None,
                     **kwargs):
    logger.debug(f'_build_and_run: {filename}')

    if no_build:
      js_file = filename
    else:
      js_file = self.build(filename, **kwargs)
    self.assertExists(js_file)

    engines = self.js_engines.copy()
    if len(engines) > 1 and not self.use_all_engines:
      engines = engines[:1]
    # In standalone mode, also add wasm vms as we should be able to run there too.
    if self.get_setting('STANDALONE_WASM'):
      # TODO once standalone wasm support is more stable, apply use_all_engines
      # like with js engines, but for now as we bring it up, test in all of them
      if not self.wasm_engines:
        if 'EMTEST_SKIP_WASM_ENGINE' in os.environ:
          self.skipTest('no wasm engine was found to run the standalone part of this test')
        else:
          logger.warning('no wasm engine was found to run the standalone part of this test (Use EMTEST_SKIP_WASM_ENGINE to skip)')
      engines += self.wasm_engines
    if len(engines) == 0:
      self.fail('No JS engine present to run this test with. Check %s and the paths therein.' % config.EM_CONFIG)
    for engine in engines:
      js_output = self.run_js(js_file, engine, args,
                              input=input,
                              assert_returncode=assert_returncode,
                              interleaved_output=interleaved_output)
      js_output = js_output.replace('\r\n', '\n')
      if expected_output:
        if type(expected_output) not in [list, tuple]:
          expected_output = [expected_output]
        try:
          if assert_identical:
            self.assertIdentical(expected_output, js_output)
          elif assert_all or len(expected_output) == 1:
            for o in expected_output:
              self.assertContained(o, js_output, regex=regex)
          else:
            self.assertContained(expected_output, js_output, regex=regex)
            if assert_returncode == 0 and check_for_error:
              self.assertNotContained('ERROR', js_output)
        except self.failureException:
          print('(test did not pass in JS engine: %s)' % engine)
          raise
    return js_output

  def get_freetype_library(self):
    self.cflags += [
      '-Wno-misleading-indentation',
      '-Wno-unused-but-set-variable',
      '-Wno-pointer-bool-conversion',
      '-Wno-shift-negative-value',
      '-Wno-gnu-offsetof-extensions',
      # And because gnu-offsetof-extensions is a new warning:
      '-Wno-unknown-warning-option',
    ]
    return self.get_library(os.path.join('third_party', 'freetype'),
                            os.path.join('objs', '.libs', 'libfreetype.a'),
                            configure_args=['--disable-shared', '--without-zlib'])

  def get_poppler_library(self, env_init=None):
    freetype = self.get_freetype_library()

    self.cflags += [
      '-I' + test_file('third_party/freetype/include'),
      '-I' + test_file('third_party/poppler/include'),
      # Poppler's configure script emits -O2 for gcc, and nothing for other
      # compilers, including emcc, so set opts manually.
      "-O2",
    ]

    # Poppler has some pretty glaring warning.  Suppress them to keep the
    # test output readable.
    self.cflags += [
      '-Wno-sentinel',
      '-Wno-logical-not-parentheses',
      '-Wno-unused-private-field',
      '-Wno-tautological-compare',
      '-Wno-unknown-pragmas',
      '-Wno-shift-negative-value',
      '-Wno-dynamic-class-memaccess',
      # The fontconfig symbols are all missing from the poppler build
      # e.g. FcConfigSubstitute
      '-sERROR_ON_UNDEFINED_SYMBOLS=0',
      # Avoid warning about ERROR_ON_UNDEFINED_SYMBOLS being used at compile time
      '-Wno-unused-command-line-argument',
      '-Wno-js-compiler',
      '-Wno-nontrivial-memaccess',
    ]
    env_init = env_init.copy() if env_init else {}
    env_init['FONTCONFIG_CFLAGS'] = ' '
    env_init['FONTCONFIG_LIBS'] = ' '

    poppler = self.get_library(
        os.path.join('third_party', 'poppler'),
        [os.path.join('utils', 'pdftoppm.o'), os.path.join('utils', 'parseargs.o'), os.path.join('poppler', '.libs', 'libpoppler.a')],
        env_init=env_init,
        configure_args=['--disable-libjpeg', '--disable-libpng', '--disable-poppler-qt', '--disable-poppler-qt4', '--disable-cms', '--disable-cairo-output', '--disable-abiword-output', '--disable-shared', '--host=wasm32-emscripten'])

    return poppler + freetype

  def get_zlib_library(self, cmake, cflags=None):
    assert cmake or not WINDOWS, 'on windows, get_zlib_library only supports cmake'

    old_args = self.cflags.copy()
    if cflags:
      self.cflags += cflags
    # inflate.c does -1L << 16
    self.cflags.append('-Wno-shift-negative-value')
    # adler32.c uses K&R style function declarations
    self.cflags.append('-Wno-deprecated-non-prototype')
    # Work around configure-script error. TODO: remove when
    # https://github.com/emscripten-core/emscripten/issues/16908 is fixed
    self.cflags.append('-Wno-pointer-sign')
    if cmake:
      rtn = self.get_library(os.path.join('third_party', 'zlib'), os.path.join('libz.a'),
                             configure=['cmake', '.'],
                             make=['cmake', '--build', '.', '--'],
                             make_args=[])
    else:
      rtn = self.get_library(os.path.join('third_party', 'zlib'), os.path.join('libz.a'), make_args=['libz.a'])
    self.cflags = old_args
    return rtn

  def build_library(self, name, build_dir, generated_libs, configure, make, make_args, cache_name, env_init, native):
    """Build a library and cache the result.  We build the library file
    once and cache it for all our tests. (We cache in memory since the test
    directory is destroyed and recreated for each test. Note that we cache
    separately for different compilers).  This cache is just during the test
    runner. There is a different concept of caching as well, see |Cache|.
    """
    if type(generated_libs) is not list:
      generated_libs = [generated_libs]
    source_dir = test_file(name.replace('_native', ''))

    project_dir = Path(build_dir, name)
    if os.path.exists(project_dir):
      shutil.rmtree(project_dir)
    # Useful in debugging sometimes to comment this out, and two lines above
    shutil.copytree(source_dir, project_dir)

    generated_libs = [os.path.join(project_dir, lib) for lib in generated_libs]

    if native:
      env = clang_native.get_clang_native_env()
    else:
      env = os.environ.copy()
    env.update(env_init)

    if not native:
      # Inject emcmake, emconfigure or emmake accordingly, but only if we are
      # cross compiling.
      if configure:
        if configure[0] == 'cmake':
          configure = [EMCMAKE] + configure
        else:
          configure = [EMCONFIGURE] + configure
      else:
        make = [EMMAKE] + make

    if configure:
      self.run_process(configure, env=env, cwd=project_dir)
      # if we run configure or cmake we don't then need any kind
      # of special env when we run make below
      env = None

    def open_make_out(mode='r'):
      return open(os.path.join(project_dir, 'make.out'), mode)

    def open_make_err(mode='r'):
      return open(os.path.join(project_dir, 'make.err'), mode)

    if EMTEST_VERBOSE:
      # VERBOSE=1 is cmake and V=1 is for autoconf
      make_args += ['VERBOSE=1', 'V=1']

    self.run_process(make + make_args, env=env, cwd=project_dir)

    # Cache the result
    self.library_cache[cache_name] = []
    for f in generated_libs:
      basename = os.path.basename(f)
      self.library_cache[cache_name].append((basename, read_binary(f)))

    return generated_libs
