'''
Autodebugger output contains -1 for function entry and -2 for function exit.
This script will indent the output nicely
'''

import os, sys

lines = sys.stdin.read().split('\n')

depth = 0
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('AD:-2,'):
    depth -= 1
  lines[i] = ('  '*depth) + line
  if line.startswith('AD:-1,'):
    depth += 1

print '\n'.join(lines)

