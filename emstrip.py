#!/usr/bin/env python3
# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Wrapper script around `llvm-strip`.

  It also supports taking a JS file as an argument and running 'llvm-strip' on
  the corresponding Wasm file. This is convenient for some build systems that
  expect to strip the output of a compile.
"""

import os
import sys

from tools import shared


def run():
  llvm_strip = shared.llvm_tool_path('llvm-strip')
  new_args = []
  for arg in sys.argv[1:]:
    base, ext = os.path.splitext(arg)
    if ext == '.js' and os.path.isfile(arg):
      wasm_file = base + '.wasm'
      if os.path.isfile(wasm_file):
        new_args.append(wasm_file)
        continue
    new_args.append(arg)

  shared.exec_process([llvm_strip] + new_args)


if __name__ == '__main__':
  run()
