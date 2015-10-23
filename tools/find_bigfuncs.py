'''
Simple tool to find big functions in a js or ll file
'''

import os, sys, re

filename = sys.argv[1]
i = 0
start = -1
curr = None
data = []
for line in open(filename):
  i += 1
  if line.startswith(('function ', 'define ')) and '}' not in line:
    start = i
    curr = line
  elif line.startswith('}') and curr:
    size = i - start
    data.append([curr, size])
    curr = None
data.sort(lambda x, y: x[1] - y[1])
print ''.join(['%6d : %s' % (x[1], x[0]) for x in data])

