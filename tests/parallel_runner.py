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


class ParallelTextTestResult(object):
  def __init__(self):
    self.resultType = None
    self.resultTest = None
    self.resultError = None

  def startTest(self, test):
    pass
  def stopTest(self, test):
    pass

  def addSuccess(self, test):
    print test, '... ok'
    self.resultType = TestResultType.Success
    self.resultTest = test

  def addFailure(self, test, err):
    print test, '... FAIL:'
    print err
    self.resultType = TestResultType.Failure
    self.resultTest = test
    self._setError(err)

  def addError(self, test, err):
    print test, '... ERROR:'
    print err
    self.resultType = TestResultType.Error
    self.resultTest = test
    self._setError(err)

  def _setError(self, err):
    a, b, _ = err
    self.resultError = (a, b, None)


class ParallelTestSuite(unittest.BaseTestSuite):
  def run(self, result):
    import Queue
    end_of_queue = 'STOP'
    def worker(work_queue, result_queue):
      for test in iter(work_queue.get, end_of_queue):
        result = ParallelTextTestResult()
        try:
          test(result)
        except Exception as e:
          result.addError(test, e)
        result_queue.put(result)

    num_workers = ParallelTestSuite.num_cores()
    processes = []
    test_queue = multiprocessing.Queue()
    result_queue = multiprocessing.Queue()
    for test in self:
      test_queue.put(test)
    for i in xrange(num_workers):
      p = multiprocessing.Process(target=worker, args=(test_queue, result_queue))
      p.start()
      processes.append(p)
      test_queue.put(end_of_queue)

    worker_status = {}
    buffered_results = []
    while len(processes) > 0:
      try:
        res = result_queue.get(True, 0.1)
        buffered_results.append(res)
      except Queue.Empty:
        processes = filter(lambda p: p.is_alive(), processes)
      except TypeError as e:
        # Quirk of python 2.7 (marked WNF) : https://bugs.python.org/issue9400
        print 'hopefully got a subprocess.CalledProcessError:', e

    for r in buffered_results:
      s = r.resultTest
      result.startTest(s)
      err = None
      if r.resultError is not None:
        tb = None
        try:
          # We need to reconstruct some traceback and this is a hacky way to do that
          1/0
        except Exception as e:
          tb = sys.exc_info()[2]
        a, b, c = r.resultError
        err = (a, b, tb)
      if r.resultType == TestResultType.Success:
        result.addSuccess(s)
      elif r.resultType == TestResultType.Failure:
        result.addFailure(s, err)
      elif r.resultType == TestResultType.Error:
        result.addError(s, err)
      result.stopTest(s)

    return result

  @staticmethod
  def num_cores():
    emcc_cores = os.environ.get('EMCC_CORES')
    if emcc_cores:
      return int(emcc_cores)
    return multiprocessing.cpu_count()
