#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Size helper script

This script acts as a frontend replacement for `size` that supports combining
JS and wasm output from emscripten.
The traditional size utility reports the size of each section in a binary
and the total. This replacement adds another pseudo-section, "JS" which
shows the size of the JavaScript loader file.

Currently there are many limitations; basically this tool is enough to
be used by the LLVM testsuite runner code to analyze size output.

Currently this tool only supports sysv output format (it accepts but ignores
any '-format' argument). It does not accept any other arguments aside from the
input file, which is expected to be a JS file. The wasm file is expected to be
in the same directory, and have the same basename with a '.wasm' extension.
"""

import os
import subprocess
import sys
import mimetypes

from tools import shared

LLVM_SIZE = os.path.expanduser(shared.build_llvm_tool_path(shared.exe_suffix('llvm-size')))


def error(text):
  print(text, file=sys.stderr, flush=True)
  return 1

def parse_args(argv):
  files = []
  params = []
  appendFormat = True
  ret = 0
  for arg in argv:
    if (arg.startswith('-')):
      params.append(arg)
    else:
      files.append(arg)
    if (arg.startswith('--format')):
      appendFormat = False
  if appendFormat:
    params.append('--format=sysv')
  for file in files:
    ret = print_sizes(file, params)
  return ret

def print_sizes(file, params):
  if not os.path.isfile(file):
    return error('Input file %s not foune' % file)
  ftype, fencoding = mimetypes.guess_type(file)
  if(ftype == 'application/wasm' or ftype == 'text/vnd.a'):
    js_size = 0
    wasm_file = file
  else:
    if '.' in file:
      file, extension = file.rsplit('.', 1)

    # Find the JS file size
    st = os.stat(file)
    js_size = st.st_size

    # Find the rest of the sizes
    wasm_file = file + '.wasm'

  if not os.path.isfile(wasm_file):
    return error('Wasm file %s not found' % wasm_file)

  callArgs = [LLVM_SIZE] + params + [wasm_file]
  sizes = shared.check_call(callArgs, stdout=subprocess.PIPE).stdout
  # llvm-size may emit some number of blank lines (after the total), ignore them
  lines = [line for line in sizes.splitlines() if line]

  # Last line is the total. Add the JS size.
  total = int(lines[-1].split()[-1])
  total += js_size

  for line in lines[:-1]:
    print(line)

  print('%s%16d%7d' % ('JS', js_size, 0))
  print('%s%13s' % ('Total', total))
  print('\n')

if __name__ == '__main__':
  sys.exit(parse_args(sys.argv[1:]))
