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
local_builtins = os.path.join(local_src, 'lib', 'builtins')
local_include = os.path.join(local_src, 'include', 'sanitizer')


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

  # Remove old version
  clear(local_builtins)
  clear(local_include)

  for pattern in ('*.c', '*.h', '*.inc'):
    for src in glob.glob(os.path.join(upstream_src, pattern)):
      shutil.copy2(src, local_builtins)

  for pattern in ('*.h', '*.inc'):
    for name in glob.glob(os.path.join(upstream_src, pattern)):
      shutil.copy2(name, local_builtins)
    for name in glob.glob(os.path.join(upstream_include, pattern)):
      shutil.copy2(name, local_include)

  shutil.copy2(os.path.join(upstream_dir, 'CREDITS.TXT'), local_src)
  shutil.copy2(os.path.join(upstream_dir, 'LICENSE.TXT'), local_src)


if __name__ == '__main__':
  main()
