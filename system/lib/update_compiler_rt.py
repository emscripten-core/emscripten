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


def main():
  upstream_dir = os.path.abspath(sys.argv[1])
  assert os.path.exists(upstream_dir)
  upstream_src = os.path.join(upstream_dir, 'lib', 'builtins')
  assert os.path.exists(upstream_src)

  # Remove old version
  shutil.rmtree(local_builtins)
  os.mkdir(local_builtins)

  listfile = os.path.join(script_dir, 'compiler-rt', 'filelist.txt')
  for name in open(listfile).read().splitlines():
    src = os.path.join(upstream_src, name);
    target = os.path.join(local_builtins, name);
    shutil.copy2(src, target)

  for pattern in ('*.h', '*.inc'):
    for name in glob.glob(os.path.join(upstream_src, pattern)):
      shutil.copy2(name, local_builtins)

  shutil.copy2(os.path.join(upstream_dir, 'CREDITS.TXT'), local_src)
  shutil.copy2(os.path.join(upstream_dir, 'LICENSE.TXT'), local_src)


if __name__ == '__main__':
  main()
