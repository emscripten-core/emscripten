# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Common decorators shared between the various suites.

When decorators are only used a single file they can be defined locally.  If they
are used from multiple locations they should be defined here.
"""

import itertools
import os
import subprocess
import unittest
from functools import wraps

import common
from common import test_file

from tools.shared import DEBUG
from tools.utils import MACOS, WINDOWS

requires_network = unittest.skipIf(os.getenv('EMTEST_SKIP_NETWORK_TESTS'), 'This test requires network access')


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
  decorated = skip_if_simple('skipping slow tests', lambda _: common.EMTEST_SKIP_SLOW)(func)
  decorated.is_slow = True
  return decorated


def flaky(note=''):
  assert not callable(note)

  if common.EMTEST_SKIP_FLAKY:
    return unittest.skip(note)

  if not common.EMTEST_RETRY_FLAKY:
    return lambda f: f

  def decorated(func):
    @wraps(func)
    def modified(self, *args, **kwargs):
      # Browser tests have there own method of retrying tests.
      if self.is_browser_test():
        self.flaky = True
        return func(self, *args, **kwargs)

      for i in range(common.EMTEST_RETRY_FLAKY):
        try:
          return func(self, *args, **kwargs)
        except (AssertionError, subprocess.TimeoutExpired) as exc:
          preserved_exc = exc
          common.record_flaky_test(self.id(), i, common.EMTEST_RETRY_FLAKY, exc)

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

requires_native_clang = skip_if_simple('native clang tests are disabled', lambda _: common.EMTEST_LACKS_NATIVE_CLANG)

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


# Decorator version of env_modify
def with_env_modify(updates):
  assert not callable(updates)

  def decorated(func):
    @wraps(func)
    def modified(self, *args, **kwargs):
      with common.env_modify(updates):
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


def also_with_fetch_streaming(f):
  assert callable(f)

  @wraps(f)
  def metafunc(self, with_fetch, *args, **kwargs):
    if with_fetch:
      self.set_setting('FETCH_STREAMING', '2')
      self.cflags += ['-DSKIP_SYNC_FETCH_TESTS']
    f(self, *args, **kwargs)

  parameterize(metafunc, {'': (False,),
                          'fetch_backend': (True,)})
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
