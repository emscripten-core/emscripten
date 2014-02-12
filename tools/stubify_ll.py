
# given a file of ll and a list of names to retain, makes all other functions into stubs.
# you can then build with -s LINKABLE=1 and see any compilation errors on the specific functions

import os, sys

kill = False

valids = sys.argv[2].split(',')

for line in open(sys.argv[1]).readlines():
  line = line.replace('\n', '')
  if line.startswith('define ') and line.endswith('{'):
    ok = False
    for valid in valids:
      if valid in line: ok = True
    if not ok:
      line = line.replace('define ', 'declare ').replace(' internal ', ' ').replace(' weak ', ' ')[:-1]
      kill = True
    else:
      line = line.replace(' internal ', ' ')
    print line
  else:
    if not kill:
      print line
    else:
      if line == '}': kill = False

