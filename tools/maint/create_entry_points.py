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
import sys
import stat

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(os.path.dirname(__scriptdir__))

compiler_entry_points = '''
emcc
em++
'''.split()

entry_points = '''
bootstrap
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
emdump
emprofile
emdwp
emnm
emstrip
emsymbolizer
system/bin/sdl-config
system/bin/sdl2-config
tools/file_packager
tools/webidl_binder
test/runner
'''.split()


# For some tools the entry point doesn't live alongside the python
# script.
entry_remap = {
  'emdump': 'tools/emdump',
  'emprofile': 'tools/emprofile',
  'emdwp': 'tools/emdwp',
  'emnm': 'tools/emnm',
}


def main():
  def generate_entry_points(cmd, path):
    sh_file = path + '.sh'
    bat_file = path + '.bat'
    ps1_file = path + '.ps1'
    with open(sh_file) as f:
      sh_file = f.read()
    with open(bat_file) as f:
      bat_file = f.read()
    with open(ps1_file) as f:
      ps1_file = f.read()

    for entry_point in cmd:
      sh_data = sh_file
      bat_data = bat_file
      ps1_data = ps1_file
      if entry_point in entry_remap:
        sh_data = sh_data.replace('$0', '$(dirname $0)/' + entry_remap[entry_point])
        bat_data = bat_data.replace('%~n0', entry_remap[entry_point].replace('/', '\\'))
        ps1_data = ps1_data.replace(r"$MyInvocation.MyCommand.Path -replace '\.ps1$', '.py'", fr'"$PSScriptRoot/{entry_remap[entry_point]}.py"')

      out_sh_file = os.path.join(__rootdir__, entry_point)
      with open(out_sh_file, 'w') as f:
        f.write(sh_data)
      os.chmod(out_sh_file, stat.S_IMODE(os.stat(out_sh_file).st_mode) | stat.S_IXUSR)

      with open(os.path.join(__rootdir__, entry_point + '.bat'), 'w') as f:
        f.write(bat_data)

      with open(os.path.join(__rootdir__, entry_point + '.ps1'), 'w') as f:
        f.write(ps1_data)

  generate_entry_points(entry_points, os.path.join(__scriptdir__, 'run_python'))
  generate_entry_points(compiler_entry_points, os.path.join(__scriptdir__, 'run_python_compiler'))


if __name__ == '__main__':
  sys.exit(main())
