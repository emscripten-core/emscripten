#!/usr/bin/env python2

'''
simple tool to run emcc and clang on C testcases each in a separate subdir of the current dir
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
  print '(' + d + ') ',
  os.chdir(curr)
  if os.path.isdir(d):
    os.chdir(d)
    for c in os.listdir('.'):
      if c.endswith('.c'):
        execute([EMCC, c, '-O2', '--embed-file', 'input.txt'])
        js = jsrun.run_js('a.out.js', filter(lambda x: x != '-w', SPIDERMONKEY_ENGINE), stdout=PIPE)

        execute([CLANG_CC, '-m32', c])
        n1 = execute(['./a.out'], stdout=PIPE)[0]

        execute([CLANG_CC, '-m32', c, '-O2'])
        n2 = execute(['./a.out'], stdout=PIPE)[0]

        execute(['gcc', c, '-m32'])
        n3 = execute(['./a.out'], stdout=PIPE)[0]

        if js == n1:
          print 'ok'
        elif js == n2:
          print 'emcc and clang -O2 both equally wrong'
        elif js == n3:
          print 'emcc agrees with gcc, so probably ok'
        else:
          print
          print 'js  ', js,
          print 'c0  ', n1,
          print 'c2  ', n2,
          print 'g   ', n3,
          print 'fail!!!', d

