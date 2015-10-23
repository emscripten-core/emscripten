# Run, then execute
'''
dump(JSON.stringify(usedFunctions))
'''
# then strip with profile_strip.py

import sys

print 'var usedFunctions = {};'

for line in open(sys.argv[1]).readlines():
  line = line.strip()
  print line
  if line.startswith('function _') and line.endswith(('){', ') {')):
    name = line.split(' ')[1].split('(')[0]
    print 'usedFunctions["%s"] = 1;' % name

