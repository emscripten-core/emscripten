#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')
local_root = os.path.join(script_dir, 'libcxxabi')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')

excludes = ('CMakeLists.txt',)
preserve_files = ()


def clean_dir(dirname):
  for f in os.listdir(dirname):
    if f in preserve_files:
      continue
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      shutil.rmtree(full)
    else:
      os.remove(full)


def copy_tree(upstream_dir, local_dir):
  for f in os.listdir(upstream_dir):
    full = os.path.join(upstream_dir, f)
    if os.path.isdir(full):
      shutil.copytree(full, os.path.join(local_dir, f))
    elif f not in excludes:
      shutil.copy2(full, os.path.join(local_dir, f))


def main():
  if len(sys.argv) > 1:
    llvm_dir = os.path.join(os.path.abspath(sys.argv[1]))
  else:
    llvm_dir = default_llvm_dir
  upstream_root = os.path.join(llvm_dir, 'libcxxabi')
  upstream_src = os.path.join(upstream_root, 'src')
  upstream_inc = os.path.join(upstream_root, 'include')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)

  # Remove old version
  clean_dir(local_src)
  clean_dir(local_inc)

  copy_tree(upstream_src, local_src)
  copy_tree(upstream_inc, local_inc)
  shutil.copy2(os.path.join(upstream_root, 'CREDITS.TXT'), local_root)
  shutil.copy2(os.path.join(upstream_root, 'LICENSE.TXT'), local_root)


if __name__ == '__main__':
  main()
