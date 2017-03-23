import enum
import multiprocessing
import os
import pickle
import Queue
import unittest
import time
import traceback
import sys


class ParallelTestSuite(unittest.BaseTestSuite):
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
    tests = []
    for test in self:
      tests.append(test)
    tests.sort(key=lambda test: str(test))
    return tests[::-1]

  def init_processes(self, test_queue):
    self.processes = []
    self.result_queue = multiprocessing.Queue()
    for i in xrange(num_cores()):
      p = multiprocessing.Process(target=self.testing_thread,
                                  args=(test_queue, self.result_queue))
      p.start()
      self.processes.append(p)

  @staticmethod
  def testing_thread(work_queue, result_queue):
    for test in iter(lambda: get_from_queue(work_queue), None):
      result = BufferedParallelTestResult()
      try:
        test(result)
      except Exception as e:
        result.addError(test, e)
      result_queue.put(result)

  def collect_results(self):
    buffered_results = []
    while len(self.processes) > 0:
      res = get_from_queue(self.result_queue)
      if res is not None:
        buffered_results.append(res)
      else:
        self.clear_finished_processes()
    return buffered_results

  def clear_finished_processes(self):
    self.processes = filter(lambda p: p.is_alive(), self.processes)

  def combine_results(self, result, buffered_results):
    print
    print 'DONE: combining results on main thread'
    print
    for r in sorted(buffered_results, key=lambda res:str(res.test)):
      r.updateResult(result)
    return result


class BufferedParallelTestResult(object):
  def __init__(self):
    self.kind = None
    self.test = None
    self.error = None

  def updateResult(self, result):
    result.startTest(self.test)

    if self.kind == TestResultType.Success:
      result.addSuccess(self.test)
    elif self.kind == TestResultType.Failure:
      result.addFailure(self.test, self.error)
    elif self.kind == TestResultType.Error:
      result.addError(self.test, self.error)

    result.stopTest(self.test)

  def startTest(self, test):
    pass
  def stopTest(self, test):
    pass

  def addSuccess(self, test):
    print test, '... ok'
    self.kind = TestResultType.Success
    self.test = test

  def addFailure(self, test, err):
    print test, '... FAIL:'
    print err
    self.kind = TestResultType.Failure
    self.test = test
    self._set_error(err)

  def addError(self, test, err):
    print test, '... ERROR:'
    print err
    self.kind = TestResultType.Error
    self.test = test
    self._set_error(err)

  def _set_error(self, err):
    a, b, tb = err
    self.error = (a, b, FakeTraceback(tb))


class TestResultType(enum.Enum):
  Success = 1
  Failure = 2
  Error = 3


class FakeTraceback(object):
  def __init__(self, tb):
    self.tb_frame = FakeFrame(tb.tb_frame)
    self.tb_lineno = tb.tb_lineno
    self.tb_next = FakeTraceback(tb.tb_next) if tb.tb_next is not None else None
class FakeFrame(object):
  def __init__(self, f):
    self.f_code = FakeCode(f.f_code)
    self.f_globals = [] # f.f_globals
class FakeCode(object):
  def __init__(self, co):
    self.co_filename = co.co_filename
    self.co_name = co.co_name


def num_cores():
  emcc_cores = os.environ.get('PARALLEL_SUITE_EMCC_CORES') or os.environ.get('EMCC_CORES')
  if emcc_cores:
    return int(emcc_cores)
  return multiprocessing.cpu_count()


def get_from_queue(queue):
  try:
    return queue.get(True, 0.1)
  except Queue.Empty:
    pass
  except TypeError as e:
    print ('multiprocess.Queue.get threw a TypeError. This is a bug in '
           'python 2.7 (https://bugs.python.org/issue9400)')
    print e
  return None
