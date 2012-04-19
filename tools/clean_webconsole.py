'''
Removes timestamp and line info from a webgl log
'''

import os, sys

lines = sys.stdin.read().split('\n')

for line in lines:
  if line.startswith('['):
    line = line[15:]
  line = line.split(' @ ')[0]
  print line

for i in range(100):
  print

