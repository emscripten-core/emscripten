#!/usr/bin/python
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Runs csmith, a C fuzzer, and looks for bugs.

CSMITH_PATH should be set to something like /usr/local/include/csmith
"""
from __future__ import print_function

import os
import sys
import shutil
import random
from distutils.spawn import find_executable
from subprocess import check_call, Popen, PIPE, CalledProcessError

script_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(script_dir))))

from tools import shared

# can add flags like --no-threads --ion-offthread-compile=off
engine = eval('shared.' + sys.argv[1]) if len(sys.argv) > 1 else shared.JS_ENGINES[0]

print('testing js engine', engine)

TEST_BINARYEN = 1

CSMITH = os.environ.get('CSMITH', find_executable('csmith'))
assert CSMITH, 'Could not find CSmith on your PATH. Please set the environment variable CSMITH.'
CSMITH_PATH = os.environ.get('CSMITH_PATH', '/usr/include/csmith')
assert os.path.exists(CSMITH_PATH), 'Please set the environment variable CSMITH_PATH.'
CSMITH_CFLAGS = ['-I', CSMITH_PATH]

filename = os.path.join(os.getcwd(), 'temp_fuzzcode' + str(os.getpid()) + '_')

shared.DEFAULT_TIMEOUT = 5

tried = 0

notes = {'invalid': 0, 'embug': 0}

fails = 0

while 1:
  if random.random() < 0.666:
    opts = '-O' + str(random.randint(0, 3))
  else:
    if random.random() < 0.5:
      opts = '-Os'
    else:
      opts = '-Oz'
  print('opt level:', opts)

  llvm_opts = []
  if random.random() < 0.5:
    llvm_opts = ['--llvm-opts', str(random.randint(0, 3))]

  print('Tried %d, notes: %s' % (tried, notes))
  print('1) Generate source')
  extra_args = []
  if random.random() < 0.5:
    extra_args += ['--no-math64']
  extra_args += ['--no-bitfields'] # due to pnacl bug 4027, "LLVM ERROR: can't convert calls with illegal types"
  # if random.random() < 0.5: extra_args += ['--float'] # XXX hits undefined behavior on float=>int conversions (too big to fit)
  if random.random() < 0.5:
    extra_args += ['--max-funcs', str(random.randint(10, 30))]
  suffix = '.c'
  COMP = shared.CLANG_CC
  fullname = filename + suffix
  check_call([CSMITH, '--no-volatiles', '--no-packed-struct'] + extra_args,
             # ['--max-block-depth', '2', '--max-block-size', '2', '--max-expr-complexity', '2', '--max-funcs', '2'],
             stdout=open(fullname, 'w'))
  print('1) Generate source... %.2f K' % (len(open(fullname).read()) / 1024.))

  tried += 1

  print('2) Compile natively')
  shared.try_delete(filename)
  try:
    shared.run_process([COMP, '-m32', opts, fullname, '-o', filename + '1'] + CSMITH_CFLAGS + ['-w']) # + shared.EMSDK_OPTS
  except CalledProcessError:
    print('Failed to compile natively using clang')
    notes['invalid'] += 1
    continue

  shared.run_process([COMP, '-m32', opts, '-emit-llvm', '-c', fullname, '-o', filename + '.bc'] + CSMITH_CFLAGS + shared.EMSDK_OPTS + ['-w'])
  shared.run_process([shared.path_from_root('tools', 'nativize_llvm.py'), filename + '.bc'], stderr=PIPE)
  shutil.move(filename + '.bc.run', filename + '2')
  shared.run_process([COMP, fullname, '-o', filename + '3'] + CSMITH_CFLAGS + ['-w'])
  print('3) Run natively')
  try:
    correct1 = shared.jsrun.timeout_run(Popen([filename + '1'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct1 or len(correct1) < 10:
      raise Exception('segfault')
    correct2 = shared.jsrun.timeout_run(Popen([filename + '2'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct2 or len(correct2) < 10:
      raise Exception('segfault')
    correct3 = shared.jsrun.timeout_run(Popen([filename + '3'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct3 or len(correct3) < 10:
      raise Exception('segfault')
    if correct1 != correct3:
      raise Exception('clang opts change result')
  except Exception as e:
    print('Failed or infinite looping in native, skipping', e)
    notes['invalid'] += 1
    continue

  fail_output_name = 'newfail_%d_%d%s' % (os.getpid(), fails, suffix)

  print('4) Compile JS-ly and compare')

  def try_js(args=[]):
    shared.try_delete(filename + '.js')
    js_args = [shared.PYTHON, shared.EMCC, fullname, '-o', filename + '.js'] + [opts] + llvm_opts + CSMITH_CFLAGS + args + ['-w']
    if TEST_BINARYEN:
      js_args += ['-s', 'BINARYEN=1', '-s', 'BINARYEN_TRAP_MODE="js"']
      if random.random() < 0.5:
        js_args += ['-g']
      if random.random() < 0.1:
        if random.random() < 0.5:
          js_args += ['--js-opts', '0']
        else:
          js_args += ['--js-opts', '1']
      if random.random() < 0.5:
        # pick random passes
        BINARYEN_PASSES = [
          "code-pushing",
          "duplicate-function-elimination",
          "dce",
          "remove-unused-brs",
          "remove-unused-names",
          "local-cse",
          "optimize-instructions",
          "post-emscripten",
          "precompute",
          "simplify-locals",
          "simplify-locals-nostructure",
          "vacuum",
          "coalesce-locals",
          "reorder-locals",
          "merge-blocks",
          "remove-unused-module-elements",
          "memory-packing",
        ]
        passes = []
        while 1:
          passes.append(random.choice(BINARYEN_PASSES))
          if random.random() < 0.1:
            break
        js_args += ['-s', 'BINARYEN_PASSES="' + ','.join(passes) + '"']
    if random.random() < 0.5:
      js_args += ['-s', 'ALLOW_MEMORY_GROWTH=1']
    if random.random() < 0.5 and 'ALLOW_MEMORY_GROWTH=1' not in js_args and 'BINARYEN=1' not in js_args:
      js_args += ['-s', 'MAIN_MODULE=1']
    if random.random() < 0.25:
      js_args += ['-s', 'INLINING_LIMIT=1'] # inline nothing, for more call interaction
    if random.random() < 0.01:
      js_args += ['-s', 'EMTERPRETIFY=1']
      if random.random() < 0.5:
        if random.random() < 0.5:
          js_args += ['-s', 'EMTERPRETIFY_BLACKLIST=["_main"]'] # blacklist main and all inlined into it, but interpret the rest, tests mixing
        else:
          js_args += ['-s', 'EMTERPRETIFY_WHITELIST=["_main"]'] # the opposite direction
      if random.random() < 0.5:
        js_args += ['-s', 'EMTERPRETIFY_ASYNC=1']
    if random.random() < 0.5:
      js_args += ["--memory-init-file", "0", "-s", "MEM_INIT_METHOD=2"]
    if random.random() < 0.5:
      js_args += ['-s', 'ASSERTIONS=1']
    print('(compile)', ' '.join(js_args))
    short_args = [shared.PYTHON, shared.EMCC, fail_output_name] + js_args[5:]
    escaped_short_args = map(lambda x: ("'" + x + "'") if '"' in x else x, short_args)
    open(fullname, 'a').write('\n// ' + ' '.join(escaped_short_args) + '\n\n')
    try:
      shared.run_process(js_args)
      assert os.path.exists(filename + '.js')
      return js_args
    except Exception:
      return False

  def execute_js(engine):
    print('(run in %s)' % engine)
    try:
      js = shared.jsrun.run_js(filename + '.js', engine=engine, check_timeout=True, assert_returncode=None)
    except CalledProcessError:
      print('failed to run in primary')
      return False
    js = js.split('\n')[0] + '\n' # remove any extra printed stuff (node workarounds)
    return correct1 == js or correct2 == js

  def fail():
    global fails
    print("EMSCRIPTEN BUG")
    notes['embug'] += 1
    fails += 1
    shutil.copyfile(fullname, fail_output_name)

  js_args = try_js()
  if not js_args:
    fail()
    continue
  if not execute_js(engine):
    fail()
    continue
