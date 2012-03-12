'''
Finds why an .ll file is large by printing functions by size
'''

import os, sys

funcs = []
i = 0
for line in open(sys.argv[1]):
  i += 1
  if line.startswith('define '):
    inside = line.replace('define ', '').replace('\n', '')
    start = i
  elif line.startswith('}'):
    funcs.append((inside, i-start))

print '\n'.join(map(lambda func: str(func[1]) + ':' + func[0], sorted(funcs, key=lambda func: -func[1])))

