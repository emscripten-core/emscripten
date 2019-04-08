#!/usr/bin/env python2
# Copyright 2010 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""This is the Emscripten test runner. To run some tests, specify which tests
you want, for example

  python tests/runner.py asm1.test_hello_world

There are many options for which tests to run and how to run them. For details,
see

http://kripken.github.io/emscripten-site/docs/getting_started/test-suite.html
"""

# XXX Use EMTEST_ALL_ENGINES=1 in the env to test all engines!

from __future__ import print_function
from subprocess import PIPE, STDOUT
import argparse
import atexit
import contextlib
import difflib
import fnmatch
import glob
import hashlib
import json
import logging
import math
import multiprocessing
import operator
import os
import random
import re
import shlex
import shutil
import string
import subprocess
import sys
import tempfile
import time
import unittest
import urllib
import webbrowser

if sys.version_info.major == 2:
  from BaseHTTPServer import HTTPServer
  from SimpleHTTPServer import SimpleHTTPRequestHandler
  from urllib import unquote
else:
  from http.server import HTTPServer, SimpleHTTPRequestHandler
  from urllib.parse import unquote

# Setup

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

import parallel_runner
from tools.shared import EM_CONFIG, TEMP_DIR, EMCC, DEBUG, PYTHON, LLVM_TARGET, ASM_JS_TARGET, EMSCRIPTEN_TEMP_DIR, WASM_TARGET, SPIDERMONKEY_ENGINE, WINDOWS, V8_ENGINE, NODE_JS, EM_BUILD_VERBOSE
from tools.shared import asstr, get_canonical_temp_dir, Building, run_process, try_delete, to_cc, asbytes, safe_copy, Settings
from tools import jsrun, shared, line_endings


def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)


sys.path.append(path_from_root('third_party/websockify'))

logger = logging.getLogger(__file__)

# User can specify an environment variable EMTEST_BROWSER to force the browser
# test suite to run using another browser command line than the default system
# browser.  Setting '0' as the browser disables running a browser (but we still
# see tests compile)
EMTEST_BROWSER = os.getenv('EMTEST_BROWSER')

EMTEST_DETECT_TEMPFILE_LEAKS = int(os.getenv('EMTEST_DETECT_TEMPFILE_LEAKS', '0'))

EMTEST_WASM_PTHREADS = int(os.getenv('EMTEST_WASM_PTHREADS', '1'))

# Also suppot the old name: EM_SAVE_DIR
EMTEST_SAVE_DIR = os.getenv('EMTEST_SAVE_DIR', os.getenv('EM_SAVE_DIR'))

# generally js engines are equivalent, testing 1 is enough. set this
# to force testing on all js engines, good to find js engine bugs
EMTEST_ALL_ENGINES = os.getenv('EMTEST_ALL_ENGINES')

EMTEST_SKIP_SLOW = os.getenv('EMTEST_SKIP_SLOW')

EMTEST_VERBOSE = int(os.getenv('EMTEST_VERBOSE', '0'))


# checks if browser testing is enabled
def has_browser():
  return EMTEST_BROWSER != '0'


# Generic decorator that calls a function named 'condition' on the test class and
# skips the test if that function returns true
def skip_if(func, condition, explanation='', negate=False):
  explanation_str = ' : %s' % explanation if explanation else ''

  def decorated(self):
    choice = self.__getattribute__(condition)()
    if negate:
      choice = not choice
    if choice:
      self.skipTest(condition + explanation_str)
    func(self)

  return decorated


def needs_dlfcn(func):
  def decorated(self):
    self.check_dlfcn()
    return func(self)

  return decorated


def is_slow_test(func):
  def decorated(self, *args, **kwargs):
    if EMTEST_SKIP_SLOW:
      return self.skipTest('skipping slow tests')
    return func(self, *args, **kwargs)

  return decorated


def no_wasm_backend(note=''):
  def decorated(f):
    return skip_if(f, 'is_wasm_backend', note)
  return decorated


def no_fastcomp(note=''):
  def decorated(f):
    return skip_if(f, 'is_wasm_backend', note, negate=True)
  return decorated


def no_windows(note=''):
  if WINDOWS:
    return unittest.skip(note)
  return lambda f: f


# used for tests that fail now and then on CI, due to timing or other
# random causes. this tries the test a few times, looking for at least
# one pass
def flaky(f):
  max_tries = 3

  def decorated(self):
    for i in range(max_tries - 1):
      try:
        f(self)
        return
      except Exception:
        print('flaky...')
        continue
    # run the last time normally, to get a simpler stack trace
    f(self)
  return decorated


@contextlib.contextmanager
def env_modify(updates):
  """A context manager that updates os.environ."""
  # This could also be done with mock.patch.dict() but taking a dependency
  # on the mock library is probably not worth the benefit.
  old_env = os.environ.copy()
  print("env_modify: " + str(updates))
  # Seting a value to None means clear the environment variable
  clears = [key for key, value in updates.items() if value is None]
  updates = {key: value for key, value in updates.items() if value is not None}
  os.environ.update(updates)
  for key in clears:
    if key in os.environ:
      del os.environ[key]
  try:
    yield
  finally:
    os.environ.clear()
    os.environ.update(old_env)


# Decorator version of env_modify
def with_env_modify(updates):
  def decorated(f):
    def modified(self):
      with env_modify(updates):
        return f(self)
    return modified
  return decorated


@contextlib.contextmanager
def chdir(dir):
  """A context manager that performs actions in the given directory."""
  orig_cwd = os.getcwd()
  os.chdir(dir)
  try:
    yield
  finally:
    os.chdir(orig_cwd)


def limit_size(string, MAX=800 * 20):
  if len(string) < MAX:
    return string
  return string[0:MAX // 2] + '\n[..]\n' + string[-MAX // 2:]


def create_test_file(name, contents, binary=False):
  assert not os.path.isabs(name)
  mode = 'wb' if binary else 'w'
  with open(name, mode) as f:
    f.write(contents)


# The core test modes
core_test_modes = [
  'asm0',
  'asm1',
  'asm2',
  'asm3',
  'asm2g',
  'asm2f',
  'wasm0',
  'wasm1',
  'wasm2',
  'wasm3',
  'wasms',
  'wasmz',
  'asmi',
  'asm2i',
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
]

test_index = 0


class RunnerCore(unittest.TestCase):
  emcc_args = []

  # default temporary directory settings. set_temp_dir may be called later to
  # override these
  temp_dir = TEMP_DIR
  canonical_temp_dir = get_canonical_temp_dir(TEMP_DIR)

  save_dir = EMTEST_SAVE_DIR
  save_JS = 0
  # This avoids cluttering the test runner output, which is stderr too, with compiler warnings etc.
  # Change this to None to get stderr reporting, for debugging purposes
  stderr_redirect = STDOUT

  env = {}
  settings_mods = {}

  temp_files_before_run = []

  def is_emterpreter(self):
    return self.get_setting('EMTERPRETIFY')

  def is_wasm(self):
    return self.is_wasm_backend() or self.get_setting('WASM') != 0

  def is_wasm_backend(self):
    return self.get_setting('WASM_BACKEND')

  def check_dlfcn(self):
    if self.get_setting('ALLOW_MEMORY_GROWTH') == 1 and not self.is_wasm():
      self.skipTest('no dlfcn with memory growth (without wasm)')
    if self.is_wasm_backend():
      self.skipTest('no shared modules in wasm backend')

  def uses_memory_init_file(self):
    if self.get_setting('SIDE_MODULE') or self.get_setting('WASM'):
      return False
    elif '--memory-init-file' in self.emcc_args:
      return int(self.emcc_args[self.emcc_args.index('--memory-init-file') + 1])
    else:
      # side modules handle memory differently; binaryen puts the memory in the wasm module
      opt_supports = any(opt in self.emcc_args for opt in ('-O2', '-O3', '-Oz'))
      return opt_supports

  def set_temp_dir(self, temp_dir):
    self.temp_dir = temp_dir
    self.canonical_temp_dir = get_canonical_temp_dir(self.temp_dir)
    # Explicitly set dedicated temporary directory for parallel tests
    os.environ['EMCC_TEMP_DIR'] = self.temp_dir

  @classmethod
  def setUpClass(cls):
    super(RunnerCore, cls).setUpClass()
    print('(checking sanity from test runner)') # do this after we set env stuff
    shared.check_sanity(force=True)

  def setUp(self):
    super(RunnerCore, self).setUp()
    self.settings_mods = {}

    if EMTEST_DETECT_TEMPFILE_LEAKS:
      for root, dirnames, filenames in os.walk(self.temp_dir):
        for dirname in dirnames:
          self.temp_files_before_run.append(os.path.normpath(os.path.join(root, dirname)))
        for filename in filenames:
          self.temp_files_before_run.append(os.path.normpath(os.path.join(root, filename)))

    self.banned_js_engines = []
    self.use_all_engines = EMTEST_ALL_ENGINES
    if self.save_dir:
      self.working_dir = os.path.join(self.temp_dir, 'emscripten_test')
      if not os.path.exists(self.working_dir):
        os.makedirs(self.working_dir)
    else:
      self.working_dir = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=self.temp_dir)
    os.chdir(self.working_dir)

    # Use emscripten root for node module lookup
    os.environ['NODE_PATH'] = path_from_root('node_modules')

    if not self.save_dir:
      self.has_prev_ll = False
      for temp_file in os.listdir(TEMP_DIR):
        if temp_file.endswith('.ll'):
          self.has_prev_ll = True

  def tearDown(self):
    if not self.save_dir:
      # rmtree() fails on Windows if the current working directory is inside the tree.
      os.chdir(os.path.dirname(self.get_dir()))
      try_delete(self.get_dir())

      if EMTEST_DETECT_TEMPFILE_LEAKS and not os.environ.get('EMCC_DEBUG'):
        temp_files_after_run = []
        for root, dirnames, filenames in os.walk(self.temp_dir):
          for dirname in dirnames:
            temp_files_after_run.append(os.path.normpath(os.path.join(root, dirname)))
          for filename in filenames:
            temp_files_after_run.append(os.path.normpath(os.path.join(root, filename)))

        # Our leak detection will pick up *any* new temp files in the temp dir. They may not be due to
        # us, but e.g. the browser when running browser tests. Until we figure out a proper solution,
        # ignore some temp file names that we see on our CI infrastructure.
        ignorable_files = [
          '/tmp/tmpaddon',
          '/tmp/circleci-no-output-timeout'
        ]

        left_over_files = list(set(temp_files_after_run) - set(self.temp_files_before_run) - set(ignorable_files))
        if len(left_over_files):
          print('ERROR: After running test, there are ' + str(len(left_over_files)) + ' new temporary files/directories left behind:', file=sys.stderr)
          for f in left_over_files:
            print('leaked file: ' + f, file=sys.stderr)
          self.fail('Test leaked ' + str(len(left_over_files)) + ' temporary files!')

      # Make sure we don't leave stuff around
      # if not self.has_prev_ll:
      #   for temp_file in os.listdir(TEMP_DIR):
      #     assert not temp_file.endswith('.ll'), temp_file
      #     # TODO assert not temp_file.startswith('emscripten_'), temp_file

  def get_setting(self, key):
    if key in self.settings_mods:
      return self.settings_mods[key]
    return Settings[key]

  def set_setting(self, key, value=1):
    if value is None:
      self.clear_setting(key)
    self.settings_mods[key] = value

  def clear_setting(self, key):
    self.settings_mods.pop(key, None)

  def serialize_settings(self):
    ret = []
    for key, value in self.settings_mods.items():
      if value == 1:
        ret += ['-s', key]
      else:
        ret += ['-s', '{}={}'.format(key, json.dumps(value))]
    return ret

  def get_dir(self):
    return self.working_dir

  def in_dir(self, *pathelems):
    return os.path.join(self.get_dir(), *pathelems)

  def get_stdout_path(self):
    return os.path.join(self.get_dir(), 'stdout')

  def hardcode_arguments(self, filename, args):
    # Hardcode in the arguments, so js is portable without manual commandlinearguments
    if not args:
      return
    js = open(filename).read()
    create_test_file(filename, js.replace('run();', 'run(%s + Module["arguments"]);' % str(args)))

  def prep_ll_run(self, filename, ll_file, force_recompile=False, build_ll_hook=None):
    # force_recompile = force_recompile or os.path.getsize(filename + '.o.ll') > 50000
    # If the file is big, recompile just to get ll_opts
    # Recompiling just for dfe in ll_opts is too costly

    def fix_target(ll_filename):
      if LLVM_TARGET == ASM_JS_TARGET:
         return
      with open(ll_filename) as f:
        contents = f.read()
      if LLVM_TARGET in contents:
        return
      asmjs_layout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
      wasm_layout = "e-m:e-p:32:32-i64:64-n32:64-S128"
      assert(ASM_JS_TARGET in contents)
      assert(asmjs_layout in contents)
      contents = contents.replace(asmjs_layout, wasm_layout)
      contents = contents.replace(ASM_JS_TARGET, WASM_TARGET)
      with open(ll_filename, 'w') as f:
        f.write(contents)

    if force_recompile or build_ll_hook:
      if ll_file.endswith(('.bc', '.o')):
        if ll_file != filename + '.o':
          shutil.copy(ll_file, filename + '.o')
        Building.llvm_dis(filename)
      else:
        shutil.copy(ll_file, filename + '.o.ll')
        fix_target(filename + '.o.ll')

      if build_ll_hook:
        need_post = build_ll_hook(filename)
      Building.llvm_as(filename)
      shutil.move(filename + '.o.ll', filename + '.o.ll.pre') # for comparisons later
      Building.llvm_dis(filename)
      if build_ll_hook and need_post:
        build_ll_hook(filename)
        Building.llvm_as(filename)
        shutil.move(filename + '.o.ll', filename + '.o.ll.post') # for comparisons later
        Building.llvm_dis(filename)

      Building.llvm_as(filename)
    else:
      if ll_file.endswith('.ll'):
        safe_copy(ll_file, filename + '.o.ll')
        fix_target(filename + '.o.ll')
        Building.llvm_as(filename)
      else:
        safe_copy(ll_file, filename + '.o')

  def get_emcc_args(self):
    # TODO(sbc): We should probably unify Building.COMPILER_TEST_OPTS and self.emcc_args
    return self.serialize_settings() + self.emcc_args + Building.COMPILER_TEST_OPTS

  # Generate JS from ll
  def ll_to_js(self, filename):
    Building.emcc(filename + '.o', self.get_emcc_args(), filename + '.o.js')

  # Build JavaScript code from source code
  def build(self, src, dirname, filename, main_file=None,
            additional_files=[], libraries=[], includes=[], build_ll_hook=None,
            post_build=None, js_outfile=True):

    # Copy over necessary files for compiling the source
    if main_file is None:
      with open(filename, 'w') as f:
        f.write(src)
      final_additional_files = []
      for f in additional_files:
        final_additional_files.append(os.path.join(dirname, os.path.basename(f)))
        shutil.copyfile(f, final_additional_files[-1])
      additional_files = final_additional_files
    else:
      # copy whole directory, and use a specific main .cpp file
      # (rmtree() fails on Windows if the current working directory is inside the tree.)
      if os.getcwd().startswith(os.path.abspath(dirname)):
          os.chdir(os.path.join(dirname, '..'))
      shutil.rmtree(dirname)
      shutil.copytree(src, dirname)
      shutil.move(os.path.join(dirname, main_file), filename)
      # the additional files were copied; alter additional_files to point to their full paths now
      additional_files = [os.path.join(dirname, f) for f in additional_files]
      os.chdir(self.get_dir())

    suffix = '.o.js' if js_outfile else '.o.wasm'
    if build_ll_hook:
      # "slow", old path: build to bc, then build to JS

      # C++ => LLVM binary

      for f in [filename] + additional_files:
        try:
          # Make sure we notice if compilation steps failed
          os.remove(f + '.o')
        except:
          pass
        args = [PYTHON, EMCC] + self.get_emcc_args() + \
               ['-I', dirname, '-I', os.path.join(dirname, 'include')] + \
               ['-I' + include for include in includes] + \
               ['-c', f, '-o', f + '.o']
        run_process(args, stderr=self.stderr_redirect if not DEBUG else None)
        self.assertExists(f + '.o')

      # Link all files
      object_file = filename + '.o'
      if len(additional_files) + len(libraries):
        shutil.move(object_file, object_file + '.alone')
        inputs = [object_file + '.alone'] + [f + '.o' for f in additional_files] + libraries
        Building.link_to_object(inputs, object_file)
        if not os.path.exists(object_file):
          print("Failed to link LLVM binaries:\n\n", object_file)
          self.fail("Linkage error")

      # Finalize
      self.prep_ll_run(filename, object_file, build_ll_hook=build_ll_hook)

      # BC => JS
      self.ll_to_js(filename)
    else:
      # "fast", new path: just call emcc and go straight to JS
      all_files = [filename] + additional_files + libraries
      for i in range(len(all_files)):
        if '.' not in all_files[i]:
          shutil.move(all_files[i], all_files[i] + '.bc')
          all_files[i] += '.bc'
      args = [PYTHON, EMCC] + self.get_emcc_args() + \
          ['-I', dirname, '-I', os.path.join(dirname, 'include')] + \
          ['-I' + include for include in includes] + \
          all_files + ['-o', filename + suffix]

      run_process(args, stderr=self.stderr_redirect if not DEBUG else None)
      self.assertExists(filename + suffix)

    if post_build:
      post_build(filename + suffix)

    if js_outfile and self.uses_memory_init_file():
      src = open(filename + suffix).read()
      # side memory init file, or an empty one in the js
      assert ('/* memory initializer */' not in src) or ('/* memory initializer */ allocate([]' in src)

  def validate_asmjs(self, err):
    m = re.search(r"asm.js type error: '(\w+)' is not a (standard|supported) SIMD type", err)
    if m:
      # Bug numbers for missing SIMD types:
      bugs = {
        'Int8x16': 1136226,
        'Int16x8': 1136226,
        'Uint8x16': 1244117,
        'Uint16x8': 1244117,
        'Uint32x4': 1240796,
        'Float64x2': 1124205,
      }
      simd = m.group(1)
      if simd in bugs:
        print(("\nWARNING: ignoring asm.js type error from {} due to implementation not yet available in SpiderMonkey." +
               " See https://bugzilla.mozilla.org/show_bug.cgi?id={}\n").format(simd, bugs[simd]), file=sys.stderr)
        err = err.replace(m.group(0), '')

    # check for asm.js validation
    if 'uccessfully compiled asm.js code' in err and 'asm.js link error' not in err:
      print("[was asm.js'ified]", file=sys.stderr)
    # check for an asm.js validation error, if we expect one
    elif 'asm.js' in err and not self.is_wasm() and self.get_setting('ASM_JS') == 1:
      self.fail("did NOT asm.js'ify: " + err)
    err = '\n'.join([line for line in err.split('\n') if 'uccessfully compiled asm.js code' not in line])
    return err

  def get_func(self, src, name):
    start = src.index('function ' + name + '(')
    t = start
    n = 0
    while True:
      if src[t] == '{':
        n += 1
      elif src[t] == '}':
        n -= 1
        if n == 0:
          return src[start:t + 1]
      t += 1
      assert t < len(src)

  def count_funcs(self, javascript_file):
    num_funcs = 0
    start_tok = "// EMSCRIPTEN_START_FUNCS"
    end_tok = "// EMSCRIPTEN_END_FUNCS"
    start_off = 0
    end_off = 0

    with open(javascript_file, 'rt') as f:
      blob = "".join(f.readlines())

    start_off = blob.find(start_tok) + len(start_tok)
    end_off = blob.find(end_tok)
    asm_chunk = blob[start_off:end_off]
    num_funcs = asm_chunk.count('function ')
    return num_funcs

  def count_wasm_contents(self, wasm_binary, what):
    out = run_process([os.path.join(Building.get_binaryen_bin(), 'wasm-opt'), wasm_binary, '--metrics'], stdout=PIPE).stdout
    # output is something like
    # [?]        : 125
    for line in out.splitlines():
      if '[' + what + ']' in line:
        ret = line.split(':')[1].strip()
        return int(ret)
    self.fail('Failed to find [%s] in wasm-opt output' % what)

  def get_wasm_text(self, wasm_binary):
    return run_process([os.path.join(Building.get_binaryen_bin(), 'wasm-dis'), wasm_binary], stdout=PIPE).stdout

  def is_exported_in_wasm(self, name, wasm):
    wat = self.get_wasm_text(wasm)
    return ('(export "%s"' % name) in wat

  def run_generated_code(self, engine, filename, args=[], check_timeout=True, output_nicerizer=None, assert_returncode=0):
    # use files, as PIPE can get too full and hang us
    stdout = self.in_dir('stdout')
    stderr = self.in_dir('stderr')
    # Make sure that we produced proper line endings to the .js file we are about to run.
    self.assertEqual(line_endings.check_line_endings(filename), 0)
    if EMTEST_VERBOSE:
      print("Running '%s' under '%s'" % (filename, engine))
    with chdir(self.get_dir()):
      jsrun.run_js(filename, engine, args, check_timeout,
                   stdout=open(stdout, 'w'),
                   stderr=open(stderr, 'w'),
                   assert_returncode=assert_returncode)
    out = open(stdout, 'r').read()
    err = open(stderr, 'r').read()
    if engine == SPIDERMONKEY_ENGINE and self.get_setting('ASM_JS') == 1:
      err = self.validate_asmjs(err)
    if output_nicerizer:
      ret = output_nicerizer(out, err)
    else:
      ret = out + err
    assert 'strict warning:' not in ret, 'We should pass all strict mode checks: ' + ret
    if EMTEST_VERBOSE:
      print('-- being program output --')
      print(ret, end='')
      print('-- end program output --')
    return ret

  def assertExists(self, filename, msg=None):
    if not msg:
      msg = 'Expected file not found: ' + filename
    self.assertTrue(os.path.exists(filename), msg)

  def assertNotExists(self, filename, msg=None):
    if not msg:
      msg = 'Unexpected file exists: ' + filename
    self.assertFalse(os.path.exists(filename), msg)

  # Tests that the given two paths are identical, modulo path delimiters. E.g. "C:/foo" is equal to "C:\foo".
  def assertPathsIdentical(self, path1, path2):
    path1 = path1.replace('\\', '/')
    path2 = path2.replace('\\', '/')
    return self.assertIdentical(path1, path2)

  # Tests that the given two multiline text content are identical, modulo line ending differences (\r\n on Windows, \n on Unix).
  def assertTextDataIdentical(self, text1, text2):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertIdentical(text1, text2)

  def assertIdentical(self, values, y):
    if type(values) not in [list, tuple]:
      values = [values]
    for x in values:
      if x == y:
        return # success
    self.fail("Expected to have '%s' == '%s', diff:\n\n%s" % (
      limit_size(values[0]), limit_size(y),
      limit_size(''.join([a.rstrip() + '\n' for a in difflib.unified_diff(x.split('\n'), y.split('\n'), fromfile='expected', tofile='actual')]))
    ))

  def assertTextDataContained(self, text1, text2):
    text1 = text1.replace('\r\n', '\n')
    text2 = text2.replace('\r\n', '\n')
    return self.assertContained(text1, text2)

  def assertContained(self, values, string, additional_info=''):
    if type(values) not in [list, tuple]:
      values = [values]
    values = list(map(asstr, values))
    if callable(string):
      string = string()
    for value in values:
      if value in string:
        return # success
    self.fail("Expected to find '%s' in '%s', diff:\n\n%s\n%s" % (
      limit_size(values[0]), limit_size(string),
      limit_size(''.join([a.rstrip() + '\n' for a in difflib.unified_diff(values[0].split('\n'), string.split('\n'), fromfile='expected', tofile='actual')])),
      additional_info
    ))

  def assertNotContained(self, value, string):
    if callable(value):
      value = value() # lazy loading
    if callable(string):
      string = string()
    if value in string:
      self.fail("Expected to NOT find '%s' in '%s', diff:\n\n%s" % (
        limit_size(value), limit_size(string),
        limit_size(''.join([a.rstrip() + '\n' for a in difflib.unified_diff(value.split('\n'), string.split('\n'), fromfile='expected', tofile='actual')]))
      ))

  library_cache = {}

  def get_build_dir(self):
    ret = os.path.join(self.get_dir(), 'building')
    if not os.path.exists(ret):
      os.makedirs(ret)
    return ret

  def get_library(self, name, generated_libs, configure=['sh', './configure'],
                  configure_args=[], make=['make'], make_args='help',
                  cache=True, env_init={}, cache_name_extra='', native=False):
    if make_args == 'help':
      make_args = ['-j', str(multiprocessing.cpu_count())]

    build_dir = self.get_build_dir()
    output_dir = self.get_dir()

    hash_input = (str(Building.COMPILER_TEST_OPTS) + ' $ ' + str(env_init)).encode('utf-8')
    cache_name = name + ','.join([opt for opt in Building.COMPILER_TEST_OPTS if len(opt) < 7]) + '_' + hashlib.md5(hash_input).hexdigest() + cache_name_extra

    valid_chars = "_%s%s" % (string.ascii_letters, string.digits)
    cache_name = ''.join([(c if c in valid_chars else '_') for c in cache_name])

    if self.library_cache is not None:
      if cache and self.library_cache.get(cache_name):
        print('<load %s from cache> ' % cache_name, file=sys.stderr)
        generated_libs = []
        for basename, contents in self.library_cache[cache_name]:
          bc_file = os.path.join(build_dir, cache_name + '_' + basename)
          with open(bc_file, 'wb') as f:
            f.write(contents)
          generated_libs.append(bc_file)
        return generated_libs

    print('<building and saving %s into cache> ' % cache_name, file=sys.stderr)

    return build_library(name, build_dir, output_dir, generated_libs, configure,
                         configure_args, make, make_args, self.library_cache,
                         cache_name, copy_project=True, env_init=env_init,
                         native=native)

  def clear(self):
    for name in os.listdir(self.get_dir()):
      try_delete(os.path.join(self.get_dir(), name))
    if EMSCRIPTEN_TEMP_DIR:
      for name in os.listdir(EMSCRIPTEN_TEMP_DIR):
        try_delete(os.path.join(EMSCRIPTEN_TEMP_DIR, name))

  # Shared test code between main suite and others

  def setup_runtimelink_test(self):
    create_test_file('header.h', r'''
      struct point
      {
        int x, y;
      };
    ''')

    supp = r'''
      #include <stdio.h>
      #include "header.h"

      extern void mainFunc(int x);
      extern int mainInt;

      void suppFunc(struct point &p) {
        printf("supp: %d,%d\n", p.x, p.y);
        mainFunc(p.x + p.y);
        printf("supp see: %d\n", mainInt);
      }

      int suppInt = 76;
    '''
    create_test_file('supp.cpp', supp)

    main = r'''
      #include <stdio.h>
      #include "header.h"

      extern void suppFunc(struct point &p);
      extern int suppInt;

      void mainFunc(int x) {
        printf("main: %d\n", x);
      }

      int mainInt = 543;

      int main( int argc, const char *argv[] ) {
        struct point p = { 54, 2 };
        suppFunc(p);
        printf("main see: %d\nok.\n", suppInt);
        #ifdef BROWSER
          REPORT_RESULT(suppInt);
        #endif
        return 0;
      }
    '''
    return (main, supp)

  # excercise dynamic linker.
  #
  # test that linking to shared library B, which is linked to A, loads A as well.
  # main is also linked to C, which is also linked to A. A is loaded/initialized only once.
  #
  #          B
  #   main <   > A
  #          C
  #
  # this test is used by both test_core and test_browser.
  # when run under broswer it excercises how dynamic linker handles concurrency
  # - because B and C are loaded in parallel.
  def _test_dylink_dso_needed(self, do_run):
    create_test_file('liba.cpp', r'''
        #include <stdio.h>
        #include <emscripten.h>

        static const char *afunc_prev;

        EMSCRIPTEN_KEEPALIVE void afunc(const char *s);
        void afunc(const char *s) {
          printf("a: %s (prev: %s)\n", s, afunc_prev);
          afunc_prev = s;
        }

        struct ainit {
          ainit() {
            puts("a: loaded");
          }
        };

        static ainit _;
      ''')

    create_test_file('libb.cpp', r'''
        #include <emscripten.h>

        void afunc(const char *s);
        EMSCRIPTEN_KEEPALIVE void bfunc();
        void bfunc() {
          afunc("b");
        }
      ''')

    create_test_file('libc.cpp', r'''
        #include <emscripten.h>

        void afunc(const char *s);
        EMSCRIPTEN_KEEPALIVE void cfunc();
        void cfunc() {
          afunc("c");
        }
      ''')

    # _test_dylink_dso_needed can be potentially called several times by a test.
    # reset dylink-related options first.
    self.clear_setting('MAIN_MODULE')
    self.clear_setting('SIDE_MODULE')
    self.clear_setting('RUNTIME_LINKED_LIBS')

    # XXX in wasm each lib load currently takes 5MB; default TOTAL_MEMORY=16MB is thus not enough
    self.set_setting('TOTAL_MEMORY', 32 * 1024 * 1024)

    so = '.wasm' if self.is_wasm() else '.js'

    def ccshared(src, linkto=[]):
      cmdv = [PYTHON, EMCC, src, '-o', os.path.splitext(src)[0] + so] + self.get_emcc_args()
      cmdv += ['-s', 'SIDE_MODULE=1', '-s', 'RUNTIME_LINKED_LIBS=' + str(linkto)]
      run_process(cmdv)

    ccshared('liba.cpp')
    ccshared('libb.cpp', ['liba' + so])
    ccshared('libc.cpp', ['liba' + so])

    self.set_setting('MAIN_MODULE', 1)
    self.set_setting('RUNTIME_LINKED_LIBS', ['libb' + so, 'libc' + so])
    do_run(r'''
      void bfunc();
      void cfunc();

      int _main() {
        bfunc();
        cfunc();
        return 0;
      }
      ''',
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n')

    self.set_setting('RUNTIME_LINKED_LIBS', [])
    self.emcc_args += ['--embed-file', '.@/']
    do_run(r'''
      #include <assert.h>
      #include <dlfcn.h>
      #include <stddef.h>

      int _main() {
        void *bdso, *cdso;
        void (*bfunc)(), (*cfunc)();

        // FIXME for RTLD_LOCAL binding symbols to loaded lib is not currenlty working
        bdso = dlopen("libb%(so)s", RTLD_GLOBAL);
        assert(bdso != NULL);
        cdso = dlopen("libc%(so)s", RTLD_GLOBAL);
        assert(cdso != NULL);

        bfunc = (void (*)())dlsym(bdso, "_Z5bfuncv");
        assert(bfunc != NULL);
        cfunc = (void (*)())dlsym(cdso, "_Z5cfuncv");
        assert(cfunc != NULL);

        bfunc();
        cfunc();
        return 0;
      }
    ''' % locals(),
           'a: loaded\na: b (prev: (null))\na: c (prev: b)\n')

  def filtered_js_engines(self, js_engines=None):
    if js_engines is None:
      js_engines = shared.JS_ENGINES
    for engine in js_engines:
      assert type(engine) == list
    for engine in self.banned_js_engines:
      assert type(engine) == list
    js_engines = [engine for engine in js_engines if engine and engine[0] not in [banned[0] for banned in self.banned_js_engines if banned]]
    return js_engines

  def do_run_from_file(self, src, expected_output, *args, **kwargs):
    self.do_run(open(src).read(), open(expected_output).read(), *args, **kwargs)

  ## Does a complete test - builds, runs, checks output, etc.
  def do_run(self, src, expected_output, args=[], output_nicerizer=None,
             no_build=False, main_file=None, additional_files=[],
             js_engines=None, post_build=None, basename='src.cpp', libraries=[],
             includes=[], force_c=False, build_ll_hook=None,
             assert_returncode=None, assert_identical=False):
    if self.get_setting('ASYNCIFY') == 1 and self.is_wasm_backend():
      self.skipTest("wasm backend doesn't support ASYNCIFY yet")
    if force_c or (main_file is not None and main_file[-2:]) == '.c':
      basename = 'src.c'
      Building.COMPILER = to_cc(Building.COMPILER)

    dirname = self.get_dir()
    filename = os.path.join(dirname, basename)
    if not no_build:
      self.build(src, dirname, filename, main_file=main_file, additional_files=additional_files, libraries=libraries, includes=includes,
                 build_ll_hook=build_ll_hook, post_build=post_build)

    # Run in both JavaScript engines, if optimizing - significant differences there (typed arrays)
    js_engines = self.filtered_js_engines(js_engines)
    js_file = filename + '.o.js'
    if len(js_engines) == 0:
      self.skipTest('No JS engine present to run this test with. Check %s and the paths therein.' % EM_CONFIG)
    if len(js_engines) > 1 and not self.use_all_engines:
      if SPIDERMONKEY_ENGINE in js_engines: # make sure to get asm.js validation checks, using sm
        js_engines = [SPIDERMONKEY_ENGINE]
      else:
        js_engines = js_engines[:1]
    for engine in js_engines:
      # print 'test in', engine
      js_output = self.run_generated_code(engine, js_file, args, output_nicerizer=output_nicerizer, assert_returncode=assert_returncode)
      js_output = js_output.replace('\r\n', '\n')
      if expected_output:
        try:
          if assert_identical:
            self.assertIdentical(expected_output, js_output)
          else:
            self.assertContained(expected_output, js_output)
            self.assertNotContained('ERROR', js_output)
        except Exception:
          print('(test did not pass in JS engine: %s)' % engine)
          raise

    # shutil.rmtree(dirname) # TODO: leave no trace in memory. But for now nice for debugging

    if self.save_JS:
      global test_index
      self.hardcode_arguments(js_file, args)
      shutil.copyfile(js_file, os.path.join(TEMP_DIR, str(test_index) + '.js'))
      test_index += 1

  # No building - just process an existing .ll file (or .bc, which we turn into .ll)
  def do_ll_run(self, ll_file, expected_output=None, args=[], js_engines=None,
                output_nicerizer=None, force_recompile=False,
                build_ll_hook=None, assert_returncode=None):
    filename = os.path.join(self.get_dir(), 'src.cpp')

    self.prep_ll_run(filename, ll_file, force_recompile, build_ll_hook)

    self.ll_to_js(filename)

    self.do_run(None,
                expected_output,
                args,
                no_build=True,
                js_engines=js_engines,
                output_nicerizer=output_nicerizer,
                assert_returncode=assert_returncode)

  def get_freetype_library(self):
    self.set_setting('DEAD_FUNCTIONS', self.get_setting('DEAD_FUNCTIONS') + ['_inflateEnd', '_inflate', '_inflateReset', '_inflateInit2_'])

    return self.get_library('freetype', os.path.join('objs', '.libs', 'libfreetype.a'), configure_args=['--disable-shared'])

  def get_poppler_library(self):
    # The fontconfig symbols are all missing from the poppler build
    # e.g. FcConfigSubstitute
    self.set_setting('ERROR_ON_UNDEFINED_SYMBOLS', 0)

    Building.COMPILER_TEST_OPTS += [
      '-I' + path_from_root('tests', 'freetype', 'include'),
      '-I' + path_from_root('tests', 'poppler', 'include')
    ]

    freetype = self.get_freetype_library()

    # Poppler has some pretty glaring warning.  Suppress them to keep the
    # test output readable.
    Building.COMPILER_TEST_OPTS += [
      '-Wno-sentinel',
      '-Wno-logical-not-parentheses',
      '-Wno-unused-private-field',
      '-Wno-tautological-compare',
      '-Wno-unknown-pragmas',
    ]
    poppler = self.get_library(
        'poppler',
        [os.path.join('utils', 'pdftoppm.o'), os.path.join('utils', 'parseargs.o'), os.path.join('poppler', '.libs', 'libpoppler.a')],
        env_init={'FONTCONFIG_CFLAGS': ' ', 'FONTCONFIG_LIBS': ' '},
        configure_args=['--disable-libjpeg', '--disable-libpng', '--disable-poppler-qt', '--disable-poppler-qt4', '--disable-cms', '--disable-cairo-output', '--disable-abiword-output', '--disable-shared'])

    return poppler + freetype

  def get_zlib_library(self):
    if WINDOWS:
      return self.get_library('zlib', os.path.join('libz.a'),
                              configure=[path_from_root('emconfigure.bat')],
                              configure_args=['cmake', '.'],
                              make=['mingw32-make'],
                              make_args=[])
    return self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a'])


# Run a server and a web page. When a test runs, we tell the server about it,
# which tells the web page, which then opens a window with the test. Doing
# it this way then allows the page to close() itself when done.
def harness_server_func(in_queue, out_queue, port):
  class TestServerHandler(SimpleHTTPRequestHandler):
    # Request header handler for default do_GET() path in
    # SimpleHTTPRequestHandler.do_GET(self) below.
    def send_head(self):
      if self.path.endswith('.js'):
        path = self.translate_path(self.path)
        try:
          f = open(path, 'rb')
        except IOError:
          self.send_error(404, "File not found: " + path)
          return None
        self.send_response(200)
        self.send_header("Content-type", 'application/javascript')
        self.send_header('Cache-Control', 'no-cache, must-revalidate')
        self.send_header('Connection', 'close')
        self.send_header('Expires', '-1')
        self.end_headers()
        return f
      else:
        return SimpleHTTPRequestHandler.send_head(self)

    def do_GET(self):
      if self.path == '/run_harness':
        if DEBUG:
          print('[server startup]')
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(open(path_from_root('tests', 'browser_harness.html'), 'rb').read())
      elif 'report_' in self.path:
        # for debugging, tests may encode the result and their own url (window.location) as result|url
        if '|' in self.path:
          path, url = self.path.split('|', 1)
        else:
          path = self.path
          url = '?'
        if DEBUG:
          print('[server response:', path, url, ']')
        if out_queue.empty():
          out_queue.put(path)
        else:
          # a badly-behaving test may send multiple xhrs with reported results; we just care
          # about the first (if we queued the others, they might be read as responses for
          # later tests, or maybe the test sends more than one in a racy manner).
          # we place 'None' in the queue here so that the outside knows something went wrong
          # (none is not a valid value otherwise; and we need the outside to know because if we
          # raise an error in here, it is just swallowed in python's webserver code - we want
          # the test to actually fail, which a webserver response can't do).
          out_queue.put(None)
          raise Exception('browser harness error, excessive response to server - test must be fixed! "%s"' % self.path)
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.send_header('Cache-Control', 'no-cache, must-revalidate')
        self.send_header('Connection', 'close')
        self.send_header('Expires', '-1')
        self.end_headers()
        self.wfile.write(b'OK')
      elif 'stdout=' in self.path or 'stderr=' in self.path or 'exception=' in self.path:
        '''
          To get logging to the console from browser tests, add this to
          print/printErr/the exception handler in src/shell.html:

            var xhr = new XMLHttpRequest();
            xhr.open('GET', encodeURI('http://localhost:8888?stdout=' + text));
            xhr.send();
        '''
        print('[client logging:', urllib.unquote_plus(self.path), ']')
      elif self.path == '/check':
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        if not in_queue.empty():
          url, dir = in_queue.get()
          if DEBUG:
            print('[queue command:', url, dir, ']')
          assert in_queue.empty(), 'should not be any blockage - one test runs at a time'
          assert out_queue.empty(), 'the single response from the last test was read'
          # tell the browser to load the test
          self.wfile.write('COMMAND:' + url)
          # move us to the right place to serve the files
          os.chdir(dir)
        else:
          # the browser must keep polling
          self.wfile.write('(wait)')
      else:
        # Use SimpleHTTPServer default file serving operation for GET.
        if DEBUG:
          print('[simple HTTP serving:', urllib.unquote_plus(self.path), ']')
        SimpleHTTPRequestHandler.do_GET(self)

    def log_request(code=0, size=0):
      # don't log; too noisy
      pass

  # allows streaming compilation to work
  SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'

  httpd = HTTPServer(('localhost', port), TestServerHandler)
  httpd.serve_forever() # test runner will kill us


class BrowserCore(RunnerCore):
  # note how many tests hang / do not send an output. if many of these
  # happen, likely something is broken and it is best to abort the test
  # suite early, as otherwise we will wait for the timeout on every
  # single test (hundreds of minutes)
  MAX_UNRESPONSIVE_TESTS = 10

  unresponsive_tests = 0

  def __init__(self, *args, **kwargs):
    super(BrowserCore, self).__init__(*args, **kwargs)

  @classmethod
  def setUpClass(cls):
    super(BrowserCore, cls).setUpClass()
    cls.also_asmjs = int(os.getenv('EMTEST_BROWSER_ALSO_ASMJS', '0')) == 1
    cls.port = int(os.getenv('EMTEST_BROWSER_PORT', '8888'))
    if not has_browser():
      return
    if not EMTEST_BROWSER:
      print("Using default system browser")
    else:
      cmd = shlex.split(EMTEST_BROWSER)

      def run_in_other_browser(url):
        subprocess.Popen(cmd + [url])

      webbrowser.open_new = run_in_other_browser
      print("Using Emscripten browser: " + str(cmd))
    cls.browser_timeout = 30
    cls.harness_in_queue = multiprocessing.Queue()
    cls.harness_out_queue = multiprocessing.Queue()
    cls.harness_server = multiprocessing.Process(target=harness_server_func, args=(cls.harness_in_queue, cls.harness_out_queue, cls.port))
    cls.harness_server.start()
    print('[Browser harness server on process %d]' % cls.harness_server.pid)
    webbrowser.open_new('http://localhost:%s/run_harness' % cls.port)

  @classmethod
  def tearDownClass(cls):
    super(BrowserCore, cls).tearDownClass()
    if not has_browser():
      return
    cls.harness_server.terminate()
    print('[Browser harness server terminated]')
    if WINDOWS:
      # On Windows, shutil.rmtree() in tearDown() raises this exception if we do not wait a bit:
      # WindowsError: [Error 32] The process cannot access the file because it is being used by another process.
      time.sleep(0.1)

  def assert_out_queue_empty(self, who):
    if not self.harness_out_queue.empty():
      while not self.harness_out_queue.empty():
        self.harness_out_queue.get()
      raise Exception('excessive responses from %s' % who)

  def run_browser(self, html_file, message, expectedResult=None, timeout=None):
    if not has_browser():
      return
    if BrowserCore.unresponsive_tests >= BrowserCore.MAX_UNRESPONSIVE_TESTS:
      self.skipTest('too many unresponsive tests, skipping browser launch - check your setup!')
    self.assert_out_queue_empty('previous test')
    if DEBUG:
      print('[browser launch:', html_file, ']')
    if expectedResult is not None:
      try:
        self.harness_in_queue.put((
          asbytes('http://localhost:%s/%s' % (self.port, html_file)),
          self.get_dir()
        ))
        received_output = False
        output = '[no http server activity]'
        start = time.time()
        if timeout is None:
          timeout = self.browser_timeout
        while time.time() - start < timeout:
          if not self.harness_out_queue.empty():
            output = self.harness_out_queue.get()
            received_output = True
            break
          time.sleep(0.1)
        if not received_output:
          BrowserCore.unresponsive_tests += 1
          print('[unresponsive tests: %d]' % BrowserCore.unresponsive_tests)
        if output is None:
          # the browser harness reported an error already, and sent a None to tell
          # us to also fail the test
          raise Exception('failing test due to browser harness error')
        if output.startswith('/report_result?skipped:'):
          self.skipTest(unquote(output[len('/report_result?skipped:'):]).strip())
        else:
          self.assertIdentical(expectedResult, output)
      finally:
        time.sleep(0.1) # see comment about Windows above
      self.assert_out_queue_empty('this test')
    else:
      webbrowser.open_new(os.path.abspath(html_file))
      print('A web browser window should have opened a page containing the results of a part of this test.')
      print('You need to manually look at the page to see that it works ok: ' + message)
      print('(sleeping for a bit to keep the directory alive for the web browser..)')
      time.sleep(5)
      print('(moving on..)')

  def with_report_result(self, code):
    return '#define EMTEST_PORT_NUMBER %d\n#include "%s"\n' % (self.port, path_from_root('tests', 'report_result.h')) + code

  # @manually_trigger If set, we do not assume we should run the reftest when main() is done.
  #                   Instead, call doReftest() in JS yourself at the right time.
  def reftest(self, expected, manually_trigger=False):
    # make sure the pngs used here have no color correction, using e.g.
    #   pngcrush -rem gAMA -rem cHRM -rem iCCP -rem sRGB infile outfile
    basename = os.path.basename(expected)
    shutil.copyfile(expected, os.path.join(self.get_dir(), basename))
    with open(os.path.join(self.get_dir(), 'reftest.js'), 'w') as out:
      with open(path_from_root('tests', 'browser_reporting.js')) as reporting:
        out.write('''
      function doReftest() {
        if (doReftest.done) return;
        doReftest.done = true;
        var img = new Image();
        img.onload = function() {
          assert(img.width == Module.canvas.width, 'Invalid width: ' + Module.canvas.width + ', should be ' + img.width);
          assert(img.height == Module.canvas.height, 'Invalid height: ' + Module.canvas.height + ', should be ' + img.height);

          var canvas = document.createElement('canvas');
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext('2d');
          ctx.drawImage(img, 0, 0);
          var expected = ctx.getImageData(0, 0, img.width, img.height).data;

          var actualUrl = Module.canvas.toDataURL();
          var actualImage = new Image();
          actualImage.onload = function() {
            /*
            document.body.appendChild(img); // for comparisons
            var div = document.createElement('div');
            div.innerHTML = '^=expected, v=actual';
            document.body.appendChild(div);
            document.body.appendChild(actualImage); // to grab it for creating the test reference
            */

            var actualCanvas = document.createElement('canvas');
            actualCanvas.width = actualImage.width;
            actualCanvas.height = actualImage.height;
            var actualCtx = actualCanvas.getContext('2d');
            actualCtx.drawImage(actualImage, 0, 0);
            var actual = actualCtx.getImageData(0, 0, actualImage.width, actualImage.height).data;

            var total = 0;
            var width = img.width;
            var height = img.height;
            for (var x = 0; x < width; x++) {
              for (var y = 0; y < height; y++) {
                total += Math.abs(expected[y*width*4 + x*4 + 0] - actual[y*width*4 + x*4 + 0]);
                total += Math.abs(expected[y*width*4 + x*4 + 1] - actual[y*width*4 + x*4 + 1]);
                total += Math.abs(expected[y*width*4 + x*4 + 2] - actual[y*width*4 + x*4 + 2]);
              }
            }
            var wrong = Math.floor(total / (img.width*img.height*3)); // floor, to allow some margin of error for antialiasing
            // If the main JS file is in a worker, or modularize, then we need to supply our own reporting logic.
            if (typeof reportResultToServer === 'undefined') {
              (function() {
                %s
                reportResultToServer(wrong);
              })();
            } else {
              reportResultToServer(wrong);
            }
          };
          actualImage.src = actualUrl;
        }
        img.src = '%s';
      };

      // Automatically trigger the reftest?
      if (!%s) {
        // Yes, automatically

        Module['postRun'] = doReftest;

        if (typeof WebGLClient !== 'undefined') {
          // trigger reftest from RAF as well, needed for workers where there is no pre|postRun on the main thread
          var realRAF = window.requestAnimationFrame;
          window.requestAnimationFrame = function(func) {
            realRAF(function() {
              func();
              realRAF(doReftest);
            });
          };

          // trigger reftest from canvas render too, for workers not doing GL
          var realWOM = worker.onmessage;
          worker.onmessage = function(event) {
            realWOM(event);
            if (event.data.target === 'canvas' && event.data.op === 'render') {
              realRAF(doReftest);
            }
          };
        }

      } else {
        // Manually trigger the reftest.

        // The user will call it.
        // Add an event loop iteration to ensure rendering, so users don't need to bother.
        var realDoReftest = doReftest;
        doReftest = function() {
          setTimeout(realDoReftest, 1);
        };
      }
''' % (reporting.read(), basename, int(manually_trigger)))

  def compile_btest(self, args):
    run_process([PYTHON, EMCC] + args + ['--pre-js', path_from_root('tests', 'browser_reporting.js')])

  def btest(self, filename, expected=None, reference=None, force_c=False,
            reference_slack=0, manual_reference=False, post_build=None,
            args=[], outfile='test.html', message='.', also_proxied=False,
            url_suffix='', timeout=None, also_asmjs=False,
            manually_trigger_reftest=False):
    assert expected or reference, 'a btest must either expect an output, or have a reference image'
    # if we are provided the source and not a path, use that
    filename_is_src = '\n' in filename
    src = filename if filename_is_src else ''
    original_args = args[:]
    if 'USE_PTHREADS=1' in args and not self.is_wasm_backend() and 'ALLOW_MEMORY_GROWTH=1' not in args:
      if EMTEST_WASM_PTHREADS:
        also_asmjs = True
      elif 'WASM=0' not in args:
        args += ['-s', 'WASM=0']
    if 'WASM=0' not in args:
      # Filter out separate-asm, which is implied by wasm
      args = [a for a in args if a != '--separate-asm']
    args += ['-DEMTEST_PORT_NUMBER=%d' % self.port, '-include', path_from_root('tests', 'report_result.h')]
    if filename_is_src:
      filepath = os.path.join(self.get_dir(), 'main.c' if force_c else 'main.cpp')
      with open(filepath, 'w') as f:
       f.write(src)
    else:
      filepath = path_from_root('tests', filename)
    if reference:
      self.reference = reference
      expected = [str(i) for i in range(0, reference_slack + 1)]
      self.reftest(path_from_root('tests', reference), manually_trigger=manually_trigger_reftest)
      if not manual_reference:
        args = args + ['--pre-js', 'reftest.js', '-s', 'GL_TESTING=1']
    all_args = ['-s', 'IN_TEST_HARNESS=1', filepath, '-o', outfile] + args
    # print('all args:', all_args)
    try_delete(outfile)
    self.compile_btest(all_args)
    self.assertExists(outfile)
    if post_build:
      post_build()
    if not isinstance(expected, list):
      expected = [expected]
    self.run_browser(outfile + url_suffix, message, ['/report_result?' + e for e in expected], timeout=timeout)

    # Tests can opt into being run under asmjs as well
    if 'WASM=0' not in args and (also_asmjs or self.also_asmjs):
      self.btest(filename, expected, reference, force_c, reference_slack, manual_reference, post_build,
                 args + ['-s', 'WASM=0'], outfile, message, also_proxied=False, timeout=timeout)

    if also_proxied:
      print('proxied...')
      if reference:
        assert not manual_reference
        manual_reference = True
        assert not post_build
        post_build = self.post_manual_reftest
      # run proxied
      self.btest(filename, expected, reference, force_c, reference_slack, manual_reference, post_build,
                 original_args + ['--proxy-to-worker', '-s', 'GL_TESTING=1'], outfile, message, timeout=timeout)


###################################################################################################


def build_library(name,
                  build_dir,
                  output_dir,
                  generated_libs,
                  configure=['sh', './configure'],
                  configure_args=[],
                  make=['make'],
                  make_args='help',
                  cache=None,
                  cache_name=None,
                  copy_project=False,
                  env_init={},
                  source_dir=None,
                  native=False):
  """Build a library into a .bc file. We build the .bc file once and cache it
  for all our tests. (We cache in memory since the test directory is destroyed
  and recreated for each test. Note that we cache separately for different
  compilers).  This cache is just during the test runner. There is a different
  concept of caching as well, see |Cache|.
  """

  if type(generated_libs) is not list:
    generated_libs = [generated_libs]
  if source_dir is None:
    source_dir = path_from_root('tests', name.replace('_native', ''))
  if make_args == 'help':
    make_args = ['-j', str(multiprocessing.cpu_count())]

  temp_dir = build_dir
  if copy_project:
    project_dir = os.path.join(temp_dir, name)
    if os.path.exists(project_dir):
      shutil.rmtree(project_dir)
    shutil.copytree(source_dir, project_dir) # Useful in debugging sometimes to comment this out, and two lines above
  else:
    project_dir = build_dir
  try:
    old_dir = os.getcwd()
  except:
    old_dir = None
  os.chdir(project_dir)
  generated_libs = [os.path.join(project_dir, lib) for lib in generated_libs]
  # for lib in generated_libs:
  #   try:
  #     os.unlink(lib) # make sure compilation completed successfully
  #   except:
  #     pass
  env = Building.get_building_env(native, True)
  for k, v in env_init.items():
    env[k] = v
  if configure:
    # Useful in debugging sometimes to comment this out (and the lines below
    # up to and including the |link| call)
    if EM_BUILD_VERBOSE < 2:
      stdout = open(os.path.join(project_dir, 'configure_out'), 'w')
    else:
      stdout = None
    if EM_BUILD_VERBOSE < 1:
      stderr = open(os.path.join(project_dir, 'configure_err'), 'w')
    else:
      stderr = None
    try:
      Building.configure(configure + configure_args, env=env, stdout=stdout, stderr=stderr)
    except subprocess.CalledProcessError as e:
      pass # Ignore exit code != 0

  def open_make_out(i, mode='r'):
    return open(os.path.join(project_dir, 'make_out' + str(i)), mode)

  def open_make_err(i, mode='r'):
    return open(os.path.join(project_dir, 'make_err' + str(i)), mode)

  if EM_BUILD_VERBOSE >= 3:
    make_args += ['VERBOSE=1']

  # FIXME: Sad workaround for some build systems that need to be run twice to succeed (e.g. poppler)
  for i in range(2):
    with open_make_out(i, 'w') as make_out:
      with open_make_err(i, 'w') as make_err:
        stdout = make_out if EM_BUILD_VERBOSE < 2 else None
        stderr = make_err if EM_BUILD_VERBOSE < 1 else None
        if i == 0:
          try:
            Building.make(make + make_args, stdout=stdout, stderr=stderr, env=env)
          except subprocess.CalledProcessError as e:
            pass # Ignore exit code != 0
        else:
            Building.make(make + make_args, stdout=stdout, stderr=stderr, env=env)
    try:
      if cache is not None:
        cache[cache_name] = []
        for f in generated_libs:
          basename = os.path.basename(f)
          cache[cache_name].append((basename, open(f, 'rb').read()))
      break
    except Exception as e:
      if i > 0:
        if EM_BUILD_VERBOSE == 0:
          # Due to the ugly hack above our best guess is to output the first run
          with open_make_err(0) as ferr:
            for line in ferr:
              sys.stderr.write(line)
        raise Exception('could not build library ' + name + ' due to exception ' + str(e))
  if old_dir:
    os.chdir(old_dir)
  return generated_libs


def check_js_engines():
  total_engines = len(shared.JS_ENGINES)
  shared.JS_ENGINES = list(filter(jsrun.check_engine, shared.JS_ENGINES))
  if not shared.JS_ENGINES:
    print('WARNING: None of the JS engines in JS_ENGINES appears to work.')
  elif len(shared.JS_ENGINES) < total_engines:
    print('WARNING: Not all the JS engines in JS_ENGINES appears to work, ignoring those.')

  if EMTEST_ALL_ENGINES:
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
      for test in which:
        print('will skip "%s"' % test, file=sys.stderr)
        suite_name, test_name = test.split('.')
        for m in modules:
          try:
            suite = getattr(m, suite_name)
            setattr(suite, test_name, lambda s: s.skipTest("requested to be skipped"))
            break
          except:
            pass
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
  suite_supported = module.__name__ in ('test_core', 'test_other')
  has_multiple_tests = len(tests) > 1
  has_multiple_cores = parallel_runner.num_cores() > 1
  if suite_supported and has_multiple_tests and has_multiple_cores:
    return parallel_runner.ParallelTestSuite()
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
  parser.add_argument('-j', '--js-engine', help='Set JS_ENGINE_OVERRIDE')
  parser.add_argument('tests', nargs='*')
  return parser.parse_args()


def main(args):
  options = parse_args(args)
  if options.js_engine:
    if options.js_engine == 'SPIDERMONKEY_ENGINE':
      Building.JS_ENGINE_OVERRIDE = SPIDERMONKEY_ENGINE
    elif options.js_engine == 'V8_ENGINE':
      Building.JS_ENGINE_OVERRIDE = V8_ENGINE
    elif options.js_engine == 'NODE_JS':
      Building.JS_ENGINE_OVERRIDE = NODE_JS
    else:
      print('Unknown js engine override: ' + options.js_engine)
      return 1
    print("Overriding JS engine: " + Building.JS_ENGINE_OVERRIDE[0])

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
