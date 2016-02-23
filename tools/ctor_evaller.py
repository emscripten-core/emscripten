'''
Tries to evaluate global constructors, applying their effects ahead of time.

This is an LTO-like operation, and to avoid parsing the entire tree, we operate on the text in python
'''

import os, sys, json, subprocess
import shared, js_optimizer

js_file = sys.argv[1]
mem_init_file = sys.argv[2]
total_memory = int(sys.argv[3])

temp_file = js_file + '.ctorEval.js'

# helpers

def eval_ctor(js, mem_init):
  # Find the global ctors
  ctors_start = js.find('__ATINIT__.push(')
  if ctors_start < 0: return False
  ctors_end = js.find(');', ctors_start)
  if ctors_end < 0: return False
  ctors_end += 3
  ctors_text = js[ctors_start:ctors_end]
  ctors = filter(lambda ctor: ctor.endswith('()') and not ctor == 'function()' and '.' not in ctor, ctors_text.split(' '))
  if len(ctors) == 0: return False
  print ctors_text
  ctor = ctors[0].replace('()', '')
  shared.logging.debug('trying to eval ctor: ' + ctor)
  # Find the asm module, and receive the mem init.
  asm = js[js.find(js_optimizer.start_asm_marker):js.find(js_optimizer.end_asm_marker)]
  assert len(asm) > 0
  asm = asm.replace('use asm', 'not asm') # don't try to validate this
  # Generate a safe sandboxed environment. We replace all ffis with errors. Otherwise,
  # asm.js can't call outside, so we are ok.
  open(temp_file, 'w').write('''
var buffer = new ArrayBuffer(%s);

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
};

// Instantiate asm
%s
(globalArg, libraryArg, buffer);

// Try to run the constructor
asm['%s']();

// We succeeded - verify asm global vars, and write out new mem init

''' % (total_memory, asm, ctor))
  # Execute the sandboxed code. If an error happened due to calling an ffi, that's fine,
  # us exiting with an error tells the caller that we failed.
  proc = subprocess.Popen(shared.NODE_JS + [temp_file], stdout=subprocess.PIPE)
  out, err = proc.communicate()
  if proc.returncode != 0: return False
  # Success! out contains the new mem init, write it out
  mem_init = ''.join(map(chr, json.loads(out)))
  # Remove this ctor and write that out # TODO: remove from the asm export as well
  if len(ctors) == 1:
    new_ctors = ''
  else:
    new_ctors = ctors_text[:ctors_text.find('(') + 1] + ctors_text[ctors_text.find(',')+1:]
  js = js[:ctors_start] + new_ctors + js[ctors_end:]
  return (js, mem_init)

# main

# keep running whlie we succeed in removing a constructor

removed_one = False

while True:
  shared.logging.debug('ctor_evaller: trying to eval a global constructor')
  js = open(js_file).read()
  mem_init = json.dumps(map(ord, open(mem_init_file, 'rb').read()))
  result = eval_ctor(js, mem_init)
  if not result:
    shared.logging.debug('ctor_evaller: done')
    break # that's it, no more luck. either no ctors, or we failed to eval a ctor
  shared.logging.debug('ctor_evaller: success!')
  js, mem_init = result
  open(js_file, 'w').write(js)
  open(mem_init_file, 'wb').write(mem_init)
  removed_one = True

# If we removed one, dead function elimination can help us

if removed_one:
  shared.logging.debug('ctor_evaller: JSDFE')
  proc = subprocess.Popen(shared.NODE_JS + [shared.path_from_root('tools', 'js-optimizer.js'), js_file, 'JSDFE'], stdout=subprocess.PIPE)
  out, err = proc.communicate()
  assert proc.returncode == 0
  open(js_file, 'w').write(out)

