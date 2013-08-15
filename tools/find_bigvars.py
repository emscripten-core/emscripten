'''
Simple tool to find functions with lots of vars.
'''

import os, sys, re

filename = sys.argv[1]
i = 0
curr = None
data = []
size = 0
for line in open(filename):
  i += 1
  if line.startswith('function '):
    size = len(line.split(',')) # params
    curr = line
  elif line.strip().startswith('var '):
    size += len(line.split(',')) + 1 # vars
  elif line.startswith('}') and curr:
    data.append([curr, size])
    curr = None
data.sort(lambda x, y: x[1] - y[1])
print ''.join(['%6d : %s' % (x[1], x[0]) for x in data])

