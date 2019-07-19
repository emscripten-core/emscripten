#!/usr/bin/env python
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.join(script_dir, 'libcxx')
local_inc = os.path.join(os.path.dirname(script_dir), 'include', 'libcxx')

preserve_files = ('readme.txt', 'symbols')


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
    else:
      shutil.copy2(full, os.path.join(local_dir, f))


def main():
  libcxx_dir = os.path.abspath(sys.argv[1])
  upstream_inc = os.path.join(libcxx_dir, 'include')
  upstream_src = os.path.join(libcxx_dir, 'src')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)

  # Remove old version
  clean_dir(local_src)
  clean_dir(local_inc)

  copy_tree(upstream_inc, local_inc)
  copy_tree(upstream_src, local_src)

  shutil.copy2(os.path.join(libcxx_dir, 'CREDITS.TXT'), local_src)
  shutil.copy2(os.path.join(libcxx_dir, 'LICENSE.TXT'), local_src)


if __name__ == '__main__':
  main()
