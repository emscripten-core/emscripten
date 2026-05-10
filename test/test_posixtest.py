# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Runs conformance test from the upstream posixtest.

See:
   ./test/third_party/posixtestsuite
See:
   https://github.com/emscripten-core/posixtestsuite
"""

import glob
import os
import unittest

import test_posixtest_browser
from common import RunnerCore, path_from_root
from decorators import requires_pthreads

testsuite_root = path_from_root('test/third_party/posixtestsuite')


class posixtest(RunnerCore):
  """Run the suite under node (and in parallel).

  This class get populated dynamically below.
  """

  pass  # noqa: PIE790


def filter_tests(all_tests):
  prefixes = [
    'pthread_',
    'strftime',
    'asctime',
    'gmtime',
  ]

  def enable_test(t):
    return any(t.startswith(p) for p in prefixes)

  return [t for t in all_tests if enable_test(t)]


def get_tests():
  # For now, we don't require the submodule to exist.  In this case we
  # just report no tests
  test_root = os.path.join(testsuite_root, 'conformance', 'interfaces')
  if not os.path.exists(test_root):
    print('posixtestsuite not found (run git submodule update --init?)')
    return []
  tests = filter_tests(os.listdir(test_root))
  tests = [os.path.join(test_root, t) for t in tests]
  return tests


# Mark certain tests as unsupported
# TODO: Investigate failing semaphores tests.
unsupported_noreturn = {
  'test_pthread_mutex_lock_5_1': 'signals are not supported',
  'test_pthread_spin_lock_1_1': 'signals are not supported during spin lock',
  'test_pthread_spin_lock_3_1': 'signals are not supported during spin lock',
  'test_pthread_join_6_3': 'creates too many threads',
  'test_pthread_barrier_wait_3_2': 'signals are not supported',
  'test_pthread_cond_broadcast_1_2': 'tries to create 10,0000 threads, then depends on fork()',
  'test_pthread_setcanceltype_1_1': 'async cancelation does not work withing pthread_mutex_lock',
}

unsupported = {
  'test_pthread_cond_init_4_2': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_barrierattr_setpshared_1_1': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_condattr_getpshared_1_2': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_mutexattr_setpshared_1_1': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_rwlockattr_setpshared_1_1': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_condattr_setpshared_1_2': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_mutexattr_getpshared_1_2': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_mutexattr_setpshared_2_2': 'PTHREAD_PROCESS_SHARED not supported',
  'test_pthread_exit_6_1': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_1_1': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_1_2': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_2_1': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_2_2': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_3_2': 'fork() and multiple processes are not supported',
  'test_pthread_atfork_4_1': 'fork() and multiple processes are not supported',
  'test_pthread_create_1_5': 'fork() and multiple processes are not supported',
  'test_pthread_attr_setinheritsched_2_2': 'scheduling policy/parameters are not supported',
  'test_pthread_attr_setinheritsched_2_3': 'scheduling policy/parameters are not supported',
  'test_pthread_attr_setinheritsched_2_4': 'scheduling policy/parameters are not supported',
  'test_pthread_attr_setschedparam_1_3': 'scheduling policy/parameters are not supported',
  'test_pthread_attr_setschedparam_1_4': 'scheduling policy/parameters are not supported',
  'test_pthread_attr_setschedpolicy_4_1': 'scheduling policy/parameters are not supported',
  'test_pthread_barrierattr_getpshared_2_1': 'shm_open and shm_unlink are not supported',
  'test_pthread_barrier_wait_3_1': 'signals are not supported',
  'test_pthread_cond_init_1_2': 'clock_settime() is not supported',
  'test_pthread_cond_init_1_3': 'lacking necessary mmap() support',
  'test_pthread_cond_init_2_2': 'clock_settime() is not supported',
  'test_pthread_cond_init_4_1': 'fork() and multiple processes are not supported',
  'test_pthread_create_8_1': 'signals are not supported',
  'test_pthread_create_8_2': 'signals are not supported',
  'test_pthread_create_11_1': '_POSIX_THREAD_CPUTIME not supported',
  'test_pthread_getcpuclockid_1_1': 'pthread_getcpuclockid not supported',
  'test_pthread_getschedparam_1_3': 'scheduling policy/parameters are not supported',
  'test_pthread_getschedparam_1_2': 'scheduling policy/parameters are not supported',
  'test_pthread_mutexattr_getprioceiling_1_2': 'pthread_mutexattr_setprioceiling is not supported',
  'test_pthread_mutexattr_getprotocol_1_2': 'pthread_mutexattr_setprotocol is not supported',
  'test_pthread_mutexattr_setprioceiling_1_1': 'pthread_mutexattr_setprioceiling is not supported',
  'test_pthread_mutexattr_setprioceiling_3_1': 'pthread_mutexattr_setprioceiling is not supported',
  'test_pthread_mutexattr_setprioceiling_3_2': 'pthread_mutexattr_setprioceiling is not supported',
  'test_pthread_mutexattr_setprotocol_1_1': 'setting pthread_mutexattr_setprotocol to a nonzero value is not supported',
  'test_pthread_mutex_getprioceiling_1_1': 'pthread_mutex_getprioceiling is not supported',
  'test_pthread_mutex_init_5_1': 'fork() and multiple processes are not supported',
  'test_pthread_rwlockattr_getpshared_2_1': 'shm_open and shm_unlink are not supported',
  'test_pthread_rwlock_rdlock_2_1': 'thread priorities not supported, cannot test rwlocking in priority order',
  'test_pthread_rwlock_rdlock_2_2': 'thread priorities not supported, cannot test rwlocking in priority order',
  'test_pthread_rwlock_rdlock_4_1': 'signals are not supported',
  'test_pthread_rwlock_timedrdlock_6_1': 'signals are not supported',
  'test_pthread_rwlock_timedrdlock_6_2': 'signals are not supported',
  'test_pthread_rwlock_timedwrlock_6_1': 'signals are not supported',
  'test_pthread_rwlock_timedwrlock_6_2': 'signals are not supported',
  'test_pthread_rwlock_wrlock_2_1': 'signals are not supported',
  'test_pthread_setschedparam_1_1': 'scheduling policy/parameters are not supported',
  'test_pthread_setschedparam_1_2': 'scheduling policy/parameters are not supported',
  'test_pthread_setschedparam_4_1': 'scheduling policy/parameters are not supported',
  'test_pthread_setschedprio_1_1': 'scheduling policy/parameters are not supported',
  'test_pthread_spin_init_2_1': 'shm_open and shm_unlink are not supported',
  'test_pthread_spin_init_2_2': 'shm_open and shm_unlink are not supported',
}

# Mark certain tests as flaky, which may sometimes fail.
# TODO investigate these tests.
flaky = {
  'test_pthread_cond_signal_1_1': 'flaky: https://github.com/emscripten-core/emscripten/issues/13283',
  'test_pthread_barrier_wait_2_1': 'flaky: https://github.com/emscripten-core/emscripten/issues/14508',
  'test_pthread_rwlock_unlock_3_1': 'Test fail: writer did not get write lock, when main release the lock',
  'test_pthread_mutex_init_1_2': 'flaky: https://github.com/emscripten-core/posixtestsuite/pull/9',
  'test_pthread_mutex_init_3_2': 'flaky: https://github.com/emscripten-core/posixtestsuite/pull/9',
}

no_assert_tests = {
  'test_pthread_mutexattr_settype_2_1': 'contains deliberate deadlock',
}

# Mark certain tests as disabled.  These are tests that are either flaky or never return.
disabled = {
  **flaky,
  **unsupported_noreturn,
  'test_pthread_create_10_1': 'broken test: https://github.com/emscripten-core/posixtestsuite/issues/10 (also relies on SIGSEGV which we don\'t support)',
}

# These tests are known to fail reliably.  We run them anyway so that we can
# detect fixes overtime.
expect_fail = {
  **unsupported,
  'test_pthread_attr_setscope_5_1': 'internally skipped (PTS_UNTESTED)',
}


# Debugging test mode where we only run the `unsupported_noreturn` tests.  This allows us
# to periocially confirm this test test set. Normally combined with lowering the timeout with e.g.
# EMTEST_TIMEOUT=5
NORETURN_ONLY = os.environ.get('EMTEST_NORETURN_ONLY')


def make_test(name, testfile, browser):

  @requires_pthreads
  def f(self):
    if not NORETURN_ONLY and name in disabled:
      self.skipTest(disabled[name])

    args = ['-I' + os.path.join(testsuite_root, 'include'),
            '-Werror',
            '-Wno-format-security',
            '-Wno-int-conversion',
            '-Wno-format',
            '-pthread',
            # Make sure all tests have callstacks to improve debuggability
            # of log messages on CI runs.
            '--profiling-funcs',
            '-sEXIT_RUNTIME',
            '-sTOTAL_MEMORY=256mb',
            '-sPTHREAD_POOL_SIZE=40']
    if name in no_assert_tests:
      args.append('-sASSERTIONS=0')
    if browser:
      self.btest_exit(testfile, cflags=args)
    else:
      self.do_runf(testfile, cflags=args, output_basename=name)

  if name in expect_fail:
    f = unittest.expectedFailure(f)

  return f


for testdir in get_tests():
  basename = os.path.basename(testdir)
  for test_file in glob.glob(os.path.join(testdir, '*.c')):
    if not os.path.basename(test_file)[0].isdigit():
      continue
    test_suffix = os.path.splitext(os.path.basename(test_file))[0]
    test_suffix = test_suffix.replace('-', '_')
    test_name = 'test_' + basename + '_' + test_suffix
    if NORETURN_ONLY and test_name not in unsupported_noreturn:
      continue
    setattr(posixtest, test_name, make_test(test_name, test_file, browser=False))
    setattr(test_posixtest_browser.posixtest_browser, test_name, make_test(test_name, test_file, browser=True))
