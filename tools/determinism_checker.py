'''
Runs a build command many times to search for any nondeterminism.
'''

import os
import random
import shutil
import subprocess
import time

RELEVANT_FILES = ['a.out.js', 'byn-21-optimize-instructions.wast', 'byn-37-memory-packing.wast', 'a.out.js.mem', 'byn-22-precompute.wast', 'byn-3-dce.wast', 'a.out.temp.asm.js', 'byn-23-rse.wast', 'byn-4-remove-unused-brs.wast', 'a.out.wasm', 'byn-24-vacuum.wast', 'byn-5-remove-unused-names.wast', 'byn-0-before.wast', 'byn-25-before.wast', 'byn-6-optimize-instructions.wast', 'byn-10-simplify-locals-nostructure.wast', 'byn-26-finalize-calls.wast', 'byn-7-pick-load-signs.wast', 'byn-11-vacuum.wast', 'byn-27-legalize-js-interface.wast', 'byn-8-precompute.wast', 'byn-12-reorder-locals.wast', 'byn-28-vacuum.wast', 'byn-9-code-pushing.wast', 'byn-13-remove-unused-brs.wast', 'byn-29-remove-unused-brs.wast', 'emcc-0-linktime.bc', 'byn-14-coalesce-locals.wast', 'byn-2-relooper-jump-threading.wast', 'emcc-1-original.js', 'byn-15-simplify-locals.wast', 'byn-30-optimize-instructions.wast', 'emcc-2-meminit.js', 'byn-16-vacuum.wast', 'byn-31-post-emscripten.wast', 'emcc-3-preclean.js', 'byn-17-reorder-locals.wast', 'byn-32-inlining-optimizing.wast', 'emcc-3-preclean.wasm', 'byn-18-merge-blocks.wast', 'byn-33-duplicate-function-elimination.wast', 'emcc-4-postclean.js', 'byn-19-remove-unused-brs.wast', 'byn-34-remove-unused-module-elements.wast', 'emcc-4-postclean.wasm', 'byn-1-autodrop.wast', 'byn-35-memory-packing.wast', 'emcc-5-module_export_name_substitution.js', 'byn-20-merge-blocks.wast', 'byn-36-before.wast']

def run():
  subprocess.check_call(['emcc', 'src.cpp', '-O2', '-s', 'WASM=1'])
  ret = {}
  for relevant_file in RELEVANT_FILES:
    ret[relevant_file] = open(relevant_file).read()
  return ret

def write(data, subdir):
  if not os.path.exists(subdir):
    os.mkdir(subdir)
  for relevant_file in RELEVANT_FILES:
    open(os.path.join(subdir, relevant_file), 'w').write(data[relevant_file])

old = os.getcwd()
try:
  os.chdir('/tmp/emscripten_temp')
  open('src.cpp', 'w').write('''
    #include <iostream>

    int main()
    {
      std::cout << "hello world" << std::endl << 77 << "." << std::endl;
      return 0;
    }
  ''')

  os.environ['EMCC_DEBUG'] = '1'
  os.environ['BINARYEN_PASS_DEBUG'] = '3'

  first = run()

  i = 0
  while 1:
    print(i)
    i += 1
    time.sleep(random.random()/(10*5)) # add some timing nondeterminism here, not that we need it, but whatever
    curr = run()
    if first != curr:
      print('NONDETERMINISM!!!1')
      write(first, 'first')
      write(curr, 'second')
      break

finally:
  os.chdir(old)

