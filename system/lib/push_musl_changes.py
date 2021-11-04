#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# Copy local emscripten changes into the upstream musl tree.
# This is the logical inverse of update_musl.py which copies changes
# form the upstream musl tree into emscripten.

import glob
import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
local_dir = os.path.join(script_dir, 'libc', 'musl')
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_musl_dir = os.path.join(os.path.dirname(emscripten_root), 'musl')


def main():
  if len(sys.argv) > 1:
    upstream_root = os.path.join(os.path.abspath(sys.argv[1]))
  else:
    upstream_root = default_musl_dir
  if not os.path.exists(upstream_root):
    print(f'musl tree not found: {upstream_root}')
    return 1

  print(f'copying {local_dir} -> {upstream_root}')
  shutil.copytree(local_dir, upstream_root, dirs_exist_ok=True)


if __name__ == '__main__':
  main()
