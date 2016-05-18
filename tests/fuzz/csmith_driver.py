#!/usr/bin/python

'''
Runs csmith, a C fuzzer, and looks for bugs.

CSMITH_PATH should be set to something like /usr/local/include/csmith
'''

import os, sys, difflib, shutil, random
from distutils.spawn import find_executable
from subprocess import check_call, Popen, PIPE, STDOUT, CalledProcessError

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'tools')]
import shared

# can add flags like --no-threads --ion-offthread-compile=off
engine1 = eval('shared.' + sys.argv[1]) if len(sys.argv) > 1 else shared.JS_ENGINES[0]
engine2 = shared.SPIDERMONKEY_ENGINE if os.path.exists(shared.SPIDERMONKEY_ENGINE[0]) else None

print 'testing js engines', engine1, engine2

CSMITH = os.environ.get('CSMITH') or find_executable('csmith')
assert CSMITH, 'Could not find CSmith on your PATH. Please set the environment variable CSMITH.'
CSMITH_PATH = os.environ.get('CSMITH_PATH')
assert CSMITH_PATH, 'Please set the environment variable CSMITH_PATH.'
CSMITH_CFLAGS = ['-I', os.path.join(CSMITH_PATH, 'runtime')]

filename = os.path.join(os.getcwd(), 'temp_fuzzcode' + str(os.getpid()) + '_')

shared.DEFAULT_TIMEOUT = 5

tried = 0

notes = { 'invalid': 0, 'embug': 0 }

fails = 0

while 1:
  if random.random() < 0.666:
    opts = '-O' + str(random.randint(0, 3))
  else:
    if random.random() < 0.5:
      opts = '-Os'
    else:
      opts = '-Oz'
  print 'opt level:', opts

  llvm_opts = []
  if random.random() < 0.5:
    llvm_opts = ['--llvm-opts', str(random.randint(0, 3))]

  print 'Tried %d, notes: %s' % (tried, notes)
  print '1) Generate source'
  extra_args = []
  if random.random() < 0.5: extra_args += ['--no-math64']
  extra_args += ['--no-bitfields'] # due to pnacl bug 4027, "LLVM ERROR: can't convert calls with illegal types"
  #if random.random() < 0.5: extra_args += ['--float'] # XXX hits undefined behavior on float=>int conversions (too big to fit)
  if random.random() < 0.5: extra_args += ['--max-funcs', str(random.randint(10, 30))]
  suffix = '.c'
  COMP = shared.CLANG_CC
  if random.random() < 0.5:
    extra_args += ['--lang-cpp']
    suffix += 'pp'
    COMP = shared.CLANG
  print COMP, extra_args
  fullname = filename + suffix
  check_call([CSMITH, '--no-volatiles', '--no-packed-struct'] + extra_args,
                 #['--max-block-depth', '2', '--max-block-size', '2', '--max-expr-complexity', '2', '--max-funcs', '2'],
                 stdout=open(fullname, 'w'))
  print '1) Generate source... %.2f K' % (len(open(fullname).read())/1024.)

  tried += 1

  print '2) Compile natively'
  shared.try_delete(filename)
  try:
    shared.check_execute([COMP, '-m32', opts, fullname, '-o', filename + '1'] + CSMITH_CFLAGS + ['-w']) #  + shared.EMSDK_OPTS
  except Exception, e:
    print 'Failed to compile natively using clang'
    notes['invalid'] += 1
    continue

  shared.check_execute([COMP, '-m32', opts, '-emit-llvm', '-c', fullname, '-o', filename + '.bc'] + CSMITH_CFLAGS + shared.EMSDK_OPTS + ['-w'])
  shared.check_execute([shared.path_from_root('tools', 'nativize_llvm.py'), filename + '.bc'], stderr=PIPE)
  shutil.move(filename + '.bc.run', filename + '2')
  shared.check_execute([COMP, fullname, '-o', filename + '3'] + CSMITH_CFLAGS + ['-w'])
  print '3) Run natively'
  try:
    correct1 = shared.jsrun.timeout_run(Popen([filename + '1'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct1 or len(correct1) < 10: raise Exception('segfault')
    correct2 = shared.jsrun.timeout_run(Popen([filename + '2'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct2 or len(correct2) < 10: raise Exception('segfault')
    correct3 = shared.jsrun.timeout_run(Popen([filename + '3'], stdout=PIPE, stderr=PIPE), 3)
    if 'Segmentation fault' in correct3 or len(correct3) < 10: raise Exception('segfault')
    if correct1 != correct3: raise Exception('clang opts change result')
  except Exception, e:
    print 'Failed or infinite looping in native, skipping', e
    notes['invalid'] += 1
    continue

  print '4) Compile JS-ly and compare'

  def try_js(args=[]):
    shared.try_delete(filename + '.js')
    js_args = [shared.PYTHON, shared.EMCC, opts] + llvm_opts + [fullname, '-o', filename + '.js'] + CSMITH_CFLAGS + args + ['-w']
    if random.random() < 0.5:
      js_args += ['-s', 'ALLOW_MEMORY_GROWTH=1']
    if random.random() < 0.5 and 'ALLOW_MEMORY_GROWTH=1' not in js_args:
      js_args += ['-s', 'MAIN_MODULE=1']
    if random.random() < 0.25:
      js_args += ['-s', 'INLINING_LIMIT=1'] # inline nothing, for more call interaction
    if random.random() < 0.333:
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
    #js_args += ['-s', 'BINARYEN=1']
    #if random.random() < 0.333:
    #  js_args += ['-s', 'BINARYEN_METHOD="interpret-s-expr"']
    #elif random.random() < 0.5:
    #  js_args += ['-s', 'BINARYEN_METHOD="interpret-binary"']
    #else:
    #  js_args += ['-s', 'BINARYEN_METHOD="interpret-asm2wasm"']
    print '(compile)', ' '.join(js_args)
    open(fullname, 'a').write('\n// ' + ' '.join(js_args) + '\n\n')
    try:
      shared.check_execute(js_args)
      assert os.path.exists(filename + '.js')
      return js_args
    except:
      return False

  def execute_js(engine):
    print '(run in %s)' % engine
    js = shared.run_js(filename + '.js', engine=engine, check_timeout=True, assert_returncode=None)
    js = js.split('\n')[0] + '\n' # remove any extra printed stuff (node workarounds)
    return correct1 == js or correct2 == js

  def fail():
    global fails
    print "EMSCRIPTEN BUG"
    notes['embug'] += 1
    fails += 1
    shutil.copyfile(fullname, 'newfail_%d_%d%s' % (os.getpid(), fails, suffix))

  js_args = try_js()
  if not js_args:
    fail()
    continue
  if not execute_js(engine1):
    fail()
    continue
  if engine2 and not execute_js(engine2):
    fail()
    continue

  # This is ok. Try validation in secondary JS engine
  if opts != '-O0' and 'ALLOW_MEMORY_GROWTH=1' not in js_args and engine2:
    try:
      js2 = shared.run_js(filename + '.js', stderr=PIPE, engine=engine2 + ['-w'], full_output=True, check_timeout=True, assert_returncode=None)
    except:
      print 'failed to run in secondary', js2
      break

    # asm.js testing
    if 'warning: Successfully compiled asm.js code' not in js2:
      print "ODIN VALIDATION BUG"
      notes['embug'] += 1
      fail()
      continue
    print '[asm.js validation ok in %s]' % str(engine2)

