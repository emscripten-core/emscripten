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
a given musl version.  Once any merge conflict are resolved those
change can then be copied back into emscripten using this script.
"""

import os
import sys
import shutil

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
  'arm', 'x32', 'sh', 'i386', 'x86_64', 'aarch64', 'riscv64',
  's390x', 'mips', 'mips64', 'mipsn32', 'powerpc', 'powerpc64',
  'm68k', 'microblaze', 'or1k')
exclude_files = (
  'aio.h',
  'sendfile.h',
  'auxv.h',
  'personality.h',
  'klog.h',
  'fanotify.h',
  'vt.h',
  'swap.h',
  'reboot.h',
  'quota.h',
  'kd.h',
  'io.h',
  'fsuid.h',
  'epoll.h',
  'inotify.h',
  'timerfd.h',
  'timex.h',
  'cachectl.h',
  'soundcard.h',
  'eventfd.h',
  'signalfd.h',
  'ptrace.h',
  'prctl.h',
)


if len(sys.argv) > 1:
  musl_dir = os.path.abspath(sys.argv[1])
else:
  musl_dir = default_musl_dir


def should_ignore(name):
  return name in exclude_dirs or name[0] == '.' or name in exclude_files


def ignore(dirname, contents):
  return [c for c in contents if should_ignore(c)]


def main():
  assert os.path.exists(musl_dir)

  # Remove old version
  shutil.rmtree(local_src)

  # Copy new version into place
  shutil.copytree(musl_dir, local_src, ignore=ignore)

  # Create version.h
  version = open(os.path.join(local_src, 'VERSION')).read().strip()
  with open(os.path.join(local_src, 'src', 'internal', 'version.h'), 'w') as f:
    f.write('#define VERSION "%s"\n' % version)


if __name__ == '__main__':
  main()
