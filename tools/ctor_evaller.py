#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tries to evaluate global constructors, applying their effects ahead of time.

This is an LTO-like operation, and to avoid parsing the entire tree (we might
fail to parse a massive project, we operate on the text in python.
"""

import logging
import os
import subprocess
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.append(__rootdir__)

from tools import utils


js_file = sys.argv[1]
binary_file = sys.argv[2] # mem init for js, wasm binary for wasm
total_memory = int(sys.argv[3])
total_stack = int(sys.argv[4])
global_base = int(sys.argv[5])
binaryen_bin = sys.argv[6]
debug_info = int(sys.argv[7])
extra_args = sys.argv[8:]

wasm = bool(binaryen_bin)

assert global_base > 0

logger = logging.getLogger('ctor_evaller')

# helpers

CTOR_NAME = '__wasm_call_ctors'

CTOR_ADD_PATTERN = '''addOnInit(Module['asm']['%s']);''' % CTOR_NAME

def has_ctor(js):
  return CTOR_ADD_PATTERN in js


def eval_ctors(js, wasm_file):
  cmd = [os.path.join(binaryen_bin, 'wasm-ctor-eval'), wasm_file, '-o', wasm_file, '--ctors=' + CTOR_NAME]
  cmd += extra_args
  if debug_info:
    cmd += ['-g']
  logger.warning('wasm ctor cmd: ' + str(cmd))
  try:
    err = subprocess.run(cmd, stderr=subprocess.PIPE, timeout=10, text=True).stderr
  except subprocess.TimeoutExpired:
    logger.warning('ctors timed out\n')
    return 0, js
  logger.warning(err)
  num_successful = err.count('success on')
  if num_successful:
    js = js.replace(CTOR_ADD_PATTERN, '')
  return num_successful, js


# main
def main():
  logger.warning('ctor_evaller: waka')
  js = utils.read_file(js_file)
  if not has_ctor(js):
    logger.warning('ctor_evaller: no ctors')
    sys.exit(0)

  wasm_file = binary_file
  logger.warning('ctor_evaller (wasm): trying to eval global ctor')
  num_successful, new_js = eval_ctors(js, wasm_file)
  if num_successful == 0:
    logger.warning('ctor_evaller: not successful')
    sys.exit(0)
  logger.warning('ctor_evaller: we managed to remove the ctors')
  utils.write_file(js_file, new_js)


if __name__ == '__main__':
  sys.exit(main())
