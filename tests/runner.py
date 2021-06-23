#!/usr/bin/env python3
# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is the Emscripten test runner. To run some tests, specify which tests
you want, for example

  tests/runner asm1.test_hello_world

There are many options for which tests to run and how to run them. For details,
see

http://kripken.github.io/emscripten-site/docs/getting_started/test-suite.html
"""

# XXX Use EMTEST_ALL_ENGINES=1 in the env to test all engines!

import argparse
import atexit
import fnmatch
import glob
import logging
import math
import operator
import os
import random
import sys
import unittest

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

import jsrun
import parallel_testsuite
import common
from tools import shared, config


sys.path.append(shared.path_from_root('third_party/websockify'))

logger = logging.getLogger("runner")


if common.EMTEST_VERBOSE:
  logging.root.setLevel(logging.DEBUG)


# The core test modes
core_test_modes = [
  'wasm0',
  'wasm1',
  'wasm2',
  'wasm3',
  'wasms',
  'wasmz',
  'strict',
  'wasm2js0',
  'wasm2js1',
  'wasm2js2',
  'wasm2js3',
  'wasm2jss',
  'wasm2jsz',
]

# The default core test mode, used when none is specified
default_core_test_mode = 'wasm0'

# The non-core test modes
non_core_test_modes = [
  'other',
  'browser',
  'sanity',
  'sockets',
  'interactive',
  'benchmark',
  'asan',
  'lsan',
  'wasm2ss',
  'posixtest',
  'posixtest_browser',
]


def check_js_engines():
  working_engines = [e for e in config.JS_ENGINES if jsrun.check_engine(e)]
  if len(working_engines) < len(config.JS_ENGINES):
    print('Not all the JS engines in JS_ENGINES appears to work.')
    exit(1)

  if common.EMTEST_ALL_ENGINES:
    print('(using ALL js engines)')
  else:
    logger.warning('use EMTEST_ALL_ENGINES=1 in the env to run against all JS '
                   'engines, which is slower but provides more coverage')


def get_and_import_modules():
  modules = []
  for filename in glob.glob(os.path.join(os.path.dirname(__file__), 'test*.py')):
    module_dir, module_file = os.path.split(filename)
    module_name, module_ext = os.path.splitext(module_file)
    __import__(module_name)
    modules.append(sys.modules[module_name])
  return modules


def get_all_tests(modules):
  # Create a list of all known tests so that we can choose from them based on a wildcard search
  all_tests = []
  suites = core_test_modes + non_core_test_modes
  for m in modules:
    for s in suites:
      if hasattr(m, s):
        tests = [t for t in dir(getattr(m, s)) if t.startswith('test_')]
        all_tests += [s + '.' + t for t in tests]
  return all_tests


def tests_with_expanded_wildcards(args, all_tests):
  # Process wildcards, e.g. "browser.test_pthread_*" should expand to list all pthread tests
  new_args = []
  for i, arg in enumerate(args):
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


def skip_requested_tests(args, modules):
  for i, arg in enumerate(args):
    if arg.startswith('skip:'):
      which = [arg.split('skip:')[1]]

      print(','.join(which), file=sys.stderr)
      skipped = False
      for test in which:
        print('will skip "%s"' % test, file=sys.stderr)
        suite_name, test_name = test.split('.')
        for m in modules:
          suite = getattr(m, suite_name, None)
          if suite:
            setattr(suite, test_name, lambda s: s.skipTest("requested to be skipped"))
            skipped = True
            break
      assert skipped, "Not able to skip test " + test
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
  relevant_modes = core_test_modes
  if len(arg):
    num_str = arg
    if arg.startswith('other'):
      base_module = 'other'
      relevant_modes = ['other']
      num_str = arg.replace('other', '')
    elif arg.startswith('browser'):
      base_module = 'browser'
      relevant_modes = ['browser']
      num_str = arg.replace('browser', '')
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


def load_test_suites(args, modules):
  loader = unittest.TestLoader()
  unmatched_test_names = set(args)
  suites = []
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
        suite.addTest(test)
      suites.append((m.__name__, suite))
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
  testRunner = unittest.TextTestRunner(verbosity=2)
  for mod_name, suite in suites:
    print('Running %s: (%s tests)' % (mod_name, suite.countTestCases()))
    res = testRunner.run(suite)
    msg = ('%s: %s run, %s errors, %s failures, %s skipped' %
           (mod_name, res.testsRun, len(res.errors), len(res.failures), len(res.skipped)))
    num_failures += len(res.errors) + len(res.failures)
    resultMessages.append(msg)

  if len(resultMessages) > 1:
    print('====================')
    print()
    print('TEST SUMMARY')
    for msg in resultMessages:
      print('    ' + msg)

  # Return the number of failures as the process exit code for automating success/failure reporting.
  return min(num_failures, 255)


def parse_args(args):
  parser = argparse.ArgumentParser(prog='runner.py', description=__doc__)
  parser.add_argument('tests', nargs='*')
  return parser.parse_args()


def main(args):
  options = parse_args(args)
  check_js_engines()

  def prepend_default(arg):
    if arg.startswith('test_'):
      return default_core_test_mode + '.' + arg
    return arg

  tests = [prepend_default(t) for t in options.tests]

  modules = get_and_import_modules()
  all_tests = get_all_tests(modules)
  tests = tests_with_expanded_wildcards(tests, all_tests)
  tests = skip_requested_tests(tests, modules)
  tests = args_for_random_tests(tests, modules)
  suites, unmatched_tests = load_test_suites(tests, modules)
  if unmatched_tests:
    print('ERROR: could not find the following tests: ' + ' '.join(unmatched_tests))
    return 1

  return run_tests(options, suites)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.warning('KeyboardInterrupt')
    sys.exit(1)
