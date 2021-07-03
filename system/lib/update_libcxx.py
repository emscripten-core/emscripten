#!/usr/bin/env python
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
local_root = os.path.join(script_dir, 'libcxx')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')

preserve_files = ('readme.txt', 'symbols')
excludes = ('CMakeLists.txt',)


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
  llvm_dir = os.path.abspath(sys.argv[1])
  libcxx_dir = os.path.join(llvm_dir, 'libcxx')
  upstream_src = os.path.join(libcxx_dir, 'src')
  upstream_inc = os.path.join(libcxx_dir, 'include')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)

  # Remove old version
  clean_dir(local_src)
  clean_dir(local_inc)

  copy_tree(upstream_src, local_src)
  copy_tree(upstream_inc, local_inc)

  shutil.copy2(os.path.join(libcxx_dir, 'CREDITS.TXT'), local_root)
  shutil.copy2(os.path.join(libcxx_dir, 'LICENSE.TXT'), local_root)


if __name__ == '__main__':
  main()
