#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import re
import sys
import shutil
import subprocess

from update_common import *

local_root = os.path.join(script_dir, 'libcxx')
local_src = os.path.join(local_root, 'src')
local_inc = os.path.join(local_root, 'include')
local_modules = os.path.join(local_root, 'modules')

preserve_files = ('readme.txt', '__assertion_handler', '__config_site')
# ryu_long_double_constants.h from libc is unused (and very large)
excludes = {'ryu_long_double_constants.h'}

libc_copy_dirs = [
    'hdr',
    'include/llvm-libc-macros',
    'include/llvm-libc-types',
    'shared',
    'config',
]


def generate_modules(cmake_version: str):
  dst = os.path.join(local_modules, 'prebuilt')
  lib = 'lib/emscripten'
  share = 'share/libc++/v1'

  build_dir = f'{emscripten_root}/out/libcxx/modules'
  binary_dir = f'{build_dir}/out'
  dist = f'{binary_dir}/dist/'

  clean_dir(build_dir, preserve_files)
  os.makedirs(build_dir, exist_ok=True)
  with open(f'{build_dir}/CMakeLists.txt', 'x', encoding='utf-8') as CMakeLists:
    CMakeLists.write(f'''\
      cmake_minimum_required(VERSION {cmake_version})
      project(libcxx-modules)
      add_subdirectory("{local_modules}" libcxx)
    ''')

  vars = [
    ('LIBCXX_INSTALL_LIBRARY_DIR',  lib),
    ('LIBCXX_INSTALL_MODULES_DIR',  share),
    ('LIBCXX_LIBRARY_DIR',          dist + lib),
    ('LIBCXX_GENERATED_MODULE_DIR', dist + share),
  ]

  cmd = ['cmake', '-B', binary_dir, '-S', build_dir]
  cmd += [f'-D{key}={val}' for key, val in vars]

  subprocess.run(cmd, stdout=subprocess.DEVNULL)
  subprocess.run(['cmake', '--build', binary_dir], stdout=subprocess.DEVNULL)
  shutil.copytree(dist, dst, dirs_exist_ok=True)

def main():
  llvm_dir, force = parse_args(default_llvm_dir, 'llvm_dir')
  check_clean(force)
  # Exit early if cmake is not found
  try:
    output= subprocess.check_output(['cmake', '--version'], text=True)
  except OSError:
    print('CMake not found', file=sys.stderr)
    sys.exit(1)

  cmake_version = re.search(r'^cmake version (\d+(?:\.\d+)*)', output).group(1)

  llvm_dir = get_llvm_dir()
  libcxx_dir = os.path.join(llvm_dir, 'libcxx')
  upstream_src = os.path.join(libcxx_dir, 'src')
  upstream_inc = os.path.join(libcxx_dir, 'include')
  upstream_modules = os.path.join(libcxx_dir, 'modules')
  assert os.path.exists(upstream_inc)
  assert os.path.exists(upstream_src)
  assert os.path.exists(upstream_modules)

  # Remove old version
  clean_dir(local_src, preserve_files)
  clean_dir(local_inc, preserve_files)
  clean_dir(local_modules, preserve_files)

  copy_tree(upstream_src, local_src, excludes)
  copy_tree(upstream_inc, local_inc, excludes)
  copy_tree(upstream_modules, local_modules, excludes)
  generate_modules(cmake_version)

  shutil.copy2(os.path.join(libcxx_dir, 'CREDITS.TXT'), local_root)
  shutil.copy2(os.path.join(libcxx_dir, 'LICENSE.TXT'), local_root)

  # We don't use frozen c++03 headers for now
  shutil.rmtree(os.path.join(local_inc, '__cxx03'))

  # libcxx includes headers from LLVM's libc
  libc_upstream_dir = os.path.join(llvm_dir, 'libc')
  assert os.path.exists(libc_upstream_dir)
  libc_local_dir = os.path.join(script_dir, 'llvm-libc')

  for dirname in libc_copy_dirs:
    local_dir = os.path.join(libc_local_dir, dirname)
    clean_dir(local_dir, preserve_files)

  for dirname in libc_copy_dirs:
    upstream_dir = os.path.join(libc_upstream_dir, dirname)
    local_dir = os.path.join(libc_local_dir, dirname)
    copy_tree(upstream_dir, local_dir, excludes)

  shutil.copy2(os.path.join(libc_upstream_dir, 'LICENSE.TXT'), libc_local_dir)
  update_readme(local_root, llvm_dir)

if __name__ == '__main__':
  main()
