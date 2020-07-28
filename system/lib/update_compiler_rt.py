#!/usr/bin/env python
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import glob
import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
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

def clear(dirname):
  if os.path.exists(dirname):
    shutil.rmtree(dirname)
  os.makedirs(dirname)


def main():
  upstream_dir = os.path.abspath(sys.argv[1])
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
