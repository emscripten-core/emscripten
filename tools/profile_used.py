# Run, then execute
'''
dump(JSON.stringify(usedFunctions))
'''
# then strip with profile_strip.py

import sys

print 'var usedFunctions = {};'
#print "function getFuncName(f) { return f.toString().split(' ')[1].split('(')[0] }"
print "function getFuncName(f) { return f.name }"

last = []

for line in open(sys.argv[1]).readlines():
  line = line.strip()
  print line
  if line.startswith('function _') and line.endswith(('){', ') {')):
    name = line.split(' ')[1].split('(')[0]
    print 'usedFunctions["%s"] = 1;' % name
  if line.startswith('var FUNCTION_TABLE'):
    name = line.split(' ')[1].split('=')[0]
    last += ['for (var i = 0; i < %s.length; i++) if (typeof %s[i] === "function") usedFunctions[getFuncName(%s[i])] = 1;' % (name, name, name)]
  if len(last) > 0 and line.endswith(';'):
    for l in last:
      print l
    last = []

