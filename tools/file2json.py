'''
Converts a binary file into JSON.

This lets you transform a binary file into something you can
easily bundle inside a web page.

Usage: file2json.py FILENAME VARNAME

FILENAME - the binary file
VARNAME - the variable to store it in (the output will be VARNAME = [...])
'''

import os, sys

data = open(sys.argv[1], 'rb').read()
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

'''
or (but this fails, we get a string at runtime?)

data = open(sys.argv[1], 'rb').read()
counter = 0
print '[',
for i in range(len(data)):
  last = i == len(data)-1
  print ord(data[i]),
  counter += 1
  if counter % 20 == 0:
    print
  if counter % 1005 == 0 and not last:
    print '] + [',
  elif not last: print ',',

print ']'
'''

