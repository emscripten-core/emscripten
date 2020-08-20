#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Verifies that 'docs/emcc.txt' is in sync with sphinx output."""

import os
import subprocess
import sys

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
  build_output = os.path.join(root, 'site', 'build', 'text', 'docs', 'tools_reference', 'emcc.txt')
  docs_file = os.path.join(root, 'docs', 'emcc.txt')

  if not os.path.exists(build_output):
    print('doc build output not found: %s' % build_output)
    return 1

  with open(build_output, 'r') as f:
    emcc_docs_output = f.read()

  with open(docs_file, 'r') as f:
    emcc_docs = f.read()

  if emcc_docs_output != emcc_docs:
    print('contents of checked in docs/emcc.txt does not match build output:')
    subprocess.call(['diff', '-u', build_output, docs_file])
    return 1

  print('docs look good')


if __name__ == '__main__':
  sys.exit(main())
