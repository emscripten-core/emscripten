#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil
import glob

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')

preserve_files = ('readme.txt', '__assertion_handler', '__config_site')
# ryu_long_double_constants.h from libc is unused (and very large)
excludes = ('CMakeLists.txt', 'ryu_long_double_constants.h')

libc_copy_dirs = [
    'hdr',
    'include/llvm-libc-macros',
    'include/llvm-libc-types',
    'shared',
    'config',
    'src/__support',
    'src/string',
    'src/strings',
    'src/errno',
    'src/math',
    'src/stdlib',
    'src/inttypes',
    'src/stdio/printf_core',
    'src/wchar',
]

libc_exclusion_patterns = [
    'src/math/generic/*f16*', # float16 is unsupported in Emscripten.
    'src/strings/str*casecmp_l*', # locale_t is unsupported in Overlay Mode.
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
  
  # Certain llvm-libc files that are incompatible in Emscripten
  for excludsion_pattern in libc_exclusion_patterns:
    files_to_exclude = glob.glob(os.path.join(libc_local_dir, excludsion_pattern))
    for file in files_to_exclude:
        os.remove(file)

if __name__ == '__main__':
  main()