#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Benchmark Emscripten compiler invocation speed across CI platforms.

Measures the elapsed time required to compile small source files (by default,
building libc via embuilder with batching and Ninja disabled so each C file is
invoked individually). Running this script before compiling the native launcher
benchmarks the Python driver baseline; running it after compiling benchmarks
the native C++ launcher.
"""

import argparse
import os
import subprocess
import sys
import time

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
sys.path.insert(0, root_dir)

from tools.utils import WINDOWS


def find_native_launcher():
  ext = '.exe' if WINDOWS else ''
  native_bin = os.path.join(root_dir, 'bin', 'emcc' + ext)
  if os.path.exists(native_bin):
    return native_bin
  return None


def run_benchmark(target='libc', cores=1, iterations=1):
  native_launcher = find_native_launcher()
  if native_launcher:
    mode = f'Native Launcher ({os.path.relpath(native_launcher, root_dir)})'
  else:
    mode = 'Python Launcher (baseline)'

  env = os.environ.copy()
  env['EMCC_CORES'] = str(cores)
  env['EMCC_USE_NINJA'] = '0'
  env['EMCC_BATCH_BUILD'] = '0'
  env.pop('EM_COMPILER_WRAPPER', None)
  if native_launcher:
    if 'EMCC_NATIVE' not in env:
      env['EMCC_NATIVE'] = '1'
  else:
    env.pop('EMCC_NATIVE', None)

  embuilder_py = os.path.join(root_dir, 'embuilder.py')
  cmd = [sys.executable, embuilder_py, 'build', target, '--force']

  print('=' * 60)
  print('Emscripten Compiler Benchmark')
  print('=' * 60)
  print(f'Mode:       {mode}')
  print(f'Target:     {target}')
  print(f'Iterations: {iterations}')
  print(f'Settings:   EMCC_CORES={cores}, EMCC_USE_NINJA=0, EMCC_BATCH_BUILD=0')
  print('=' * 60)

  times = []
  for i in range(1, iterations + 1):
    if iterations > 1:
      print(f'\n--- Iteration {i} of {iterations} ---')
    start_time = time.perf_counter()
    res = subprocess.run(cmd, env=env, check=False)
    elapsed = time.perf_counter() - start_time
    if res.returncode != 0:
      print(f'Error: benchmark command failed with exit code {res.returncode}')
      return res.returncode
    times.append(elapsed)
    print(f'Iteration {i} took: {elapsed:.3f} s')

  print('\n' + '=' * 60)
  print('Benchmark Summary')
  print('=' * 60)
  print(f'Mode: {mode}')
  if iterations == 1:
    print(f'Total Time: {times[0]:.3f} s')
  else:
    avg_time = sum(times) / len(times)
    min_time = min(times)
    max_time = max(times)
    print(f'Average Time: {avg_time:.3f} s')
    print(f'Min Time:     {min_time:.3f} s')
    print(f'Max Time:     {max_time:.3f} s')
  print('=' * 60)
  return 0


def main():
  parser = argparse.ArgumentParser(
      description='Benchmark Emscripten compiler invocation speed.',
  )
  parser.add_argument(
      'target',
      nargs='?',
      default='libc',
      help='Library target to build (default: libc)',
  )
  parser.add_argument(
      '--cores',
      type=int,
      default=1,
      help='Number of cores for EMCC_CORES (default: 1)',
  )
  parser.add_argument(
      '-n',
      '--iterations',
      type=int,
      default=1,
      help='Number of benchmark iterations to run (default: 1)',
  )
  args = parser.parse_args()

  return run_benchmark(
      target=args.target, cores=args.cores, iterations=args.iterations,
  )


if __name__ == '__main__':
  sys.exit(main())
