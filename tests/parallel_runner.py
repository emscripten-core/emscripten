# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import multiprocessing
import os
import subprocess
import sys
import unittest
import tempfile

from tools.tempfiles import try_delete

try:
  import queue
except ImportError:
  # Python 2 compatibility
  import Queue as queue


def g_testing_thread(work_queue, result_queue, temp_dir):
  for test in iter(lambda: get_from_queue(work_queue), None):
    result = BufferedParallelTestResult()
    test.set_temp_dir(temp_dir)
    try:
      test(result)
    except unittest.SkipTest as e:
      result.addSkip(test, e)
    except Exception as e:
      result.addError(test, e)
    result_queue.put(result)


class ParallelTestSuite(unittest.BaseTestSuite):
  """Runs a suite of tests in parallel.

  Creates worker threads, manages the task queue, and combines the results.
  """

  def __init__(self):
    super(ParallelTestSuite, self).__init__()
    self.processes = None
    self.result_queue = None

  def run(self, result):
    test_queue = self.create_test_queue()
    self.init_processes(test_queue)
    results = self.collect_results()
    return self.combine_results(result, results)

  def create_test_queue(self):
    test_queue = multiprocessing.Queue()
    for test in self.reversed_tests():
      test_queue.put(test)
    return test_queue

  def reversed_tests(self):
    """A list of this suite's tests in reverse order.

    Many of the tests in test_core are intentionally named so that long tests
    fall toward the end of the alphabet (e.g. test_the_bullet). Tests are
    loaded in alphabetical order, so here we reverse that in order to start
    running longer tasks earlier, which should lead to better core utilization.

    Future work: measure slowness of tests and sort accordingly.
    """
    tests = []
    for test in self:
      tests.append(test)
    tests.sort(key=str)
    return tests[::-1]

  def init_processes(self, test_queue):
    self.processes = []
    self.result_queue = multiprocessing.Queue()
    self.dedicated_temp_dirs = [tempfile.mkdtemp() for x in range(num_cores())]
    print('Using %s parallel test processes' % len(self.dedicated_temp_dirs))
    for temp_dir in self.dedicated_temp_dirs:
      p = multiprocessing.Process(target=g_testing_thread,
                                  args=(test_queue, self.result_queue, temp_dir))
      p.start()
      self.processes.append(p)

  def collect_results(self):
    buffered_results = []
    while len(self.processes):
      res = get_from_queue(self.result_queue)
      if res is not None:
        buffered_results.append(res)
      else:
        self.clear_finished_processes()
    for temp_dir in self.dedicated_temp_dirs:
      try_delete(temp_dir)
    return buffered_results

  def clear_finished_processes(self):
    self.processes = [p for p in self.processes if p.is_alive()]

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


class BufferedParallelTestResult(object):
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
    pass

  def stopTest(self, test):
    pass

  def addSuccess(self, test):
    print(test, '... ok', file=sys.stderr)
    self.buffered_result = BufferedTestSuccess(test)

  def addSkip(self, test, reason):
    print(test, "... skipped '%s'" % reason, file=sys.stderr)
    self.buffered_result = BufferedTestSkip(test, reason)

  def addFailure(self, test, err):
    print(test, '... FAIL', file=sys.stderr)
    self.buffered_result = BufferedTestFailure(test, err)

  def addError(self, test, err):
    print(test, '... ERROR', file=sys.stderr)
    self.buffered_result = BufferedTestError(test, err)


class BufferedTestBase(object):
  """Abstract class that holds test result data, split by type of result."""
  def __init__(self, test, err=None):
    self.test = test
    if err:
      exctype, value, tb = err
      if exctype == subprocess.CalledProcessError:
        # multiprocess.Queue can't serialize a subprocess.CalledProcessError.
        # This is a bug in python 2.7 (https://bugs.python.org/issue9400)
        exctype = Exception
        value = Exception(str(value))
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


class BufferedTestFailure(BufferedTestBase):
  def updateResult(self, result):
    result.addFailure(self.test, self.error)


class BufferedTestError(BufferedTestBase):
  def updateResult(self, result):
    result.addError(self.test, self.error)


class FakeTraceback(object):
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


class FakeFrame(object):
  def __init__(self, f):
    self.f_code = FakeCode(f.f_code)
    # f.f_globals is not picklable, not used in stack traces, and needs to be iterable
    self.f_globals = []


class FakeCode(object):
  def __init__(self, co):
    self.co_filename = co.co_filename
    self.co_name = co.co_name


def num_cores():
  emcc_cores = os.environ.get('PARALLEL_SUITE_EMCC_CORES') or os.environ.get('EMCC_CORES')
  if emcc_cores:
    return int(emcc_cores)
  return multiprocessing.cpu_count()


def get_from_queue(q):
  try:
    return q.get(True, 0.1)
  except queue.Empty:
    pass
  return None
