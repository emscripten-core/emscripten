'''
Simple tool to find big functions in an .ll file.
'''

import os, sys, re

filename = sys.argv[1]
i = 0
start = -1
curr = '?'
data = []
for line in open(filename):
  i += 1
  if line.startswith('function '):
    start = i
    curr = line
  elif line.startswith('}'):
    size = i - start
    data.append([curr, size]);
data.sort(lambda x, y: x[1] - y[1])
print ''.join(['%6d : %s' % (x[1], x[0]) for x in data])

