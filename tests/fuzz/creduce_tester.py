#!/usr/bin/python

'''
Runs csmith, a C fuzzer, and looks for bugs
'''

import os, sys, difflib
from subprocess import Popen, PIPE, STDOUT

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'tools')]
import shared

filename = sys.argv[1]
print 'testing file', filename

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
  os.exit(0) # boring 

print '4) Compile JS-ly and compare'

def try_js(args):
  shared.try_delete(filename + '.js')
  shared.execute([shared.EMCC, '-O2', '-s', 'ASM_JS=1', '-s', 'PRECISE_I64_MATH=1', '-s', 'PRECISE_I32_MUL=1', filename + '.c', '-o', filename + '.js'] + CSMITH_CFLAGS + args, stderr=PIPE)
  assert os.path.exists(filename + '.js')
  js = shared.run_js(filename + '.js', stderr=PIPE, engine=engine1)
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
if not ok: sys.exit(1)

sys.exit(0) # boring

