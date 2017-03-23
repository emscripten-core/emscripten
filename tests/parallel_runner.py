import enum
import multiprocessing
import os
import pickle
import Queue
import unittest
import time
import traceback
import sys


class TestResultType(enum.Enum):
  Success = 1
  Failure = 2
  Error = 3


def can_pickle(item):
  try:
    pickle.dumps(item)
    return True
  except TypeError as e:
    # print 'cannot pickle', item, 'because:', e
    # traceback.print_tb(sys.exc_info()[2])
    return False
  except:
    print "UNEXPECTED EXCEPTION FROM pickle.dumps"

class FakeObject(object):
  def __init__(self):
    print 'initing', self
    self.__real_attrs = {}

  def __getstate__(self):
    print 'getting state of', self
    print 'state is', self.__real_attrs
    return self.__real_attrs

  def __setstate__(self, state):
    print 'setting state of', self, state
    self.__real_attrs = state

  def __getattr__(self, key):
    print 'getting attr of', self, key
    if key in ('__getinitargs__', '__getnewargs__'):
      raise AttributeError
    if key in ('_FakeObject__real_attrs'):
      print 'oh dear'
      return self.__dict__[key]
    print 'dict is', self.__real_attrs
    return self.__real_attrs[key]

  def __setattr__(self, key, value):
    print 'setting attr of', self, key, value
    if key in ('_FakeObject__real_attrs'):
      self.__dict__[key] = value
    else:
      self.__real_attrs[key] = value

class AutoFaker(object):
  def __init__(self, wrapped):
    print 'initing w/', wrapped
    self.__wrapped = wrapped
    self.__faked = {}
    for method in dir(wrapped):
      print 'considering', method
      a = getattr(wrapped, method)
      if callable(a) and method != '__class__':
        print 'trying to set', method
        def wrapped(s, *args, **kwargs):
          a(s, *args, **kwargs)
        setattr(self, method, wrapped)
      print '  worked'
    print 'done w/ init'

  def __getattr__(self, key):
    print 'autofake get:', key
    if not self.__faked.get(key):
      val = getattr(self.__wrapped, key)
      if not can_pickle(val):
        val = AutoFaker(val)
      self.__faked[key] = val
    return self.__faked[key]

  # def __iter__(self):
  #   print 'itering'
  #   setattr(self.__faked, '__iter__', self.__wrapped.__iter__)
  #   return self.__wrapped.__iter__()

  # def next(self):
  #   print 'nexting'
  #   setattr(self.__faked, 'next', self.__wrapped.next)
  #   return self.__wrapped.next()

  def reify_fake_object(self):
    print 'reifying'
    result = FakeObject()
    for k, v in self.__faked.iteritems():
      if isinstance(v, AutoFaker):
        setattr(result, k, v.reify_fake_object())
    for method in dir(self.wrapped):
      a = getattr(self.wrapped, method)
      if callable(a):
        setattr(result, method, a)
    return result

  def make_dict(self):
    result = {}
    for k, v in self.__faked.iteritems():
      if isinstance(v, AutoFaker):
        result[k] = v.make_dict()
      else:
        result[k] = v
    return result
    # return self.__faked

def fake_for(item, funcs):
  print 'faking:', item
  fake = AutoFaker(item)
  print 'trying'
  try:
    for f in funcs:
      f(fake)
    ret = fake.reify_fake_object()
  except Exception as e:
    print 'aw dang:', e
    raise e
  print 'fake:', ret
  print ret.__dict__
  print 'fakedict:', fake.make_dict()
  print 'can pickle:', can_pickle(ret)
  print 'pickled:', pickle.dumps(ret)
  rounded = pickle.loads(pickle.dumps(ret))
  print 'roundtripped:', rounded.__dict__
  print 'wamp'
  return ret

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
      result.addFailure(self.result_test, self.result_error)
    elif self.result_type == TestResultType.Error:
      result.addError(self.result_test, self.result_error)

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
    if can_pickle(err):
      self.result_error = err
    else:
      a, b, tb = err
      self.result_error = (a, b, self.fake_traceback(tb))

  @staticmethod
  def fake_traceback(real_traceback):
    # TODO: create a fake traceback object that mirrors the string info of the real one
    '''
    Need to mock:
      tb_frame
      tb_lineno
      tb_next

    In particular:
      list = []
      n = 0
      while tb is not None and (limit is None or n < limit):
          f = tb.tb_frame
          lineno = tb.tb_lineno
          co = f.f_code
          filename = co.co_filename
          name = co.co_name
          linecache.checkcache(filename)
          line = linecache.getline(filename, lineno, f.f_globals)
          if line: line = line.strip()
          else: line = None
          list.append((filename, lineno, name, line))
          tb = tb.tb_next
          n = n+1
      return list
    '''
    return fake_for(real_traceback, [
      traceback.extract_tb,
      lambda tb: 'foo' in tb.tb_frame.f_globals
    ])
    # try:
    #   # We need to construct a traceback object and this is a hacky way to do that
    #   1/0
    # except Exception as e:
    #   return sys.exc_info()[2]


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
    tests = []
    for test in self:
      tests.append(test)
    tests.sort(key=lambda test: str(test))
    for test in tests[::-1]:
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
      if res is not None:
        buffered_results.append(res)
      else:
        self.clear_finished_processes()
    return buffered_results

  def clear_finished_processes(self):
    self.processes = filter(lambda p: p.is_alive(), self.processes)

  def combine_results(self, result, buffered_results):
    print 'DONE: combining results'
    for r in sorted(buffered_results, key=lambda res:str(res.result_test)):
      r.updateResult(result)
    return result


def get_from_queue(queue):
  try:
    return queue.get(True, 0.1)
  except Queue.Empty:
    return None
  except TypeError as e:
    print ('multiprocess.Queue.get threw a TypeError. This is a bug in '
           'python 2.7 (https://bugs.python.org/issue9400)')
    print e
    return None
