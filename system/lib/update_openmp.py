#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import subprocess

from update_common import (
  clean_dir,
  copy_tree,
  default_llvm_dir,
  emscripten_root,
  parse_args,
  script_dir,
)

# system/lib/openmp (to be updated)
local_root = os.path.join(script_dir, 'openmp')
local_src = os.path.join(local_root, 'src')
local_include = os.path.join(local_root, 'include')
local_prebuilt = os.path.join(local_root, 'prebuilt')

# Files to ignore during copy_tree
excludes = (
    'doc',
    'build',
    'tests',
    'CMakeFiles',
    'libgomp.a',
    'libiomp5.a',
    'libomp.a',
)


def main():
  llvm_dir = parse_args(default_llvm_dir, 'llvm_dir')

  # Output directory for build
  output_dir = os.path.join(emscripten_root, 'out')
  build_dir = os.path.join(output_dir, 'build_openmp')

  # LLVM/OpenMP folder containing latest version
  upstream_runtimes = os.path.join(llvm_dir, 'runtimes')
  upstream_root = os.path.join(llvm_dir, 'openmp')
  upstream_runtime_root = os.path.join(upstream_root, 'runtime/src')
  assert os.path.exists(upstream_runtime_root)

  # Build output paths
  upstream_build_src = os.path.join(build_dir, 'openmp/runtime/src') # contains various *.a and generated *.h

  # Remove old version
  clean_dir(local_src)
  clean_dir(local_include)
  clean_dir(local_prebuilt)

  # Update source
  copy_tree(upstream_runtime_root, local_src, excludes)

  # Generates header files for OpenMP library build
  subprocess.run([
      os.path.join(emscripten_root, 'emcmake'), 'cmake',
      '-S', upstream_runtimes,
      '-B', build_dir,
      '-G', 'Ninja',
      '-DLLVM_ENABLE_RUNTIMES=openmp',
      '-DLLVM_DEFAULT_TARGET_TRIPLE=wasm32-unknown-emscripten',
      '-DOPENMP_ENABLE_LIBOMPTARGET=OFF',
      '-DLIBOMP_HAVE_OMPT_SUPPORT=OFF',
      '-DLIBOMP_OMPT_SUPPORT=OFF',
      '-DLIBOMP_OMPD_SUPPORT=OFF',
      '-DLIBOMP_USE_DEBUGGER=OFF',
      '-DLIBOMP_FORTRAN_MODULES=OFF',
      '-DLIBOMP_ENABLE_SHARED=OFF',
      '-DLIBOMP_ARCH=wasm32',
      '-DOPENMP_ENABLE_LIBOMPTARGET_PROFILING=OFF',
  ], check=True)
  subprocess.run(['cmake', '--build', '.'], cwd=build_dir, check=True)

  # Update license file
  shutil.copy2(os.path.join(upstream_root, 'LICENSE.TXT'), local_root)

  # Update include headers
  header_files = ['omp.h', 'ompx.h']
  for file in header_files:
    shutil.copy2(os.path.join(upstream_build_src, file), local_include)

  # Update generated header files
  built_files = ['kmp_config.h', 'kmp_i18n_id.inc', 'kmp_i18n_default.inc']
  for file in built_files:
    shutil.copy2(os.path.join(upstream_build_src, file), local_prebuilt)


if __name__ == '__main__':
  main()
