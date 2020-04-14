#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool for creating/maintains the python launcher scripts for all the emscripten
python tools.

This tools makes copies or `run_python.sh` and `run_python.bat` script for each
entry point.  On UNIX we previously used symbolic links for simplicity but this
breaks MINGW users on windows who want use the shell script launcher but don't
have symlink support (sigh).
"""

import os
import shutil
import sys

entry_points = '''
emar
embuilder
emcc
emcmake
em-config
emconfigure
emmake
em++
emranlib
emrun
emscons
emsize
'''.split()


def main():
  tools_dir = os.path.dirname(os.path.abspath(__file__))
  root_dir = os.path.dirname(tools_dir)
  sh_file = os.path.join(tools_dir, 'run_python.sh')
  bat_file = os.path.join(tools_dir, 'run_python.bat')
  for entry_point in entry_points:
    if os.path.exists(os.path.join(root_dir, entry_point)):
        os.remove(os.path.join(root_dir, entry_point))
    shutil.copy2(sh_file, os.path.join(root_dir, entry_point))
    shutil.copy2(bat_file, os.path.join(root_dir, entry_point) + '.bat')


if __name__ == '__main__':
  sys.exit(main())
