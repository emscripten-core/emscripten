'''
Receives an emscripten output, and integrates the WebAssembly polyfill

XXX THIS IS A PROTOTYPE XXX The wasm format is changing, and also this tool has not yet been fully tested

See browser.test_wasm_polyfill_prototype for a concrete example. Basically, usage can look like this:

  emcc main.cpp -O2 -o test.html
  third_party/wasm-polyfill/wasmator.py test.js test.wasm

That builds test.html, then wasm-ifies it, modifying test.js with code using the polyfill, and creating
test.wasm that contains the wasmified asm.js.
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

jsfile = sys.argv[1]
wasmfile = sys.argv[2]

tempfile = jsfile + '.temp.js'
tempfile2 = jsfile + '.temp2.js'

print 'save the before js'
shutil.copyfile(jsfile, 'before.js')

print 'process input'
Popen([PYTHON, path_from_root('tools', 'distill_asm.py'), jsfile, tempfile]).communicate()
module = open(tempfile).read()
start = module.index('function')
end = module.rindex(')')
asm = module[start:end]
open(tempfile2, 'w').write(asm)

print 'run polyfill packer on', tempfile2
proc = Popen([path_in_polyfill('tools', 'pack-asmjs'), tempfile2, wasmfile])
proc.communicate()
if proc.returncode: sys.exit(proc.returncode)

print 'create patched out js'
js = open(jsfile).read()
patched = js.replace(asm, 'unwasmed') # we assume the module is right there
assert patched != js
patched = '''
function runEmscriptenModule(Module, unwasmed_) {
  var unwasmed = unwasmed_;
  arguments = undefined; // emscripten shell code looks at arguments, which it uses as commandline args

''' + patched + '''

}

''' + open(path_in_polyfill('jslib', 'load-wasm.js')).read() + '''

loadWebAssembly("''' + wasmfile + '''", 'load-wasm-worker.js').then(function(unwasmed) {
  runEmscriptenModule(Module, unwasmed);
});
'''
open(jsfile, 'w').write(patched)
shutil.copyfile(path_in_polyfill('jslib', 'load-wasm-worker.js'), 'load-wasm-worker.js')

