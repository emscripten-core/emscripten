#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil
import subprocess


script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')

# system/lib/llvm-openmp (to be updated)
local_root = os.path.join(script_dir, 'llvm-openmp')
local_src = os.path.join(local_root, 'src')
local_prebuilt = os.path.join(local_root, 'prebuilt')

# Files to ignore during copy_tree
excludes = [
  'doc',
  'build',
  'tests',
  'CMakeFiles',
  'libgomp.a',
  'libiomp5.a',
  'libomp.a',
]


def clean_dir(dirname):
  for f in os.listdir(dirname):
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      shutil.rmtree(full)
    else:
      os.remove(full)


def copy_tree(upstream_dir, local_dir):
  for f in os.listdir(upstream_dir):
    full = os.path.join(upstream_dir, f)
    if f not in excludes:
      if os.path.isdir(full):
        if not os.path.exists(os.path.join(local_dir, f)):
          os.makedirs(os.path.join(local_dir, f))
        copy_tree(full, os.path.join(local_dir, f))
      else:
        shutil.copy2(full, os.path.join(local_dir, f))


def main():
  if len(sys.argv) > 1:
    llvm_dir = os.path.join(os.path.abspath(sys.argv[1]))
  else:
    llvm_dir = default_llvm_dir

  # Output directory for build
  output_dir = os.path.join(emscripten_root, 'out')
  build_dir = os.path.join(output_dir, 'build_openmp')

  # LLVM/OpenMP folder containing latest version
  upstream_runtimes = os.path.join(llvm_dir, 'runtimes/')
  upstream_root = os.path.join(llvm_dir, 'openmp/')
  upstream_runtime_root = os.path.join(upstream_root, 'runtime/src')
  assert os.path.exists(upstream_runtime_root)

  # Build output paths
  upstream_build_src = os.path.join(build_dir, 'openmp/runtime/src') # contains various *.a and generated *.h

  # Remove old version
  clean_dir(local_root)
  os.mkdir(local_src)
  os.mkdir(local_prebuilt)

  # Update source
  copy_tree(upstream_runtime_root, local_src)

  # Generates header files for OpenMP library build
  subprocess.run(
    [
      'emcmake',
      'cmake',
      '-S',
      f'{upstream_runtimes}',
      '-B',
      f'{build_dir}',
      '-G',
      'Ninja',
      '-DLLVM_ENABLE_RUNTIMES=openmp',
      f'-DLLVM_BINARY_DIR={os.environ.get("LLVM_ROOT", "")}',
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
    ]
  )
  subprocess.run(['cmake', '--build', '.'], cwd=build_dir)

  # Update license file
  shutil.copy2(os.path.join(upstream_root, 'LICENSE.TXT'), local_root)

  # Update generated header files
  built_files = ['omp.h', 'ompx.h', 'kmp_config.h', 'kmp_i18n_id.inc',  'kmp_i18n_default.inc']
  for file in built_files:
    shutil.copy2(os.path.join(upstream_build_src, file), local_prebuilt)


if __name__ == '__main__':
  main()
