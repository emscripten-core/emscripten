'''
Simple tool to find big i types in an .ll file. Anything over i64 is of interest.
'''

import os, sys, re

filename = sys.argv[1]
data = open(filename).read()
iss = re.findall(' i\d+ [^=]', data)
set_iss = set(iss)
bigs = []
for iss in set_iss:
  size = int(iss[2:-2])
  if size > 64:
    bigs.append(size)
bigs.sort()
print bigs

