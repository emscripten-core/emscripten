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

engine1 = eval('shared.' + sys.argv[1]) if len(sys.argv) > 1 else shared.JS_ENGINES[0]
engine2 = eval('shared.' + sys.argv[2]) if len(sys.argv) > 2 else None

print 'testing js engines', engine1, engine2

CSMITH = os.environ.get('CSMITH') or find_executable('csmith')
assert CSMITH, 'Could not find CSmith on your PATH. Please set the environment variable CSMITH.'
CSMITH_PATH = os.environ.get('CSMITH_PATH')
assert CSMITH_PATH, 'Please set the environment variable CSMITH_PATH.'
CSMITH_CFLAGS = ['-I', os.path.join(CSMITH_PATH, 'runtime')]

filename = os.path.join(shared.CANONICAL_TEMP_DIR, 'fuzzcode')

shared.DEFAULT_TIMEOUT = 5

tried = 0

notes = { 'invalid': 0, 'unaligned': 0, 'embug': 0 }

fails = 0

while 1:
  opts = '-O' + str(random.randint(0, 2))
  print 'opt level:', opts

  print 'Tried %d, notes: %s' % (tried, notes)
  print '1) Generate C'
  check_call([CSMITH, '--no-volatiles', '--no-packed-struct'],# '--no-math64'
                 #['--max-block-depth', '2', '--max-block-size', '2', '--max-expr-complexity', '2', '--max-funcs', '2'],
                 stdout=open(filename + '.c', 'w'))
  #shutil.copyfile(filename + '.c', 'testcase%d.c' % tried)
  print '1) Generate C... %.2f K of C source' % (len(open(filename + '.c').read())/1024.)

  tried += 1

  print '2) Compile natively'
  shared.try_delete(filename)
  shared.check_execute([shared.CLANG_CC, opts, filename + '.c', '-o', filename + '1'] + CSMITH_CFLAGS) #  + shared.EMSDK_OPTS
  shared.check_execute([shared.CLANG_CC, opts, '-emit-llvm', '-c', '-Xclang', '-triple=i386-pc-linux-gnu', filename + '.c', '-o', filename + '.bc'] + CSMITH_CFLAGS + shared.EMSDK_OPTS)
  shared.check_execute([shared.path_from_root('tools', 'nativize_llvm.py'), filename + '.bc'])
  shutil.move(filename + '.bc.run', filename + '2')
  shared.check_execute([shared.CLANG_CC, filename + '.c', '-o', filename + '3'] + CSMITH_CFLAGS)
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

  def try_js(args):
    shared.try_delete(filename + '.js')
    print '(compile)'
    shared.check_execute([shared.EMCC, opts, filename + '.c', '-o', filename + '.js'] + CSMITH_CFLAGS + args)
    assert os.path.exists(filename + '.js')
    print '(run)'
    js = shared.run_js(filename + '.js', stderr=PIPE, engine=engine1, check_timeout=True)
    assert correct1 == js or correct2 == js, ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(correct1.split('\n'), js.split('\n'), fromfile='expected', tofile='actual')])

  # Try normally, then try unaligned because csmith does generate nonportable code that requires x86 alignment
  ok = False
  normal = True
  for args, note in [([], None), (['-s', 'UNALIGNED_MEMORY=1'], 'unaligned')]:
    try:
      try_js(args)
      ok = True
      if note:
        notes[note] += 1
      break
    except Exception, e:
      print e
      normal = False
  #open('testcase%d.js' % tried, 'w').write(
  #  open(filename + '.js').read().replace('  var ret = run();', '  var ret = run(["1"]);')
  #)
  if not ok:
    print "EMSCRIPTEN BUG"
    notes['embug'] += 1
    fails += 1
    shutil.copyfile(filename + '.c', 'newfail%d.c' % fails)
    continue
  #if not ok:
  #  try: # finally, try with safe heap. if that is triggered, this is nonportable code almost certainly
  #    try_js(['-s', 'SAFE_HEAP=1'])
  #  except Exception, e:
  #    print e
  #    js = shared.run_js(filename + '.js', stderr=PIPE, full_output=True)
  #  print js
  #  if 'SAFE_HEAP' in js:
  #    notes['safeheap'] += 1
  #  else:
  #    break

  # This is ok. Try in secondary JS engine too
  if engine2 and normal:
    try:
      js2 = shared.run_js(filename + '.js', stderr=PIPE, engine=engine2, full_output=True, check_timeout=True)
    except:
      print 'failed to run in secondary', js2
      break

    # asm.js testing
    if 'warning: Successfully compiled asm.js code' not in js2:
      print "ODIN VALIDATION BUG"
      notes['embug'] += 1
      fails += 1
      shutil.copyfile(filename + '.c', 'newfail%d.c' % fails)
      continue

    js2 = js2.replace('\nwarning: Successfully compiled asm.js code\n', '')

    assert js2 == correct1 or js2 == correct2, ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(correct1.split('\n'), js2.split('\n'), fromfile='expected', tofile='actual')]) + 'ODIN FAIL'
    print 'odin ok'

