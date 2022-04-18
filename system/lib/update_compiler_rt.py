#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import glob
import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')
local_src = os.path.join(script_dir, 'compiler-rt')

copy_dirs = [
    ('include', 'sanitizer'),
    ('lib', 'sanitizer_common'),
    ('lib', 'asan'),
    ('lib', 'interception'),
    ('lib', 'builtins'),
    ('lib', 'lsan'),
    ('lib', 'ubsan'),
    ('lib', 'ubsan_minimal'),
]

preserve_files = ('readme.txt',)


def clear(dirname):
  for f in os.listdir(dirname):
    if f in preserve_files or 'emscripten' in f:
      continue
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      shutil.rmtree(full)
    else:
      os.remove(full)


def main():
  if len(sys.argv) > 1:
    llvm_dir = os.path.join(os.path.abspath(sys.argv[1]))
  else:
    llvm_dir = default_llvm_dir
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
    clear(dest)
    for name in os.listdir(srcdir):
      if name in ('.clang-format', 'CMakeLists.txt', 'README.txt', 'weak_symbols.txt'):
        continue
      if name.endswith('.syms.extra') or name.endswith('.S'):
        continue
      if os.path.isfile(os.path.join(srcdir, name)):
        shutil.copy2(os.path.join(srcdir, name), dest)

  shutil.copy2(os.path.join(upstream_dir, 'CREDITS.TXT'), local_src)
  shutil.copy2(os.path.join(upstream_dir, 'LICENSE.TXT'), local_src)


if __name__ == '__main__':
  main()
