# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
A semi-smart diff for autodebugger logs.

Run it with filenames of two autodebugger logs as parameters
'''

from __future__ import print_function
import os, sys


def process_line(line):
  #AD:2041,0.900000
  if not line.startswith('AD:'): return
  line = line.split(':')[1]
  num, val = line.split(',')
  return [int(num), float(val)]

a = open(sys.argv[1], 'r').readlines()
b = open(sys.argv[2], 'r').readlines()
MIN = 0.0001 if len(sys.argv) < 4 else sys.argv[3]

ai = 0
bi = 0
maxx = max(len(a), len(b))

while max(ai, bi) < maxx:
  while 1:
    av = process_line(a[ai])
    if av: break
    ai += 1
  while 1:
    bv = process_line(b[bi])
    if bv: break
    bi += 1
  #print 'curr:', ai, bi, av, bv
  # Find the nearest matching num, if not already matched
  if not av[0] == bv[0]:
    tai = ai+1
    tbi = bi+1
    while 1:
      tav = process_line(a[tai])
      tbv = process_line(b[tbi])
      #print 'seek:', tai, tbi, tav, tbv
      if tav and tav[0] == bv[0]:
        ai = tai
        av = tav
        break
      elif tbv and tbv[0] == av[0]:
        bi = tbi
        bv = tbv
        break
      tai += 1
      tbi += 1

  assert av[0] == bv[0]
  diff = abs(av[1] - bv[1])
  if diff > MIN:
    print('<<%d %d>>  %d : %.5f' % (ai, bi, av[0], diff))
  ai += 1
  bi += 1

