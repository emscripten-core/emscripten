#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Verifies that 'docs/emcc.txt' is in sync with sphinx output."""

import os
import subprocess
import sys
from pathlib import Path

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))


def main():
  build_output = os.path.join(root_dir, 'site', 'build', 'text', 'docs', 'tools_reference', 'emcc.txt')
  docs_file = os.path.join(root_dir, 'docs', 'emcc.txt')

  if not os.path.exists(build_output):
    print('doc build output not found: %s' % build_output)
    return 1

  emcc_docs_output = Path(build_output).read_text()
  emcc_docs = Path(docs_file).read_text()

  if emcc_docs_output != emcc_docs:
    print('contents of checked in docs/emcc.txt does not match build output:')
    subprocess.call(['diff', '-u', build_output, docs_file])
    return 1

  print('docs look good')


if __name__ == '__main__':
  sys.exit(main())
