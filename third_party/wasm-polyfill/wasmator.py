'''
Receives an emscripten output, and integrates the WebAssembly polyfill

XXX THIS IS A PROTOTYPE XXX The wasm format is changing, and also this tool has not yet been fully tested

See browser.test_wasm_polyfill_prototype for a concrete example. Basically, usage can look like this:

  emcc main.cpp -O2 -o test.html
  third_party/wasm-polyfill/wasmator.py test.js test.wasm

That builds test.html, then wasm-ifies it, modifying test.js with code using the polyfill, and creating
test.wasm that contains the wasmified asm.js.
'''

import os, sys, shutil, json
from subprocess import Popen, PIPE, STDOUT, check_call

PYTHON = sys.executable

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

polyfill_dir = os.path.abspath(os.path.dirname(__file__))
def path_in_polyfill(*pathelems):
  return os.path.join(polyfill_dir, *pathelems)

sys.path += [path_from_root('tools')]
import shared as emscripten

jsfile = sys.argv[1]
wasmfile = sys.argv[2]
export_name = sys.argv[3]

tempfile = jsfile + '.temp.js'
tempfile2 = jsfile + '.temp2.js'

print 'build executables'
dir = os.getcwd()
try:
  os.chdir(path_from_root('third_party', 'wasm-polyfill'))
  #proc = check_call([PYTHON, path_from_root('emmake'), 'make', 'clean'])
  proc = check_call([PYTHON, path_from_root('emmake'), 'make'])
finally:
  os.chdir(dir)

print 'save the before js'
emscripten.safe_copy(jsfile, 'before.js')

print 'process input'
check_call([PYTHON, path_from_root('tools', 'distill_asm.py'), jsfile, tempfile])

module = open(tempfile).read()
if 'use asm' not in module:
  if 'almost asm' in module:
    print >> sys.stderr, 'cannot use WASM=1 when full asm.js validation was disabled (make sure to run in at least -O1, and look for warnings about other options that might force asm.js off)'
  else:
    print >> sys.stderr, 'cannot use WASM=1, no asm.js module to wasm-ify'
  sys.exit(1)
start = module.index('function')
end = module.rindex(')')
asm = module[start:end]
open(tempfile2, 'w').write(asm)

print 'run polyfill packer on', tempfile2
dir = os.getcwd()
try:
  os.chdir(os.path.dirname(os.path.abspath(tempfile2)))
  # try in spidermonkey first, because it's faster
  try:
    out = emscripten.run_js(path_in_polyfill('tools', 'pack-asmjs.js'), args=[os.path.basename(tempfile2), 'output.binary'], stdout=PIPE, engine=emscripten.SPIDERMONKEY_ENGINE)
  except:
    out = emscripten.run_js(path_in_polyfill('tools', 'pack-asmjs.js'), args=[os.path.basename(tempfile2), 'output.binary'], stdout=PIPE)
finally:
  os.chdir(dir)
out = json.loads(out)
open(wasmfile, 'w').write(''.join(map(chr, out)))

print 'create patched out js'
js = open(jsfile).read()
patched = js.replace(asm, 'unwasmed') # we assume the module is right there
assert patched != js
patched = '''
function runEmscriptenModule(Module, unwasmed_) {
  var unwasmed = unwasmed_;
  arguments = undefined; // emscripten shell code looks at arguments, which it uses as commandline args

''' + patched + '''

  return Module;
}

''' + open(path_in_polyfill('jslib', 'load-wasm.js')).read() + '''

// note: web worker messages must be queued, as we delay the scripts chance to listen to them

var ''' + export_name + ''';

loadWebAssembly("''' + wasmfile + '''", 'load-wasm-worker.js').then(function(unwasmed) {
  if (typeof importScripts === 'function') {
    onmessage = null;
  }
  ''' + export_name + ''' = runEmscriptenModule(typeof Module !== 'undefined' ? Module : {}, unwasmed);
  if (typeof importScripts === 'function' && onmessage) {
    queuedMessages.forEach(function(e) {
      onmessage(e);
    });
  }
});

if (typeof importScripts === 'function') {
  var queuedMessages = [];
  onmessage = function(e) {
    queuedMessages.push(e);
  };
}
'''
open(jsfile, 'w').write(patched)
shutil.copyfile(path_in_polyfill('jslib', 'load-wasm-worker.js'), 'load-wasm-worker.js')

