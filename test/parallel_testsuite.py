# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import json
import multiprocessing
import os
import sys
import tempfile
import time
import unittest
from tools import emprofile
from tools import utils

import common

from tools.shared import cap_max_workers_in_pool


NUM_CORES = None
seen_class = set()


def run_test(test, failfast_event):
  # If failfast mode is in effect and any of the tests have failed,
  # and then we should abort executing further tests immediately.
  if failfast_event and failfast_event.is_set():
    return None

  olddir = os.getcwd()
  result = BufferedParallelTestResult()
  temp_dir = tempfile.mkdtemp(prefix='emtest_')
  test.set_temp_dir(temp_dir)
  try:
    if test.__class__ not in seen_class:
      seen_class.add(test.__class__)
      test.__class__.setUpClass()
    test(result)

    # Alert all other multiprocess pool runners that they need to stop executing further tests.
    if failfast_event is not None and result.test_result not in ['success', 'skipped']:
      failfast_event.set()
  except unittest.SkipTest as e:
    result.addSkip(test, e)
  except Exception as e:
    result.addError(test, e)
    if failfast_event is not None:
      failfast_event.set()
  # Before attempting to delete the tmp dir make sure the current
  # working directory is not within it.
  os.chdir(olddir)
  common.force_delete_dir(temp_dir)
  return result


class ParallelTestSuite(unittest.BaseTestSuite):
  """Runs a suite of tests in parallel.

  Creates worker threads, manages the task queue, and combines the results.
  """

  def __init__(self, max_cores, options):
    super().__init__()
    self.max_cores = max_cores
    self.failfast = options.failfast
    self.failing_and_slow_first = options.failing_and_slow_first

  def addTest(self, test):
    super().addTest(test)
    test.is_parallel = True

  def run(self, result):
    # The 'spawn' method is used on windows and it can be useful to set this on
    # all platforms when debugging multiprocessing issues.  Without this we
    # default to 'fork' on unix which is better because global state is
    # inherited by the child process, but can lead to hard-to-debug windows-only
    # issues.
    # multiprocessing.set_start_method('spawn')

    # Remove any old stale list of flaky tests before starting the run
    if common.EMTEST_FLAKY_TEST_LOG_FILE:
      utils.delete_file(common.EMTEST_FLAKY_TEST_LOG_FILE)

    # If we are running with --failing-and-slow-first, then the test list has been
    # pre-sorted based on previous test run results. Otherwise run the tests in
    # reverse alphabetical order.
    tests = list(self if self.failing_and_slow_first else self.reversed_tests())
    use_cores = cap_max_workers_in_pool(min(self.max_cores, len(tests), num_cores()))
    print('Using %s parallel test processes' % use_cores)
    with multiprocessing.Manager() as manager:
      pool = multiprocessing.Pool(use_cores)
      failfast_event = manager.Event() if self.failfast else None
      results = [pool.apply_async(run_test, (t, failfast_event)) for t in tests]
      results = [r.get() for r in results]
      results = [r for r in results if r is not None]

    if self.failing_and_slow_first:
      previous_test_run_results = common.load_previous_test_run_results()
      for r in results:
        # Save a test result record with the specific suite name (e.g. "core0.test_foo")
        test_failed = r.test_result not in ['success', 'skipped']

        def update_test_results_to(test_name):
          fail_frequency = previous_test_run_results[test_name]['fail_frequency'] if test_name in previous_test_run_results else int(test_failed)
          # Apply exponential moving average with 50% weighting to merge previous fail frequency with new fail frequency
          fail_frequency = (fail_frequency + int(test_failed)) / 2
          previous_test_run_results[test_name] = {
            'result': r.test_result,
            'duration': r.test_duration,
            'fail_frequency': fail_frequency,
          }

        update_test_results_to(r.test_name)
        # Also save a test result record without suite name (e.g. just "test_foo"). This enables different suite runs to order tests
        # for quick --failfast termination, in case a test fails in multiple suites
        update_test_results_to(r.test_name.split(' ')[0])

      json.dump(previous_test_run_results, open(common.PREVIOUS_TEST_RUN_RESULTS_FILE, 'w'), indent=2)
    pool.close()
    pool.join()
    return self.combine_results(result, results)

  def reversed_tests(self):
    """A list of this suite's tests, sorted reverse alphabetical order.

    Many of the tests in test_core are intentionally named so that long tests
    fall toward the end of the alphabet (e.g. test_the_bullet). Tests are
    loaded in alphabetical order, so here we reverse that in order to start
    running longer tasks earlier, which should lead to better core utilization.

    Future work: measure slowness of tests and sort accordingly.
    """
    return sorted(self, key=str, reverse=True)

  def combine_results(self, result, buffered_results):
    print()
    print('DONE: combining results on main thread')
    print()
    # Sort the results back into alphabetical order. Running the tests in
    # parallel causes mis-orderings, this makes the results more readable.
    results = sorted(buffered_results, key=lambda res: str(res.test))
    result.core_time = 0

    flaky_tests = open(common.EMTEST_FLAKY_TEST_LOG_FILE).read().split() if os.path.isfile(common.EMTEST_FLAKY_TEST_LOG_FILE) else []

    for r in results:
      # Merge information of flaky tests into the test result
      if r.test_result == 'success' and r.test_short_name() in flaky_tests:
        r.test_result = 'warnings'
      # And integrate the test result to the global test object
      r.updateResult(result)

    # Generate the parallel test run visualization
    if os.getenv('EMTEST_VISUALIZE'):
      emprofile.create_profiling_graph(utils.path_from_root('out/graph'))
      emprofile.delete_profiler_logs()

    return result


test_task_counter = 0

class BufferedParallelTestResult:
  """A picklable struct used to communicate test results across processes

  Fulfills the interface for unittest.TestResult
  """
  def __init__(self):
    self.buffered_result = None
    self.test_duration = 0
    self.test_result = 'errored'
    self.test_name = ''

  @property
  def test(self):
    return self.buffered_result.test

  def test_short_name(self):
    # Given a test name e.g. "test_atomic_cxx (test_core.core0.test_atomic_cxx)"
    # returns a short form "test_atomic_cxx" of the test.
    return self.test_name[:self.test_name.find(' ')]

  def addDuration(self, test, elapsed):
    self.test_duration = elapsed

  def calculateElapsed(self):
    return time.perf_counter() - self.start_time

  def updateResult(self, result):
    result.startTest(self.test)
    self.buffered_result.updateResult(result)
    result.stopTest(self.test)
    result.core_time += self.test_duration
    self.log_test_run_for_visualization()

  def log_test_run_for_visualization(self):
    if os.getenv('EMTEST_VISUALIZE') and (self.test_result != 'skipped' or self.test_duration > 0.2):
      profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')
      os.makedirs(profiler_logs_path, exist_ok=True)
      profiler_log_file = os.path.join(profiler_logs_path, 'toolchain_profiler.pid_0.json')
      prof = open(profiler_log_file, 'a')
      colors = {
        'success': '#80ff80',
        'warnings': '#ffb040',
        'skipped': '#a0a0a0',
        'expected failure': '#ff8080',
        'unexpected success': '#ff8080',
        'failed': '#ff8080',
        'errored': '#ff8080',
      }
      # Write profiling entries for emprofile.py tool to visualize. This needs a unique identifier for each
      # block, so generate one on the fly.
      dummy_test_task_counter = os.path.getsize(profiler_log_file) if os.path.isfile(profiler_log_file) else 0
      prof.write(f',\n{{"pid":{dummy_test_task_counter},"op":"start","time":{self.start_time},"cmdLine":["{self.test_short_name()}"],"color":"{colors[self.test_result]}"}}')
      prof.write(f',\n{{"pid":{dummy_test_task_counter},"op":"exit","time":{self.start_time+self.test_duration},"returncode":0}}')

  def startTest(self, test):
    self.test_name = str(test)
    self.start_time = time.perf_counter()

  def stopTest(self, test):
    # TODO(sbc): figure out a way to display this duration information again when
    # these results get passed back to the TextTestRunner/TextTestResult.
    self.buffered_result.duration = self.test_duration

  def addSuccess(self, test):
    print(test, '... ok (%.2fs)' % (self.calculateElapsed()), file=sys.stderr)
    self.buffered_result = BufferedTestSuccess(test)
    self.test_result = 'success'

  def addExpectedFailure(self, test, err):
    print(test, '... expected failure (%.2fs)' % (self.calculateElapsed()), file=sys.stderr)
    self.buffered_result = BufferedTestExpectedFailure(test, err)
    self.test_result = 'expected failure'

  def addUnexpectedSuccess(self, test):
    print(test, '... unexpected success (%.2fs)' % (self.calculateElapsed()), file=sys.stderr)
    self.buffered_result = BufferedTestUnexpectedSuccess(test)
    self.test_result = 'unexpected success'

  def addSkip(self, test, reason):
    print(test, "... skipped '%s'" % reason, file=sys.stderr)
    self.buffered_result = BufferedTestSkip(test, reason)
    self.test_result = 'skipped'

  def addFailure(self, test, err):
    print(test, '... FAIL', file=sys.stderr)
    self.buffered_result = BufferedTestFailure(test, err)
    self.test_result = 'failed'

  def addError(self, test, err):
    print(test, '... ERROR', file=sys.stderr)
    self.buffered_result = BufferedTestError(test, err)
    self.test_result = 'errored'


class BufferedTestBase:
  """Abstract class that holds test result data, split by type of result."""
  def __init__(self, test, err=None):
    self.test = test
    if err:
      exctype, value, tb = err
      self.error = exctype, value, FakeTraceback(tb)

  def updateResult(self, result):
    assert False, 'Base class should not be used directly'


class BufferedTestSuccess(BufferedTestBase):
  def updateResult(self, result):
    result.addSuccess(self.test)


class BufferedTestSkip(BufferedTestBase):
  def __init__(self, test, reason):
    self.test = test
    self.reason = reason

  def updateResult(self, result):
    result.addSkip(self.test, self.reason)


def fixup_fake_exception(fake_exc):
  ex = fake_exc[2]
  if ex.tb_frame.f_code.positions is None:
    return
  while ex is not None:
    # .co_positions is supposed to be a function that returns an enumerable
    # to the list of code positions. Create a function object wrapper around
    # the data
    def make_wrapper(rtn):
      return lambda: rtn
    ex.tb_frame.f_code.co_positions = make_wrapper(ex.tb_frame.f_code.positions)
    ex = ex.tb_next


class BufferedTestFailure(BufferedTestBase):
  def updateResult(self, result):
    fixup_fake_exception(self.error)
    result.addFailure(self.test, self.error)


class BufferedTestExpectedFailure(BufferedTestBase):
  def updateResult(self, result):
    fixup_fake_exception(self.error)
    result.addExpectedFailure(self.test, self.error)


class BufferedTestError(BufferedTestBase):
  def updateResult(self, result):
    fixup_fake_exception(self.error)
    result.addError(self.test, self.error)


class BufferedTestUnexpectedSuccess(BufferedTestBase):
  def updateResult(self, result):
    fixup_fake_exception(self.error)
    result.addUnexpectedSuccess(self.test)


class FakeTraceback:
  """A fake version of a traceback object that is picklable across processes.

  Python's traceback objects contain hidden stack information that isn't able
  to be pickled. Further, traceback objects aren't constructable from Python,
  so we need a dummy object that fulfills its interface.

  The fields we expose are exactly those which are used by
  unittest.TextTestResult to show a text representation of a traceback. Any
  other use is not intended.
  """

  def __init__(self, tb):
    self.tb_frame = FakeFrame(tb.tb_frame)
    self.tb_lineno = tb.tb_lineno
    self.tb_next = FakeTraceback(tb.tb_next) if tb.tb_next is not None else None
    self.tb_lasti = tb.tb_lasti


class FakeFrame:
  def __init__(self, f):
    self.f_code = FakeCode(f.f_code)
    # f.f_globals is not picklable, not used in stack traces, and needs to be iterable
    self.f_globals = []


class FakeCode:
  def __init__(self, co):
    self.co_filename = co.co_filename
    self.co_name = co.co_name
    # co.co_positions() returns an iterator. Flatten it to a list so that it can
    # be pickled to the parent process
    if hasattr(co, 'co_positions'):
      self.positions = list(co.co_positions())
    else:
      self.positions = None


def num_cores():
  if NUM_CORES:
    return int(NUM_CORES)
  return multiprocessing.cpu_count()
