'''
Receives an emscripten output, and integrates the polyfill
'''

import os, sys
from subprocess import Popen, PIPE, STDOUT

PYTHON = sys.executable

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

polyfill_dir = os.path.abspath(os.path.dirname(__file__))
def path_in_polyfill(*pathelems):
  return os.path.join(polyfill_dir, *pathelems)

infile = sys.argv[1]
outfile = sys.argv[2]

tempfile = infile + '.temp.js'
tempfile2 = infile + '.temp2.js'

print 'process input'
Popen([PYTHON, path_from_root('tools', 'distill_asm.py'), infile, tempfile]).communicate()
module = open(tempfile).read()
start = module.index('function')
end = module.rindex(')')
open(tempfile2, 'w').write(module[start:end])

print 'run polyfill packer'
Popen([path_in_polyfill('tools', 'pack-asmjs'), tempfile2, outfile]).communicate()

