# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import multiprocessing
import os
import sys
import tempfile
import time
import unittest

import common

from tools.shared import cap_max_workers_in_pool


NUM_CORES = None


def run_test(test):
  olddir = os.getcwd()
  result = BufferedParallelTestResult()
  temp_dir = tempfile.mkdtemp(prefix='emtest_')
  test.set_temp_dir(temp_dir)
  try:
    test(result)
  except unittest.SkipTest as e:
    result.addSkip(test, e)
  except Exception as e:
    result.addError(test, e)
  # Before attempting to delete the tmp dir make sure the current
  # working directory is not within it.
  os.chdir(olddir)
  common.force_delete_dir(temp_dir)
  return result


class ParallelTestSuite(unittest.BaseTestSuite):
  """Runs a suite of tests in parallel.

  Creates worker threads, manages the task queue, and combines the results.
  """

  def __init__(self, max_cores):
    super().__init__()
    self.max_cores = max_cores

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
    tests = list(self.reversed_tests())
    use_cores = cap_max_workers_in_pool(min(self.max_cores, len(tests), num_cores()))
    print('Using %s parallel test processes' % use_cores)
    pool = multiprocessing.Pool(use_cores)
    results = [pool.apply_async(run_test, (t,)) for t in tests]
    results = [r.get() for r in results]
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
    return reversed(sorted(self, key=str))

  def combine_results(self, result, buffered_results):
    print()
    print('DONE: combining results on main thread')
    print()
    # Sort the results back into alphabetical order. Running the tests in
    # parallel causes mis-orderings, this makes the results more readable.
    results = sorted(buffered_results, key=lambda res: str(res.test))
    for r in results:
      r.updateResult(result)
    return result


class BufferedParallelTestResult():
  """A picklable struct used to communicate test results across processes

  Fulfills the interface for unittest.TestResult
  """
  def __init__(self):
    self.buffered_result = None

  @property
  def test(self):
    return self.buffered_result.test

  def updateResult(self, result):
    result.startTest(self.test)
    self.buffered_result.updateResult(result)
    result.stopTest(self.test)

  def startTest(self, test):
    self.start_time = time.perf_counter()

  def stopTest(self, test):
    # TODO(sbc): figure out a way to display this duration information again when
    # these results get passed back to the TextTestRunner/TextTestResult.
    if hasattr(time, 'perf_counter'):
      self.buffered_result.duration = time.perf_counter() - self.start_time

  def addSuccess(self, test):
    if hasattr(time, 'perf_counter'):
      print(test, '... ok (%.2fs)' % (time.perf_counter() - self.start_time), file=sys.stderr)
    self.buffered_result = BufferedTestSuccess(test)

  def addExpectedFailure(self, test, err):
    if hasattr(time, 'perf_counter'):
      print(test, '... expected failure (%.2fs)' % (time.perf_counter() - self.start_time), file=sys.stderr)
    self.buffered_result = BufferedTestExpectedFailure(test, err)

  def addUnexpectedSuccess(self, test):
    if hasattr(time, 'perf_counter'):
      print(test, '... unexpected success (%.2fs)' % (time.perf_counter() - self.start_time), file=sys.stderr)
    self.buffered_result = BufferedTestUnexpectedSuccess(test)

  def addSkip(self, test, reason):
    print(test, "... skipped '%s'" % reason, file=sys.stderr)
    self.buffered_result = BufferedTestSkip(test, reason)

  def addFailure(self, test, err):
    print(test, '... FAIL', file=sys.stderr)
    self.buffered_result = BufferedTestFailure(test, err)

  def addError(self, test, err):
    print(test, '... ERROR', file=sys.stderr)
    self.buffered_result = BufferedTestError(test, err)


class BufferedTestBase():
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


class FakeTraceback():
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


class FakeFrame():
  def __init__(self, f):
    self.f_code = FakeCode(f.f_code)
    # f.f_globals is not picklable, not used in stack traces, and needs to be iterable
    self.f_globals = []


class FakeCode():
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
