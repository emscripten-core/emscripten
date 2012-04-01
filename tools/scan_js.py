'''
Finds why a .js file is large by printing functions by size
'''

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

print '\n'.join(map(lambda func: str(func[1]) + ':' + func[0], sorted(funcs, key=lambda func: -func[1])))

