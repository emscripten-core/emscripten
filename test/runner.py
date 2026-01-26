#!/usr/bin/env python3
# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is the Emscripten test runner. To run some tests, specify which tests
you want, for example

  test/runner core0.test_hello_world

There are many options for which tests to run and how to run them. For details,
see

http://kripken.github.io/emscripten-site/docs/getting_started/test-suite.html
"""

# Use EMTEST_ALL_ENGINES=1 in the environment or pass --all-engines to test all engines!

import argparse
import atexit
import datetime
import fnmatch
import glob
import logging
import math
import operator
import os
import random
import re
import subprocess
import sys
import time
import unittest
from functools import cmp_to_key

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, __rootpath__)

import browser_common
import common
import jsrun
import parallel_testsuite
from color_runner import ColorTextRunner
from common import errlog
from single_line_runner import SingleLineTestRunner

from tools import building, colored_logger, config, shared, utils

logger = logging.getLogger("runner")

# In git checkouts of emscripten `bootstrap.py` exists to run post-checkout
# steps.  In packaged versions (e.g. emsdk) this file does not exist (because
# it is excluded in tools/install.py) and these steps are assumed to have been
# run already.
if os.path.exists('.git') and os.path.exists('bootstrap.py'):
  import bootstrap
  bootstrap.check()
else:
  # The test framework depends on writing files to the `out/` directory.
  # For git checkouts the bootstrap.py script would take care of creating this.
  os.makedirs('out', exist_ok=True)


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
  'lto0',
  'lto1',
  'lto2',
  'lto3',
  'ltos',
  'ltoz',
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
  'wasm64_4gb',
  'esm_integration',
  'instance',
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
  'jslib',
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
  if not all(jsrun.check_engine(e) for e in config.JS_ENGINES):
    errlog('Not all the JS engines in JS_ENGINES appear to work.')
    sys.exit(1)

  if common.EMTEST_ALL_ENGINES:
    errlog('(using ALL js engines)')


def get_and_import_modules():
  modules = []
  for filename in glob.glob(os.path.join(common.TEST_ROOT, 'test*.py')):
    module_file = os.path.basename(filename)
    module_name = os.path.splitext(module_file)[0]
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
    errlog('No tests found to run in set: ' + str(args))
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
      errlog(f'will skip "{which}"')
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


# Creates a sorter object that sorts the test run order to find the best possible
# order to run the tests in. Generally this is slowest-first to maximize
# parallelization, but if running with fail-fast, then the tests with recent
# known failure frequency are run first, followed by slowest first.
def create_test_run_sorter(sort_failing_tests_at_front):
  previous_test_run_results = common.load_previous_test_run_results()

  def read_approx_fail_freq(test_name):
    if test_name in previous_test_run_results and 'fail_frequency' in previous_test_run_results[test_name]:
      # Quantize the float value to relatively fine-grained buckets for sorting.
      # This bucketization is needed to merge two competing sorting goals: we may
      # want to fail early (so tests with previous history of failures should sort first)
      # but we also want to run the slowest tests first.
      # We cannot sort for both goals at the same time, so have failure frequency
      # take priority over test runtime, and quantize the failures to distinct
      # frequencies, to be able to then sort by test runtime inside the same failure
      # frequency bucket.
      NUM_BUCKETS = 20
      return round(previous_test_run_results[test_name]['fail_frequency'] * NUM_BUCKETS) / NUM_BUCKETS
    return 0

  def sort_tests_failing_and_slowest_first_comparator(x, y):
    x = str(x)
    y = str(y)

    # Look at the number of times this test has failed, and order by failures count first
    # Only do this if we are looking to fail early. (otherwise sorting by last test run duration is more productive)
    if sort_failing_tests_at_front:
      x_fail_freq = read_approx_fail_freq(x)
      y_fail_freq = read_approx_fail_freq(y)
      if x_fail_freq != y_fail_freq:
        return y_fail_freq - x_fail_freq

      # Look at the number of times this test has failed overall in any other suite, and order by failures count first
      x_fail_freq = read_approx_fail_freq(x.split(' ')[0])
      y_fail_freq = read_approx_fail_freq(y.split(' ')[0])
      if x_fail_freq != y_fail_freq:
        return y_fail_freq - x_fail_freq

    if x in previous_test_run_results:
      X = previous_test_run_results[x]

      # if test Y has not been run even once, run Y before X
      if y not in previous_test_run_results:
        return 1
      Y = previous_test_run_results[y]

      # If both X and Y have been run before, order the tests based on what the previous result was (failures first, skips very last)
      # N.b. it is important to sandwich all skipped tests between fails and successes. This is to maximize the chances that when
      # a failing test is detected, then the other cores will fail-fast as well. (successful tests are run slowest-first to help
      # scheduling)
      order_by_result = {'errored': 0, 'failed': 1, 'expected failure': 2, 'unexpected success': 3, 'skipped': 4, 'success': 5}
      x_result = order_by_result[X['result']]
      y_result = order_by_result[Y['result']]
      if x_result != y_result:
        return x_result - y_result

      # Finally, order by test duration from last run
      if X['duration'] != Y['duration']:
        if X['result'] == 'success':
          # If both tests were successful tests, run the slower test first to improve parallelism
          return Y['duration'] - X['duration']
        else:
          # If both tests were failing tests, run the quicker test first to improve --failfast detection time
          return X['duration'] - Y['duration']

    # if test X has not been run even once, but Y has, run X before Y
    if y in previous_test_run_results:
      return -1

    # Neither test have been run before, so run them in alphabetical order
    return (x > y) - (x < y)

  return sort_tests_failing_and_slowest_first_comparator


def load_test_suites(args, modules, options):
  found_start = not options.start_at

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
    if names_in_module:
      # Ensure verbose output for the benchmark suite, as otherwise no benchmark
      # results are emitted.
      if m.__name__ == 'test_benchmark':
        options.verbose = max(options.verbose, 1)

      loaded_tests = loader.loadTestsFromNames(sorted(names_in_module), m)
      tests = flattened_tests(loaded_tests)
      suite = suite_for_module(m, tests, options)
      if options.failing_and_slow_first:
        tests = sorted(tests, key=cmp_to_key(create_test_run_sorter(options.max_failures < len(tests) / 2)))
      for test in tests:
        if not found_start:
          # Skip over tests until we find the start
          if test.id().endswith(options.start_at):
            found_start = True
          else:
            continue
        for _x in range(options.repeat):
          total_tests += 1
          suite.addTest(test)
      suites.append((m.__name__, suite))
  if not found_start:
    utils.exit_with_error(f'unable to find --start-at test: {options.start_at}')
  return suites, unmatched_test_names


def flattened_tests(loaded_tests):
  tests = []
  for subsuite in loaded_tests:
    tests.extend(subsuite)
  return tests


def suite_for_module(module, tests, options):
  suite_supported = module.__name__ not in ('test_sanity', 'test_benchmark', 'test_sockets', 'test_interactive', 'test_stress')
  if not common.EMTEST_SAVE_DIR and not shared.DEBUG:
    has_multiple_tests = len(tests) > 1
    has_multiple_cores = parallel_testsuite.num_cores() > 1
    if suite_supported and has_multiple_tests and has_multiple_cores:
      return parallel_testsuite.ParallelTestSuite(options)
  return unittest.TestSuite()


def run_tests(options, suites):
  resultMessages = []
  num_failures = 0

  if len(suites) > 1:
    print('Test suites:', [s[0] for s in suites])
  # Run the discovered tests

  if os.getenv('CI'):
    # output fd must remain open until after testRunner.run() below
    output = open('out/test-results.xml', 'wb')
    import xmlrunner  # type: ignore  # noqa: PLC0415
    testRunner = xmlrunner.XMLTestRunner(output=output, verbosity=2,
                                         failfast=options.failfast)
    print('Writing XML test output to ' + os.path.abspath(output.name))
  elif options.ansi and not options.verbose:
    # When not in verbose mode and ansi color output is available use our nice single-line
    # result display.
    testRunner = SingleLineTestRunner(failfast=options.failfast)
  else:
    if not options.ansi:
      print('using verbose test runner (ANSI not available)')
    else:
      print('using verbose test runner (verbose output requested)')
    testRunner = ColorTextRunner(failfast=options.failfast)

  total_core_time = 0
  run_start_time = time.perf_counter()
  for mod_name, suite in suites:
    errlog('Running %s: (%s tests)' % (mod_name, suite.countTestCases()))
    res = testRunner.run(suite)
    msg = ('%s: %s run, %s errors, %s failures, %s skipped' %
           (mod_name, res.testsRun, len(res.errors), len(res.failures), len(res.skipped)))
    num_failures += len(res.errors) + len(res.failures) + len(res.unexpectedSuccesses)
    resultMessages.append(msg)
    if hasattr(res, 'core_time'):
      total_core_time += res.core_time
  total_run_time = time.perf_counter() - run_start_time
  if total_core_time > 0:
    errlog('Total core time: %.3fs. Wallclock time: %.3fs. Parallelization: %.2fx.' % (total_core_time, total_run_time, total_core_time / total_run_time))

  if len(resultMessages) > 1:
    errlog('====================')
    errlog()
    errlog('TEST SUMMARY')
    for msg in resultMessages:
      errlog('    ' + msg)

  if options.bell:
    sys.stdout.write('\a')
    sys.stdout.flush()

  return num_failures


def parse_args():
  parser = argparse.ArgumentParser(prog='runner.py', description=__doc__)
  parser.add_argument('--save-dir', action='store_true',
                      help='Save the temporary directory used during for each '
                           'test.  Implies --cores=1.  Defaults to true when running a single test')
  parser.add_argument('--no-clean', action='store_true',
                      help='Do not clean the temporary directory before each test run')
  parser.add_argument('--verbose', '-v', action='count', default=0,
                      help="Show test stdout and stderr, and don't use the single-line test reporting. "
                           'Specifying `-v` twice will enable test framework logging (i.e. EMTEST_VERBOSE)')
  parser.add_argument('--ansi', action=argparse.BooleanOptionalAction, default=None)
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
  parser.add_argument('--headless', action='store_true',
                      help='Run browser tests in headless mode.', default=None)
  parser.add_argument('--browser-auto-config', action=argparse.BooleanOptionalAction, default=None,
                      help='Use the default CI browser configuration.')
  parser.add_argument('tests', nargs='*')
  parser.add_argument('--failfast', action='store_true', help='If true, test run will abort on first failed test.')
  parser.add_argument('--max-failures', type=int, default=2**31 - 1, help='If specified, test run will abort after N failed tests.')
  parser.add_argument('--failing-and-slow-first', action='store_true', help='Run failing tests first, then sorted by slowest first. Combine with --failfast for fast fail-early CI runs.')
  parser.add_argument('--start-at', metavar='NAME', help='Skip all tests up until <NAME>')
  parser.add_argument('--continue', dest='_continue', action='store_true',
                      help='Resume from the last run test.'
                           'Useful when combined with --failfast')
  parser.add_argument('--crossplatform-only', action='store_true')
  parser.add_argument('--log-test-environment', action='store_true', help='Prints out detailed information about the current environment. Useful for adding more info to CI test runs.')
  parser.add_argument('--force-browser-process-termination', action='store_true', help='If true, a fail-safe method is used to ensure that all browser processes are terminated before and after the test suite run. Note that this option will terminate all browser processes, not just those launched by the harness, so will result in loss of all open browsing sessions.')
  parser.add_argument('--repeat', type=int, default=1,
                      help='Repeat each test N times (default: 1).')
  parser.add_argument('--bell', action='store_true', help='Play a sound after the test suite finishes.')

  options = parser.parse_args()

  if options.ansi is None:
    options.ansi = colored_logger.ansi_color_available()
  else:
    if options.ansi:
      colored_logger.enable(force=True)
    else:
      colored_logger.disable()

  if options.failfast:
    if options.max_failures != 2**31 - 1:
      utils.exit_with_error('--failfast and --max-failures are mutually exclusive!')
    options.max_failures = 0

  return options


def configure():
  browser_common.EMTEST_BROWSER = os.getenv('EMTEST_BROWSER')
  browser_common.EMTEST_BROWSER_AUTO_CONFIG = int(os.getenv('EMTEST_BROWSER_AUTO_CONFIG', '1'))
  browser_common.EMTEST_HEADLESS = int(os.getenv('EMTEST_HEADLESS', '0'))
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

  browser_common.configure_test_browser()


def cleanup_emscripten_temp():
  """Deletes all files and directories under Emscripten
  that look like they might have been created by Emscripten."""
  for entry in os.listdir(shared.TEMP_DIR):
    if entry.startswith(('emtest_', 'emscripten_')):
      entry = os.path.join(shared.TEMP_DIR, entry)
      try:
        if os.path.isdir(entry):
          utils.delete_dir(entry)
      except Exception:
        pass


def print_repository_info(directory, repository_name):
  current_commit = utils.run_process(['git', 'log', '--abbrev-commit', '-n1', '--pretty=oneline'], cwd=directory, stdout=subprocess.PIPE).stdout.strip()
  print(f'\n{repository_name} {current_commit}\n')
  local_changes = utils.run_process(['git', 'diff'], cwd=directory, stdout=subprocess.PIPE).stdout.strip()
  if local_changes:
    print(f'\n{local_changes}\n')


def log_test_environment():
  """Print detailed information about the current test environment. Useful for
  logging test run configuration in a CI."""
  print('======================== Test Setup ========================')
  print(f'Test time: {datetime.datetime.now(datetime.timezone.utc).strftime("%A, %B %d, %Y %H:%M:%S %Z")}')
  print(f'Python: "{sys.executable}". Version: {sys.version}')
  print(f'Emscripten test runner path: "{os.path.realpath(__file__)}"')

  if os.path.isdir(utils.path_from_root('.git')):
    print(f'\nEmscripten repository: "{__rootpath__}"')

  emscripten_version = utils.path_from_root('emscripten-version.txt')
  if os.path.isfile(emscripten_version):
    print(f'emscripten-version.txt: {utils.EMSCRIPTEN_VERSION}')

  if os.path.isdir(os.path.join(__rootpath__, '.git')):
    print_repository_info(__rootpath__, 'Emscripten')

  print(f'EM_CONFIG: "{config.EM_CONFIG}"')
  if os.path.isfile(config.EM_CONFIG):
    print(f'\n{utils.read_file(config.EM_CONFIG).strip()}\n')

  node_js_version = utils.run_process(config.NODE_JS + ['--version'], stdout=subprocess.PIPE).stdout.strip()
  print(f'NODE_JS: {config.NODE_JS}. Version: {node_js_version}')

  print(f'BINARYEN_ROOT: {config.BINARYEN_ROOT}')
  wasm_opt_version = building.get_binaryen_version(building.get_binaryen_bin()).strip()
  print(f'wasm-opt version: {wasm_opt_version}')

  binaryen_git_dir = config.BINARYEN_ROOT
  # Detect emsdk directory structure (build root vs source root)
  if re.match(r'main_.*_64bit_binaryen', os.path.basename(binaryen_git_dir)):
    binaryen_git_dir = os.path.realpath(os.path.join(binaryen_git_dir, '..', 'main'))
  if os.path.isdir(os.path.join(binaryen_git_dir, '.git')):
    print(f'Binaryen git directory: "{binaryen_git_dir}"')
    print_repository_info(binaryen_git_dir, 'Binaryen')

  print(f'LLVM_ROOT: {config.LLVM_ROOT}')

  # Find LLVM git directory in emsdk aware fashion
  def find_llvm_git_root(dir):
    while True:
      if os.path.isdir(os.path.join(dir, ".git")):
        return dir
      if os.path.isdir(os.path.join(dir, "src", ".git")):
        return os.path.join(dir, "src")
      if os.path.dirname(dir) == dir:
        return None
      dir = os.path.dirname(dir)

  llvm_git_root = find_llvm_git_root(config.LLVM_ROOT)
  if llvm_git_root:
    print(f'LLVM git directory: "{llvm_git_root}"')
    print_repository_info(llvm_git_root, 'LLVM')

  clang_version = utils.run_process([shared.CLANG_CC, '--version'], stdout=subprocess.PIPE).stdout.strip()
  print(f'Clang: "{shared.CLANG_CC}"\n{clang_version}\n')

  print(f'EMTEST_BROWSER: {browser_common.EMTEST_BROWSER}')
  if browser_common.is_firefox():
    print(f'Firefox version: {browser_common.get_firefox_version()}')
  else:
    print('Not detected as a Firefox browser')
  if browser_common.is_safari():
    print(f'Safari version: {browser_common.get_safari_version()}')
  else:
    print('Not detected as a Safari browser')
  if browser_common.is_chrome():
    print('Browser is Chrome.')
  else:
    print('Not detected as a Chrome browser')

  emsdk_dir = os.getenv('EMSDK')
  print(f'\nEMSDK: "{emsdk_dir}"')
  if emsdk_dir:
    if os.path.isdir(os.path.join(emsdk_dir, '.git')):
      print_repository_info(emsdk_dir, 'Emsdk')

  print('==================== End of Test Setup =====================')


def main():
  options = parse_args()

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
  set_env('EMTEST_BROWSER_AUTO_CONFIG', options.browser_auto_config)
  set_env('EMTEST_HEADLESS', options.headless)
  set_env('EMTEST_DETECT_TEMPFILE_LEAKS', options.detect_leaks)
  if options.save_dir:
    common.EMTEST_SAVE_DIR = 1
  if options.no_clean:
    common.EMTEST_SAVE_DIR = 2
  set_env('EMTEST_SKIP_SLOW', options.skip_slow)
  set_env('EMTEST_ALL_ENGINES', options.all_engines)
  set_env('EMTEST_REBASELINE', options.rebaseline)
  set_env('EMTEST_VERBOSE', options.verbose > 1)
  set_env('EMTEST_CORES', options.cores)

  if common.EMTEST_DETECT_TEMPFILE_LEAKS:
    if shared.DEBUG:
      # In EMCC_DEBUG mode emscripten explicitly leaves stuff in the tmp directory
      utils.exit_with_error('EMTEST_DETECT_TEMPFILE_LEAKS is not compatible with EMCC_DEBUG')
    if common.EMTEST_SAVE_DIR:
      # In --save-dir/--no-clean mode the parallel test runner leaves files in the temp directory
      utils.exit_with_error('EMTEST_DETECT_TEMPFILE_LEAKS is not compatible with --save-dir/--no-clean')

  configure()

  check_js_engines()

  # Remove any old test files before starting the run. Skip cleanup when we're running in debug mode
  # where we want to preserve any files created (e.g. emscripten.lock from shared.py).
  if not (shared.DEBUG or common.EMTEST_SAVE_DIR):
    cleanup_emscripten_temp()
  utils.delete_file(common.flaky_tests_log_filename)

  browser_common.init(options.force_browser_process_termination)

  if options.log_test_environment or os.getenv('CI'):
    log_test_environment()

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

  if not tests:
    errlog('ERROR: no tests to run')
    return 1

  if not options.start_at and options._continue:
    if os.path.exists(common.LAST_TEST):
      options.start_at = utils.read_file(common.LAST_TEST).strip()

  suites, unmatched_tests = load_test_suites(tests, modules, options)
  if unmatched_tests:
    errlog('ERROR: could not find the following tests: ' + ' '.join(unmatched_tests))
    return 1

  num_failures = run_tests(options, suites)
  # Return the number of failures as the process exit code
  # for automating success/failure reporting.  Return codes
  # over 125 are not well supported on UNIX.
  return min(num_failures, 125)


configure()

if __name__ == '__main__':
  try:
    sys.exit(main())
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
