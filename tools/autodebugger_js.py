'''
Processes a C source file, adding debugging information.

Similar to autodebugger.py, but runs on .js files.
'''

import os, sys, re

filename = sys.argv[1]
func = sys.argv[2]

f = open(filename, 'r')
data = f.read()
f.close()

lines = data.split('\n')
in_func = False
for i in range(len(lines)):
  if lines[i].startswith('function ') and '}' not in lines[i]:
    name = lines[i].split('(')[0].split(' ')[1]
    args = lines[i].split('(')[1].split(')')[0]
    lines[i] += ' print("call %s(" + [%s] + ")");' % (name, args)
  if lines[i].startswith('function ' + func + '('):
    in_func = True
    continue
  elif lines[i].startswith('}'):
    in_func = False
    continue
  if in_func:
    m = re.match('^ +([$_\w\d \[\]]+) = +([^;]+);$', lines[i])
    if m and (' if ' not in lines[i-1] or '{' in lines[i-1]) and \
             (' if ' not in lines[i+1] or '{' in lines[i+1]) and \
             (' else' not in lines[i-1] or '{' in lines[i-1]) and \
             (' else' not in lines[i+1] or '{' in lines[i+1]):
      var = m.groups(1)[0].rstrip().split(' ')[-1]
      if 'STACKTOP' not in lines[i] and 'sp' not in lines[i]:
        #lines[i] += ''' print("[%4d] %s = " + %s);''' % (i+1, var, var)
        lines[i] += ''' print("%s = " + %s);''' % (var, var)
    m = re.match('^ +HEAP.*$', lines[i])
    if m and lines[i].count(' = ') == 1:
      left, right = lines[i].split(' = ')
      lines[i] += ''' print("%s = " + %s);''' % (left, left)

print '\n'.join(lines)

print >> sys.stderr, 'Success.'

