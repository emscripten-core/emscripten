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
sdata = map(lambda x: str(ord(x)), data)
json = '[' + ','.join(sdata) + ']'

print 'var ' + sys.argv[2] + '=' + json + ';'

