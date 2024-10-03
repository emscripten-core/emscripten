# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from enum import Enum
from functools import wraps
from pathlib import Path
from subprocess import PIPE, STDOUT
from typing import Dict, Tuple
from urllib.parse import unquote, unquote_plus, urlparse, parse_qs
from http.server import HTTPServer, SimpleHTTPRequestHandler
import contextlib
import difflib
import hashlib
import itertools
import logging
import multiprocessing
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
import time
import webbrowser
import unittest
import queue

import clang_native
import jsrun
from tools.shared import EMCC, EMXX, DEBUG, EMCONFIGURE, EMCMAKE
from tools.shared import get_canonical_temp_dir, path_from_root
from tools.utils import MACOS, WINDOWS, read_file, read_binary, write_binary, exit_with_error
from tools.settings import COMPILE_TIME_SETTINGS
from tools import shared, line_endings, building, config, utils

logger = logging.getLogger('common')

# User can specify an environment variable EMTEST_BROWSER to force the browser
# test suite to run using another browser command line than the default system
# browser.
# There are two special value that can be used here if running in an actual
# browser is not desired:
#  EMTEST_BROWSER=0 : This will disable the actual running of the test and simply
#                     verify that it compiles and links.
#  EMTEST_BROWSER=node : This will attempt to run the browser test under node.
#                        For most browser tests this does not work, but it can
#                        be useful for running pthread tests under node.
EMTEST_BROWSER = None
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
if 'EM_BUILD_VERBOSE' in os.environ:
  exit_with_error('EM_BUILD_VERBOSE has been renamed to EMTEST_BUILD_VERBOSE')

# Special value for passing to assert_returncode which means we expect that program
# to fail with non-zero return code, but we don't care about specifically which one.
NON_ZERO = -1

TEST_ROOT = path_from_root('test')
LAST_TEST = path_from_root('out/last_test.txt')

WEBIDL_BINDER = shared.bat_suffix(path_from_root('tools/webidl_binder'))

EMBUILDER = shared.bat_suffix(path_from_root('embuilder'))
EMMAKE = shared.bat_suffix(path_from_root('emmake'))
EMRUN = shared.bat_suffix(shared.path_from_root('emrun'))
WASM_DIS = Path(building.get_binaryen_bin(), 'wasm-dis')
LLVM_OBJDUMP = os.path.expanduser(shared.build_llvm_tool_path(shared.exe_suffix('llvm-objdump')))
PYTHON = sys.executable
if not config.NODE_JS_TEST:
  config.NODE_JS_TEST = config.NODE_JS


requires_network = unittest.skipIf(os.getenv('EMTEST_SKIP_NETWORK_TESTS'), 'This test requires network access')


def test_file(*path_components):
  """Construct a path relative to the emscripten "tests" directory."""
  return str(Path(TEST_ROOT, *path_components))


# checks if browser testing is enabled
def has_browser():
  return EMTEST_BROWSER != '0'


def compiler_for(filename, force_c=False):
  if shared.suffix(filename) in ('.cc', '.cxx', '.cpp') and not force_c:
    return EMXX
  else:
    return EMCC


# Generic decorator that calls a function named 'condition' on the test class and
# skips the test if that function returns true
def skip_if(func, condition, explanation='', negate=False):
  assert callable(func)
  explanation_str = ' : %s' % explanation if explanation else ''

  @wraps(func)
  def decorated(self, *args, **kwargs):
    choice = self.__getattribute__(condition)()
    if negate:
      choice = not choice
    if choice:
      self.skipTest(condition + explanation_str)
    func(self, *args, **kwargs)

  return decorated


def is_slow_test(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    if EMTEST_SKIP_SLOW:
      return self.skipTest('skipping slow tests')
    return func(self, *args, **kwargs)

  return decorated


def flaky(note=''):
  assert not callable(note)

  if EMTEST_SKIP_FLAKY:
    return unittest.skip(note)

  if not EMTEST_RETRY_FLAKY:
    return lambda f: f

  def decorated(f):
    @wraps(f)
    def modified(*args, **kwargs):
      for i in range(EMTEST_RETRY_FLAKY):
        try:
          return f(*args, **kwargs)
        except AssertionError as exc:
          preserved_exc = exc
          logging.info(f'Retrying flaky test (attempt {i}/{EMTEST_RETRY_FLAKY} failed): {exc}')
      raise AssertionError('Flaky test has failed too many times') from preserved_exc

    return modified

  return decorated


def disabled(note=''):
  assert not callable(note)
  return unittest.skip(note)


def no_mac(note=''):
  assert not callable(note)
  if MACOS:
    return unittest.skip(note)
  return lambda f: f


def no_windows(note=''):
  assert not callable(note)
  if WINDOWS:
    return unittest.skip(note)
  return lambda f: f


def no_wasm64(note=''):
  assert not callable(note)

  def decorated(f):
    return skip_if(f, 'is_wasm64', note)
  return decorated


def no_2gb(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      # 2200mb is the value used by the core_2gb test mode
      if self.get_setting('INITIAL_MEMORY') == '2200mb':
        self.skipTest(note)
      f(self, *args, **kwargs)
    return decorated
  return decorator


def no_4gb(note):
  assert not callable(note)

  def decorator(f):
    assert callable(f)

    @wraps(f)
    def decorated(self, *args, **kwargs):
      if self.is_4gb():
        self.skipTest(note)
      f(self, *args, **kwargs)
    return decorated
  return decorator


def only_windows(note=''):
  assert not callable(note)
  if not WINDOWS:
    return unittest.skip(note)
  return lambda f: f


def requires_native_clang(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    if EMTEST_LACKS_NATIVE_CLANG:
      return self.skipTest('native clang tests are disabled')
    return func(self, *args, **kwargs)

  return decorated


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


def requires_wasm_exnref(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_wasm_exnref()
    return func(self, *args, **kwargs)

  return decorated


def requires_v8(func):
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    self.require_v8()
    return func(self, *args, **kwargs)

  return decorated


def requires_wasm2js(f):
  assert callable(f)

  @wraps(f)
  def decorated(self, *args, **kwargs):
    self.require_wasm2js()
    return f(self, *args, **kwargs)

  return decorated


def node_pthreads(f):
  assert callable(f)

  @wraps(f)
  def decorated(self, *args, **kwargs):
    self.setup_node_pthreads()
    f(self, *args, **kwargs)
  return decorated


def crossplatform(f):
  f.is_crossplatform_test = True
  return f


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

  def decorated(f):
    @wraps(f)
    def modified(self, *args, **kwargs):
      with env_modify(updates):
        return f(self, *args, **kwargs)
    return modified

  return decorated


def also_with_wasmfs(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, wasmfs, *args, **kwargs):
    if DEBUG:
      print('parameterize:wasmfs=%d' % wasmfs)
    if wasmfs:
      self.set_setting('WASMFS')
      self.emcc_args.append('-DWASMFS')
      f(self, *args, **kwargs)
    else:
      f(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'wasmfs': (True,)})
  return metafunc


def also_with_noderawfs(func):
  assert callable(func)

  @wraps(func)
  def metafunc(self, rawfs, *args, **kwargs):
    if DEBUG:
      print('parameterize:rawfs=%d' % rawfs)
    if rawfs:
      self.require_node()
      self.emcc_args += ['-DNODERAWFS']
      self.set_setting('NODERAWFS')
    func(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'rawfs': (True,)})
  return metafunc


# Decorator version of env_modify
def also_with_env_modify(name_updates_mapping):

  def decorated(f):
    @wraps(f)
    def metafunc(self, updates, *args, **kwargs):
      if DEBUG:
        print('parameterize:env_modify=%s' % (updates))
      if updates:
        with env_modify(updates):
          return f(self, *args, **kwargs)
      else:
        return f(self, *args, **kwargs)

    params = {'': (None,)}
    for name, updates in name_updates_mapping.items():
      params[name] = (updates,)

    parameterize(metafunc, params)

    return metafunc

  return decorated


def also_with_minimal_runtime(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, with_minimal_runtime, *args, **kwargs):
    if DEBUG:
      print('parameterize:minimal_runtime=%s' % with_minimal_runtime)
    assert self.get_setting('MINIMAL_RUNTIME') is None
    if with_minimal_runtime:
      self.set_setting('MINIMAL_RUNTIME', 1)
    f(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'minimal_runtime': (True,)})
  return metafunc


def also_with_wasm_bigint(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, with_bigint, *args, **kwargs):
    if DEBUG:
      print('parameterize:bigint=%s' % with_bigint)
    if with_bigint:
      if self.is_wasm2js():
        self.skipTest('wasm2js does not support WASM_BIGINT')
      if self.get_setting('WASM_BIGINT') is not None:
        self.skipTest('redundant in bigint test config')
      self.set_setting('WASM_BIGINT')
      nodejs = self.require_node()
      self.node_args += shared.node_bigint_flags(nodejs)
      f(self, *args, **kwargs)
    else:
      f(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'bigint': (True,)})
  return metafunc


def also_with_wasm64(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, with_wasm64, *args, **kwargs):
    if DEBUG:
      print('parameterize:wasm64=%s' % with_wasm64)
    if with_wasm64:
      self.require_wasm64()
      self.set_setting('MEMORY64')
      self.emcc_args.append('-Wno-experimental')
      f(self, *args, **kwargs)
    else:
      f(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'wasm64': (True,)})
  return metafunc


def also_with_wasm2js(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, with_wasm2js, *args, **kwargs):
    assert self.get_setting('WASM') is None
    if DEBUG:
      print('parameterize:wasm2js=%s' % with_wasm2js)
    if with_wasm2js:
      self.require_wasm2js()
      self.set_setting('WASM', 0)
      f(self, *args, **kwargs)
    else:
      f(self, *args, **kwargs)

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
      not self.get_setting('SAFE_HEAP') and \
      not any(a.startswith('-fsanitize=') for a in self.emcc_args)


# Impure means a test that cannot run in a wasm VM yet, as it is not 100%
# standalone. We can still run them with the JS code though.
def also_with_standalone_wasm(impure=False):
  def decorated(func):
    @wraps(func)
    def metafunc(self, standalone):
      if DEBUG:
        print('parameterize:standalone=%s' % standalone)
      if not standalone:
        func(self)
      else:
        if not can_do_standalone(self, impure):
          self.skipTest('Test configuration is not compatible with STANDALONE_WASM')
        self.set_setting('STANDALONE_WASM')
        if not impure:
          self.set_setting('PURE_WASI')
        # we will not legalize the JS ffi interface, so we must use BigInt
        # support in order for JS to have a chance to run this without trapping
        # when it sees an i64 on the ffi.
        self.set_setting('WASM_BIGINT')
        self.emcc_args.append('-Wno-unused-command-line-argument')
        # if we are impure, disallow all wasm engines
        if impure:
          self.wasm_engines = []
        nodejs = self.require_node()
        self.node_args += shared.node_bigint_flags(nodejs)
        func(self)

    parameterize(metafunc, {'': (False,),
                            'standalone': (True,)})
    return metafunc

  return decorated


# Tests exception handling / setjmp/longjmp handling in Emscripten EH/SjLj mode
# and new wasm EH/SjLj modes. This tests three combinations:
# - Emscripten EH + Emscripten SjLj
# - Wasm EH + Wasm SjLj (Phase 3, to be deprecated)
# - Wasm EH + Wasm SjLj (New proposal witn exnref, experimental)
def with_all_eh_sjlj(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, mode, *args, **kwargs):
    if DEBUG:
      print('parameterize:eh_mode=%s' % mode)
    if mode == 'wasm' or mode == 'wasm_exnref':
      # Wasm EH is currently supported only in wasm backend and V8
      if self.is_wasm2js():
        self.skipTest('wasm2js does not support wasm EH/SjLj')
      # FIXME Temporarily disabled. Enable this later when the bug is fixed.
      if '-fsanitize=address' in self.emcc_args:
        self.skipTest('Wasm EH does not work with asan yet')
      self.emcc_args.append('-fwasm-exceptions')
      self.set_setting('SUPPORT_LONGJMP', 'wasm')
      if mode == 'wasm':
        self.require_wasm_eh()
      if mode == 'wasm_exnref':
        self.require_wasm_exnref()
        self.set_setting('WASM_EXNREF')
      f(self, *args, **kwargs)
    else:
      self.set_setting('DISABLE_EXCEPTION_CATCHING', 0)
      self.set_setting('SUPPORT_LONGJMP', 'emscripten')
      # DISABLE_EXCEPTION_CATCHING=0 exports __cxa_can_catch,
      # so if we don't build in C++ mode, wasm-ld will
      # error out because libc++abi is not included. See
      # https://github.com/emscripten-core/emscripten/pull/14192 for details.
      self.set_setting('DEFAULT_TO_CXX')
      f(self, *args, **kwargs)

  parameterize(metafunc, {'emscripten': ('emscripten',),
                          'wasm': ('wasm',),
                          'wasm_exnref': ('wasm_exnref',)})
  return metafunc


# This works just like `with_all_eh_sjlj` above but doesn't enable exceptions.
# Use this for tests that use setjmp/longjmp but not exceptions handling.
def with_all_sjlj(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, mode):
    if mode == 'wasm' or mode == 'wasm_exnref':
      if self.is_wasm2js():
        self.skipTest('wasm2js does not support wasm SjLj')
      # FIXME Temporarily disabled. Enable this later when the bug is fixed.
      if '-fsanitize=address' in self.emcc_args:
        self.skipTest('Wasm EH does not work with asan yet')
      self.set_setting('SUPPORT_LONGJMP', 'wasm')
      if mode == 'wasm':
        self.require_wasm_eh()
      if mode == 'wasm_exnref':
        self.require_wasm_exnref()
        self.set_setting('WASM_EXNREF')
      f(self)
    else:
      self.set_setting('SUPPORT_LONGJMP', 'emscripten')
      f(self)

  parameterize(metafunc, {'emscripten': ('emscripten',),
                          'wasm': ('wasm',),
                          'wasm_exnref': ('wasm_exnref',)})
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


def make_executable(name):
  Path(name).chmod(stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_dir_writeable(dirname):
  # Ensure all files are readable and writable by the current user.
  permission_bits = stat.S_IWRITE | stat.S_IREAD

  def is_writable(path):
    return (os.stat(path).st_mode & permission_bits) != permission_bits

  def make_writable(path):
    new_mode = os.stat(path).st_mode | permission_bits
    os.chmod(path, new_mode)

  # Some tests make files and subdirectories read-only, so rmtree/unlink will not delete
  # them. Force-make everything writable in the subdirectory to make it
  # removable and re-attempt.
  if not is_writable(dirname):
    make_writable(dirname)

  for directory, subdirs, files in os.walk(dirname):
    for item in files + subdirs:
      i = os.path.join(directory, item)
      if not os.path.islink(i):
        make_writable(i)


def force_delete_dir(dirname):
  make_dir_writeable(dirname)
  utils.delete_dir(dirname)


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


class RunnerCore(unittest.TestCase, metaclass=RunnerMeta):
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
    if self.is_wasm2js():
      self.skipTest('no dynamic linking support in wasm2js yet')
    if '-fsanitize=undefined' in self.emcc_args:
      self.skipTest('no dynamic linking support in UBSan yet')
    # MEMORY64=2 mode doesn't currently support dynamic linking because
    # The side modules are lowered to wasm32 when they are built, making
    # them unlinkable with wasm64 binaries.
    if self.get_setting('MEMORY64') == 2:
      self.skipTest('MEMORY64=2 + dynamic linking is not currently supported')

  def require_v8(self):
    if not config.V8_ENGINE or config.V8_ENGINE not in config.JS_ENGINES:
      if 'EMTEST_SKIP_V8' in os.environ:
        self.skipTest('test requires v8 and EMTEST_SKIP_V8 is set')
      else:
        self.fail('d8 required to run this test.  Use EMTEST_SKIP_V8 to skip')
    self.require_engine(config.V8_ENGINE)
    self.emcc_args.append('-sENVIRONMENT=shell')

  def get_nodejs(self):
    if config.NODE_JS_TEST not in self.js_engines:
      return None
    return config.NODE_JS_TEST

  def require_node(self):
    nodejs = self.get_nodejs()
    if not nodejs:
      if 'EMTEST_SKIP_NODE' in os.environ:
        self.skipTest('test requires node and EMTEST_SKIP_NODE is set')
      else:
        self.fail('node required to run this test.  Use EMTEST_SKIP_NODE to skip')
    self.require_engine(nodejs)
    return nodejs

  def node_is_canary(self, nodejs):
    return nodejs and nodejs[0] and 'canary' in nodejs[0]

  def require_node_canary(self):
    nodejs = self.get_nodejs()
    if self.node_is_canary(nodejs):
      self.require_engine(nodejs)
      return

    if 'EMTEST_SKIP_NODE_CANARY' in os.environ:
      self.skipTest('test requires node canary and EMTEST_SKIP_NODE_CANARY is set')
    else:
      self.fail('node canary required to run this test.  Use EMTEST_SKIP_NODE_CANARY to skip')

  def require_engine(self, engine):
    logger.debug(f'require_engine: {engine}')
    if self.required_engine and self.required_engine != engine:
      self.skipTest(f'Skipping test that requires `{engine}` when `{self.required_engine}` was previously required')
    self.required_engine = engine
    self.js_engines = [engine]
    self.wasm_engines = []

  def require_wasm64(self):
    if self.is_browser_test():
      return

    nodejs = self.get_nodejs()
    if nodejs:
      version = shared.get_node_version(nodejs)
      if version >= (23, 0, 0):
        self.js_engines = [nodejs]
        self.node_args += shared.node_memory64_flags()
        return

    if config.V8_ENGINE and config.V8_ENGINE in self.js_engines:
      self.emcc_args.append('-sENVIRONMENT=shell')
      self.js_engines = [config.V8_ENGINE]
      self.v8_args.append('--experimental-wasm-memory64')
      return

    if 'EMTEST_SKIP_WASM64' in os.environ:
      self.skipTest('test requires node >= 23 or d8 (and EMTEST_SKIP_WASM64 is set)')
    else:
      self.fail('either d8 or node >= 23 required to run wasm64 tests.  Use EMTEST_SKIP_WASM64 to skip')

  def require_simd(self):
    if self.is_browser_test():
      return

    nodejs = self.get_nodejs()
    if nodejs:
      version = shared.get_node_version(nodejs)
      if version >= (16, 0, 0):
        self.js_engines = [nodejs]
        return

    if config.V8_ENGINE and config.V8_ENGINE in self.js_engines:
      self.emcc_args.append('-sENVIRONMENT=shell')
      self.js_engines = [config.V8_ENGINE]
      return

    if 'EMTEST_SKIP_SIMD' in os.environ:
      self.skipTest('test requires node >= 16 or d8 (and EMTEST_SKIP_SIMD is set)')
    else:
      self.fail('either d8 or node >= 16 required to run wasm64 tests.  Use EMTEST_SKIP_SIMD to skip')

  def require_wasm_eh(self):
    nodejs = self.get_nodejs()
    if nodejs:
      version = shared.get_node_version(nodejs)
      if version >= (17, 0, 0):
        self.js_engines = [nodejs]
        return

    if config.V8_ENGINE and config.V8_ENGINE in self.js_engines:
      self.emcc_args.append('-sENVIRONMENT=shell')
      self.js_engines = [config.V8_ENGINE]
      return

    if 'EMTEST_SKIP_EH' in os.environ:
      self.skipTest('test requires node >= 17 or d8 (and EMTEST_SKIP_EH is set)')
    else:
      self.fail('either d8 or node >= 17 required to run wasm-eh tests.  Use EMTEST_SKIP_EH to skip')

  def require_wasm_exnref(self):
    nodejs = self.get_nodejs()
    if nodejs:
      if self.node_is_canary(nodejs):
        self.js_engines = [nodejs]
        self.node_args.append('--experimental-wasm-exnref')
        return

    if config.V8_ENGINE and config.V8_ENGINE in self.js_engines:
      self.emcc_args.append('-sENVIRONMENT=shell')
      self.js_engines = [config.V8_ENGINE]
      self.v8_args.append('--experimental-wasm-exnref')
      return

    if 'EMTEST_SKIP_EH' in os.environ:
      self.skipTest('test requires canary or d8 (and EMTEST_SKIP_EH is set)')
    else:
      self.fail('either d8 or node canary required to run wasm-eh tests.  Use EMTEST_SKIP_EH to skip')

  def require_jspi(self):
    # emcc warns about stack switching being experimental, and we build with
    # warnings-as-errors, so disable that warning
    self.emcc_args += ['-Wno-experimental']
    self.set_setting('JSPI')
    if self.is_wasm2js():
      self.skipTest('JSPI is not currently supported for WASM2JS')

    if self.is_browser_test():
      if 'EMTEST_SKIP_JSPI' in os.environ:
        self.skipTest('skipping JSPI (EMTEST_SKIP_JSPI is set)')
      return

    exp_args = ['--experimental-wasm-stack-switching', '--experimental-wasm-type-reflection']
    nodejs = self.get_nodejs()
    if nodejs:
      # Support for JSPI came earlier than 22, but the new API changes are not yet in any node
      if self.node_is_canary(nodejs):
        self.js_engines = [nodejs]
        self.node_args += exp_args
        return

    if config.V8_ENGINE and config.V8_ENGINE in self.js_engines:
      self.emcc_args.append('-sENVIRONMENT=shell')
      self.js_engines = [config.V8_ENGINE]
      self.v8_args += exp_args
      return

    if 'EMTEST_SKIP_JSPI' in os.environ:
      self.skipTest('test requires node canary or d8 (and EMTEST_SKIP_JSPI is set)')
    else:
      self.fail('either d8 or node canary required to run JSPI tests.  Use EMTEST_SKIP_JSPI to skip')

  def require_wasm2js(self):
    if self.is_wasm64():
      self.skipTest('wasm2js is not compatible with MEMORY64')
    if self.is_2gb() or self.is_4gb():
      self.skipTest('wasm2js does not support over 2gb of memory')

  def setup_node_pthreads(self):
    self.require_node()
    self.emcc_args += ['-Wno-pthreads-mem-growth', '-pthread']
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

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    print('(checking sanity from test runner)') # do this after we set env stuff
    shared.check_sanity(force=True)

  def setUp(self):
    super().setUp()
    self.js_engines = config.JS_ENGINES.copy()
    self.settings_mods = {}
    self.skip_exec = None
    self.proxied = False
    self.emcc_args = ['-Wclosure', '-Werror', '-Wno-limited-postlink-optimizations']
    # TODO(https://github.com/emscripten-core/emscripten/issues/11121)
    # For historical reasons emcc compiles and links as C++ by default.
    # However we want to run our tests in a more strict manner.  We can
    # remove this if the issue above is ever fixed.
    self.set_setting('NO_DEFAULT_TO_CXX')
    self.ldflags = []
    # Increate stack trace limit to maximise usefulness of test failure reports
    self.node_args = ['--stack-trace-limit=50']

    nodejs = self.get_nodejs()
    if nodejs:
      node_version = shared.get_node_version(nodejs)
      if node_version < (11, 0, 0):
        self.node_args.append('--unhandled-rejections=strict')
        self.node_args.append('--experimental-wasm-se')
      else:
        # Include backtrace for all uncuaght exceptions (not just Error).
        self.node_args.append('--trace-uncaught')
        if node_version < (15, 0, 0):
          # Opt in to node v15 default behaviour:
          # https://nodejs.org/api/cli.html#cli_unhandled_rejections_mode
          self.node_args.append('--unhandled-rejections=throw')

      # If the version we are running tests in is lower than the version that
      # emcc targets then we need to tell emcc to target that older version.
      emcc_min_node_version_str = str(shared.settings.MIN_NODE_VERSION)
      emcc_min_node_version = (
        int(emcc_min_node_version_str[0:2]),
        int(emcc_min_node_version_str[2:4]),
        int(emcc_min_node_version_str[4:6])
      )
      if node_version < emcc_min_node_version:
        self.emcc_args += building.get_emcc_node_flags(node_version)
        self.emcc_args.append('-Wno-transpile')

    self.v8_args = ['--wasm-staging']
    self.env = {}
    self.temp_files_before_run = []
    self.uses_es6 = False
    self.required_engine = None
    self.wasm_engines = config.WASM_ENGINES.copy()
    self.use_all_engines = EMTEST_ALL_ENGINES
    if self.js_engines[0] != config.NODE_JS_TEST:
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
          '/tmp/wasmer'
        ]

        left_over_files = set(temp_files_after_run) - set(self.temp_files_before_run)
        left_over_files = [f for f in left_over_files if not any([f.startswith(prefix) for prefix in ignorable_file_prefixes])]
        if len(left_over_files):
          print('ERROR: After running test, there are ' + str(len(left_over_files)) + ' new temporary files/directories left behind:', file=sys.stderr)
          for f in left_over_files:
            print('leaked file: ' + f, file=sys.stderr)
          self.fail('Test leaked ' + str(len(left_over_files)) + ' temporary files!')

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
    self.emcc_args += ['--pre-js', 'prerun.js']

  def add_post_run(self, code):
    assert not self.get_setting('MINIMAL_RUNTIME')
    create_file('postrun.js', 'Module.postRun = function() { %s }\n' % code)
    self.emcc_args += ['--pre-js', 'postrun.js']

  def add_on_exit(self, code):
    assert not self.get_setting('MINIMAL_RUNTIME')
    create_file('onexit.js', 'Module.onExit = function() { %s }\n' % code)
    self.emcc_args += ['--pre-js', 'onexit.js']

  # returns the full list of arguments to pass to emcc
  # param @main_file whether this is the main file of the test. some arguments
  #                  (like --pre-js) do not need to be passed when building
  #                  libraries, for example
  def get_emcc_args(self, main_file=False, compile_only=False, asm_only=False):
    def is_ldflag(f):
      return any(f.startswith(s) for s in ['-sEXPORT_ES6', '-sPROXY_TO_PTHREAD', '-sENVIRONMENT=', '--pre-js=', '--post-js=', '-sPTHREAD_POOL_SIZE='])

    args = self.serialize_settings(compile_only or asm_only) + self.emcc_args
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
    es_check_env['PATH'] = os.path.dirname(config.NODE_JS_TEST[0]) + os.pathsep + es_check_env['PATH']
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
  def build(self, filename, libraries=None, includes=None, force_c=False, js_outfile=True, emcc_args=None, output_basename=None):
    if not os.path.exists(filename):
      filename = test_file(filename)
    suffix = '.js' if js_outfile else '.wasm'
    compiler = [compiler_for(filename, force_c)]

    if force_c:
      assert shared.suffix(filename) != '.c', 'force_c is not needed for source files ending in .c'
      compiler.append('-xc')

    if output_basename:
      output = output_basename + suffix
    else:
      basename = os.path.basename(filename)
      output = shared.unsuffixed(basename) + suffix
    cmd = compiler + [filename, '-o', output] + self.get_emcc_args(main_file=True)
    if emcc_args:
      cmd += emcc_args
    if libraries:
      cmd += libraries
    if includes:
      cmd += ['-I' + str(include) for include in includes]

    self.run_process(cmd, stderr=self.stderr_redirect if not DEBUG else None)
    self.assertExists(output)

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

  def count_funcs(self, javascript_file):
    num_funcs = 0
    start_tok = "// EMSCRIPTEN_START_FUNCS"
    end_tok = "// EMSCRIPTEN_END_FUNCS"
    start_off = 0
    end_off = 0

    js = read_file(javascript_file)
    blob = "".join(js.splitlines())

    start_off = blob.find(start_tok) + len(start_tok)
    end_off = blob.find(end_tok)
    asm_chunk = blob[start_off:end_off]
    num_funcs = asm_chunk.count('function ')
    return num_funcs

  def count_wasm_contents(self, wasm_binary, what):
    out = self.run_process([os.path.join(building.get_binaryen_bin(), 'wasm-opt'), wasm_binary, '--metrics'], stdout=PIPE).stdout
    # output is something like
    # [?]        : 125
    for line in out.splitlines():
      if '[' + what + ']' in line:
        ret = line.split(':')[1].strip()
        return int(ret)
    self.fail('Failed to find [%s] in wasm-opt output' % what)

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

  def run_js(self, filename, engine=None, args=None,
             assert_returncode=0,
             interleaved_output=True):
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
    if not engine:
      engine = self.js_engines[0]
    if engine == config.NODE_JS_TEST:
      engine = engine + self.node_args
    if engine == config.V8_ENGINE:
      engine = engine + self.v8_args
    try:
      jsrun.run_js(filename, engine, args,
                   stdout=stdout,
                   stderr=stderr,
                   assert_returncode=assert_returncode)
    except subprocess.TimeoutExpired as e:
      timeout_error = e
    except subprocess.CalledProcessError as e:
      error = e
    finally:
      stdout.close()
      if stderr != STDOUT:
        stderr.close()

    # Make sure that we produced proper line endings to the .js file we are about to run.
    if not filename.endswith('.wasm'):
      self.assertEqual(line_endings.check_line_endings(filename), 0)

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

    #  We should pass all strict mode checks
    self.assertNotContained('strict warning:', ret)
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
      print("Expected to have '%s' == '%s'" % (limit_size(values[0]), limit_size(y)))
    fail_message = 'Unexpected difference:\n' + limit_size(diff)
    if not EMTEST_VERBOSE:
      fail_message += '\nFor full output run with --verbose.'
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
        self.assertTrue(re.search(values, string, re.DOTALL), 'Expected regex "%s" to match on:\n%s' % (values, string))
      else:
        match_any = any(re.search(o, string, re.DOTALL) for o in values)
        self.assertTrue(match_any, 'Expected at least one of "%s" to match on:\n%s' % (values, string))
      return

    if type(values) not in [list, tuple]:
      values = [values]

    if not any(v in string for v in values):
      diff = difflib.unified_diff(values[0].split('\n'), string.split('\n'), fromfile='expected', tofile='actual')
      diff = ''.join(a.rstrip() + '\n' for a in diff)
      self.fail("Expected to find '%s' in '%s', diff:\n\n%s\n%s" % (
        limit_size(values[0]), limit_size(string), limit_size(diff),
        additional_info
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
      make_args = ['-j', str(shared.get_num_cores())]

    build_dir = self.get_build_dir()
    output_dir = self.get_dir()

    emcc_args = []
    if not native:
      # get_library() is used to compile libraries, and not link executables,
      # so we don't want to pass linker flags here (emscripten warns if you
      # try to pass linker settings when compiling).
      emcc_args = self.get_emcc_args(compile_only=True)

    hash_input = (str(emcc_args) + ' $ ' + str(env_init)).encode('utf-8')
    cache_name = name + ','.join([opt for opt in emcc_args if len(opt) < 7]) + '_' + hashlib.md5(hash_input).hexdigest() + cache_name_extra

    valid_chars = "_%s%s" % (string.ascii_letters, string.digits)
    cache_name = ''.join([(c if c in valid_chars else '_') for c in cache_name])

    if not force_rebuild and self.library_cache.get(cache_name):
      print('<load %s from cache> ' % cache_name, file=sys.stderr)
      generated_libs = []
      for basename, contents in self.library_cache[cache_name]:
        bc_file = os.path.join(build_dir, cache_name + '_' + basename)
        write_binary(bc_file, contents)
        generated_libs.append(bc_file)
      return generated_libs

    print(f'<building and saving {cache_name} into cache>', file=sys.stderr)
    if configure and configure_args:
      # Make to copy to avoid mutating default param
      configure = list(configure)
      configure += configure_args

    cflags = ' '.join(emcc_args)
    env_init.setdefault('CFLAGS', cflags)
    env_init.setdefault('CXXFLAGS', cflags)
    return build_library(name, build_dir, output_dir, generated_libs, configure,
                         make, make_args, self.library_cache,
                         cache_name, env_init=env_init, native=native)

  def clear(self):
    force_delete_contents(self.get_dir())
    if shared.EMSCRIPTEN_TEMP_DIR:
      utils.delete_contents(shared.EMSCRIPTEN_TEMP_DIR)

  def run_process(self, cmd, check=True, **args):
    # Wrapper around shared.run_process.  This is desirable so that the tests
    # can fail (in the unittest sense) rather than error'ing.
    # In the long run it would nice to completely remove the dependency on
    # core emscripten code (shared.py) here.
    try:
      return shared.run_process(cmd, check=check, **args)
    except subprocess.CalledProcessError as e:
      if check and e.returncode != 0:
        print(e.stdout)
        print(e.stderr)
        self.fail(f'subprocess exited with non-zero return code({e.returncode}): `{shared.shlex_join(cmd)}`')

  def emcc(self, filename, args=[], output_filename=None, **kwargs):  # noqa
    cmd = [compiler_for(filename), filename] + self.get_emcc_args(compile_only='-c' in args) + args
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
      cmdv = [EMCC, src, '-o', shared.unsuffixed(src) + so, '-sSIDE_MODULE'] + self.get_emcc_args()
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
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n', emcc_args=extra_args)

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
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n', emcc_args=extra_args)

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
    if not os.path.exists(srcfile):
      srcfile = test_file(srcfile)
    out_suffix = kwargs.pop('out_suffix', '')
    outfile = shared.unsuffixed(srcfile) + out_suffix + '.out'
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
                     libraries=None,
                     includes=None,
                     assert_returncode=0, assert_identical=False, assert_all=False,
                     check_for_error=True, force_c=False, emcc_args=None,
                     interleaved_output=True,
                     regex=False,
                     output_basename=None):
    logger.debug(f'_build_and_run: {filename}')

    if no_build:
      js_file = filename
    else:
      js_file = self.build(filename, libraries=libraries, includes=includes,
                           force_c=force_c, emcc_args=emcc_args,
                           output_basename=output_basename)
    self.assertExists(js_file)

    engines = self.js_engines.copy()
    if len(engines) > 1 and not self.use_all_engines:
      engines = engines[:1]
    # In standalone mode, also add wasm vms as we should be able to run there too.
    if self.get_setting('STANDALONE_WASM'):
      # TODO once standalone wasm support is more stable, apply use_all_engines
      # like with js engines, but for now as we bring it up, test in all of them
      if not self.wasm_engines:
        logger.warning('no wasm engine was found to run the standalone part of this test')
      engines += self.wasm_engines
    if len(engines) == 0:
      self.fail('No JS engine present to run this test with. Check %s and the paths therein.' % config.EM_CONFIG)
    for engine in engines:
      js_output = self.run_js(js_file, engine, args,
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
    self.emcc_args += [
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

    self.emcc_args += [
      '-I' + test_file('third_party/freetype/include'),
      '-I' + test_file('third_party/poppler/include')
    ]

    # Poppler has some pretty glaring warning.  Suppress them to keep the
    # test output readable.
    self.emcc_args += [
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
    ]
    env_init = env_init.copy() if env_init else {}
    env_init['FONTCONFIG_CFLAGS'] = ' '
    env_init['FONTCONFIG_LIBS'] = ' '

    poppler = self.get_library(
        os.path.join('third_party', 'poppler'),
        [os.path.join('utils', 'pdftoppm.o'), os.path.join('utils', 'parseargs.o'), os.path.join('poppler', '.libs', 'libpoppler.a')],
        env_init=env_init,
        configure_args=['--disable-libjpeg', '--disable-libpng', '--disable-poppler-qt', '--disable-poppler-qt4', '--disable-cms', '--disable-cairo-output', '--disable-abiword-output', '--disable-shared'])

    return poppler + freetype

  def get_zlib_library(self, cmake):
    assert cmake or not WINDOWS, 'on windows, get_zlib_library only supports cmake'

    old_args = self.emcc_args.copy()
    # inflate.c does -1L << 16
    self.emcc_args.append('-Wno-shift-negative-value')
    # adler32.c uses K&R sytyle function declarations
    self.emcc_args.append('-Wno-deprecated-non-prototype')
    # Work around configure-script error. TODO: remove when
    # https://github.com/emscripten-core/emscripten/issues/16908 is fixed
    self.emcc_args.append('-Wno-pointer-sign')
    if cmake:
      rtn = self.get_library(os.path.join('third_party', 'zlib'), os.path.join('libz.a'),
                             configure=['cmake', '.'],
                             make=['cmake', '--build', '.', '--'],
                             make_args=[])
    else:
      rtn = self.get_library(os.path.join('third_party', 'zlib'), os.path.join('libz.a'), make_args=['libz.a'])
    self.emcc_args = old_args
    return rtn


# Run a server and a web page. When a test runs, we tell the server about it,
# which tells the web page, which then opens a window with the test. Doing
# it this way then allows the page to close() itself when done.
def harness_server_func(in_queue, out_queue, port):
  class TestServerHandler(SimpleHTTPRequestHandler):
    # Request header handler for default do_GET() path in
    # SimpleHTTPRequestHandler.do_GET(self) below.
    def send_head(self):
      if self.path.endswith('.js'):
        path = self.translate_path(self.path)
        try:
          f = open(path, 'rb')
        except IOError:
          self.send_error(404, "File not found: " + path)
          return None
        self.send_response(200)
        self.send_header('Content-type', 'application/javascript')
        self.send_header('Connection', 'close')
        self.end_headers()
        return f
      else:
        return SimpleHTTPRequestHandler.send_head(self)

    # Add COOP, COEP, CORP, and no-caching headers
    def end_headers(self):
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
      self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
      self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')
      self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
      return SimpleHTTPRequestHandler.end_headers(self)

    def do_POST(self):
      urlinfo = urlparse(self.path)
      query = parse_qs(urlinfo.query)
      if query['file']:
        print('do_POST: got file: %s' % query['file'])
        filename = query['file'][0]
        contentLength = int(self.headers['Content-Length'])
        create_file(filename, self.rfile.read(contentLength), binary=True)
        self.send_response(200)
        self.end_headers()
      else:
        print(f'do_POST: unexpected POST: {urlinfo.query}')

    def do_GET(self):
      if self.path == '/run_harness':
        if DEBUG:
          print('[server startup]')
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(read_binary(test_file('browser_harness.html')))
      elif 'report_' in self.path:
        # the test is reporting its result. first change dir away from the
        # test dir, as it will be deleted now that the test is finishing, and
        # if we got a ping at that time, we'd return an error
        os.chdir(path_from_root())
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
        self.send_header('Cache-Control', 'no-cache, must-revalidate')
        self.send_header('Connection', 'close')
        self.send_header('Expires', '-1')
        self.end_headers()
        self.wfile.write(b'OK')

      elif 'stdout=' in self.path or 'stderr=' in self.path:
        '''
          To get logging to the console from browser tests, add this to
          print/printErr/the exception handler in src/shell.html:

            fetch(encodeURI('http://localhost:8888?stdout=' + text));
        '''
        print('[client logging:', unquote_plus(self.path), ']')
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
      elif self.path == '/check':
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
          # move us to the right place to serve the files for the new test
          os.chdir(dir)
        else:
          # the browser must keep polling
          self.wfile.write(b'(wait)')
      else:
        # Use SimpleHTTPServer default file serving operation for GET.
        if DEBUG:
          print('[simple HTTP serving:', unquote_plus(self.path), ']')
        SimpleHTTPRequestHandler.do_GET(self)

    def log_request(code=0, size=0):
      # don't log; too noisy
      pass

  # allows streaming compilation to work
  SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'

  httpd = HTTPServer(('localhost', port), TestServerHandler)
  httpd.serve_forever() # test runner will kill us


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


class BrowserCore(RunnerCore):
  # note how many tests hang / do not send an output. if many of these
  # happen, likely something is broken and it is best to abort the test
  # suite early, as otherwise we will wait for the timeout on every
  # single test (hundreds of minutes)
  MAX_UNRESPONSIVE_TESTS = 10

  unresponsive_tests = 0

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)

  @classmethod
  def browser_restart(cls):
    # Kill existing browser
    logger.info('Restarting browser process')
    cls.browser_proc.terminate()
    # If the browser doesn't shut down gracefully (in response to SIGTERM)
    # after 2 seconds kill it with force (SIGKILL).
    try:
      cls.browser_proc.wait(2)
    except subprocess.TimeoutExpired:
      logger.info('Browser did not respond to `terminate`.  Using `kill`')
      cls.browser_proc.kill()
      cls.browser_proc.wait()
    cls.browser_open(cls.harness_url)

  @classmethod
  def browser_open(cls, url):
    global EMTEST_BROWSER
    if not EMTEST_BROWSER:
      logger.info('No EMTEST_BROWSER set. Defaulting to `google-chrome`')
      EMTEST_BROWSER = 'google-chrome'
    browser_args = shlex.split(EMTEST_BROWSER)
    logger.info('Launching browser: %s', str(browser_args))
    cls.browser_proc = subprocess.Popen(browser_args + [url])

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    cls.port = int(os.getenv('EMTEST_BROWSER_PORT', '8888'))
    if not has_browser() or EMTEST_BROWSER == 'node':
      return
    cls.browser_timeout = 60
    cls.harness_in_queue = multiprocessing.Queue()
    cls.harness_out_queue = multiprocessing.Queue()
    cls.harness_server = multiprocessing.Process(target=harness_server_func, args=(cls.harness_in_queue, cls.harness_out_queue, cls.port))
    cls.harness_server.start()
    print('[Browser harness server on process %d]' % cls.harness_server.pid)
    cls.harness_url = 'http://localhost:%s/run_harness' % cls.port
    cls.browser_open(cls.harness_url)

  @classmethod
  def tearDownClass(cls):
    super().tearDownClass()
    if not has_browser() or EMTEST_BROWSER == 'node':
      return
    cls.harness_server.terminate()
    print('[Browser harness server terminated]')
    if WINDOWS:
      # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
      # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
      time.sleep(0.1)

  def is_browser_test(self):
    return True

  def assert_out_queue_empty(self, who):
    if not self.harness_out_queue.empty():
      while not self.harness_out_queue.empty():
        self.harness_out_queue.get()
      raise Exception('excessive responses from %s' % who)

  # @param extra_tries: how many more times to try this test, if it fails. browser tests have
  #                     many more causes of flakiness (in particular, they do not run
  #                     synchronously, so we have a timeout, which can be hit if the VM
  #                     we run on stalls temporarily), so we let each test try more than
  #                     once by default
  def run_browser(self, html_file, expected=None, message=None, timeout=None, extra_tries=1):
    if not has_browser():
      return
    if self.skip_exec:
      self.skipTest('skipping test execution: ' + self.skip_exec)
    if BrowserCore.unresponsive_tests >= BrowserCore.MAX_UNRESPONSIVE_TESTS:
      self.skipTest('too many unresponsive tests, skipping remaining tests')
    self.assert_out_queue_empty('previous test')
    if DEBUG:
      print('[browser launch:', html_file, ']')
    assert not (message and expected), 'run_browser expects `expected` or `message`, but not both'
    if expected is not None:
      try:
        self.harness_in_queue.put((
          'http://localhost:%s/%s' % (self.port, html_file),
          self.get_dir()
        ))
        if timeout is None:
          timeout = self.browser_timeout
        try:
          output = self.harness_out_queue.get(block=True, timeout=timeout)
        except queue.Empty:
          BrowserCore.unresponsive_tests += 1
          print('[unresponsive tests: %d]' % BrowserCore.unresponsive_tests)
          self.browser_restart()
          # Rather than fail the test here, let fail on the `assertContained` so
          # that the test can be retried via `extra_tries`
          output = '[no http server activity]'
        if output is None:
          # the browser harness reported an error already, and sent a None to tell
          # us to also fail the test
          self.fail('browser harness error')
        if output.startswith('/report_result?skipped:'):
          self.skipTest(unquote(output[len('/report_result?skipped:'):]).strip())
        else:
          # verify the result, and try again if we should do so
          output = unquote(output)
          try:
            self.assertContained(expected, output)
          except self.failureException as e:
            if extra_tries > 0:
              print('[test error (see below), automatically retrying]')
              print(e)
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

  def compile_btest(self, filename, args, reporting=Reporting.FULL):
    # Inject support code for reporting results. This adds an include a header so testcases can
    # use REPORT_RESULT, and also adds a cpp file to be compiled alongside the testcase, which
    # contains the implementation of REPORT_RESULT (we can't just include that implementation in
    # the header as there may be multiple files being compiled here).
    if reporting != Reporting.NONE:
      # For basic reporting we inject JS helper funtions to report result back to server.
      args += ['--pre-js', test_file('browser_reporting.js')]
      if reporting == Reporting.FULL:
        # If C reporting (i.e. the REPORT_RESULT macro) is required we
        # also include report_result.c and force-include report_result.h
        self.run_process([EMCC, '-c', '-I' + TEST_ROOT,
                          '-DEMTEST_PORT_NUMBER=%d' % self.port,
                          test_file('report_result.c')] + self.get_emcc_args(compile_only=True) + (['-fPIC'] if '-fPIC' in args else []))
        args += ['report_result.o', '-include', test_file('report_result.h')]
    if EMTEST_BROWSER == 'node':
      args.append('-DEMTEST_NODE')
    if not os.path.exists(filename):
      filename = test_file(filename)
    self.run_process([compiler_for(filename), filename] + self.get_emcc_args() + args)

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
            args=None, url_suffix='', timeout=None,
            extra_tries=1,
            reporting=Reporting.FULL,
            output_basename='test'):
    assert expected, 'a btest must have an expected output'
    if args is None:
      args = []
    args = args.copy()
    filename = find_browser_test_file(filename)

    # Run via --proxy-to-worker.  This gets set by the @also_with_proxying.
    if self.proxied:
      args += ['--proxy-to-worker', '-sGL_TESTING']

    outfile = output_basename + '.html'
    args += ['-o', outfile]
    # print('all args:', args)
    utils.delete_file(outfile)
    self.compile_btest(filename, args, reporting=reporting)
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
      self.run_browser(outfile + url_suffix, expected=['/report_result?' + e for e in expected], timeout=timeout, extra_tries=extra_tries)


###################################################################################################


def build_library(name,
                  build_dir,
                  output_dir,
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
