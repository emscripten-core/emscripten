#!/usr/bin/python

'''
Runs csmith, a C fuzzer, and looks for bugs
'''

import os, sys, difflib
from subprocess import Popen, PIPE, STDOUT

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'tools')]
import shared

engine1 = eval('shared.' + sys.argv[1]) if len(sys.argv) > 1 else shared.JS_ENGINES[0]
engine2 = eval('shared.' + sys.argv[2]) if len(sys.argv) > 2 else None

print 'testing js engines', engine1, engine2

CSMITH = os.path.expanduser('~/Dev/csmith/src/csmith')
CSMITH_CFLAGS = ['-I' + os.path.expanduser('~/Dev/csmith/runtime/')]

filename = os.path.join(shared.CANONICAL_TEMP_DIR, 'fuzzcode')

shared.DEFAULT_TIMEOUT = 1

tried = 0

notes = { 'invalid': 0, 'unaligned': 0, 'embug': 0 }

while 1:
  print 'Tried %d, notes: %s' % (tried, notes)
  tried += 1
  print '1) Generate C'
  shared.execute([CSMITH, '--no-volatiles', '--no-math64'], stdout=open(filename + '.c', 'w'))

  print '2) Compile natively'
  shared.try_delete(filename)
  shared.execute([shared.CLANG_CC, '-O2', filename + '.c', '-o', filename] + CSMITH_CFLAGS, stderr=PIPE)
  assert os.path.exists(filename)
  print '3) Run natively'
  try:
    correct = shared.timeout_run(Popen([filename], stdout=PIPE, stderr=PIPE), 3)
  except Exception, e:
    print 'Failed or infinite looping in native, skipping', e
    notes['invalid'] += 1
    continue

  print '4) Compile JS-ly and compare'

  def try_js(args):
    shared.try_delete(filename + '.js')
    print '(compile)'
    shared.execute([shared.EMCC, '-O2', '-s', 'ASM_JS=1', '-s', 'PRECISE_I64_MATH=1', '-s', 'PRECISE_I32_MUL=1', filename + '.c', '-o', filename + '.js'] + CSMITH_CFLAGS + args, stderr=PIPE)
    assert os.path.exists(filename + '.js')
    print '(run)'
    js = shared.run_js(filename + '.js', stderr=PIPE, engine=engine1, check_timeout=True)
    assert correct == js, ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(correct.split('\n'), js.split('\n'), fromfile='expected', tofile='actual')])

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
  if not ok:
    print "EMSCRIPTEN BUG"
    notes['embug'] += 1
    continue #break
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
    assert 'warning: Successfully compiled asm.js code' in js2, 'must validate'
    js2 = js2.replace('\nwarning: Successfully compiled asm.js code\n', '')

    assert js2 == correct, ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(correct.split('\n'), js2.split('\n'), fromfile='expected', tofile='actual')]) + 'ODIN FAIL'
    print 'odin ok'


