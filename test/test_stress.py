# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Stress test versions of some existing tests from test_core.py
These don't run in test_core.py itself because that is already run in parallel and these
stress tests each saturate the CPU cores.

TODO: Find a way to replace these tests with an `@also_with_stress_test` decorator.
Hopfully we can replace the current parallelism with `taskset -u 0` to force the test
only run on a single core (would limit the tests to linux-only).
"""

import multiprocessing
import subprocess
import threading

from common import (
  NON_ZERO,
  RunnerCore,
  also_with_modularize,
  is_slow_test,
  node_pthreads,
)


class stress(RunnerCore):
  def parallel_stress_test_js_file(self, js_file, assert_returncode=None, expected=None, not_expected=None):
    # If no expectations were passed, expect a successful run exit code
    if assert_returncode is None and expected is None and not_expected is None:
      assert_returncode = 0

    # We will use Python multithreading, so prepare the command to run in advance, and keep the threading kernel
    # compact to avoid accessing unexpected data/functions across threads.
    cmd = self.get_engine_with_args() + [js_file]

    exception_thrown = threading.Event()
    error_lock = threading.Lock()
    error_exception = None

    def test_run():
      nonlocal error_exception
      try:
        # Each thread repeatedly runs the test case in a tight loop, which is critical to coax out timing related issues
        for _ in range(16):
          # Early out from the test, if error was found
          if exception_thrown.is_set():
            return
          result = subprocess.run(cmd, capture_output=True, text=True)

          output = f'\n----------------------------\n{result.stdout}{result.stderr}\n----------------------------'
          if not_expected is not None and not_expected in output:
            raise Exception(f'\n\nWhen running command "{cmd}",\nexpected string "{not_expected}" to NOT be present in output:{output}')
          if expected is not None and expected not in output:
            raise Exception(f'\n\nWhen running command "{cmd}",\nexpected string "{expected}" was not found in output:{output}')
          if assert_returncode is not None:
            if assert_returncode == NON_ZERO:
              if result.returncode != 0:
                raise Exception(f'\n\nCommand "{cmd}" was expected to fail, but did not (returncode=0). Output:{output}')
            elif assert_returncode != result.returncode:
              raise Exception(f'\n\nWhen running command "{cmd}",\nreturn code {result.returncode} does not match expected return code {assert_returncode}. Output:{output}')
      except Exception as e:
        if not exception_thrown.is_set():
          exception_thrown.set()
          with error_lock:
            error_exception = e
        return

    threads = []
    # Oversubscribe hardware threads to make sure scheduling becomes erratic
    while len(threads) < 2 * multiprocessing.cpu_count() and not exception_thrown.is_set():
      threads += [threading.Thread(target=test_run)]
      threads[-1].start()
    for t in threads:
      t.join()
    if error_exception:
      raise error_exception

  # This is a stress test version that focuses on https://github.com/emscripten-core/emscripten/issues/20067
  @node_pthreads
  @is_slow_test
  def test_stress_proxy_to_pthread_hello_world(self):
    self.skipTest('Occassionally hangs. https://github.com/emscripten-core/emscripten/issues/20067')
    self.set_setting('ABORT_ON_WASM_EXCEPTIONS')
    self.set_setting('PROXY_TO_PTHREAD')
    self.set_setting('EXIT_RUNTIME')
    js_file = self.build('core/test_hello_world.c')
    self.parallel_stress_test_js_file(js_file, assert_returncode=0, expected='hello, world!', not_expected='error')

  # This is a stress test to verify that the Node.js postMessage() vs uncaughtException
  # race does not affect Emscripten execution.
  @node_pthreads
  @is_slow_test
  def test_stress_pthread_abort(self):
    self.set_setting('PROXY_TO_PTHREAD')
    # Add the onAbort handler at runtime during preRun.  This means that onAbort
    # handler will only be present in the main thread (much like it would if it
    # was passed in by pre-populating the module object on prior to loading).
    self.add_pre_run("Module.onAbort = () => console.log('My custom onAbort called');")
    self.cflags += ['-sINCOMING_MODULE_JS_API=preRun,onAbort']
    js_file = self.build('pthread/test_pthread_abort.c')
    self.parallel_stress_test_js_file(js_file, expected='My custom onAbort called')
    # TODO: investigate why adding assert_returncode=NON_ZERO to above doesn't work.
    # Is the test test_pthread_abort still flaky?

  @node_pthreads
  @also_with_modularize
  @is_slow_test
  def test_stress_pthread_proxying(self):
    if '-sMODULARIZE' in self.cflags:
      if self.get_setting('WASM') == 0:
        self.skipTest('MODULARIZE + WASM=0 + pthreads does not work (#16794)')
      self.set_setting('EXPORT_NAME=ModuleFactory')
    self.set_setting('PROXY_TO_PTHREAD')
    if not self.has_changed_setting('INITIAL_MEMORY'):
      self.set_setting('INITIAL_MEMORY=32mb')

    js_file = self.build('pthread/test_pthread_proxying_reduced_stress_test_case.c')
    self.parallel_stress_test_js_file(js_file, not_expected='pthread_self() == unknown', expected='pthread_self() == worker2', assert_returncode=0)
