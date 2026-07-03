#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Simple script for updating musl from external git repo.

The upstream sources, along with our local changes, live at:

  https://github.com/emscripten-core/musl

To update musl first make sure all changes from the emscripten repo
are present in the `emscripten` branch of the above repo.  Then run
`git merge v<musl_version>` to pull in the latest musl changes from
a given musl version.  Once any merge conflicts are resolved those
changes can then be copied back into emscripten using this script.
"""

import os
import sys
import shutil

from pathlib import Path

script_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.join(script_dir, 'libc', 'musl')
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_musl_dir = os.path.join(os.path.dirname(emscripten_root), 'musl')
exclude_dirs = (
  # Top level directories we don't include
  'tools', 'obj', 'lib', 'crt', 'musl', 'compat',
  # Parts of src we don't build
  'malloc',
  # Arch-specific code we don't use
  'aarch64', 'arm', 'i386', 'loongarch64', 'm68k',
  'microblaze', 'mips', 'mips64', 'mipsn32', 'or1k',
  'powerpc', 'powerpc64', 'riscv32', 'riscv64', 's390x',
  'sh', 'x32', 'x86_64'
)
exclude_files = (
  'aio.h',
  'auxv.h',
  'cachectl.h',
  'eventfd.h',
  'fanotify.h',
  'fsuid.h',
  'inotify.h',
  'io.h',
  'kd.h',
  'klog.h',
  'personality.h',
  'prctl.h',
  'ptrace.h',
  'quota.h',
  'reboot.h',
  'sendfile.h',
  'signalfd.h',
  'soundcard.h',
  'stdint.h',
  'swap.h',
  'timerfd.h',
  'timex.h',
  'vt.h',
)
# Allowed files even if they match exclude rules above
allowed_files = (
  Path('include/stdint.h'),
)


if len(sys.argv) > 1:
  musl_dir = os.path.abspath(sys.argv[1])
else:
  musl_dir = default_musl_dir
if not os.path.isdir(musl_dir):
  print(f'musl directory not found: {musl_dir}', file=sys.stderr)
  print(f'Usage: {sys.argv[0]} [musl_dir]', file=sys.stderr)
  sys.exit(1)


def make_ignore(root):
  root = Path(root).resolve()

  def ignore(directory, contents):
    directory = Path(directory)

    ignored = []

    for name in contents:
      rel_path = (directory / name).relative_to(root)

      if (name.startswith('.') or
          name in exclude_dirs or
          name in exclude_files) and \
          rel_path not in allowed_files:
        ignored.append(name)

    return ignored

  return ignore


def main():
  assert os.path.exists(musl_dir)

  # Remove old version
  shutil.rmtree(local_src)

  # Copy new version into place
  shutil.copytree(musl_dir, local_src, ignore=make_ignore(musl_dir))

  # Create version.h
  version = open(os.path.join(local_src, 'VERSION')).read().strip()
  with open(os.path.join(local_src, 'src', 'internal', 'version.h'), 'w') as f:
    f.write('#define VERSION "%s"\n' % version)


if __name__ == '__main__':
  main()
