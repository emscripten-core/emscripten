#!/usr/bin/python

'''
Usage: creduce ./creduce_tester.py newfail1.c
'''

import os, sys
from subprocess import Popen, PIPE, STDOUT

sys.path += [os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'tools')]
import shared, jsrun

# creduce will only pass the filename of the C file as the first arg, so other
# configuration options will have to be hardcoded.
CSMITH_CFLAGS = ['-I', os.path.join(os.environ['CSMITH_PATH'], 'runtime')]
ENGINE = shared.JS_ENGINES[0]
EMCC_ARGS = ['-O2', '-s', 'ASM_JS=1', '-s', 'PRECISE_I64_MATH=1']

filename = sys.argv[1]
obj_filename = os.path.splitext(filename)[0]
js_filename = obj_filename + '.js'
print 'testing file', filename

try:
  print '2) Compile natively'
  shared.check_execute([shared.CLANG_CC, '-O2', filename, '-o', obj_filename] + CSMITH_CFLAGS)
  print '3) Run natively'
  correct = jsrun.timeout_run(Popen([obj_filename], stdout=PIPE, stderr=PIPE), 3)
except Exception, e:
  print 'Failed or infinite looping in native, skipping', e
  sys.exit(1) # boring

print '4) Compile JS-ly and compare'

def try_js(args):
  shared.check_execute([shared.PYTHON, shared.EMCC] + EMCC_ARGS + CSMITH_CFLAGS + args +
    [filename, '-o', js_filename])
  js = shared.run_js(js_filename, stderr=PIPE, engine=ENGINE)
  assert correct == js

# Try normally, then try unaligned because csmith does generate nonportable code that requires x86 alignment
# If you are sure that alignment is not the cause, disable it for a faster reduction
for args in [[]]:
  try:
    try_js(args)
    break
  except Exception, e:
    pass
else:
  sys.exit(0)

sys.exit(1) # boring
