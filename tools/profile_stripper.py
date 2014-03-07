# See profile_used.py
#
# profile file, js file

import sys, json

used = json.loads(open(sys.argv[1]).read())

show = True

for orig in open(sys.argv[2]).readlines():
  line = orig.strip()
  if orig.startswith('function _') and line.endswith(('){', ') {')):
    name = line.split(' ')[1].split('(')[0]
    if name.startswith('_') and not used.get(name):
      #print >> sys.stderr, 'remove', name
      show = False
  if show: print orig,
  if orig.startswith('}'):
    show = True

