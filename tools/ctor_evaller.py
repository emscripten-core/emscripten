'''
Tries to evaluate global constructors, applying their effects ahead of time.

This is an LTO-like operation, and to avoid parsing the entire tree, we operate on the text in python
'''

import os, sys, json, subprocess
import shared, js_optimizer

js_file = sys.argv[1]
mem_init_file = sys.argv[2]
total_memory = int(sys.argv[3])
global_base = int(sys.argv[4])

assert global_base > 0

temp_file = js_file + '.ctorEval.js'
#temp_file = '/tmp/emscripten_temp/ctorEval.js'

# helpers

def get_asm(js):
  return js[js.find(js_optimizer.start_asm_marker):js.find(js_optimizer.end_asm_marker)]

def eval_ctor(js, mem_init):

  def kill_func(asm, name):
    before = len(asm)
    asm = asm.replace('function ' + name + '(', 'function KILLED_' + name + '(', 1)
    assert len(asm) > before
    return asm

  def add_func(asm, func):
    before = len(asm)
    asm = asm.replace('function ', ' ' + func + '\nfunction ', 1)
    assert len(asm) > before
    name = func[func.find(' ')+1 : func.find('(')]
    asm = asm.replace('return {', 'return { ' + name + ': ' + name + ',')
    return asm

  # Find the global ctors
  ctors_start = js.find('__ATINIT__.push(')
  if ctors_start < 0: return False
  ctors_end = js.find(');', ctors_start)
  if ctors_end < 0: return False
  ctors_end += 3
  ctors_text = js[ctors_start:ctors_end]
  ctors = filter(lambda ctor: ctor.endswith('()') and not ctor == 'function()' and '.' not in ctor, ctors_text.split(' '))
  if len(ctors) == 0: return False
  ctor = ctors[0].replace('()', '')
  shared.logging.debug('trying to eval ctor: ' + ctor)
  # Find the asm module, and receive the mem init.
  asm = get_asm(js)
  assert len(asm) > 0
  asm = asm.replace('use asm', 'not asm') # don't try to validate this
  # find all global vars, and provide only safe ones TODO. Also add dumping for those.
  pre_funcs_start = asm.find(';') + 1
  pre_funcs_end = asm.find('function ', pre_funcs_start)
  pre_funcs_end = asm.rfind(';', pre_funcs_start, pre_funcs_end) + 1
  pre_funcs = asm[pre_funcs_start:pre_funcs_end]
  parts = filter(lambda x: x.startswith('var ') and ' new ' not in x, map(lambda x: x.strip(), pre_funcs.split(';')))
  global_vars = []
  for part in parts:
    part = part[4:] # skip 'var '
    bits = map(lambda x: x.replace(' ', ''), part.split(','))
    for bit in bits:
      name, value = bit.split('=')
      if value in ['0', '+0', '0.0'] or name in []: # ['STACKTOP', 'STATIC_MAX', 'DYNAMICTOP']
        global_vars.append(name)
  asm = add_func(asm, 'function dumpGlobals() { return [ ' + ', '.join(global_vars) + '] }')
  # find static bump. this is the maximum area we'll write to during startup.
  static_bump_op = 'STATICTOP = STATIC_BASE + '
  static_bump_start = js.find(static_bump_op)
  static_bump_end = js.find(';', static_bump_start)
  static_bump = int(js[static_bump_start + len(static_bump_op):static_bump_end])
  # Generate a safe sandboxed environment. We replace all ffis with errors. Otherwise,
  # asm.js can't call outside, so we are ok.
  open(temp_file, 'w').write('''
var totalMemory = %s;

var buffer = new ArrayBuffer(totalMemory);
var heap = new Uint8Array(buffer);

var memInit = %s;

var globalBase = %d;
var staticBump = %d;

heap.set(memInit, globalBase);

var stacktop = globalBase + staticBump;
while (stacktop %% 16 !== 0) stacktop++;
var stackMax = totalMemory;
var dynamicTop = stackMax;

if (!Math.imul) {
  Math.imul = Math.imul || function(a, b) {
    var ah = (a >>> 16) & 0xffff;
    var al = a & 0xffff;
    var bh = (b >>> 16) & 0xffff;
    var bl = b & 0xffff;
    // the shift by 0 fixes the sign on the high part
    // the final |0 converts the unsigned value into a signed value
    return ((al * bl) + (((ah * bl + al * bh) << 16) >>> 0)|0);
  };
}

var globalArg = {
  Int8Array: Int8Array,
  Int16Array: Int16Array,
  Int32Array: Int32Array,
  Uint8Array: Uint8Array,
  Uint16Array: Uint16Array,
  Uint32Array: Uint32Array,
  Float32Array: Float32Array,
  Float64Array: Float64Array,
  NaN: NaN,
  Infinity: Infinity,
  Math: Math,
};

var libraryArg = {
  STACKTOP: stacktop,
  STACK_MAX: stackMax,
  DYNAMICTOP: dynamicTop,
};

// Instantiate asm
%s
(globalArg, libraryArg, buffer);

var globalsBefore = asm['dumpGlobals']();

// Try to run the constructor
asm['%s']();
// We succeeded!

// Verify asm global vars
var globalsAfter = asm['dumpGlobals']();

if (JSON.stringify(globalsBefore) !== JSON.stringify(globalsAfter)) throw 'globals changed ' + globalsBefore + ' vs ' + globalsAfter;

// Write out new mem init. It might be bigger, look for non-0 bytes
var newSize = globalBase + staticBump;
while (newSize > globalBase && heap[newSize-1] == 0) newSize--;
console.log(Array.prototype.slice.call(heap.subarray(globalBase, newSize)));

''' % (total_memory, mem_init, global_base, static_bump, asm, ctor))
  # Execute the sandboxed code. If an error happened due to calling an ffi, that's fine,
  # us exiting with an error tells the caller that we failed.
  proc = subprocess.Popen(shared.NODE_JS + [temp_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = proc.communicate()
  if proc.returncode != 0:
    shared.logging.debug('failed to eval ctor:\n' + err)
    return False
  # Success! out contains the new mem init, write it out
  mem_init = ''.join(map(chr, json.loads(out)))
  # Remove this ctor and write that out # TODO: remove from the asm export as well
  if len(ctors) == 1:
    new_ctors = ''
  else:
    new_ctors = ctors_text[:ctors_text.find('(') + 1] + ctors_text[ctors_text.find(',')+1:]
  js = js[:ctors_start] + new_ctors + js[ctors_end:]
  removed.append(ctor)
  return (js, mem_init)

# main

# keep running while we succeed in removing a constructor

removed = []

while True:
  shared.logging.debug('ctor_evaller: trying to eval a global constructor')
  js = open(js_file).read()
  if os.path.exists(mem_init_file):
    mem_init = json.dumps(map(ord, open(mem_init_file, 'rb').read()))
  else:
    mem_init = []
  result = eval_ctor(js, mem_init)
  if not result:
    shared.logging.debug('ctor_evaller: not successful any more, done')
    break # that's it, no more luck. either no ctors, or we failed to eval a ctor
  shared.logging.debug('ctor_evaller: success!')
  js, mem_init = result
  open(js_file, 'w').write(js)
  open(mem_init_file, 'wb').write(mem_init)

# If we removed one, dead function elimination can help us

if len(removed) > 0:
  shared.logging.debug('ctor_evaller: eliminate no longer needed functions after ctor elimination')
  # find exports
  asm = get_asm(open(js_file).read())
  exports_start = asm.find('return {')
  exports_end = asm.find('};', exports_start)
  exports_text = asm[asm.find('{', exports_start) + 1 : exports_end]
  exports = map(lambda x: x.split(':')[1].strip(), exports_text.replace(' ', '').split(','))
  for r in removed:
    assert r in exports, 'global ctors were exported'
  exports = filter(lambda e: e not in removed, exports)
  # fix up the exports
  js = open(js_file).read()
  absolute_exports_start = js.find(exports_text)
  js = js[:absolute_exports_start] + ', '.join(map(lambda e: e + ': ' + e, exports)) + js[absolute_exports_start + len(exports_text):]
  open(js_file, 'w').write(js)
  # find unreachable methods and remove them
  reachable = shared.Building.calculate_reachable_functions(js_file, exports, can_reach=False)['reachable']
  for r in removed:
    assert r not in reachable, 'removed ctors must NOT be reachable'
  shared.Building.js_optimizer(js_file, ['removeFuncs'], extra_info={ 'keep': reachable }, output_filename=js_file)

