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

import browser_common
import common
from color_runner import BufferingMixin
from common import errlog

from tools import emprofile, utils
from tools.utils import WINDOWS

EMTEST_VISUALIZE = os.getenv('EMTEST_VISUALIZE')
NUM_CORES = None
seen_class = set()
torn_down = False


# Older Python versions have a bug with multiprocessing shared data
# structures. https://github.com/emscripten-core/emscripten/issues/25103
# and https://github.com/python/cpython/issues/71936
def python_multiprocessing_structures_are_buggy():
  v = sys.version_info
  return (v.major, v.minor, v.micro) <= (3, 12, 7) or (v.major, v.minor, v.micro) == (3, 13, 0)


def cap_max_workers_in_pool(max_workers, is_browser):
  if is_browser and 'EMTEST_CORES' not in os.environ and 'EMCC_CORES' not in os.environ:
    # TODO experiment with this number. In browser tests we'll be creating
    # a browser instance per worker which is expensive.
    max_workers = max_workers // 2
  # Python has an issue that it can only use max 61 cores on Windows: https://github.com/python/cpython/issues/89240
  if WINDOWS:
    return min(max_workers, 61)
  return max_workers


def run_test(args):
  test, allowed_failures_counter, buffer = args
  # If we have exceeded the number of allowed failures during the test run,
  # abort executing further tests immediately.
  if allowed_failures_counter and allowed_failures_counter.value < 0:
    return None

  # Handle setUpClass which needs to be called on each worker
  # TODO: Better handling of exceptions that happen during setUpClass
  if test.__class__ not in seen_class:
    seen_class.add(test.__class__)
    test.__class__.setUpClass()

  start_time = time.perf_counter()
  olddir = os.getcwd()
  result = BufferedParallelTestResult()
  result.start_time = start_time
  result.buffer = buffer
  temp_dir = tempfile.mkdtemp(prefix='emtest_')
  test.set_temp_dir(temp_dir)
  try:
    test(result)
  except KeyboardInterrupt:
    # In case of KeyboardInterrupt do not emit buffered stderr/stdout
    # as we unwind.
    result._mirrorOutput = False
  finally:
    result.elapsed = time.perf_counter() - start_time

  # Before attempting to delete the tmp dir make sure the current
  # working directory is not within it.
  os.chdir(olddir)
  common.force_delete_dir(temp_dir)

  # Since we are returning this result to the main thread we need to make sure
  # that it is serializable/picklable. To do this, we delete any non-picklable
  # fields from the instance.
  del result._original_stdout
  del result._original_stderr

  return result


# Executes the teardown process once. Returns True if the teardown was
# performed, False if it was already torn down.
def tear_down():
  global torn_down
  if torn_down:
    return False
  torn_down = True
  for cls in seen_class:
    cls.tearDownClass()
  return True


class ParallelTestSuite(unittest.BaseTestSuite):
  """Runs a suite of tests in parallel.

  Creates worker threads, manages the task queue, and combines the results.
  """

  def __init__(self, max_cores, options):
    super().__init__()
    self.max_cores = max_cores
    self.max_failures = options.max_failures
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

    # No need to worry about stdout/stderr buffering since are a not
    # actually running the test here, only setting the results.
    buffer = result.buffer
    result.buffer = False

    result.core_time = 0
    tests = self.get_sorted_tests()
    self.num_tests = self.countTestCases()
    contains_browser_test = any(test.is_browser_test() for test in tests)
    use_cores = cap_max_workers_in_pool(min(self.max_cores, len(tests), num_cores()), contains_browser_test)
    errlog(f'Using {use_cores} parallel test processes')
    with multiprocessing.Manager() as manager:
      # Give each worker a unique ID.
      worker_id_counter = manager.Value('i', 0) # 'i' for integer, starting at 0
      worker_id_lock = manager.Lock()
      with multiprocessing.Pool(
        processes=use_cores,
        initializer=browser_common.init_worker,
        initargs=(worker_id_counter, worker_id_lock),
      ) as pool:
        if python_multiprocessing_structures_are_buggy():
          # When multiprocessing shared structures are buggy we don't support failfast
          # or the progress bar.
          allowed_failures_counter = None
          if self.max_failures < 2**31 - 1:
            errlog('The version of python being used is not compatible with --failfast and --max-failures options. See https://github.com/python/cpython/issues/71936')
            sys.exit(1)
        else:
          allowed_failures_counter = manager.Value('i', self.max_failures)

        results = []
        args = ((t, allowed_failures_counter, buffer) for t in tests)
        for res in pool.imap_unordered(run_test, args, chunksize=1):
          # results may be be None if # of allowed errors was exceeded
          # and the harness aborted.
          if res:
            if res.test_result not in ['success', 'skipped'] and allowed_failures_counter is not None:
              # Signal existing multiprocess pool runners so that they can exit early if needed.
              allowed_failures_counter.value -= 1
            res.integrate_result(result)
            results.append(res)

        # Send a task to each worker to tear down the browser and server. This
        # relies on the implementation detail in the worker pool that all workers
        # are cycled through once.
        num_tear_downs = sum([pool.apply(tear_down, ()) for i in range(use_cores)])
        # Assert the assumed behavior above hasn't changed.
        if num_tear_downs != use_cores and not buffer:
          errlog(f'Expected {use_cores} teardowns, got {num_tear_downs}')

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

    if EMTEST_VISUALIZE:
      self.visualize_results(results)
    return result

  def get_sorted_tests(self):
    """A list of this suite's tests, sorted with the @is_slow_test tests first.

    Future work: measure and store the speed of tests each test sort more accurately.
    """
    if self.failing_and_slow_first:
      # If we are running with --failing-and-slow-first, then the test list has been
      # pre-sorted based on previous test run results (see `runner.py`)
      return list(self)

    def test_key(test):
      testMethod = getattr(test, test._testMethodName)
      is_slow = getattr(testMethod, 'is_slow', False)
      return (is_slow, str(test))

    return sorted(self, key=test_key, reverse=True)

  def visualize_results(self, results):
    assert EMTEST_VISUALIZE
    # Sort the results back into alphabetical order. Running the tests in
    # parallel causes mis-orderings, this makes the results more readable.
    results = sorted(results, key=lambda res: str(res.test))

    # shared data structures are hard in the python multi-processing world, so
    # use a file to share the flaky test information across test processes.
    flaky_tests = open(common.flaky_tests_log_filename).read().split() if os.path.isfile(common.flaky_tests_log_filename) else []
    # Extract only the test short names
    flaky_tests = [x.split('.')[-1] for x in flaky_tests]

    for r in results:
      r.log_test_run_for_visualization(flaky_tests)

    # Generate the parallel test run visualization
    emprofile.create_profiling_graph(utils.path_from_root('out/graph'))
    # Cleanup temp files that were used for the visualization
    emprofile.delete_profiler_logs()
    utils.delete_file(common.flaky_tests_log_filename)


class BufferedParallelTestResult(BufferingMixin, unittest.TestResult):
  """A picklable struct used to communicate test results across processes
  """
  def __init__(self):
    super().__init__()
    self.test_duration = 0
    self.test_result = 'errored'
    self.test_name = ''
    self.test = None

  def test_short_name(self):
    # Given a test name e.g. "test_atomic_cxx (test_core.core0.test_atomic_cxx)"
    # returns a short form "test_atomic_cxx" of the test.
    return self.test_name.split(' ', 1)[0]

  def addDuration(self, test, elapsed):
    self.test_duration = elapsed

  def integrate_result(self, overall_results):
    """This method get called on the main thread once the buffered result
    is received.  It adds the buffered result to the overall result."""

    # Turns a <test, string> pair back into something that looks enoough
    # link a <test, exc_info> pair. The exc_info tripple has the exception
    # type as its first element. This is needed in particilar in the
    # XMLTestRunner.
    def restore_exc_info(pair):
      test, exn_string = pair
      assert self.last_err_type, exn_string
      return (test, (self.last_err_type, exn_string, None))

    # Our fame exc_info tripple keep the pre-serialized string in the
    # second element of the triple so we overide _exc_info_to_string
    # _exc_info_to_string to simply return it.
    overall_results._exc_info_to_string = lambda x, _y: x[1]

    overall_results.startTest(self.test)
    if self.test_result == 'success':
      overall_results.addSuccess(self.test)
    elif self.test_result == 'failed':
      overall_results.addFailure(*restore_exc_info(self.failures[0]))
    elif self.test_result == 'errored':
      overall_results.addError(*restore_exc_info(self.errors[0]))
    elif self.test_result == 'skipped':
      overall_results.addSkip(*self.skipped[0])
    elif self.test_result == 'unexpected success':
      overall_results.addUnexpectedSuccess(self.unexpectedSuccesses[0])
    elif self.test_result == 'expected failure':
      overall_results.addExpectedFailure(*restore_exc_info(self.expectedFailures[0]))
    else:
      assert False, f'unhandled test result {self.test_result}'
    overall_results.stopTest(self.test)
    overall_results.core_time += self.test_duration

  def log_test_run_for_visualization(self, flaky_tests):
    assert EMTEST_VISUALIZE
    if self.test_result != 'skipped' or self.test_duration > 0.2:
      test_result = self.test_result
      if test_result == 'success' and self.test_short_name() in flaky_tests:
        test_result = 'warnings'
      profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')
      os.makedirs(profiler_logs_path, exist_ok=True)
      profiler_log_file = os.path.join(profiler_logs_path, 'toolchain_profiler.pid_0.json')
      color = {
        'success': '#80ff80',
        'warnings': '#ffb040',
        'skipped': '#a0a0a0',
        'expected failure': '#ff8080',
        'unexpected success': '#ff8080',
        'failed': '#ff8080',
        'errored': '#ff8080',
      }[test_result]
      # Write profiling entries for emprofile.py tool to visualize. This needs a unique identifier for each
      # block, so generate one on the fly.
      dummy_test_task_counter = os.path.getsize(profiler_log_file) if os.path.isfile(profiler_log_file) else 0
      # Remove the redundant 'test_' prefix from each test, since character space is at a premium in the visualized graph.
      test_name = utils.removeprefix(self.test_short_name(), 'test_')
      with open(profiler_log_file, 'a') as prof:
        prof.write(f',\n{{"pid":{dummy_test_task_counter},"op":"start","time":{self.start_time},"cmdLine":["{test_name}"],"color":"{color}"}}')
        prof.write(f',\n{{"pid":{dummy_test_task_counter},"op":"exit","time":{self.start_time + self.test_duration},"returncode":0}}')

  def startTest(self, test):
    super().startTest(test)
    self.test = test
    self.test_name = str(test)

  def addSuccess(self, test):
    super().addSuccess(test)
    self.test_result = 'success'

  def addExpectedFailure(self, test, err):
    super().addExpectedFailure(test, err)
    self.last_err_type = err[0]
    self.test_result = 'expected failure'

  def addUnexpectedSuccess(self, test):
    super().addUnexpectedSuccess(test)
    self.test_result = 'unexpected success'

  def addSkip(self, test, reason):
    super().addSkip(test, reason)
    self.test_result = 'skipped'

  def addFailure(self, test, err):
    super().addFailure(test, err)
    self.last_err_type = err[0]
    self.test_result = 'failed'

  def addError(self, test, err):
    super().addError(test, err)
    self.last_err_type = err[0]
    self.test_result = 'errored'


def num_cores():
  if NUM_CORES:
    return int(NUM_CORES)
  return utils.get_num_cores()
