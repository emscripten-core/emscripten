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

sys.path.insert(1, path.dirname(path.dirname(path.abspath(__file__))))
from tools.shared import get_emscripten_temp_dir

tmpdir = get_emscripten_temp_dir()
emdir = path.join(path.dirname(path.realpath(__file__)), "..")


def main():
  if len(sys.argv) == 2:
    simde_dir = sys.argv[1]
  elif len(sys.argv) == 1:
    simde_dir = None
  else:
    print('''USAGE:
./simde_update.py [SIMDE_REPO_DIRECTORY]''')

  if not simde_dir:
    os.mkdir(path.join(tmpdir, "simde"))
    os.system("git clone git@github.com:simd-everywhere/simde " + path.join(tmpdir, "simde"))
    simde_dir = path.join(tmpdir, "simde")

  else:
    print("Using provided repository without updating [make sure it's up to date!]")

  try:
    neon_h_buf = subprocess.check_output([path.join(simde_dir, "amalgamate.py"), path.join(simde_dir, "simde", "arm", "neon.h")])
  except subprocess.CalledProcessError as e:
    print("amalgamate.py returned error: " + str(e))
    return 1

  try:
    os.mkdir(path.join(emdir, "system", "include", "neon"))
  except FileExistsError:
    if not path.isdir(path.join(emdir, "system", "include", "neon")):
      print("system/include/neon exists and is not a directory, exiting...")
      return 1

  with open(path.join(emdir, "system", "include", "neon", "arm_neon.h"), "wb+") as f:
    try:
      f.write(b"#define SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES\n")
      f.write(b"#define SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES\n")
      f.write(neon_h_buf)
      f.write(b"#undef SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES\n")
      f.write(b"#undef SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES\n")
      print("'system/include/neon/arm_neon.h' updated")
    except Exception:
      print("error writing 'system/include/neon/arm_neon.h'")
      return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
