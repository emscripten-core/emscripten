#!/usr/bin/env python
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
local_root = os.path.join(script_dir, 'libcxxabi')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')


def main():
  upstream_root = os.path.abspath(sys.argv[1])
  upstream_src = os.path.join(upstream_root, 'src')
  upstream_inc = os.path.join(upstream_root, 'include')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)

  # Remove old version
  shutil.rmtree(local_src)
  shutil.rmtree(local_inc)

  shutil.copytree(upstream_src, local_src)
  shutil.copytree(upstream_inc, local_inc)
  shutil.copy2(os.path.join(upstream_root, 'CREDITS.TXT'), local_root)
  shutil.copy2(os.path.join(upstream_root, 'LICENSE.TXT'), local_root)


if __name__ == '__main__':
  main()
