#!/usr/bin/env python3

"""Find files in the tests/ that are not referenced and can
be deleted.  This is a work in progress and still contains
false positives in the output."""

import os
import sys
import fnmatch
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
test_dir = os.path.join(root_dir, 'tests')
ignore_dirs = set([
  'third_party',
  'metadce',
  '__pycache__',
])
ignore_root_patterns = ['runner.*', 'test_*.py']
ignore_root_files = set([
  'jsrun.py',
  'clang_native.py',
  'common.py',
  'parallel_testsuite.py',
  'parse_benchmark_output.py',
  'malloc_bench.c',
])


def grep(string, subdir=''):
  cmd = subprocess.run(['git', 'grep', '--quiet', string, 'tests/' + subdir], check=False)
  return not cmd.returncode


def check_file(dirpath, filename):
  normpath = os.path.normpath(os.path.join(dirpath, filename))
  relpath = os.path.relpath(normpath, test_dir)
  stem, ext = os.path.splitext(normpath)

  if dirpath == test_dir:
    if filename in ignore_root_files:
      return
    if any(fnmatch.fnmatch(filename, pattern) for pattern in ignore_root_patterns):
      return

  parts = relpath.split(os.path.sep)
  if parts[0] == 'core':
    pattern = "do_core_test('" + os.path.join(*parts[1:]) + "'"
    if grep(pattern, 'test_core.py'):
      return
    pattern = "'" + os.path.basename(stem) + "'"
    if grep(pattern, 'test_core.py'):
      return

  if parts[0] == 'other':
    pattern = "do_other_test('" + os.path.join(*parts[1:]) + "'"
    if grep(pattern, 'test_other.py'):
      return

  if parts[0] == 'code_size':
    if ext == '.json' and grep("'" + os.path.basename(stem) + "'"):
      return

  if ext == '.out' and os.path.exists(stem + '.cpp') or os.path.exists(stem + '.c'):
    return

  if grep(relpath):
    return

  print('Unreferenced: ' + os.path.relpath(normpath, root_dir))


def main(args):
  if len(args):
    for arg in args:
      check_file(test_dir, arg)
    return 0

  for (dirpath, dirnames, filenames) in os.walk(test_dir):
    if os.path.basename(dirpath) in ignore_dirs:
      dirnames.clear()
      filenames.clear()
      continue
    for filename in filenames:
      check_file(dirpath, filename)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
