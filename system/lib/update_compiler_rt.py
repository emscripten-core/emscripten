#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

from update_common import (
  clean_dir,
  default_llvm_dir,
  parse_args,
  script_dir,
  update_readme,
)

local_src = os.path.join(script_dir, 'compiler-rt')

copy_dirs = [
    ('include', 'sanitizer'),
    ('include', 'profile'),
    ('lib', 'sanitizer_common'),
    ('lib', 'asan'),
    ('lib', 'interception'),
    ('lib', 'builtins'),
    ('lib', 'lsan'),
    ('lib', 'ubsan'),
    ('lib', 'ubsan_minimal'),
    ('lib', 'profile'),
]

preserve_files = ('readme.txt',)


def main():
  llvm_dir = parse_args(default_llvm_dir, 'llvm_dir')
  upstream_dir = os.path.join(llvm_dir, 'compiler-rt')
  assert os.path.exists(upstream_dir)
  upstream_src = os.path.join(upstream_dir, 'lib', 'builtins')
  upstream_include = os.path.join(upstream_dir, 'include', 'sanitizer')
  assert os.path.exists(upstream_src)
  assert os.path.exists(upstream_include)

  for dirname in copy_dirs:
    srcdir = os.path.join(upstream_dir, *dirname)
    assert os.path.exists(srcdir)
    dest = os.path.join(local_src, *dirname)
    clean_dir(dest, preserve_files)
    for name in os.listdir(srcdir):
      if name in {'.clang-format', 'CMakeLists.txt', 'README.txt', 'weak_symbols.txt'}:
        continue
      if name.endswith(('.syms.extra', '.S')):
        continue
      if os.path.isfile(os.path.join(srcdir, name)):
        shutil.copy2(os.path.join(srcdir, name), dest)

  shutil.copy2(os.path.join(upstream_dir, 'CREDITS.TXT'), local_src)
  shutil.copy2(os.path.join(upstream_dir, 'LICENSE.TXT'), local_src)
  update_readme(local_src, llvm_dir)


if __name__ == '__main__':
  main()
