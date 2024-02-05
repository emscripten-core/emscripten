#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# Copy local llvm library changes into the upstream llvm tree.
# This is the logical inverse of update_compiler_rt.py, update_libcxx.py
# and update_libcxxabi.py which copy changes form the upstream llvm 
# into emscripten.

import glob
import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')
copy_dirs = [
  'compiler-rt',
  'libcxx',
  'libcxxabi',
  'libunwind',
]


def main():
  if len(sys.argv) > 1:
    upstream_root = os.path.join(os.path.abspath(sys.argv[1]))
  else:
    upstream_root = default_llvm_dir
  if not os.path.exists(upstream_root):
    print(f'llvm tree not found: {upstream_root}')
    return 1

  for dir in copy_dirs:
    assert os.path.exists(os.path.join(upstream_root, dir))

  for dir in copy_dirs:
    local_dir = os.path.join(script_dir, dir)
    upstream_dir = os.path.join(upstream_root, dir)
    print(f'copying {local_dir} -> {upstream_dir}')
    shutil.copytree(local_dir, upstream_dir, dirs_exist_ok=True)


if __name__ == '__main__':
  main()
