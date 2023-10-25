#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
This operates on a JS+wasm emitted from compiling with

  -sMAYBE_WASM2JS

and it does wasm2js on it. That is, it converts the wasm to JS, and lets
you run it. You can also pick between the wasm and JS at runtime.

This can be convenient for reducing a testcase: compile once to JS+wasm,
then reduce the wasm, and run this script on those to get wasm2js output.

Usage:

  maybe_wasm2js.py JS_FILE WASM_FILE [OPTS for wasm2js]

Usually adding something like -O for OPTS is what you want, as unoptimized
wasm2js is quite large and slow.

This emits the modified JS to stdout.
'''

import os
import subprocess
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import shared, building

js_file = sys.argv[1]
wasm_file = sys.argv[2]
opts = sys.argv[3:]

# main

cmd = [os.path.join(building.get_binaryen_bin(), 'wasm2js'), '--emscripten', wasm_file]
cmd += opts
js = shared.run_process(cmd, stdout=subprocess.PIPE).stdout
# assign the instantiate function to where it will be used
js = shared.do_replace(js, 'function instantiate(info) {',
                       "Module['__wasm2jsInstantiate__'] = function(info) {")

# create the combined js to run in wasm2js mode
print('var Module = { doWasm2JS: true };\n')
print('\n')
print(js)
print('\n')
with open(js_file) as original:
  print(original.read())
