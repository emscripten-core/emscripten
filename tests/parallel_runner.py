import enum
import multiprocessing
import os
import unittest
import time
import sys


class TestResultType(enum.Enum):
  Success = 1
  Failure = 2
  Error = 3


class BufferedParallelTestResult(object):
  def __init__(self):
    self.result_type = None
    self.result_test = None
    self.result_error = None

  def updateResult(self, result):
    result.startTest(self.result_test)

    if self.result_type == TestResultType.Success:
      result.addSuccess(self.result_test)
    elif self.result_type == TestResultType.Failure:
      result.addFailure(self.result_test, self._get_error())
    elif self.result_type == TestResultType.Error:
      result.addError(self.result_test, self._get_error())

    result.stopTest(self.result_test)

  def startTest(self, test):
    pass
  def stopTest(self, test):
    pass

  def addSuccess(self, test):
    print test, '... ok'
    self.result_type = TestResultType.Success
    self.result_test = test

  def addFailure(self, test, err):
    print test, '... FAIL:'
    print err
    self.result_type = TestResultType.Failure
    self.result_test = test
    self._set_error(err)

  def addError(self, test, err):
    print test, '... ERROR:'
    print err
    self.result_type = TestResultType.Error
    self.result_test = test
    self._set_error(err)

  def _set_error(self, err):
    a, b, _ = err
    self.result_error = (a, b, None)

  def _get_error(self):
    if self.result_error is None:
      return None

    a, b, tb = self.result_error
    fake_tb = self.fake_traceback(tb)
    return (a, b, fake_tb)

  @staticmethod
  def fake_traceback(real_traceback):
    # TODO: create a fake traceback object that mirrors the string info of the real one
    try:
      # We need to construct a traceback object and this is a hacky way to do that
      1/0
    except Exception as e:
      return sys.exc_info()[2]


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
    for test in self:
      test_queue.put(test)
    return test_queue

  def init_processes(self, test_queue):
    self.processes = []
    self.result_queue = multiprocessing.Queue()
    for i in xrange(self.num_cores()):
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

  @staticmethod
  def num_cores():
    emcc_cores = os.environ.get('EMCC_CORES')
    if emcc_cores:
      return int(emcc_cores)
    return multiprocessing.cpu_count()

  def collect_results(self):
    buffered_results = []
    while len(self.processes) > 0:
      res = get_from_queue(self.result_queue)
      if res is None:
        self.processes = filter(lambda p: p.is_alive(), self.processes)
      else:
        buffered_results.append(res)
    return buffered_results

  def combine_results(self, result, buffered_results):
    for r in buffered_results:
      r.updateResult(result)
    return result


def get_from_queue(queue):
  import Queue
  try:
    return queue.get(True, 0.1)
  except Queue.Empty:
    return None
  except TypeError as e:
    print ('multiprocess.Queue.get threw a TypeError. This is a bug in '
           'python 2.7 (https://bugs.python.org/issue9400)')
    print e
    return None
