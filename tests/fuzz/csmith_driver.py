#!/usr/bin/python

'''
Runs csmith, a C fuzzer, and looks for bugs
'''

import os, sys, difflib
from subprocess import Popen, PIPE, STDOUT

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'tools')]
import shared

CSMITH = os.path.expanduser('~/Dev/csmith/src/csmith')
CSMITH_CFLAGS = ['-I' + os.path.expanduser('~/Dev/csmith/runtime/')]

filename = os.path.join(shared.CANONICAL_TEMP_DIR, 'fuzzcode')

shared.DEFAULT_TIMEOUT = 3

tried = 0

notes = { 'invalid': 0, 'unaligned': 0 }

while 1:
  print 'Tried %d, notes: %s' % (tried, notes)
  tried += 1
  print '1) Generate C'
  shared.execute([CSMITH, '--no-volatiles', '--no-math64', '--max-block-depth', '2', '--max-block-size', '2', '--max-expr-complexity', '2', '--max-funcs', '2'], stdout=open(filename + '.c', 'w'))

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
    shared.execute([shared.EMCC, '-O2', '-s', 'ASM_JS=1', '-s', 'PRECISE_I64_MATH=1', '-s', 'PRECISE_I32_MUL=1', filename + '.c', '-o', filename + '.js'] + CSMITH_CFLAGS, stderr=PIPE)
    assert os.path.exists(filename + '.js')
    print '5) Run JS-ly'
    js = shared.run_js(filename + '.js', stderr=PIPE) #, engine=...)
    assert correct == js, ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(correct.split('\n'), js.split('\n'), fromfile='expected', tofile='actual')])

  # Try normally, then try unaligned because csmith does generate nonportable code that requires x86 alignment
  ok = False
  for args, note in [([], None), (['-s', 'UNALIGNED_MEMORY=1'], 'unaligned')]:
    try:
      try_js(args)
      ok = True
      if note:
        notes[note] += 1
      break
    except Exception, e:
      print e
  if not ok: break

