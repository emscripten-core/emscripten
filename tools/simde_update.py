#!/usr/bin/env python
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Updates the arm_neon.h header taken from SIMDe
(https://github.com/simd-everywhere/simde) in system/include/neon
"""


import os
import subprocess
import sys

from os import path

# Why are the packages being stupid
# from .shared import get_emscripten_temp_dir


tmpdir = "/tmp"
emdir = path.join(path.dirname(path.realpath(__file__)), "..")

def main(simde_path=None):
  #tmpdir = get_emscripten_temp_dir()
  print(tmpdir)

  try:
    os.mkdir(path.join(tmpdir, "simde"))
    os.system("git clone git@github.com:simd-everywhere/simde " + path.join(tmpdir, "simde"))
  except FileExistsError as e:
    if not path.isdir(path.join(tmpdir, "simde")):
      print("/tmp/simde not a directory, exiting...")
      return 1
    else:
      print("simde repository already found in tmpdir, using found repository")
      os.system("git -C " + path.join(tmpdir, "simde") + " pull")

  simde_dir = path.join(tmpdir, "simde")

  try:
    neon_h_buf = subprocess.check_output([path.join(simde_dir, "amalgamate.py"), path.join(simde_dir, "simde", "arm", "neon.h")])
  except CalledProcessError as e:
    print("amalgamate.py returned error: " + e)
    return 1

  try:
    os.mkdir(path.join(emdir, "system", "include", "neon"))
  except FileExistsError as e:
    if not path.isdir(path.join(emdir, "system", "include", "neon")):
      print("system/include/neon exists and is not a directory, exiting...")
      return 1

  with open(path.join(emdir, "system", "include", "neon", "arm_neon.h"), "wb+") as f:
    try:
      f.write(b"#define SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES\n")
      f.write(b"#define SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES\n")
      f.write(neon_h_buf)
      print("'system/include/neon/arm_neon.h' updated")
    except:
      print("error writing 'system/include/neon/arm_neon.h'")
      return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
