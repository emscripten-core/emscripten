'''
Takes the output from a JS test, and creates a file from it. So if
you do JSON.stringify() of an array in JS, this script will take
that and make a proper file out of it
'''

import os, sys, re

data = open(sys.argv[1], 'r').read()
m = re.search('\[[\d, -]*\]', data)
data = eval(m.group(0))
data = [x&0xff for x in data]
string = ''.join([chr(item) for item in data])
out = open(sys.argv[1]+'.' + (sys.argv[2] if len(sys.argv) >= 3 else 'raw'), 'wb')
print data[0:80]
print string[0:80]
out.write(string)
out.close()

