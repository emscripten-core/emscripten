'''
Processes a C source file, adding debugging information.

Similar to autodebugger.py, but runs on .c files. Will
overwrite the files it is given!
'''

import os, sys, re

filenames = sys.argv[1:]
for filename in filenames:
  print '..%s..' % filename

  f = open(filename, 'r')
  data = f.read()
  f.close()

  lines = data.split('\n')
  for i in range(len(lines)):
    m = re.match('^  [ ]*([\w\d \[\]]+) = +([^;]+);$', lines[i])
    if m and (' if ' not in lines[i-1] or '{' in lines[i-1]) and \
             (' if ' not in lines[i+1] or '{' in lines[i+1]) and \
             (' else' not in lines[i-1] or '{' in lines[i-1]) and \
             (' else' not in lines[i+1] or '{' in lines[i+1]):
      var = m.groups(1)[0].rstrip().split(' ')[-1]
      lines[i] += ''' printf("%s:%d:%s=%%d\\n", %s);''' % (filename, i+1, var, var)

  f = open(filename, 'w')
  f.write('\n'.join(lines))
  f.close()

print 'Success.'

