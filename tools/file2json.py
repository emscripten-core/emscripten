'''
Converts a binary file into JSON.

This lets you transform a binary file into something you can
easily bundle inside a web page.

Usage: file2json.py FILENAME VARNAME

FILENAME - the binary file
VARNAME - the variable to store it in (the output will be VARNAME = [...])
'''

import os, sys

data = open(sys.argv[1], 'r').read()
sdata = map(lambda x: str(ord(x)) + ',', data)
sdata[-1] = sdata[-1].replace(',', '')
lined = []
while len(sdata) > 0:
  lined += sdata[:30]
  sdata = sdata[30:]
  if len(sdata) > 0:
    lined += ['\n']
json = '[' + ''.join(lined) + ']'

if len(sys.argv) < 3:
  print json
else:
  print 'var ' + sys.argv[2] + '=' + json + ';'

