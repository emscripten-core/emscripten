#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil
import glob

from update_common import *

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
    'src/assert',
    'src/complex',
    'src/errno',
    'src/inttypes',
    'src/math',
    'src/setjmp',
    'src/stdio/printf_core',
    'src/stdlib',
    'src/string',
    'src/strings',
    'src/wchar',
]

libc_exclusion_patterns = [
     # float16 is unsupported in Emscripten.
    'src/complex/**/*f16*',
    'src/math/generic/*f16*',

    'src/setjmp/**/*',  # setjmp in Emscripten is implemented by the clang backend.
    'src/strings/str*casecmp_l*',  # locale_t is unsupported in Overlay Mode.
]


def main():
  llvm_dir, force = parse_args(default_llvm_dir, 'llvm_dir')
  check_clean(force)
  libc_upstream_dir = os.path.join(llvm_dir, 'libc')
  assert os.path.exists(libc_upstream_dir)
  libc_local_dir = os.path.join(script_dir, 'llvm-libc')

  for dirname in libc_copy_dirs:
    local_dir = os.path.join(libc_local_dir, dirname)
    clean_dir(local_dir, preserve_files)

  for dirname in libc_copy_dirs:
    upstream_dir = os.path.join(libc_upstream_dir, dirname)
    local_dir = os.path.join(libc_local_dir, dirname)
    copy_tree(upstream_dir, local_dir, excludes)

  # Certain llvm-libc files that are incompatible in Emscripten
  for exclusion_pattern in libc_exclusion_patterns:
    files_to_exclude = glob.glob(os.path.join(libc_local_dir, exclusion_pattern))
    for file in files_to_exclude:
      os.remove(file)

  update_readme(libc_local_dir, llvm_dir)

if __name__ == '__main__':
  main()
