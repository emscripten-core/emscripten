#!/usr/bin/env python
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

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools import shared


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


def find_ctors(js):
  ctors_start = js.find('__ATINIT__.push(')
  if ctors_start < 0:
    return (-1, -1)
  ctors_end = js.find(');', ctors_start)
  assert ctors_end > 0
  ctors_end += 3
  return (ctors_start, ctors_end)


def find_ctors_data(js, num):
  ctors_start, ctors_end = find_ctors(js)
  assert ctors_start > 0
  ctors_text = js[ctors_start:ctors_end]
  all_ctors = [ctor for ctor in ctors_text.split(' ') if ctor.endswith('()') and not ctor == 'function()' and '.' not in ctor]
  all_ctors = [ctor.replace('()', '') for ctor in all_ctors]
  assert all(ctor.startswith('_') for ctor in all_ctors)
  all_ctors = [ctor[1:] for ctor in all_ctors]
  assert len(all_ctors)
  ctors = all_ctors[:num]
  return ctors_start, ctors_end, all_ctors, ctors


def eval_ctors(js, wasm_file, num):
  ctors_start, ctors_end, all_ctors, ctors = find_ctors_data(js, num)
  cmd = [os.path.join(binaryen_bin, 'wasm-ctor-eval'), wasm_file, '-o', wasm_file, '--ctors=' + ','.join(ctors)]
  cmd += extra_args
  if debug_info:
    cmd += ['-g']
  logger.debug('wasm ctor cmd: ' + str(cmd))
  proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
  try:
    err = shared.timeout_run(proc, timeout=10, full_output=True, check=False)
  except Exception as e:
    if 'Timed out' not in str(e):
      raise
    logger.debug('ctors timed out\n')
    return 0, js
  if proc.returncode != 0:
    shared.exit_with_error('unexpected error while trying to eval ctors:\n' + err)
  num_successful = err.count('success on')
  logger.debug(err)
  if len(ctors) == num_successful:
    new_ctors = ''
  else:
    elements = []
    for ctor in all_ctors[num_successful:]:
      elements.append('{ func: function() { %s() } }' % ctor)
    new_ctors = '__ATINIT__.push(' + ', '.join(elements) + ');'
  js = js[:ctors_start] + new_ctors + js[ctors_end:]
  return num_successful, js


# main
def main():
  js = open(js_file).read()
  ctors_start, ctors_end = find_ctors(js)
  if ctors_start < 0:
    logger.debug('ctor_evaller: no ctors')
    sys.exit(0)

  ctors_text = js[ctors_start:ctors_end]
  if ctors_text.count('(') == 1:
    logger.debug('ctor_evaller: push, but no ctors')
    sys.exit(0)

  num_ctors = ctors_text.count('function()')
  logger.debug('ctor_evaller: %d ctors, from |%s|' % (num_ctors, ctors_text))

  wasm_file = binary_file
  logger.debug('ctor_evaller (wasm): trying to eval %d global constructors' % num_ctors)
  num_successful, new_js = eval_ctors(js, wasm_file, num_ctors)
  if num_successful == 0:
    logger.debug('ctor_evaller: not successful')
    sys.exit(0)
  logger.debug('ctor_evaller: we managed to remove %d ctors' % num_successful)
  open(js_file, 'w').write(new_js)


if __name__ == '__main__':
  sys.exit(main())
