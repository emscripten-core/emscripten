#!/usr/bin/env python2

'''
simple tool to run emcc and clang on testcases each in a separate subdir, as in the case of output from Moh's fuzzer
'''

import os, sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
import tools.shared
from tools.shared import *

curr = os.getcwd()

for d in os.listdir(curr):
  #print d
  os.chdir(curr)
  if os.path.isdir(d):
    os.chdir(d)
    for c in os.listdir('.'):
      if c.endswith('.c'):
        execute([CLANG_CC, c])
        out1 = execute(['./a.out'], stdout=PIPE)[0]
        execute([EMCC, c, '-O2', '--embed-file', 'input.txt'])
        out2 = jsrun.run_js('a.out.js', filter(lambda x: x != '-w', SPIDERMONKEY_ENGINE), stdout=PIPE)
        if out1 != out2:
          print '  ', out1,
          print '  ', out2,
          print 'fail', d

