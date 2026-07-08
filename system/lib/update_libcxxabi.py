#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

from update_common import *

local_root = os.path.join(script_dir, 'libcxxabi')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')

excludes = ('CMakeLists.txt',)
preserve_files = ('cxa_exception_js_utils.cpp', '__cpp_exception.S')


def main():
  llvm_dir = parse_args(default_llvm_dir, 'llvm_dir')
  upstream_root = os.path.join(llvm_dir, 'libcxxabi')
  upstream_src = os.path.join(upstream_root, 'src')
  upstream_inc = os.path.join(upstream_root, 'include')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)

  # Remove old version
  clean_dir(local_src, preserve_files)
  clean_dir(local_inc, preserve_files)

  copy_tree(upstream_src, local_src, excludes)
  copy_tree(upstream_inc, local_inc, excludes)
  shutil.copy2(os.path.join(upstream_root, 'CREDITS.TXT'), local_root)
  shutil.copy2(os.path.join(upstream_root, 'LICENSE.TXT'), local_root)
  update_readme(local_root, llvm_dir)


if __name__ == '__main__':
  main()
