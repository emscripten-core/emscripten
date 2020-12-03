# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Runs the pthreads test from the upstream posixtest suite in:
   ./tests/third_party/posixtestsuite
See
   https://github.com/juj/posixtestsuite
"""

import glob
import os

from runner import RunnerCore, path_from_root
from tools import config
from tools.shared import EMCC
import test_posixtest_browser

testsuite_root = path_from_root('tests', 'third_party', 'posixtestsuite')


class posixtest(RunnerCore):
  """Run the suite under node (and in parallel)

  This class get populated dynamically below.
  """
  pass


def filter_tests(all_tests):
  pthread_tests = [t for t in all_tests if t.startswith('pthread_')]
  # filter out some tests we don't support
  pthread_tests = [t for t in pthread_tests if not t.startswith('pthread_atfork')]
  pthread_tests = [t for t in pthread_tests if not t.startswith('pthread_sigmask')]
  return pthread_tests


def get_pthread_tests():
  # For now, we don't require the submodule to exist.  In this case we just report
  # no tests
  pthread_test_root = os.path.join(testsuite_root, 'conformance', 'interfaces')
  if not os.path.exists(pthread_test_root):
    print('posixtestsuite not found (run git submodule update --init?)')
    return []
  pthread_tests = filter_tests(os.listdir(pthread_test_root))
  pthread_tests = [os.path.join(pthread_test_root, t) for t in pthread_tests]
  return pthread_tests


engine = config.NODE_JS + ['--experimental-wasm-threads', '--experimental-wasm-bulk-memory']

# Mark certain tests as not passing
disabled = {
  'test_pthread_create_11_1': 'never returns',
  'test_pthread_barrier_wait_2_1': 'never returns',
  'test_pthread_cond_timedwait_2_6': 'never returns',
  'test_pthread_cond_timedwait_4_3': 'never returns',
  'test_pthread_attr_setscope_5_1': 'internally skipped (PTS_UNTESTED)',
  'test_pthread_cond_wait_2_3': 'never returns',
  'test_pthread_create_5_1': 'never returns',
  'test_pthread_exit_1_2': 'never returns',
  'test_pthread_exit_2_2': 'never returns',
  'test_pthread_exit_3_2': 'never returns',
  'test_pthread_exit_4_1': 'never returns',
  'test_pthread_getcpuclockid_1_1': 'never returns',
  'test_pthread_key_create_1_2': 'never returns',
  'test_pthread_rwlock_rdlock_1_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedrdlock_1_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedrdlock_3_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedrdlock_5_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedwrlock_1_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedwrlock_3_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_timedwrlock_5_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_wrlock_1_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_rwlock_trywrlock_1_1': 'fails with "main: Unexpected thread state"',
  'test_pthread_spin_destroy_3_1': 'never returns',
  'test_pthread_spin_init_4_1': 'never returns',
}


def make_test(name, testfile, browser):

  def f(self):
    if name in disabled:
      self.skipTest(disabled[name])
    args = ['-I' + os.path.join(testsuite_root, 'include'),
            '-Werror',
            '-Wno-format-security',
            '-Wno-int-conversion',
            '-sUSE_PTHREADS',
            '-sEXIT_RUNTIME',
            '-sTOTAL_MEMORY=268435456',
            '-sPTHREAD_POOL_SIZE=40']
    if browser:
      # Only are only needed for browser tests of the was btest
      # injects headers using `-include` flag.
      args += ['-Wno-macro-redefined', '-D_GNU_SOURCE']
      self.btest(testfile, args=args, expected='exit:0')
    else:
      self.run_process([EMCC, testfile, '-o', 'test.js'] + args)
      self.run_js('test.js', engine=engine)

  return f


for testdir in get_pthread_tests():
  basename = os.path.basename(testdir)
  for test_file in glob.glob(os.path.join(testdir, '*.c')):
    if not os.path.basename(test_file)[0].isdigit():
      continue
    test_suffix = os.path.splitext(os.path.basename(test_file))[0]
    test_suffix = test_suffix.replace('-', '_')
    test_name = 'test_' + basename + '_' + test_suffix
    setattr(posixtest, test_name, make_test(test_name, test_file, browser=False))
    setattr(test_posixtest_browser.posixtest_browser, test_name, make_test(test_name, test_file, browser=True))
