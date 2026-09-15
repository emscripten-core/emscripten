#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool for creating/maintaining the python launcher scripts for emscripten tools.

This tool creates symbolic links (on UNIX) or executables/batch scripts (on Windows)
for each entry point.
"""

import os
import platform
import shutil
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


windows_exe = os.path.join(__rootdir__, 'tools/pylauncher/pylauncher.exe')
if platform.machine().lower() in {'arm64', 'aarch64'}:
  windows_exe = os.path.join(__rootdir__, 'tools/pylauncher/pylauncher-arm64.exe')


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


def main(all_platforms, use_bat_file):
  if 'EM_USE_BAT_FILES' in os.environ:
    use_bat_file = True
  is_windows = sys.platform.startswith('win')
  is_msys2 = 'MSYSTEM' in os.environ
  do_unix = all_platforms or not is_windows or is_msys2
  do_windows = all_platforms or is_windows

  def generate_entry_points(cmd, name):
    sh_file_path = os.path.join(__rootdir__, 'tools', name + '.sh')
    bat_file_path = os.path.join(__scriptdir__, name + '.bat')
    ps1_file_path = os.path.join(__scriptdir__, name + '.ps1')
    sh_file = read_file(sh_file_path)
    bat_file = read_file(bat_file_path)
    ps1_file = read_file(ps1_file_path)

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
        if entry_point in entry_remap:
          write_file(launcher, sh_data)
          make_executable(launcher)
        else:
          target = os.path.relpath(sh_file_path, os.path.dirname(launcher))
          maybe_remove(launcher)
          os.symlink(target, launcher)

      if do_windows:
        maybe_remove(launcher + '.ps1')
        maybe_remove(launcher + '.exe')
        if use_bat_file:
          write_file(launcher + '.bat', bat_data)
          write_file(launcher + '.ps1', ps1_data)
        else:
          shutil.copyfile(windows_exe, launcher + '.exe')

  generate_entry_points(entry_points, 'run_python')
  generate_entry_points(compiler_entry_points, 'run_python_compiler')


if __name__ == '__main__':
  sys.exit(main('--all' in sys.argv, '--bat-files' in sys.argv))
