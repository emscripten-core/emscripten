#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool for creating/maintaining the python launcher scripts for emscripten tools.

Note: This tool is deprecated and being replaced by emcc_native (built via `bootstrap.py`).

This tool makes copies of `run_python.sh/.bat` and `run_python_compiler.sh/.bat`
script for each entry point. On UNIX we previously used symbolic links for
simplicity but this breaks MINGW users on windows who want to use the shell script
launcher but don't have symlink support.
"""

import os
import stat
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(os.path.dirname(__scriptdir__))

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
emprofile
emdwp
emnm
emstrip
emsymbolizer
emscan-deps
empath-split
tools/file_packager
tools/webidl_binder
test/runner
'''.split()


# For some tools the entry point doesn't live alongside the python
# script.
entry_remap = {
  'emprofile': 'tools/emprofile',
  'emdwp': 'tools/emdwp',
  'emnm': 'tools/emnm',
  'emsymbolizer': 'tools/emsymbolizer',
  'empath-split': 'tools/empath-split',
}


def make_executable(filename):
  old_mode = stat.S_IMODE(os.stat(filename).st_mode)
  os.chmod(filename, old_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def maybe_remove(filename):
  if os.path.lexists(filename):
    os.remove(filename)


def read_file(filename):
  with open(filename, encoding='utf-8') as f:
    return f.read()


def write_file(filename, content):
  maybe_remove(filename)
  with open(filename, 'w', encoding='utf-8') as f:
    f.write(content)


def main(all_platforms):
  print('warning: create_entry_points.py is deprecated and being replaced by emcc_native', file=sys.stderr)
  is_windows = sys.platform.startswith('win')
  is_msys2 = 'MSYSTEM' in os.environ
  do_unix = all_platforms or not is_windows or is_msys2
  do_windows = all_platforms or is_windows

  def generate_entry_points(cmd, path):
    sh_file = path + '.sh'
    bat_file = path + '.bat'
    ps1_file = path + '.ps1'
    sh_file = read_file(sh_file)
    bat_file = read_file(bat_file)
    ps1_file = read_file(ps1_file)

    for entry_point in cmd:
      sh_data = sh_file
      bat_data = bat_file
      ps1_data = ps1_file
      if entry_point in entry_remap:
        sh_data = sh_data.replace('$0', '$(dirname $0)/' + entry_remap[entry_point])
        bat_data = bat_data.replace('%~n0', entry_remap[entry_point].replace('/', '\\'))
        ps1_data = ps1_data.replace(r"$MyInvocation.MyCommand.Path -replace '\.ps1$', '.py'", fr'"$PSScriptRoot/{entry_remap[entry_point]}.py"')

      launcher = os.path.join(__rootdir__, entry_point)
      if do_unix:
        write_file(launcher, sh_data)
        make_executable(launcher)

      if do_windows:
        write_file(launcher + '.bat', bat_data)
        write_file(launcher + '.ps1', ps1_data)

  generate_entry_points(entry_points, os.path.join(__scriptdir__, 'run_python'))
  generate_entry_points(compiler_entry_points, os.path.join(__scriptdir__, 'run_python_compiler'))


if __name__ == '__main__':
  sys.exit(main('--all' in sys.argv))
