'''
Simple tool to find big functions in an .ll file. Anything over i64 is of interest.
'''

import os, sys, re

filename = sys.argv[1]
i = 0
maxx = -1
maxxest = '?'
start = -1
curr = '?'
for line in open(filename):
  i += 1
  if line.startswith('function '):
    start = i
    curr = line
  elif line.startswith('}'):
    size = i - start
    if size > maxx:
      maxx = size
      maxxest = curr
print maxx, 'lines in', maxxest
