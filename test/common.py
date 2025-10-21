# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import difflib
import hashlib
import io
import itertools
import json
import logging
import os
import queue
import re
import shlex
import shutil
import stat
import string
import subprocess
import sys
import tempfile
import textwrap
import threading
import time
import unittest
import webbrowser
from enum import Enum
from functools import wraps
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from subprocess import PIPE, STDOUT
from typing import Dict, Tuple
from urllib.parse import parse_qs, unquote, unquote_plus, urlparse

import clang_native
import jsrun
import line_endings
import psutil
from retryable_unittest import RetryableTestCase

from tools import building, config, feature_matrix, shared, utils
from tools.settings import COMPILE_TIME_SETTINGS
from tools.shared import DEBUG, EMCC, EMXX, get_canonical_temp_dir, path_from_root
from tools.utils import (
  MACOS,
  WINDOWS,
  exit_with_error,
  read_binary,
  read_file,
  write_binary,
)

logger = logging.getLogger('common')

# User can specify an environment variable EMTEST_BROWSER to force the browser
# test suite to run using another browser command line than the default system
# browser. If only the path to the browser executable is given, the tests
# will run in headless mode with a temporary profile with the same options
# used in CI. To use a custom start command specify the executable and command
# line flags.
#
# Note that when specifying EMTEST_BROWSER to run tests on a Safari browser:
# the command line must point to the root of the app bundle, and not to the
# Safari executable inside the bundle. I.e. pass EMTEST_BROWSER=/Applications/Safari.app
# instead of EMTEST_BROWSER=/Applications/Safari.app/Contents/MacOS/Safari
#
# There are two special values that can be used here if running in an actual
# browser is not desired:
#  EMTEST_BROWSER=0 : This will disable the actual running of the test and simply
#                     verify that it compiles and links.
#  EMTEST_BROWSER=node : This will attempt to run the browser test under node.
#                        For most browser tests this does not work, but it can
#                        be useful for running pthread tests under node.
EMTEST_BROWSER = None
EMTEST_BROWSER_AUTO_CONFIG = None
EMTEST_HEADLESS = None
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

# Verbosity level control for subprocess calls to configure + make.
# 0: disabled.
# 1: Log stderr of configure/make.
# 2: Log stdout and stderr configure/make. Print out subprocess commands that were executed.
# 3: Log stdout and stderr, and pass VERBOSE=1 to CMake/configure/make steps.
EMTEST_BUILD_VERBOSE = int(os.getenv('EMTEST_BUILD_VERBOSE', '0'))
EMTEST_CAPTURE_STDIO = int(os.getenv('EMTEST_CAPTURE_STDIO', '0'))
if 'EM_BUILD_VERBOSE' in os.environ:
  exit_with_error('EM_BUILD_VERBOSE has been renamed to EMTEST_BUILD_VERBOSE')

# Triggers the browser to restart after every given number of tests.
# 0: Disabled (reuse the browser instance to run all tests. Default)
# 1: Restart a fresh browser instance for every browser test.
# 2,3,...: Restart a fresh browser instance after given number of tests have been run in it.
# Helps with e.g. https://bugzil.la/1992558
EMTEST_RESTART_BROWSER_EVERY_N_TESTS = int(os.getenv('EMTEST_RESTART_BROWSER_EVERY_N_TESTS', '0'))

# If we are drawing a parallel swimlane graph of test output, we need to use a temp
# file to track which tests were flaky so they can be graphed in orange color to
# visually stand out.
flaky_tests_log_filename = os.path.join(path_from_root('out/flaky_tests.txt'))
browser_spawn_lock_filename = os.path.join(path_from_root('out/browser_spawn_lock'))


# Default flags used to run browsers in CI testing:
class ChromeConfig:
  data_dir_flag = '--user-data-dir='
  default_flags = (
    # --no-sandbox because we are running as root and chrome requires
    # this flag for now: https://crbug.com/638180
    '--no-first-run -start-maximized --no-sandbox --enable-unsafe-swiftshader --use-gl=swiftshader --enable-experimental-web-platform-features --enable-features=JavaScriptSourcePhaseImports',
    '--enable-experimental-webassembly-features --js-flags="--experimental-wasm-type-reflection --experimental-wasm-rab-integration"',
    # The runners lack sound hardware so fallback to a dummy device (and
    # bypass the user gesture so audio tests work without interaction)
    '--use-fake-device-for-media-stream --autoplay-policy=no-user-gesture-required',
    # Cache options.
    '--disk-cache-size=1 --media-cache-size=1 --disable-application-cache',
    # Disable various background tasks downloads (e.g. updates).
    '--disable-background-networking',
    # Disable native password pop-ups
    '--password-store=basic',
    # Send console messages to browser stderr
    '--enable-logging=stderr',
  )
  headless_flags = '--headless=new --window-size=1024,768'

  @staticmethod
  def configure(data_dir):
    """Chrome has no special configuration step."""


class FirefoxConfig:
  data_dir_flag = '-profile '
  default_flags = ('-new-instance',)
  headless_flags = '-headless'
  executable_name = utils.exe_suffix('firefox')

  @staticmethod
  def configure(data_dir):
    shutil.copy(test_file('firefox_user.js'), os.path.join(data_dir, 'user.js'))


class SafariConfig:
  default_flags = ('', )
  executable_name = 'Safari'
  # For the macOS 'open' command, pass
  #   --new: to make a new Safari app be launched, rather than add a tab to an existing Safari process/window
  #   --fresh: do not restore old tabs (e.g. if user had old navigated windows open)
  #   --background: Open the new Safari window behind the current Terminal window, to make following the test run more pleasing (this is for convenience only)
  #   -a <exe_name>: The path to the executable to open, in this case Safari
  launch_prefix = ('open', '--new', '--fresh', '--background', '-a')

  @staticmethod
  def configure(data_dir):
    """ Safari has no special configuration step."""


# Special value for passing to assert_returncode which means we expect that program
# to fail with non-zero return code, but we don't care about specifically which one.
NON_ZERO = -1

TEST_ROOT = path_from_root('test')
LAST_TEST = path_from_root('out/last_test.txt')
PREVIOUS_TEST_RUN_RESULTS_FILE = path_from_root('out/previous_test_run_results.json')

DEFAULT_BROWSER_DATA_DIR = path_from_root('out/browser-profile')

WEBIDL_BINDER = utils.bat_suffix(path_from_root('tools/webidl_binder'))

EMBUILDER = utils.bat_suffix(path_from_root('embuilder'))
EMMAKE = utils.bat_suffix(path_from_root('emmake'))
EMCMAKE = utils.bat_suffix(path_from_root('emcmake'))
EMCONFIGURE = utils.bat_suffix(path_from_root('emconfigure'))
EMRUN = utils.bat_suffix(shared.path_from_root('emrun'))
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

requires_network = unittest.skipIf(os.getenv('EMTEST_SKIP_NETWORK_TESTS'), 'This test requires network access')


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


# checks if browser testing is enabled
def has_browser():
  return EMTEST_BROWSER != '0'


CHROMIUM_BASED_BROWSERS = ['chrom', 'edge', 'opera']


def is_chrome():
  return EMTEST_BROWSER and any(pattern in EMTEST_BROWSER.lower() for pattern in CHROMIUM_BASED_BROWSERS)


def is_firefox():
  return EMTEST_BROWSER and 'firefox' in EMTEST_BROWSER.lower()


def is_safari():
  return EMTEST_BROWSER and 'safari' in EMTEST_BROWSER.lower()


def get_browser_config():
  if is_chrome():
    return ChromeConfig()
  elif is_firefox():
    return FirefoxConfig()
  elif is_safari():
    return SafariConfig()
  return None


def compiler_for(filename, force_c=False):
  if utils.suffix(filename) in ('.cc', '.cxx', '.cpp') and not force_c:
    return EMXX
  else:
    return EMCC


# Generic decorator that calls a function named 'condition' on the test class and
# skips the test if that function returns true
def skip_if_simple(name, condition, note=''):
  assert callable(condition)
  assert not callable(note)

  def decorator(func):
    assert callable(func)

    @wraps(func)
    def decorated(self, *args, **kwargs):
      if condition(self):
        explanation_str = name
        if note:
          explanation_str += ': %s' % note
        self.skipTest(explanation_str)
      return func(self, *args, **kwargs)

    return decorated

  return decorator


# Same as skip_if_simple but creates a decorator that takes a note as an argument.
def skip_if(name, condition, default_note=''):
  assert callable(condition)

  def decorator(note=default_note):
    return skip_if_simple(name, condition, note)

  return decorator


def is_slow_test(func):
  assert callable(func)
  decorated = skip_if_simple('skipping slow tests', lambda _: EMTEST_SKIP_SLOW)(func)
  decorated.is_slow = True
  return decorated


def record_flaky_test(test_name, attempt_count, max_attempts, exception_msg):
  logging.info(f'Retrying flaky test "{test_name}" (attempt {attempt_count}/{max_attempts} failed):\n{exception_msg}')
  open(flaky_tests_log_filename, 'a').write(f'{test_name}\n')


def flaky(note=''):
  assert not callable(note)

  if EMTEST_SKIP_FLAKY:
    return unittest.skip(note)

  if not EMTEST_RETRY_FLAKY:
    return lambda f: f

  def decorated(func):
    @wraps(func)
    def modified(self, *args, **kwargs):
      # Browser tests have there own method of retrying tests.
      if self.is_browser_test():
        self.flaky = True
        return func(self, *args, **kwargs)

      for i in range(EMTEST_RETRY_FLAKY):
        try:
          return func(self, *args, **kwargs)
        except (AssertionError, subprocess.TimeoutExpired) as exc:
          preserved_exc = exc
          record_flaky_test(self.id(), i, EMTEST_RETRY_FLAKY, exc)

      raise AssertionError('Flaky test has failed too many times') from preserved_exc

    return modified

  return decorated


def disabled(note=''):
  assert not callable(note)
  return unittest.skip(note)


no_mac = skip_if('no_mac', lambda _: MACOS)

no_windows = skip_if('no_windows', lambda _: WINDOWS)

no_wasm64 = skip_if('no_wasm64', lambda t: t.is_wasm64())

# 2200mb is the value used by the core_2gb test mode
no_2gb = skip_if('no_2gb', lambda t: t.get_setting('INITIAL_MEMORY') == '2200mb')

no_4gb = skip_if('no_4gb', lambda t: t.is_4gb())

only_windows = skip_if('only_windows', lambda _: not WINDOWS)

requires_native_clang = skip_if_simple('native clang tests are disabled', lambda _: EMTEST_LACKS_NATIVE_CLANG)

needs_make = skip_if('tool not available on windows bots', lambda _: WINDOWS)


def requires_node(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_node()
    return func(self, *args, **kwargs)

  return decorated


def requires_node_canary(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_node_canary()
    return func(self, *args, **kwargs)

  return decorated


def node_bigint_flags(node_version):
  # The --experimental-wasm-bigint flag was added in v12, and then removed (enabled by default)
  # in v16.
  if node_version and node_version < (16, 0, 0):
    return ['--experimental-wasm-bigint']
  else:
    return []


# Used to mark dependencies in various tests to npm developer dependency
# packages, which might not be installed on Emscripten end users' systems.
def requires_dev_dependency(package):
  assert not callable(package)
  note = f'requires npm development package "{package}" and EMTEST_SKIP_NODE_DEV_PACKAGES is set'
  return skip_if_simple('requires_dev_dependency', lambda _: 'EMTEST_SKIP_NODE_DEV_PACKAGES' in os.environ, note)


def requires_wasm64(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_wasm64()
    return func(self, *args, **kwargs)

  return decorated


def requires_wasm_eh(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_wasm_eh()
    return func(self, *args, **kwargs)

  return decorated


def requires_v8(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_v8()
    return func(self, *args, **kwargs)

  return decorated


def requires_wasm2js(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_wasm2js()
    return func(self, *args, **kwargs)

  return decorated


def requires_jspi(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_jspi()
    return func(self, *args, **kwargs)

  return decorated


def node_pthreads(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.setup_node_pthreads()
    return func(self, *args, **kwargs)
  return decorated


def crossplatform(func):
  assert callable(func)
  func.is_crossplatform_test = True
  return func


# without EMTEST_ALL_ENGINES set we only run tests in a single VM by
# default. in some tests we know that cross-VM differences may happen and
# so are worth testing, and they should be marked with this decorator
def all_engines(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.use_all_engines = True
    self.set_setting('ENVIRONMENT', 'web,node,shell')
    return func(self, *args, **kwargs)

  return decorated


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


# Decorator version of env_modify
def with_env_modify(updates):
  assert not callable(updates)

  def decorated(func):
    @wraps(func)
    def modified(self, *args, **kwargs):
      with env_modify(updates):
        return func(self, *args, **kwargs)
    return modified

  return decorated


def also_with_wasmfs(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, wasmfs, *args, **kwargs):
    if DEBUG:
      print('parameterize:wasmfs=%d' % wasmfs)
    if wasmfs:
      self.setup_wasmfs_test()
    else:
      self.cflags += ['-DMEMFS']
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'wasmfs': (True,)})
  return metafunc


def also_with_nodefs(func):
  @wraps(func)
  def metafunc(self, fs, *args, **kwargs):
    if DEBUG:
      print('parameterize:fs=%s' % (fs))
    if fs == 'nodefs':
      self.setup_nodefs_test()
    else:
      self.cflags += ['-DMEMFS']
      assert fs is None
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (None,),
                          'nodefs': ('nodefs',)})
  return metafunc


def also_with_nodefs_both(func):
  @wraps(func)
  def metafunc(self, fs, *args, **kwargs):
    if DEBUG:
      print('parameterize:fs=%s' % (fs))
    if fs == 'nodefs':
      self.setup_nodefs_test()
    elif fs == 'rawfs':
      self.setup_noderawfs_test()
    else:
      self.cflags += ['-DMEMFS']
      assert fs is None
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (None,),
                          'nodefs': ('nodefs',),
                          'rawfs': ('rawfs',)})
  return metafunc


def with_all_fs(func):
  @wraps(func)
  def metafunc(self, wasmfs, fs, *args, **kwargs):
    if DEBUG:
      print('parameterize:fs=%s' % (fs))
    if wasmfs:
      self.setup_wasmfs_test()
    if fs == 'nodefs':
      self.setup_nodefs_test()
    elif fs == 'rawfs':
      self.setup_noderawfs_test()
    else:
      self.cflags += ['-DMEMFS']
      assert fs is None
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False, None),
                          'nodefs': (False, 'nodefs'),
                          'rawfs': (False, 'rawfs'),
                          'wasmfs': (True, None),
                          'wasmfs_nodefs': (True, 'nodefs'),
                          'wasmfs_rawfs': (True, 'rawfs')})
  return metafunc


def also_with_noderawfs(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, rawfs, *args, **kwargs):
    if DEBUG:
      print('parameterize:rawfs=%d' % rawfs)
    if rawfs:
      self.setup_noderawfs_test()
    else:
      self.cflags += ['-DMEMFS']
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'rawfs': (True,)})
  return metafunc


def also_with_minimal_runtime(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, with_minimal_runtime, *args, **kwargs):
    if DEBUG:
      print('parameterize:minimal_runtime=%s' % with_minimal_runtime)
    if self.get_setting('MINIMAL_RUNTIME'):
      self.skipTest('MINIMAL_RUNTIME already enabled in test config')
    if with_minimal_runtime:
      if self.get_setting('MODULARIZE') == 'instance' or self.get_setting('WASM_ESM_INTEGRATION'):
        self.skipTest('MODULARIZE=instance is not compatible with MINIMAL_RUNTIME')
      self.set_setting('MINIMAL_RUNTIME', 1)
      # This extra helper code is needed to cleanly handle calls to exit() which throw
      # an ExitCode exception.
      self.cflags += ['--pre-js', test_file('minimal_runtime_exit_handling.js')]
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'minimal_runtime': (True,)})
  return metafunc


def also_without_bigint(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, no_bigint, *args, **kwargs):
    if DEBUG:
      print('parameterize:no_bigint=%s' % no_bigint)
    if no_bigint:
      if self.get_setting('WASM_BIGINT') is not None:
        self.skipTest('redundant in bigint test config')
      self.set_setting('WASM_BIGINT', 0)
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'no_bigint': (True,)})
  return metafunc


def also_with_wasm64(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, with_wasm64, *args, **kwargs):
    if DEBUG:
      print('parameterize:wasm64=%s' % with_wasm64)
    if with_wasm64:
      self.require_wasm64()
      self.set_setting('MEMORY64')
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'wasm64': (True,)})
  return metafunc


def also_with_wasm2js(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, with_wasm2js, *args, **kwargs):
    assert self.get_setting('WASM') is None
    if DEBUG:
      print('parameterize:wasm2js=%s' % with_wasm2js)
    if with_wasm2js:
      self.require_wasm2js()
      self.set_setting('WASM', 0)
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'wasm2js': (True,)})
  return metafunc


def can_do_standalone(self, impure=False):
  # Pure standalone engines don't support MEMORY64 yet.  Even with MEMORY64=2 (lowered)
  # the WASI APIs that take pointer values don't have 64-bit variants yet.
  if not impure:
    if self.get_setting('MEMORY64'):
      return False
    # This is way to detect the core_2gb test mode in test_core.py
    if self.get_setting('INITIAL_MEMORY') == '2200mb':
      return False
  return self.is_wasm() and \
      self.get_setting('STACK_OVERFLOW_CHECK', 0) < 2 and \
      not self.get_setting('MINIMAL_RUNTIME') and \
      not self.get_setting('WASM_ESM_INTEGRATION') and \
      not self.get_setting('SAFE_HEAP') and \
      not any(a.startswith('-fsanitize=') for a in self.cflags)


# Impure means a test that cannot run in a wasm VM yet, as it is not 100%
# standalone. We can still run them with the JS code though.
def also_with_standalone_wasm(impure=False):
  def decorated(func):
    @wraps(func)
    def metafunc(self, standalone, *args, **kwargs):
      if DEBUG:
        print('parameterize:standalone=%s' % standalone)
      if standalone:
        if not can_do_standalone(self, impure):
          self.skipTest('Test configuration is not compatible with STANDALONE_WASM')
        self.set_setting('STANDALONE_WASM')
        if not impure:
          self.set_setting('PURE_WASI')
        self.cflags.append('-Wno-unused-command-line-argument')
        # if we are impure, disallow all wasm engines
        if impure:
          self.wasm_engines = []
      return func(self, *args, **kwargs)

    parameterize(metafunc, {'': (False,),
                            'standalone': (True,)})
    return metafunc

  return decorated


def also_with_asan(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, asan, *args, **kwargs):
    if asan:
      if self.is_wasm64():
        self.skipTest('TODO: ASAN in memory64')
      if self.is_2gb() or self.is_4gb():
        self.skipTest('asan doesnt support GLOBAL_BASE')
      self.cflags.append('-fsanitize=address')
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'asan': (True,)})
  return metafunc


def also_with_modularize(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, modularize, *args, **kwargs):
    if modularize:
      if self.get_setting('DECLARE_ASM_MODULE_EXPORTS') == 0:
        self.skipTest('DECLARE_ASM_MODULE_EXPORTS=0 is not compatible with MODULARIZE')
      if self.get_setting('STRICT_JS'):
        self.skipTest('MODULARIZE is not compatible with STRICT_JS')
      if self.get_setting('WASM_ESM_INTEGRATION'):
        self.skipTest('MODULARIZE is not compatible with WASM_ESM_INTEGRATION')
      self.cflags += ['--extern-post-js', test_file('modularize_post_js.js'), '-sMODULARIZE']
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'modularize': (True,)})
  return metafunc


# Tests exception handling and setjmp/longjmp handling. This tests three
# combinations:
# - Emscripten EH + Emscripten SjLj
# - Wasm EH + Wasm SjLj
# - Wasm EH + Wasm SjLj (Legacy)
def with_all_eh_sjlj(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, mode, *args, **kwargs):
    if DEBUG:
      print('parameterize:eh_mode=%s' % mode)
    if mode in {'wasm', 'wasm_legacy'}:
      # Wasm EH is currently supported only in wasm backend and V8
      if self.is_wasm2js():
        self.skipTest('wasm2js does not support wasm EH/SjLj')
      self.cflags.append('-fwasm-exceptions')
      self.set_setting('SUPPORT_LONGJMP', 'wasm')
      if mode == 'wasm':
        self.require_wasm_eh()
      if mode == 'wasm_legacy':
        self.require_wasm_legacy_eh()
    else:
      self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
      self.set_setting('SUPPORT_LONGJMP', 'emscripten')
      # DISABLE_EXCEPTION_CATCHING=0 exports __cxa_can_catch,
      # so if we don't build in C++ mode, wasm-ld will
      # error out because libc++abi is not included. See
      # https://github.com/emscripten-core/emscripten/pull/14192 for details.
      self.set_setting('DEFAULT_TO_CXX')
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'emscripten': ('emscripten',),
                          'wasm': ('wasm',),
                          'wasm_legacy': ('wasm_legacy',)})
  return metafunc


# This works just like `with_all_eh_sjlj` above but doesn't enable exceptions.
# Use this for tests that use setjmp/longjmp but not exceptions handling.
def with_all_sjlj(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, mode, *args, **kwargs):
    if mode in {'wasm', 'wasm_legacy'}:
      if self.is_wasm2js():
        self.skipTest('wasm2js does not support wasm SjLj')
      self.set_setting('SUPPORT_LONGJMP', 'wasm')
      if mode == 'wasm':
        self.require_wasm_eh()
      if mode == 'wasm_legacy':
        self.require_wasm_legacy_eh()
    else:
      self.set_setting('SUPPORT_LONGJMP', 'emscripten')
    return func(self, *args, **kwargs)

  parameterize(metafunc, {'emscripten': ('emscripten',),
                          'wasm': ('wasm',),
                          'wasm_legacy': ('wasm_legacy',)})
  return metafunc


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
    # occassionally a Firefox browser process can be left behind, holding on
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


def find_browser_test_file(filename):
  """Looks for files in test/browser and then in test/
  """
  if not os.path.exists(filename):
    fullname = test_file('browser', filename)
    if not os.path.exists(fullname):
      fullname = test_file(filename)
    filename = fullname
  return filename


def parameterize(func, parameters):
  """Add additional parameterization to a test function.

  This function create or adds to the `_parameterize` property of a function
  which is then expanded by the RunnerMeta metaclass into multiple separate
  test functions.
  """
  prev = getattr(func, '_parameterize', None)
  assert not any(p.startswith('_') for p in parameters)
  if prev:
    # If we're parameterizing 2nd time, construct a cartesian product for various combinations.
    func._parameterize = {
      '_'.join(filter(None, [k1, k2])): v2 + v1 for (k1, v1), (k2, v2) in itertools.product(prev.items(), parameters.items())
    }
  else:
    func._parameterize = parameters


def parameterized(parameters):
  """
  Mark a test as parameterized.

  Usage:
    @parameterized({
      'subtest1': (1, 2, 3),
      'subtest2': (4, 5, 6),
    })
    def test_something(self, a, b, c):
      ... # actual test body

  This is equivalent to defining two tests:

    def test_something_subtest1(self):
      # runs test_something(1, 2, 3)

    def test_something_subtest2(self):
      # runs test_something(4, 5, 6)
  """
  def decorator(func):
    parameterize(func, parameters)
    return func
  return decorator


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
    if self.try_require_node_version(24):
      self.node_args.append('--experimental-wasm-exnref')
      return

    if self.is_browser_test():
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

    exp_args = ['--experimental-wasm-stack-switching', '--experimental-wasm-type-reflection']
    # Support for JSPI came earlier than 22, but the new API changes require v24
    if self.try_require_node_version(24):
      self.node_args += exp_args
      return

    v8 = self.get_v8()
    if v8:
      self.cflags.append('-sENVIRONMENT=shell')
      self.js_engines = [v8]
      self.v8_args += exp_args
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
    # Also, include backtrace for all uncuaght exceptions (not just Error).
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

      if EMTEST_DETECT_TEMPFILE_LEAKS and not DEBUG:
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
      return f.startswith('-l') or any(f.startswith(s) for s in ['-sEXPORT_ES6', '--proxy-to-worker', '-sGL_TESTING', '-sPROXY_TO_WORKER', '-sPROXY_TO_PTHREAD', '-sENVIRONMENT=', '--pre-js=', '--post-js=', '-sPTHREAD_POOL_SIZE='])

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
      shared.run_process(es_check + ['es5', inputfile], stdout=PIPE, stderr=STDOUT, env=es_check_env)
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
    """Cleaup the JS output prior to running verification steps on it.

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

  library_cache: Dict[str, Tuple[str, object]] = {}

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
    return build_library(name, build_dir, generated_libs, configure,
                         make, make_args, self.library_cache,
                         cache_name, env_init=env_init, native=native)

  def clear(self):
    force_delete_contents(self.get_dir())
    if shared.EMSCRIPTEN_TEMP_DIR:
      utils.delete_contents(shared.EMSCRIPTEN_TEMP_DIR)

  def run_process(self, cmd, check=True, **kwargs):
    # Wrapper around shared.run_process.  This is desirable so that the tests
    # can fail (in the unittest sense) rather than error'ing.
    # In the long run it would nice to completely remove the dependency on
    # core emscripten code (shared.py) here.

    # Handle buffering for subprocesses.  The python unittest buffering mechanism
    # will only buffer output from the current process (by overwriding sys.stdout
    # and sys.stderr), not from sub-processes.
    stdout_buffering = 'stdout' not in kwargs and isinstance(sys.stdout, io.StringIO)
    stderr_buffering = 'stderr' not in kwargs and isinstance(sys.stderr, io.StringIO)
    if stdout_buffering:
      kwargs['stdout'] = PIPE
    if stderr_buffering:
      kwargs['stderr'] = PIPE

    try:
      rtn = shared.run_process(cmd, check=check, **kwargs)
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

  def emcc(self, filename, args=[], output_filename=None, **kwargs):  # noqa
    filename = maybe_test_file(filename)
    compile_only = '-c' in args or '-sSIDE_MODULE' in args
    cmd = [compiler_for(filename), filename] + self.get_cflags(compile_only=compile_only) + args
    if output_filename:
      cmd += ['-o', output_filename]
    self.run_process(cmd, **kwargs)

  # Shared test code between main suite and others

  def expect_fail(self, cmd, expect_traceback=False, **args):
    """Run a subprocess and assert that it returns non-zero.

    Return the stderr of the subprocess.
    """
    proc = self.run_process(cmd, check=False, stderr=PIPE, **args)
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

  # excercise dynamic linker.
  #
  # test that linking to shared library B, which is linked to A, loads A as well.
  # main is also linked to C, which is also linked to A. A is loaded/initialized only once.
  #
  #          B
  #   main <   > A
  #          C
  #
  # this test is used by both test_core and test_browser.
  # when run under browser it excercises how dynamic linker handles concurrency
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

    # _test_dylink_dso_needed can be potentially called several times by a test.
    # reset dylink-related options first.
    self.clear_setting('MAIN_MODULE')
    self.clear_setting('SIDE_MODULE')

    # XXX in wasm each lib load currently takes 5MB; default INITIAL_MEMORY=16MB is thus not enough
    if not self.has_changed_setting('INITIAL_MEMORY'):
      self.set_setting('INITIAL_MEMORY', '32mb')

    so = '.wasm' if self.is_wasm() else '.js'

    def ccshared(src, linkto=None):
      cmdv = [EMCC, src, '-o', utils.unsuffixed(src) + so, '-sSIDE_MODULE'] + self.get_cflags()
      if linkto:
        cmdv += linkto
      self.run_process(cmdv)

    ccshared('liba.cpp')
    ccshared('libb.c', ['liba' + so])
    ccshared('libc.c', ['liba' + so])

    self.set_setting('MAIN_MODULE')
    extra_args = ['-L.', 'libb' + so, 'libc' + so]
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
    for libname in ('liba', 'libb', 'libc'):
      extra_args += ['--embed-file', libname + so]
    do_run(r'''
      #include <assert.h>
      #include <dlfcn.h>
      #include <stddef.h>

      int test_main() {
        void *bdso, *cdso;
        void (*bfunc_ptr)(), (*cfunc_ptr)();

        // FIXME for RTLD_LOCAL binding symbols to loaded lib is not currently working
        bdso = dlopen("libb%(so)s", RTLD_NOW|RTLD_GLOBAL);
        assert(bdso != NULL);
        cdso = dlopen("libc%(so)s", RTLD_NOW|RTLD_GLOBAL);
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
    # adler32.c uses K&R sytyle function declarations
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


# Create a server and a web page. When a test runs, we tell the server about it,
# which tells the web page, which then opens a window with the test. Doing
# it this way then allows the page to close() itself when done.
def make_test_server(in_queue, out_queue, port):
  class TestServerHandler(SimpleHTTPRequestHandler):
    # Request header handler for default do_GET() path in
    # SimpleHTTPRequestHandler.do_GET(self) below.
    def send_head(self):
      if self.headers.get('Range'):
        path = self.translate_path(self.path)
        try:
          fsize = os.path.getsize(path)
          f = open(path, 'rb')
        except OSError:
          self.send_error(404, f'File not found {path}')
          return None
        self.send_response(206)
        ctype = self.guess_type(path)
        self.send_header('Content-Type', ctype)
        pieces = self.headers.get('Range').split('=')[1].split('-')
        start = int(pieces[0]) if pieces[0] != '' else 0
        end = int(pieces[1]) if pieces[1] != '' else fsize - 1
        end = min(fsize - 1, end)
        length = end - start + 1
        self.send_header('Content-Range', f'bytes {start}-{end}/{fsize}')
        self.send_header('Content-Length', str(length))
        self.end_headers()
        return f
      else:
        return SimpleHTTPRequestHandler.send_head(self)

    # Add COOP, COEP, CORP, and no-caching headers
    def end_headers(self):
      self.send_header('Accept-Ranges', 'bytes')
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
      self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
      self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')

      self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate, private, max-age=0')
      self.send_header('Expires', '0')
      self.send_header('Pragma', 'no-cache')
      self.send_header('Vary', '*') # Safari insists on caching if this header is not present in addition to the above

      return SimpleHTTPRequestHandler.end_headers(self)

    def do_POST(self):  # noqa: DC04
      urlinfo = urlparse(self.path)
      query = parse_qs(urlinfo.query)
      content_length = int(self.headers['Content-Length'])
      post_data = self.rfile.read(content_length)
      if urlinfo.path == '/log':
        # Logging reported by reportStdoutToServer / reportStderrToServer.
        #
        # To automatically capture stderr/stdout message from browser tests, modify
        # `captureStdoutStderr` in `test/browser_reporting.js`.
        filename = query['file'][0]
        print(f"[client {filename}: '{post_data.decode()}']")
        self.send_response(200)
        self.end_headers()
      elif urlinfo.path == '/upload':
        filename = query['file'][0]
        print(f'do_POST: got file: {filename}')
        create_file(filename, post_data, binary=True)
        self.send_response(200)
        self.end_headers()
      elif urlinfo.path.startswith('/status/'):
        code_str = urlinfo.path[len('/status/'):]
        code = int(code_str)
        if code in (301, 302, 303, 307, 308):
          self.send_response(code)
          self.send_header('Location', '/status/200')
          self.end_headers()
        elif code == 200:
          self.send_response(200)
          self.send_header('Content-type', 'text/plain')
          self.end_headers()
          self.wfile.write(b'OK')
        else:
          self.send_error(400, f'Not implemented for {code}')
      else:
        print(f'do_POST: unexpected POST: {urlinfo}')

    def do_GET(self):
      info = urlparse(self.path)
      if info.path == '/run_harness':
        if DEBUG:
          print('[server startup]')
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(read_binary(test_file('browser_harness.html')))
      elif info.path.startswith('/status/'):
        code_str = info.path[len('/status/'):]
        code = int(code_str)
        if code in (301, 302, 303, 307, 308):
          # Redirect to /status/200
          self.send_response(code)
          self.send_header('Location', '/status/200')
          self.end_headers()
        elif code == 200:
          self.send_response(200)
          self.send_header('Content-type', 'text/plain')
          self.end_headers()
          self.wfile.write(b'OK')
        else:
          self.send_error(400, f'Not implemented for {code}')
      elif 'report_' in self.path:
        # for debugging, tests may encode the result and their own url (window.location) as result|url
        if '|' in self.path:
          path, url = self.path.split('|', 1)
        else:
          path = self.path
          url = '?'
        if DEBUG:
          print('[server response:', path, url, ']')
        if out_queue.empty():
          out_queue.put(path)
        else:
          # a badly-behaving test may send multiple xhrs with reported results; we just care
          # about the first (if we queued the others, they might be read as responses for
          # later tests, or maybe the test sends more than one in a racy manner).
          # we place 'None' in the queue here so that the outside knows something went wrong
          # (none is not a valid value otherwise; and we need the outside to know because if we
          # raise an error in here, it is just swallowed in python's webserver code - we want
          # the test to actually fail, which a webserver response can't do).
          out_queue.put(None)
          raise Exception('browser harness error, excessive response to server - test must be fixed! "%s"' % self.path)
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.send_header('Connection', 'close')
        self.end_headers()
        self.wfile.write(b'OK')

      elif info.path == '/check':
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        if not in_queue.empty():
          # there is a new test ready to be served
          url, dir = in_queue.get()
          if DEBUG:
            print('[queue command:', url, dir, ']')
          assert in_queue.empty(), 'should not be any blockage - one test runs at a time'
          assert out_queue.empty(), 'the single response from the last test was read'
          # tell the browser to load the test
          self.wfile.write(b'COMMAND:' + url.encode('utf-8'))
        else:
          # the browser must keep polling
          self.wfile.write(b'(wait)')
      else:
        # Use SimpleHTTPServer default file serving operation for GET.
        if DEBUG:
          print('[simple HTTP serving:', unquote_plus(self.path), ']')
        if self.headers.get('Range'):
          self.send_response(206)
          path = self.translate_path(self.path)
          data = read_binary(path)
          ctype = self.guess_type(path)
          self.send_header('Content-type', ctype)
          pieces = self.headers.get('Range').split('=')[1].split('-')
          start = int(pieces[0]) if pieces[0] != '' else 0
          end = int(pieces[1]) if pieces[1] != '' else len(data) - 1
          end = min(len(data) - 1, end)
          length = end - start + 1
          self.send_header('Content-Length', str(length))
          self.send_header('Content-Range', f'bytes {start}-{end}/{len(data)}')
          self.end_headers()
          self.wfile.write(data[start:end + 1])
        else:
          SimpleHTTPRequestHandler.do_GET(self)

    def log_request(code=0, size=0):
      # don't log; too noisy
      pass

  # allows streaming compilation to work
  SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'
  # Firefox browser security does not allow loading .mjs files if they
  # do not have the correct MIME type
  SimpleHTTPRequestHandler.extensions_map['.mjs'] = 'text/javascript'

  return ThreadingHTTPServer(('localhost', port), TestServerHandler)


class HttpServerThread(threading.Thread):
  """A generic thread class to create and run an http server."""
  def __init__(self, server):
    super().__init__()
    self.server = server

  def stop(self):
    """Shuts down the server if it is running."""
    self.server.shutdown()

  def run(self):
    """Creates the server instance and serves forever until stop() is called."""
    # Start the server's main loop (this blocks until shutdown() is called)
    self.server.serve_forever()


class Reporting(Enum):
  """When running browser tests we normally automatically include support
  code for reporting results back to the browser.  This enum allows tests
  to decide what type of support code they need/want.
  """
  NONE = 0
  # Include the JS helpers for reporting results
  JS_ONLY = 1
  # Include C/C++ reporting code (REPORT_RESULT macros) as well as JS helpers
  FULL = 2


# This will hold the ID for each worker process if running in parallel mode,
# otherwise None if running in non-parallel mode.
worker_id = None


def init_worker(counter, lock):
  """ Initializer function for each worker.
  It acquires a lock, gets a unique ID from the shared counter,
  and stores it in a global variable specific to this worker process.
  """
  global worker_id
  with lock:
    # Get the next available ID
    worker_id = counter.value
    # Increment the counter for the next worker
    counter.value += 1


def configure_test_browser():
  global EMTEST_BROWSER

  if not has_browser():
    return

  if not EMTEST_BROWSER:
    EMTEST_BROWSER = 'google-chrome'

  if WINDOWS and '"' not in EMTEST_BROWSER and "'" not in EMTEST_BROWSER:
    # On Windows env. vars canonically use backslashes as directory delimiters, e.g.
    # set EMTEST_BROWSER=C:\Program Files\Mozilla Firefox\firefox.exe
    # and spaces are not escaped. But make sure to also support args, e.g.
    # set EMTEST_BROWSER="C:\Users\clb\AppData\Local\Google\Chrome SxS\Application\chrome.exe" --enable-unsafe-webgpu
    EMTEST_BROWSER = '"' + EMTEST_BROWSER.replace("\\", "\\\\") + '"'

  if EMTEST_BROWSER_AUTO_CONFIG:
    config = get_browser_config()
    if config:
      EMTEST_BROWSER += ' ' + ' '.join(config.default_flags)
      if EMTEST_HEADLESS == 1:
        EMTEST_BROWSER += f" {config.headless_flags}"


def list_processes_by_name(exe_name):
  pids = []
  if exe_name:
    for proc in psutil.process_iter():
      try:
        pinfo = proc.as_dict(attrs=['pid', 'name', 'exe'])
        if pinfo['exe'] and exe_name in pinfo['exe'].replace('\\', '/').split('/'):
          pids.append(psutil.Process(pinfo['pid']))
      except psutil.NoSuchProcess: # E.g. "process no longer exists (pid=13132)" (code raced to acquire the iterator and process it)
        pass

  return pids


def terminate_list_of_processes(proc_list):
  for proc in proc_list:
    try:
      proc.terminate()
      # If the browser doesn't shut down gracefully (in response to SIGTERM)
      # after 2 seconds kill it with force (SIGKILL).
      try:
        proc.wait(2)
      except (subprocess.TimeoutExpired, psutil.TimeoutExpired):
        logger.info('Browser did not respond to `terminate`.  Using `kill`')
        proc.kill()
        proc.wait()
    except (psutil.NoSuchProcess, ProcessLookupError):
      pass


class FileLock:
  """Implements a filesystem-based mutex, with an additional feature that it
  returns an integer counter denoting how many times the lock has been locked
  before (during the current python test run instance)"""
  def __init__(self, path):
    self.path = path
    self.counter = 0

  def __enter__(self):
    # Acquire the lock
    while True:
      try:
        self.fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        break
      except FileExistsError:
        time.sleep(0.1)
    # Return the locking count number
    try:
      self.counter = int(open(f'{self.path}_counter').read())
    except Exception:
      pass
    return self.counter

  def __exit__(self, *a):
    # Increment locking count number before releasing the lock
    with open(f'{self.path}_counter', 'w') as f:
      f.write(str(self.counter + 1))
    # And release the lock
    os.close(self.fd)
    try:
      os.remove(self.path)
    except Exception:
      pass # Another process has raced to acquire the lock, and will delete it.


def move_browser_window(pid, x, y):
  """Utility function to move the top-level window owned by given process to
  (x,y) coordinate. Used to ensure each browser window has some visible area."""
  import win32con
  import win32gui
  import win32process

  def enum_windows_callback(hwnd, _unused):
    _, win_pid = win32process.GetWindowThreadProcessId(hwnd)
    if win_pid == pid and win32gui.IsWindowVisible(hwnd):
      # If the browser window is maximized, it won't react to MoveWindow, so
      # un-maximize the window first to show it in windowed mode.
      if win32gui.GetWindowPlacement(hwnd)[1] == win32con.SW_SHOWMAXIMIZED:
        win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)

      # Then cascade the window, but also resize the window size to cover a
      # smaller area of the desktop, in case the original size was full screen.
      win32gui.MoveWindow(hwnd, x, y, 800, 600, True)
    return True

  win32gui.EnumWindows(enum_windows_callback, None)


def increment_suffix_number(str_with_maybe_suffix):
  match = re.match(r"^(.*?)(?:_(\d+))?$", str_with_maybe_suffix)
  if match:
    base, number = match.groups()
    if number:
      return f'{base}_{int(number) + 1}'

  return f'{str_with_maybe_suffix}_1'


class BrowserCore(RunnerCore):
  # note how many tests hang / do not send an output. if many of these
  # happen, likely something is broken and it is best to abort the test
  # suite early, as otherwise we will wait for the timeout on every
  # single test (hundreds of minutes)
  MAX_UNRESPONSIVE_TESTS = 10
  BROWSER_TIMEOUT = 60

  unresponsive_tests = 0
  num_tests_ran = 0

  def __init__(self, *args, **kwargs):
    self.capture_stdio = EMTEST_CAPTURE_STDIO
    super().__init__(*args, **kwargs)

  @classmethod
  def browser_terminate(cls):
    terminate_list_of_processes(cls.browser_procs)

  @classmethod
  def browser_restart(cls):
    # Kill existing browser
    assert has_browser()
    logger.info('Restarting browser process')
    cls.browser_terminate()
    cls.browser_open(cls.HARNESS_URL)
    BrowserCore.num_tests_ran = 0

  @classmethod
  def browser_open(cls, url):
    assert has_browser()
    browser_args = EMTEST_BROWSER
    parallel_harness = worker_id is not None

    config = get_browser_config()
    if not config and EMTEST_BROWSER_AUTO_CONFIG:
      exit_with_error(f'EMTEST_BROWSER_AUTO_CONFIG only currently works with firefox, chrome and safari. EMTEST_BROWSER was "{EMTEST_BROWSER}"')

    # Prepare the browser data directory, if it uses one.
    if EMTEST_BROWSER_AUTO_CONFIG and config and hasattr(config, 'data_dir_flag'):
      logger.info('Using default CI configuration.')
      browser_data_dir = DEFAULT_BROWSER_DATA_DIR
      if parallel_harness:
        # Running in parallel mode, give each browser its own profile dir.
        browser_data_dir += '-' + str(worker_id)

      # Delete old browser data directory.
      if WINDOWS:
        # If we cannot (the data dir is in use on Windows), switch to another dir.
        while not force_delete_dir(browser_data_dir):
          browser_data_dir = increment_suffix_number(browser_data_dir)
      else:
        force_delete_dir(browser_data_dir)

      # Recreate the new data directory.
      os.mkdir(browser_data_dir)

      if WINDOWS:
        # Escape directory delimiter backslashes for shlex.split.
        browser_data_dir = browser_data_dir.replace('\\', '\\\\')
      config.configure(browser_data_dir)
      browser_args += f' {config.data_dir_flag}"{browser_data_dir}"'

    browser_args = shlex.split(browser_args)
    if hasattr(config, 'launch_prefix'):
      browser_args = list(config.launch_prefix) + browser_args

    logger.info('Launching browser: %s', str(browser_args))

    if (WINDOWS and is_firefox()) or is_safari():
      cls.launch_browser_harness_with_proc_snapshot_workaround(parallel_harness, config, browser_args, url)
    else:
      cls.browser_procs = [subprocess.Popen(browser_args + [url])]

  @classmethod
  def launch_browser_harness_with_proc_snapshot_workaround(cls, parallel_harness, config, browser_args, url):
    ''' Dedicated function for launching browser harness in scenarios where
    we need to identify the launched browser processes via a before-after
    subprocess snapshotting delta workaround.'''

    # In order for this to work, each browser needs to be launched one at a time
    # so that we know which process belongs to which browser.
    with FileLock(browser_spawn_lock_filename) as count:
      # Take a snapshot before spawning the browser to find which processes
      # existed before launching the browser.
      if parallel_harness or is_safari():
        procs_before = list_processes_by_name(config.executable_name)

      # Browser launch
      cls.browser_procs = [subprocess.Popen(browser_args + [url])]

      # Give the browser time to spawn its subprocesses. Use an increasing
      # timeout as a crude way to account for system load.
      if parallel_harness or is_safari():
        time.sleep(min(2 + count * 0.3, 10))
        procs_after = list_processes_by_name(config.executable_name)

        # Take a snapshot again to find which processes exist after launching
        # the browser. Then the newly launched browser processes are determined
        # by the delta before->after.
        cls.browser_procs = list(set(procs_after).difference(set(procs_before)))
        if len(cls.browser_procs) == 0:
          exit_with_error('Could not detect the launched browser subprocesses. The test harness will not be able to close the browser after testing is done, so aborting the test run here.')

      # Firefox on Windows quirk:
      # Make sure that each browser window is visible on the desktop. Otherwise
      # browser might decide that the tab is backgrounded, and not load a test,
      # or it might not tick rAF()s forward, causing tests to hang.
      if WINDOWS and parallel_harness and not EMTEST_HEADLESS:
        # Wrap window positions on a Full HD desktop area modulo primes.
        for proc in cls.browser_procs:
          move_browser_window(proc.pid, (300 + count * 47) % 1901, (10 + count * 37) % 997)

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    cls.PORT = 8888 + (0 if worker_id is None else worker_id)
    cls.SERVER_URL = f'http://localhost:{cls.PORT}'
    cls.HARNESS_URL = f'{cls.SERVER_URL}/run_harness'

    if not has_browser() or EMTEST_BROWSER == 'node':
      errlog(f'[Skipping browser launch (EMTEST_BROWSER={EMTEST_BROWSER})]')
      return

    cls.harness_in_queue = queue.Queue()
    cls.harness_out_queue = queue.Queue()
    cls.harness_server = HttpServerThread(make_test_server(cls.harness_in_queue, cls.harness_out_queue, cls.PORT))
    cls.harness_server.start()

    errlog(f'[Browser harness server on thread {cls.harness_server.name}]')
    cls.browser_open(cls.HARNESS_URL)

  @classmethod
  def tearDownClass(cls):
    super().tearDownClass()
    if not has_browser() or EMTEST_BROWSER == 'node':
      return
    cls.harness_server.stop()
    cls.harness_server.join()
    cls.browser_terminate()

    if WINDOWS:
      # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
      # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
      time.sleep(0.1)

  def is_browser_test(self):
    return True

  def add_browser_reporting(self):
    contents = read_file(test_file('browser_reporting.js'))
    contents = contents.replace('{{{REPORTING_URL}}}', self.SERVER_URL)
    create_file('browser_reporting.js', contents)

  def assert_out_queue_empty(self, who):
    if not self.harness_out_queue.empty():
      responses = []
      while not self.harness_out_queue.empty():
        responses += [self.harness_out_queue.get()]
      raise Exception('excessive responses from %s: %s' % (who, '\n'.join(responses)))

  # @param extra_tries: how many more times to try this test, if it fails. browser tests have
  #                     many more causes of flakiness (in particular, they do not run
  #                     synchronously, so we have a timeout, which can be hit if the VM
  #                     we run on stalls temporarily).
  def run_browser(self, html_file, expected=None, message=None, timeout=None, extra_tries=None):
    if not has_browser():
      return
    assert '?' not in html_file, 'URL params not supported'
    if extra_tries is None:
      extra_tries = EMTEST_RETRY_FLAKY if self.flaky else 0
    url = html_file
    if self.capture_stdio:
      url += '?capture_stdio'
    if self.skip_exec:
      self.skipTest('skipping test execution: ' + self.skip_exec)
    if BrowserCore.unresponsive_tests >= BrowserCore.MAX_UNRESPONSIVE_TESTS:
      self.skipTest('too many unresponsive tests, skipping remaining tests')

    if EMTEST_RESTART_BROWSER_EVERY_N_TESTS and BrowserCore.num_tests_ran >= EMTEST_RESTART_BROWSER_EVERY_N_TESTS:
      logger.warning(f'[EMTEST_RESTART_BROWSER_EVERY_N_TESTS={EMTEST_RESTART_BROWSER_EVERY_N_TESTS} workaround: restarting browser]')
      self.browser_restart()
    BrowserCore.num_tests_ran += 1

    self.assert_out_queue_empty('previous test')
    if DEBUG:
      print('[browser launch:', html_file, ']')
    assert not (message and expected), 'run_browser expects `expected` or `message`, but not both'

    if expected is not None:
      try:
        self.harness_in_queue.put((
          'http://localhost:%s/%s' % (self.PORT, url),
          self.get_dir(),
        ))
        if timeout is None:
          timeout = self.BROWSER_TIMEOUT
        try:
          output = self.harness_out_queue.get(block=True, timeout=timeout)
        except queue.Empty:
          BrowserCore.unresponsive_tests += 1
          print(f'[unresponsive test: {self.id()} total unresponsive={str(BrowserCore.unresponsive_tests)}]')
          self.browser_restart()
          # Rather than fail the test here, let fail on the `assertContained` so
          # that the test can be retried via `extra_tries`
          output = '[no http server activity]'
        if output is None:
          # the browser harness reported an error already, and sent a None to tell
          # us to also fail the test
          self.fail('browser harness error')
        output = unquote(output)
        if output.startswith('/report_result?skipped:'):
          self.skipTest(unquote(output[len('/report_result?skipped:'):]).strip())
        else:
          # verify the result, and try again if we should do so
          try:
            self.assertContained(expected, output)
          except self.failureException as e:
            if extra_tries > 0:
              record_flaky_test(self.id(), EMTEST_RETRY_FLAKY - extra_tries, EMTEST_RETRY_FLAKY, e)
              if not self.capture_stdio:
                print('[enabling stdio/stderr reporting]')
                self.capture_stdio = True
              return self.run_browser(html_file, expected, message, timeout, extra_tries - 1)
            else:
              raise e
      finally:
        time.sleep(0.1) # see comment about Windows above
      self.assert_out_queue_empty('this test')
    else:
      webbrowser.open_new(os.path.abspath(html_file))
      print('A web browser window should have opened a page containing the results of a part of this test.')
      print('You need to manually look at the page to see that it works ok: ' + message)
      print('(sleeping for a bit to keep the directory alive for the web browser..)')
      time.sleep(5)
      print('(moving on..)')

  def compile_btest(self, filename, cflags, reporting=Reporting.FULL):
    # Inject support code for reporting results. This adds an include a header so testcases can
    # use REPORT_RESULT, and also adds a cpp file to be compiled alongside the testcase, which
    # contains the implementation of REPORT_RESULT (we can't just include that implementation in
    # the header as there may be multiple files being compiled here).
    if reporting != Reporting.NONE:
      # For basic reporting we inject JS helper funtions to report result back to server.
      self.add_browser_reporting()
      cflags += ['--pre-js', 'browser_reporting.js']
      if reporting == Reporting.FULL:
        # If C reporting (i.e. the REPORT_RESULT macro) is required we
        # also include report_result.c and force-include report_result.h
        self.run_process([EMCC, '-c', '-I' + TEST_ROOT,
                          test_file('report_result.c')] + self.get_cflags(compile_only=True) + (['-fPIC'] if '-fPIC' in cflags else []))
        cflags += ['report_result.o', '-include', test_file('report_result.h')]
    if EMTEST_BROWSER == 'node':
      cflags.append('-DEMTEST_NODE')
    filename = maybe_test_file(filename)
    self.run_process([compiler_for(filename), filename] + self.get_cflags() + cflags)
    # Remove the file since some tests have assertions for how many files are in
    # the output directory.
    utils.delete_file('browser_reporting.js')

  def btest_exit(self, filename, assert_returncode=0, *args, **kwargs):
    """Special case of `btest` that reports its result solely via exiting
    with a given result code.

    In this case we set EXIT_RUNTIME and we don't need to provide the
    REPORT_RESULT macro to the C code.
    """
    self.set_setting('EXIT_RUNTIME')
    assert 'reporting' not in kwargs
    assert 'expected' not in kwargs
    kwargs['reporting'] = Reporting.JS_ONLY
    kwargs['expected'] = 'exit:%d' % assert_returncode
    return self.btest(filename, *args, **kwargs)

  def btest(self, filename, expected=None,
            post_build=None,
            cflags=None,
            timeout=None,
            reporting=Reporting.FULL,
            output_basename='test'):
    assert expected, 'a btest must have an expected output'
    if cflags is None:
      cflags = []
    cflags = cflags.copy()
    filename = find_browser_test_file(filename)
    outfile = output_basename + '.html'
    cflags += ['-o', outfile]
    # print('cflags:', cflags)
    utils.delete_file(outfile)
    self.compile_btest(filename, cflags, reporting=reporting)
    self.assertExists(outfile)
    if post_build:
      post_build()
    if not isinstance(expected, list):
      expected = [expected]
    if EMTEST_BROWSER == 'node':
      nodejs = self.require_node()
      self.node_args += shared.node_pthread_flags(nodejs)
      output = self.run_js('test.js')
      self.assertContained('RESULT: ' + expected[0], output)
    else:
      self.run_browser(outfile, expected=['/report_result?' + e for e in expected], timeout=timeout)


###################################################################################################


def build_library(name,
                  build_dir,
                  generated_libs,
                  configure,
                  make,
                  make_args,
                  cache,
                  cache_name,
                  env_init,
                  native):
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
    try:
      with open(os.path.join(project_dir, 'configure_out'), 'w') as out:
        with open(os.path.join(project_dir, 'configure_err'), 'w') as err:
          stdout = out if EMTEST_BUILD_VERBOSE < 2 else None
          stderr = err if EMTEST_BUILD_VERBOSE < 1 else None
          shared.run_process(configure, env=env, stdout=stdout, stderr=stderr,
                             cwd=project_dir)
    except subprocess.CalledProcessError:
      print('-- configure stdout --')
      print(read_file(Path(project_dir, 'configure_out')))
      print('-- end configure stdout --')
      print('-- configure stderr --')
      print(read_file(Path(project_dir, 'configure_err')))
      print('-- end configure stderr --')
      raise
    # if we run configure or cmake we don't then need any kind
    # of special env when we run make below
    env = None

  def open_make_out(mode='r'):
    return open(os.path.join(project_dir, 'make.out'), mode)

  def open_make_err(mode='r'):
    return open(os.path.join(project_dir, 'make.err'), mode)

  if EMTEST_BUILD_VERBOSE >= 3:
    # VERBOSE=1 is cmake and V=1 is for autoconf
    make_args += ['VERBOSE=1', 'V=1']

  try:
    with open_make_out('w') as make_out:
      with open_make_err('w') as make_err:
        stdout = make_out if EMTEST_BUILD_VERBOSE < 2 else None
        stderr = make_err if EMTEST_BUILD_VERBOSE < 1 else None
        shared.run_process(make + make_args, stdout=stdout, stderr=stderr, env=env,
                           cwd=project_dir)
  except subprocess.CalledProcessError:
    with open_make_out() as f:
      print('-- make stdout --')
      print(f.read())
      print('-- end make stdout --')
    with open_make_err() as f:
      print('-- make stderr --')
      print(f.read())
      print('-- end stderr --')
    raise

  if cache is not None:
    cache[cache_name] = []
    for f in generated_libs:
      basename = os.path.basename(f)
      cache[cache_name].append((basename, read_binary(f)))

  return generated_libs
