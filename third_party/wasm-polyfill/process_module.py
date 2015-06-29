'''
Receives an emscripten output, and integrates the polyfill
'''

import os, sys, shutil
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
wasmfile = sys.argv[3]

tempfile = infile + '.temp.js'
tempfile2 = infile + '.temp2.js'

print 'process input'
Popen([PYTHON, path_from_root('tools', 'distill_asm.py'), infile, tempfile]).communicate()
module = open(tempfile).read()
start = module.index('function')
end = module.rindex(')')
asm = module[start:end]
open(tempfile2, 'w').write(asm)

print 'run polyfill packer'
Popen([path_in_polyfill('tools', 'pack-asmjs'), tempfile2, wasmfile]).communicate()

print 'create patched out js'
js = open(infile).read()
patched = js.replace(asm, 'unwasmed') # we assume the module is right there
assert patched != js
patched = '''
function runEmscriptenModule(unwasmed) {

''' + patched + '''

}

''' + open(path_in_polyfill('jslib', 'load-wasm.js')).read() + '''

loadWebAssembly("''' + wasmfile + '''", 'load-wasm-worker.js').then(runEmscriptenModule);
'''
open(outfile, 'w').write(patched)
shutil.copyfile(path_in_polyfill('jslib', 'load-wasm-worker.js'), 'load-wasm-worker.js')

