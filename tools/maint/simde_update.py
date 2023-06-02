#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Updates the arm_neon.h header taken from SIMDe
(https://github.com/simd-everywhere/simde) in system/include/neon
"""

import os
import re
import subprocess
import sys

from os import path

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(os.path.dirname(__scriptdir__))
sys.path.insert(0, __rootdir__)

from tools.shared import get_emscripten_temp_dir

tmpdir = get_emscripten_temp_dir()
emdir = __rootdir__


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
    neon_h_buf = subprocess.check_output(
        [path.join(simde_dir, "amalgamate.py"), path.join(simde_dir, "simde", "arm", "neon.h")],
        text=True)
  except subprocess.CalledProcessError as e:
    print("amalgamate.py returned error: " + str(e))
    return 1

  try:
    os.mkdir(path.join(emdir, "system", "include", "compat"))
  except FileExistsError:
    if not path.isdir(path.join(emdir, "system", "include", "compat")):
      print("system/include/compat exists and is not a directory, exiting...")
      return 1

  with open(path.join(emdir, "system", "include", "compat", "arm_neon.h"), "w+") as f:
    try:
      f.write("#define SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES\n")
      f.write("#define SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES\n")
      SIMDE_FILE_HEADER_RE = r'^(/\* :: )(Begin |End )[^ ]+/(simde/simde/[^ ]+ :: \*/$)'
      # Replace file headers, which contains tmp directory names and changes every time we
      # update simde, causing a larger diff than necessary.
      f.write(re.sub(SIMDE_FILE_HEADER_RE, r'\1\2\3', neon_h_buf, count=0, flags=re.MULTILINE))
      f.write("#undef SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES\n")
      f.write("#undef SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES\n")
      print("'system/include/compat/arm_neon.h' updated")
    except Exception:
      print("error writing 'system/include/compat/arm_neon.h'")
      return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
