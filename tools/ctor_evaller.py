'''
Tries to evaluate global constructors, applying their effects ahead of time.

This is an LTO-like operation, and to avoid parsing the entire tree (we might fail to parse a massive project, we operate on the text in python.
'''

import os, sys, json, subprocess
import shared, js_optimizer

js_file = sys.argv[1]
mem_init_file = sys.argv[2]
total_memory = int(sys.argv[3])
total_stack = int(sys.argv[4])
global_base = int(sys.argv[5])

assert global_base > 0

config = shared.Configuration()

if shared.DEBUG:
  temp_file = os.path.join(shared.CANONICAL_TEMP_DIR, 'ctorEval.js')
  shared.safe_ensure_dirs(shared.CANONICAL_TEMP_DIR)
else:
  temp_file = config.get_temp_files().get('.ctorEval.js').name

# helpers

def get_asm(js):
  return js[js.find(js_optimizer.start_asm_marker):js.find(js_optimizer.end_asm_marker)]

def find_ctors(js):
  ctors_start = js.find('__ATINIT__.push(')
  if ctors_start < 0:
    return (-1, -1)
  ctors_end = js.find(');', ctors_start)
  assert ctors_end > 0
  ctors_end += 3
  return (ctors_start, ctors_end)

def eval_ctors(js, mem_init, num):

  def kill_func(asm, name):
    before = len(asm)
    asm = asm.replace('function ' + name + '(', 'function KILLED_' + name + '(', 1)
    return asm

  def add_func(asm, func):
    before = len(asm)
    asm = asm.replace('function ', ' ' + func + '\nfunction ', 1)
    assert len(asm) > before
    name = func[func.find(' ')+1 : func.find('(')]
    asm = asm.replace('return {', 'return { ' + name + ': ' + name + ',')
    return asm

  # Find the global ctors
  ctors_start, ctors_end = find_ctors(js)
  assert ctors_start > 0
  ctors_text = js[ctors_start:ctors_end]
  all_ctors = filter(lambda ctor: ctor.endswith('()') and not ctor == 'function()' and '.' not in ctor, ctors_text.split(' '))
  all_ctors = map(lambda ctor: ctor.replace('()', ''), all_ctors)
  total_ctors = len(all_ctors)
  assert total_ctors > 0
  ctors = all_ctors[:num]
  shared.logging.debug('trying to eval ctors: ' + ', '.join(ctors))
  # Find the asm module, and receive the mem init.
  asm = get_asm(js)
  assert len(asm) > 0
  asm = asm.replace('use asm', 'not asm') # don't try to validate this
  # find all global vars, and provide only safe ones. Also add dumping for those.
  pre_funcs_start = asm.find(';') + 1
  pre_funcs_end = asm.find('function ', pre_funcs_start)
  pre_funcs_end = asm.rfind(';', pre_funcs_start, pre_funcs_end) + 1
  pre_funcs = asm[pre_funcs_start:pre_funcs_end]
  parts = filter(lambda x: x.startswith('var '), map(lambda x: x.strip(), pre_funcs.split(';')))
  global_vars = []
  new_globals = '\n'
  for part in parts:
    part = part[4:] # skip 'var '
    bits = map(lambda x: x.strip(), part.split(','))
    for bit in bits:
      name, value = map(lambda x: x.strip(), bit.split('='))
      if value in ['0', '+0', '0.0'] or name in [
        'STACKTOP', 'STACK_MAX', 'DYNAMICTOP',
        'HEAP8', 'HEAP16', 'HEAP32',
        'HEAPU8', 'HEAPU16', 'HEAPU32',
        'HEAPF32', 'HEAPF64',
        'Int8View', 'Int16View', 'Int32View', 'Uint8View', 'Uint16View', 'Uint32View', 'Float32View', 'Float64View',
        'nan', 'inf',
        '_emscripten_memcpy_big', '_sbrk', '___dso_handle',
        '_atexit', '___cxa_atexit',
      ] or name.startswith('Math_'):
        if 'new ' not in value:
          global_vars.append(name)
        new_globals += ' var ' + name + ' = ' + value + ';\n'
  asm = asm[:pre_funcs_start] + new_globals + asm[pre_funcs_end:]
  asm = add_func(asm, 'function dumpGlobals() { return [ ' + ', '.join(global_vars) + '] }')
  # find static bump. this is the maximum area we'll write to during startup.
  static_bump_op = 'STATICTOP = STATIC_BASE + '
  static_bump_start = js.find(static_bump_op)
  static_bump_end = js.find(';', static_bump_start)
  static_bump = int(js[static_bump_start + len(static_bump_op):static_bump_end])
  # Generate a safe sandboxed environment. We replace all ffis with errors. Otherwise,
  # asm.js can't call outside, so we are ok.
  open(temp_file, 'w').write('''
var totalMemory = %d;
var totalStack = %d;

var buffer = new ArrayBuffer(totalMemory);
var heap = new Uint8Array(buffer);

var memInit = %s;

var globalBase = %d;
var staticBump = %d;

heap.set(memInit, globalBase);

var staticTop = globalBase + staticBump;
var staticBase = staticTop;

var stackTop = staticTop;
while (stackTop %% 16 !== 0) stackTop--;
var stackBase = stackTop;
var stackMax = stackTop + totalStack;
if (stackMax >= totalMemory) throw 'not enough room for stack';

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
if (!Math.fround) {
  var froundBuffer = new Float32Array(1);
  Math.fround = function(x) { froundBuffer[0] = x; return froundBuffer[0] };
}

var atexits = []; // we record and replay atexits

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
  STACKTOP: stackTop,
  STACK_MAX: stackMax,
  DYNAMICTOP: dynamicTop,
  ___dso_handle: 0, // used by atexit, value doesn't matter
  _emscripten_memcpy_big: function(dest, src, num) {
    heap.set(heap.subarray(src, src+num), dest);
    return dest;
  },
  _atexit: function(x) {
    atexits.push([x, 0]);
    return 0;
  },
  ___cxa_atexit: function(x, y) {
    atexits.push([x, y]);
    return 0;
  },
};

// Instantiate asm
%s
(globalArg, libraryArg, buffer);

// Try to run the constructors

var allCtors = %s;
var numSuccessful = 0;

for (var i = 0; i < allCtors.length; i++) {
  try {
    var globalsBefore = asm['dumpGlobals']();

    asm[allCtors[i]]();

    var globalsAfter = asm['dumpGlobals']();
    if (JSON.stringify(globalsBefore) !== JSON.stringify(globalsAfter)) {
      console.warn('globals modified');
      break;
    }

    // this one was ok.
    numSuccessful = i + 1;

  } catch (e) {
    console.warn(e.stack);
    break;
  }
}

// Write out new mem init. It might be bigger if we added to the zero section, look for zeros
var newSize = globalBase + staticBump;
while (newSize > globalBase && heap[newSize-1] == 0) newSize--;
console.log(JSON.stringify([numSuccessful, Array.prototype.slice.call(heap.subarray(globalBase, newSize)), atexits]));

''' % (total_memory, total_stack, mem_init, global_base, static_bump, asm, json.dumps(ctors)))
  # Execute the sandboxed code. If an error happened due to calling an ffi, that's fine,
  # us exiting with an error tells the caller that we failed. If it times out, give up.
  out_file = config.get_temp_files().get('.out').name
  err_file = config.get_temp_files().get('.err').name
  proc = subprocess.Popen(shared.NODE_JS + [temp_file], stdout=open(out_file, 'w'), stderr=open(err_file, 'w'))
  try:
    shared.jsrun.timeout_run(proc, timeout=10, full_output=True)
    if proc.returncode != 0:
      shared.logging.debug('unexpected error while trying to eval ctors:\n' + open(err_file).read())
      return (0, 0, 0, 0)
  except Exception, e:
    if 'Timed out' not in str(e): raise e
    shared.logging.debug('ctors timed out\n')
    return (0, 0, 0, 0)
  # out contains the new mem init and other info
  num_successful, mem_init_raw, atexits = json.loads(open(out_file).read())
  mem_init = ''.join(map(chr, mem_init_raw))
  if num_successful < total_ctors:
    shared.logging.debug('not all ctors could be evalled, something was used that was not safe (and therefore was not defined, and caused an error):\n========\n' + open(err_file).read() + '========')
  # Remove the evalled ctors, add a new one for atexits if needed, and write that out
  if len(ctors) == total_ctors and len(atexits) == 0:
    new_ctors = ''
  else:
    elements = []
    if len(atexits) > 0:
      elements.append('{ func: function() { %s } }' % '; '.join(map(lambda x: '_atexit(' + str(x[0]) + ',' + str(x[1]) + ')', atexits)))
    for ctor in all_ctors[num:]:
      elements.append('{ func: function() { %s() } }' % ctor)
    new_ctors = '__ATINIT__.push(' + ', '.join(elements) + ');'
  js = js[:ctors_start] + new_ctors + js[ctors_end:]
  return (num_successful, js, mem_init, ctors)

# main
if __name__ == '__main__':
  js = open(js_file).read()
  ctors_start, ctors_end = find_ctors(js)
  if ctors_start < 0:
    shared.logging.debug('ctor_evaller: no ctors')
    sys.exit(0)

  ctors_text = js[ctors_start:ctors_end];
  if ctors_text.count('(') == 1:
    shared.logging.debug('ctor_evaller: push, but no ctors')
    sys.exit(0)

  num_ctors = ctors_text.count('function()')
  shared.logging.debug('ctor_evaller: %d ctors, from |%s|' % (num_ctors, ctors_text))

  if os.path.exists(mem_init_file):
    mem_init = json.dumps(map(ord, open(mem_init_file, 'rb').read()))
  else:
    mem_init = []

  # find how many ctors we can remove, by bisection (if there are hundreds, running them sequentially is silly slow)

  shared.logging.debug('ctor_evaller: trying to eval %d global constructors' % num_ctors)
  num_successful, new_js, new_mem_init, removed = eval_ctors(js, mem_init, num_ctors)
  if num_successful == 0:
    shared.logging.debug('ctor_evaller: not successful')
    sys.exit(0)

  shared.logging.debug('ctor_evaller: we managed to remove %d ctors' % num_successful)
  if num_successful == num_ctors:
    js = new_js
    mem_init = new_mem_init
  else:
    shared.logging.debug('ctor_evaller: final execution')
    check, js, mem_init, removed = eval_ctors(js, mem_init, num_successful)
    assert check == num_successful
  open(js_file, 'w').write(js)
  open(mem_init_file, 'wb').write(mem_init)

  # Dead function elimination can help us

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

