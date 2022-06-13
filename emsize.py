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

import argparse
import os
import subprocess
import sys

from tools import shared

LLVM_SIZE = os.path.expanduser(shared.build_llvm_tool_path(shared.exe_suffix('llvm-size')))


def error(text):
  print(text, file=sys.stderr, flush=True)
  return 1


def parse_args(argv):
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-format', '--format')
  parser.add_argument('file')
  args = parser.parse_args(argv)
  return args.file


def print_sizes(js_file):
  if not os.path.isfile(js_file):
    return error('Input JS file %s not foune' % js_file)
  if not js_file.endswith('.js'):
    return error('Input file %s does not have a JS extension' % js_file)

  basename = js_file[:-3]

  # Find the JS file size
  st = os.stat(js_file)
  js_size = st.st_size

  # Find the rest of the sizes
  wasm_file = basename + '.wasm'
  if not os.path.isfile(wasm_file):
    return error('Wasm file %s not found' % wasm_file)

  sizes = shared.check_call([LLVM_SIZE, '--format=sysv', wasm_file],
                            stdout=subprocess.PIPE).stdout
  # llvm-size may emit some number of blank lines (after the total), ignore them
  lines = [line for line in sizes.splitlines() if line]

  # Last line is the total. Add the JS size.
  total = int(lines[-1].split()[-1])
  total += js_size

  for line in lines[:-1]:
    print(line)

  print('JS\t\t%s\t0' % js_size)
  print('Total\t\t%s' % total)


if __name__ == '__main__':
  sys.exit(print_sizes(parse_args(sys.argv[1:])))
