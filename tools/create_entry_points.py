#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool for creating/maintains the python launcher scripts for all the emscripten
python tools.

This tools makes copies or `run_python.sh/.bat` and `run_python_compiler.sh/.bat`
script for each entry point. On UNIX we previously used symbolic links for
simplicity but this breaks MINGW users on windows who want use the shell script
launcher but don't have symlink support.
"""

import os
import shutil
import sys

compiler_entry_points = '''
emcc
em++
'''.split()

entry_points = '''
emar
embuilder
emcmake
em-config
emconfigure
emmake
emranlib
emrun
emscons
emsize
tools/emdump
tools/file_packager
tests/runner
'''.split()


def main():
  tools_dir = os.path.dirname(os.path.abspath(__file__))
  root_dir = os.path.dirname(tools_dir)

  def generate_entry_points(cmd, path):
    sh_file = path + '.sh'
    bat_file = path + '.bat'
    for entry_point in cmd:
      dst = os.path.join(root_dir, entry_point)
      if os.path.exists(dst):
          os.remove(dst)
      shutil.copy2(sh_file, dst)

      dst += '.bat'
      if os.path.exists(dst):
          os.remove(dst)
      shutil.copy2(bat_file, dst)

  generate_entry_points(entry_points, os.path.join(tools_dir, 'run_python'))
  generate_entry_points(compiler_entry_points, os.path.join(tools_dir, 'run_python_compiler'))


if __name__ == '__main__':
  sys.exit(main())
