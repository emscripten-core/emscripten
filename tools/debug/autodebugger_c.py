# Copyright 2017 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Processes a C source file, adding debugging information.

Similar to autodebugger.py, but runs on .c files. Will
overwrite the files it is given!
'''

from __future__ import print_function
import os, sys, re

filenames = sys.argv[1:]
for filename in filenames:
  print('..%s..' % filename)

  f = open(filename, 'r')
  data = f.read()
  f.close()

  lines = data.split('\n')
  for i in range(len(lines)):
    m = re.match('^  [ ]*([\w\d \[\]]+) = +([^;]+);$', lines[i])
    if m and (' if ' not in lines[i-1] or '{' in lines[i-1]) and \
             (' if ' not in lines[i+1] or '{' in lines[i+1]) and \
             (' else' not in lines[i-1] or '{' in lines[i-1]) and \
             (' else' not in lines[i+1] or '{' in lines[i+1]) and \
             (' for' not in lines[i-1]) and \
             ('struct' not in lines[i]):
      raw = m.groups(1)[0].rstrip()
      var = raw.split(' ')[-1]
      if ' ' in raw and '[' in var: continue
      lines[i] += ''' printf("%s:%d:%s=%%d\\n", (int)%s);''' % (filename, i+1, var, var)

  f = open(filename, 'w')
  f.write('\n'.join(lines))
  f.close()

print('Success.')

