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
local_root = os.path.join(script_dir, 'libcxx')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')

preserve_files = ('readme.txt', '__assertion_handler', '__config_site')
# ryu_long_double_constants.h from libc is unused (and very large)
excludes = ('CMakeLists.txt', 'ryu_long_double_constants.h')

libc_copy_dirs = [
    'hdr',
    'include/llvm-libc-macros',
    'include/llvm-libc-types',
    'shared',
    'config',
]

def clean_dir(dirname):
  if not os.path.exists(dirname):
    return
  for f in os.listdir(dirname):
    if f in preserve_files:
      continue
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      shutil.rmtree(full)
    else:
      os.remove(full)


def copy_tree(upstream_dir, local_dir):
  if not os.path.exists(local_dir):
    os.makedirs(local_dir)
  for f in os.listdir(upstream_dir):
    full = os.path.join(upstream_dir, f)
    if os.path.isdir(full):
      shutil.copytree(full, os.path.join(local_dir, f))
    elif f not in excludes:
      shutil.copy2(full, os.path.join(local_dir, f))
  for root, dirs, files in os.walk(local_dir):
    for f in files:
      if f in excludes:
        full = os.path.join(root, f)
        os.remove(full)


def main():
  if len(sys.argv) > 1:
    llvm_dir = os.path.abspath(sys.argv[1])
  else:
    llvm_dir = default_llvm_dir
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

  # We don't use frozen c++03 headers for now
  shutil.rmtree(os.path.join(local_inc, '__cxx03'))

  # libcxx includes headers from LLVM's libc
  libc_upstream_dir = os.path.join(llvm_dir, 'libc')
  assert os.path.exists(libc_upstream_dir)
  libc_local_dir = os.path.join(script_dir, 'llvm-libc')

  for dirname in libc_copy_dirs:
    local_dir = os.path.join(libc_local_dir, dirname)
    clean_dir(local_dir)

  for dirname in libc_copy_dirs:
    upstream_dir = os.path.join(libc_upstream_dir, dirname)
    local_dir = os.path.join(libc_local_dir, dirname)
    copy_tree(upstream_dir, local_dir)

  shutil.copy2(os.path.join(libc_upstream_dir, 'LICENSE.TXT'), libc_local_dir)

if __name__ == '__main__':
  main()
