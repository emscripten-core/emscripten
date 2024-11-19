#!/usr/bin/env python3
# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is the Emscripten test runner. To run some tests, specify which tests
you want, for example

  test/runner asm1.test_hello_world

There are many options for which tests to run and how to run them. For details,
see

http://kripken.github.io/emscripten-site/docs/getting_started/test-suite.html
"""

# Use EMTEST_ALL_ENGINES=1 in the environment or pass --all-engines to test all engines!

import argparse
import atexit
import fnmatch
import glob
import logging
import math
import operator
import os
import platform
import random
import sys
import unittest

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, __rootpath__)

import jsrun
import parallel_testsuite
import common
from tools import shared, config, utils


sys.path.append(utils.path_from_root('third_party/websockify'))

logger = logging.getLogger("runner")


# Test modes from 'core' that fully pass all tests. When running a random
# selection of tests (e.g. "random100" runs 100 random tests) they will be
# chosen from here.
passing_core_test_modes = [
  'core0',
  'core1',
  'core2',
  'core3',
  'cores',
  'corez',
  'core_2gb',
  'strict',
  'strict_js',
  'wasm2js0',
  'wasm2js1',
  'wasm2js2',
  'wasm2js3',
  'wasm2jss',
  'wasm2jsz',
  'asan',
  'lsan',
  'ubsan',
  'wasm64',
  'wasm64_v8',
  'wasm64_4gb',
]

# The default core test mode, used when none is specified
default_core_test_mode = 'core0'

# All test modes aside from passing_core_test_modes. These modes might be wip
# and not all pass yet, or they might be non-'core' (e.g., if they open a
# browser window or otherwise have custom functionality). Random tests are not
# picked from here, but you can force them to be, using something like
# randombrowser10 (which runs 10 random tests from 'browser').
misc_test_modes = [
  'other',
  'browser',
  'sanity',
  'sockets',
  'interactive',
  'benchmark',
  'wasm2ss',
  'posixtest',
  'posixtest_browser',
  'minimal0',
  'wasmfs',
  'wasm64',
  'wasm64l',
  'bigint',
  'browser64',
  'browser64_4gb',
  'browser_2gb',
]


def check_js_engines():
  working_engines = [e for e in config.JS_ENGINES if jsrun.check_engine(e)]
  if len(working_engines) < len(config.JS_ENGINES):
    print('Not all the JS engines in JS_ENGINES appears to work.')
    exit(1)

  if common.EMTEST_ALL_ENGINES:
    print('(using ALL js engines)')


def get_and_import_modules():
  modules = []
  for filename in glob.glob(os.path.join(common.TEST_ROOT, 'test*.py')):
    module_dir, module_file = os.path.split(filename)
    module_name, module_ext = os.path.splitext(module_file)
    __import__(module_name)
    modules.append(sys.modules[module_name])
  return modules


def get_all_tests(modules):
  # Create a list of all known tests so that we can choose from them based on a wildcard search
  all_tests = []
  suites = passing_core_test_modes + misc_test_modes
  for m in modules:
    for s in suites:
      if hasattr(m, s):
        tests = [t for t in dir(getattr(m, s)) if t.startswith('test_')]
        all_tests += [s + '.' + t for t in tests]
  return all_tests


def get_crossplatform_tests(modules):
  suites = ['core0', 'other', 'sanity'] # We don't need all versions of every test
  crossplatform_tests = []
  # Walk over the test suites and find the test functions with the
  # is_crossplatform_test attribute applied by @crossplatform decorator
  for m in modules:
    for s in suites:
      if hasattr(m, s):
        testclass = getattr(m, s)
        for funcname in dir(testclass):
          if hasattr(getattr(testclass, funcname), 'is_crossplatform_test'):
            crossplatform_tests.append(s + '.' + funcname)
  return crossplatform_tests


def tests_with_expanded_wildcards(args, all_tests):
  # Process wildcards, e.g. "browser.test_pthread_*" should expand to list all pthread tests
  new_args = []
  for arg in args:
    if '*' in arg:
      if arg.startswith('skip:'):
        arg = arg[5:]
        matching_tests = fnmatch.filter(all_tests, arg)
        new_args += ['skip:' + t for t in matching_tests]
      else:
        new_args += fnmatch.filter(all_tests, arg)
    else:
      new_args += [arg]
  if not new_args and args:
    print('No tests found to run in set: ' + str(args))
    sys.exit(1)
  return new_args


def skip_test(tests_to_skip, modules):
  suite_name, test_name = tests_to_skip.split('.')
  skipped = False
  for m in modules:
    suite = getattr(m, suite_name, None)
    if suite:
      setattr(suite, test_name, lambda s: s.skipTest("requested to be skipped"))
      skipped = True
      break
  assert skipped, "Not able to skip test " + tests_to_skip


def skip_requested_tests(args, modules):
  os.environ['EMTEST_SKIP'] = ''
  for i, arg in enumerate(args):
    if arg.startswith('skip:'):
      which = arg.split('skip:')[1]
      os.environ['EMTEST_SKIP'] = os.environ['EMTEST_SKIP'] + ' ' + which
      print('will skip "%s"' % which, file=sys.stderr)
      skip_test(which, modules)
      args[i] = None
  return [a for a in args if a is not None]


def args_for_random_tests(args, modules):
  if not args:
    return args
  first = args[0]
  if first.startswith('random'):
    random_arg = first[6:]
    num_tests, base_module, relevant_modes = get_random_test_parameters(random_arg)
    for m in modules:
      if hasattr(m, base_module):
        base = getattr(m, base_module)
        new_args = choose_random_tests(base, num_tests, relevant_modes)
        print_random_test_statistics(num_tests)
        return new_args
  return args


def get_random_test_parameters(arg):
  num_tests = 1
  base_module = default_core_test_mode
  relevant_modes = passing_core_test_modes
  if len(arg):
    num_str = arg
    for mode in passing_core_test_modes + misc_test_modes:
      if arg.startswith(mode):
        base_module = mode
        relevant_modes = [mode]
        num_str = arg.replace(mode, '')
        break
    num_tests = int(num_str)
  return num_tests, base_module, relevant_modes


def choose_random_tests(base, num_tests, relevant_modes):
  tests = [t for t in dir(base) if t.startswith('test_')]
  print()
  chosen = set()
  while len(chosen) < num_tests:
    test = random.choice(tests)
    mode = random.choice(relevant_modes)
    new_test = mode + '.' + test
    before = len(chosen)
    chosen.add(new_test)
    if len(chosen) > before:
      print('* ' + new_test)
    else:
      # we may have hit the limit
      if len(chosen) == len(tests) * len(relevant_modes):
        print('(all possible tests chosen! %d = %d*%d)' % (len(chosen), len(tests), len(relevant_modes)))
        break
  return list(chosen)


def print_random_test_statistics(num_tests):
  std = 0.5 / math.sqrt(num_tests)
  expected = 100.0 * (1.0 - std)
  print()
  print('running those %d randomly-selected tests. if they all pass, then there is a '
        'greater than 95%% chance that at least %.2f%% of the test suite will pass'
        % (num_tests, expected))
  print()

  def show():
    print('if all tests passed then there is a greater than 95%% chance that at least '
          '%.2f%% of the test suite will pass'
          % (expected))
  atexit.register(show)


def error_on_legacy_suite_names(args):
  for a in args:
    if a.startswith('wasm') and not any(a.startswith(p) for p in ('wasm2js', 'wasmfs', 'wasm64')):
      new = a.replace('wasm', 'core', 1)
      utils.exit_with_error('`%s` test suite has been replaced with `%s`', a, new)


def load_test_suites(args, modules, start_at, repeat):
  found_start = not start_at

  loader = unittest.TestLoader()
  error_on_legacy_suite_names(args)
  unmatched_test_names = set(args)
  suites = []

  total_tests = 0
  for m in modules:
    names_in_module = []
    for name in list(unmatched_test_names):
      try:
        operator.attrgetter(name)(m)
        names_in_module.append(name)
        unmatched_test_names.remove(name)
      except AttributeError:
        pass
    if len(names_in_module):
      loaded_tests = loader.loadTestsFromNames(sorted(names_in_module), m)
      tests = flattened_tests(loaded_tests)
      suite = suite_for_module(m, tests)
      for test in tests:
        if not found_start:
          # Skip over tests until we find the start
          if test.id().endswith(start_at):
            found_start = True
          else:
            continue
        for _x in range(repeat):
          total_tests += 1
          suite.addTest(test)
      suites.append((m.__name__, suite))
  if not found_start:
    utils.exit_with_error(f'unable to find --start-at test: {start_at}')
  if total_tests == 1 or parallel_testsuite.num_cores() == 1:
    # TODO: perhaps leave it at 2 if it was 2 before?
    common.EMTEST_SAVE_DIR = 1
  return suites, unmatched_test_names


def flattened_tests(loaded_tests):
  tests = []
  for subsuite in loaded_tests:
    for test in subsuite:
      tests.append(test)
  return tests


def suite_for_module(module, tests):
  suite_supported = module.__name__ in ('test_core', 'test_other', 'test_posixtest')
  if not common.EMTEST_SAVE_DIR and not shared.DEBUG:
    has_multiple_tests = len(tests) > 1
    has_multiple_cores = parallel_testsuite.num_cores() > 1
    if suite_supported and has_multiple_tests and has_multiple_cores:
      return parallel_testsuite.ParallelTestSuite(len(tests))
  return unittest.TestSuite()


def run_tests(options, suites):
  resultMessages = []
  num_failures = 0

  print('Test suites:')
  print([s[0] for s in suites])
  # Run the discovered tests

  # We currently don't support xmlrunner on macOS M1 runner since
  # `pip` doesn't seeem to yet have pre-built binaries for M1.
  if os.getenv('CI') and not (utils.MACOS and platform.machine() == 'arm64'):
    os.makedirs('out', exist_ok=True)
    # output fd must remain open until after testRunner.run() below
    output = open('out/test-results.xml', 'wb')
    import xmlrunner # type: ignore
    testRunner = xmlrunner.XMLTestRunner(output=output, verbosity=2,
                                         failfast=options.failfast)
    print('Writing XML test output to ' + os.path.abspath(output.name))
  else:
    testRunner = unittest.TextTestRunner(verbosity=2, failfast=options.failfast)

  for mod_name, suite in suites:
    print('Running %s: (%s tests)' % (mod_name, suite.countTestCases()))
    res = testRunner.run(suite)
    msg = ('%s: %s run, %s errors, %s failures, %s skipped' %
           (mod_name, res.testsRun, len(res.errors), len(res.failures), len(res.skipped)))
    num_failures += len(res.errors) + len(res.failures) + len(res.unexpectedSuccesses)
    resultMessages.append(msg)

  if len(resultMessages) > 1:
    print('====================')
    print()
    print('TEST SUMMARY')
    for msg in resultMessages:
      print('    ' + msg)

  return num_failures


def parse_args(args):
  parser = argparse.ArgumentParser(prog='runner.py', description=__doc__)
  parser.add_argument('--save-dir', action='store_true',
                      help='Save the temporary directory used during for each '
                           'test.  Implies --cores=1.  Defaults to true when running a single test')
  parser.add_argument('--no-clean', action='store_true',
                      help='Do not clean the temporary directory before each test run')
  parser.add_argument('--verbose', '-v', action='store_true')
  parser.add_argument('--all-engines', action='store_true')
  parser.add_argument('--detect-leaks', action='store_true')
  parser.add_argument('--skip-slow', action='store_true', help='Skip tests marked as slow')
  parser.add_argument('--cores', '-j',
                      help='Set the number tests to run in parallel.  Defaults '
                           'to the number of CPU cores.', default=None)
  parser.add_argument('--rebaseline', action='store_true',
                      help='Automatically update test expectations for tests that support it.')
  parser.add_argument('--browser',
                      help='Command to launch web browser in which to run browser tests.')
  parser.add_argument('tests', nargs='*')
  parser.add_argument('--failfast', action='store_true')
  parser.add_argument('--start-at', metavar='NAME', help='Skip all tests up until <NAME>')
  parser.add_argument('--continue', dest='_continue', action='store_true',
                      help='Resume from the last run test.'
                           'Useful when combined with --failfast')
  parser.add_argument('--force64', action='store_true')
  parser.add_argument('--crossplatform-only', action='store_true')
  parser.add_argument('--repeat', type=int, default=1,
                      help='Repeat each test N times (default: 1).')
  return parser.parse_args()


def configure():
  common.EMTEST_BROWSER = os.getenv('EMTEST_BROWSER')
  common.EMTEST_DETECT_TEMPFILE_LEAKS = int(os.getenv('EMTEST_DETECT_TEMPFILE_LEAKS', '0'))
  common.EMTEST_ALL_ENGINES = int(os.getenv('EMTEST_ALL_ENGINES', '0'))
  common.EMTEST_SKIP_SLOW = int(os.getenv('EMTEST_SKIP_SLOW', '0'))
  common.EMTEST_SKIP_FLAKY = int(os.getenv('EMTEST_SKIP_FLAKY', '0'))
  common.EMTEST_RETRY_FLAKY = int(os.getenv('EMTEST_RETRY_FLAKY', '0'))
  common.EMTEST_LACKS_NATIVE_CLANG = int(os.getenv('EMTEST_LACKS_NATIVE_CLANG', '0'))
  common.EMTEST_REBASELINE = int(os.getenv('EMTEST_REBASELINE', '0'))
  common.EMTEST_VERBOSE = int(os.getenv('EMTEST_VERBOSE', '0')) or shared.DEBUG
  if common.EMTEST_VERBOSE:
    logging.root.setLevel(logging.DEBUG)

  assert 'PARALLEL_SUITE_EMCC_CORES' not in os.environ, 'use EMTEST_CORES rather than PARALLEL_SUITE_EMCC_CORES'
  parallel_testsuite.NUM_CORES = os.environ.get('EMTEST_CORES') or os.environ.get('EMCC_CORES')


def main(args):
  options = parse_args(args)

  # Some options make sense being set in the environment, others not-so-much.
  # TODO(sbc): eventually just make these command-line only.
  if os.getenv('EMTEST_SAVE_DIR'):
    print('ERROR: use --save-dir instead of EMTEST_SAVE_DIR=1, and --no-clean instead of EMTEST_SAVE_DIR=2')
    return 1
  if os.getenv('EMTEST_REBASELINE'):
    print('Prefer --rebaseline over setting $EMTEST_REBASELINE')
  if os.getenv('EMTEST_VERBOSE'):
    print('Prefer --verbose over setting $EMTEST_VERBOSE')

  # We set the environments variables here and then call configure,
  # to apply them.  This means the python's multiprocessing child
  # process will see the same configuration even though they don't
  # parse the command line.
  def set_env(name, option_value):
    if option_value is None:
      return
    if option_value is False:
      value = '0'
    elif option_value is True:
      value = '1'
    else:
      value = str(option_value)
    os.environ[name] = value

  set_env('EMTEST_BROWSER', options.browser)
  set_env('EMTEST_DETECT_TEMPFILE_LEAKS', options.detect_leaks)
  if options.save_dir:
    common.EMTEST_SAVE_DIR = 1
  if options.no_clean:
    common.EMTEST_SAVE_DIR = 2
  set_env('EMTEST_SKIP_SLOW', options.skip_slow)
  set_env('EMTEST_ALL_ENGINES', options.all_engines)
  set_env('EMTEST_REBASELINE', options.rebaseline)
  set_env('EMTEST_VERBOSE', options.verbose)
  set_env('EMTEST_CORES', options.cores)
  set_env('EMTEST_FORCE64', options.force64)

  configure()

  check_js_engines()

  def prepend_default(arg):
    if arg.startswith('test_'):
      return default_core_test_mode + '.' + arg
    return arg

  tests = [prepend_default(t) for t in options.tests]

  modules = get_and_import_modules()
  all_tests = get_all_tests(modules)
  if options.crossplatform_only:
    tests = get_crossplatform_tests(modules)
    skip_requested_tests(options.tests, modules)
  else:
    tests = tests_with_expanded_wildcards(tests, all_tests)
    tests = skip_requested_tests(tests, modules)
    tests = args_for_random_tests(tests, modules)

  if not options.start_at and options._continue:
    if os.path.exists(common.LAST_TEST):
      options.start_at = utils.read_file(common.LAST_TEST).strip()

  suites, unmatched_tests = load_test_suites(tests, modules, options.start_at, options.repeat)
  if unmatched_tests:
    print('ERROR: could not find the following tests: ' + ' '.join(unmatched_tests))
    return 1

  num_failures = run_tests(options, suites)
  # Return the number of failures as the process exit code
  # for automating success/failure reporting.  Return codes
  # over 125 are not well supported on UNIX.
  return min(num_failures, 125)


configure()

if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.warning('KeyboardInterrupt')
    sys.exit(1)
else:
  # We are not the main process, and most likely a child process of
  # the multiprocess pool.  In this mode the modifications made to the
  # test class by `skip_test` need to be re-applied in each child
  # subprocess (sad but true).  This is needed in particular on macOS
  # and Windows where the default mode for multiprocessing is `spawn`
  # rather than `fork`
  if 'EMTEST_SKIP' in os.environ:
    modules = get_and_import_modules()
    for skip in os.environ['EMTEST_SKIP'].split():
      skip_test(skip, modules)
