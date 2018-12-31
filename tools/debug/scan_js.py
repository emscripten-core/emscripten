'''
Finds why a .js file is large by printing functions by size
'''

from __future__ import print_function
import os, sys

funcs = []
i = 0
inside = None
for line in open(sys.argv[1]):
  i += 1
  if line.startswith('function _'):
    inside = line.replace('function ', '').replace('\n', '')
    start = i
  elif inside and line.startswith('}'):
    funcs.append((inside, i-start))
    inside = None

print('\n'.join([str(func[1]) + ':' + func[0] for func in sorted(funcs, key=lambda func: -func[1])]))

