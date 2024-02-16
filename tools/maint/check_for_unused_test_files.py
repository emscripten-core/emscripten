#!/usr/bin/env python3

"""Find files in the test/ that are not referenced and can
be deleted.  This is a work in progress and still contains
false positives in the output."""

import os
import sys
import fnmatch
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
test_dir = os.path.join(root_dir, 'test')
ignore_dirs = {
  'third_party',
  'metadce',
  'cmake',
  '__pycache__',
}
ignore_files = {
  'getValue_setValue_assert.out',
  'test_emsize.wasm',
  'legacy_exported_runtime_numbers_assert.out',
  'test_asyncify_during_exit_no_async.out',
}
ignore_root_patterns = ['runner.*', 'test_*.py']
ignore_root_files = {
  'check_clean.py',
  'jsrun.py',
  'clang_native.py',
  'common.py',
  'parallel_testsuite.py',
  'parse_benchmark_output.py',
  'malloc_bench.c',
}


def grep(string, subdir=''):
  cmd = subprocess.run(['git', 'grep', '--quiet', string, 'test/' + subdir], check=False)
  return not cmd.returncode


def check_file(dirpath, filename):
  normpath = os.path.normpath(os.path.join(dirpath, filename))
  relpath = os.path.relpath(normpath, test_dir)
  stem, ext = os.path.splitext(normpath)

  # Ignore explicit exceptions
  if dirpath == test_dir:
    if filename in ignore_root_files:
      return
    if any(fnmatch.fnmatch(filename, pattern) for pattern in ignore_root_patterns):
      return

  if os.path.basename(filename) in ignore_files:
    return

  # .out files are live if and only if they live alongside a live source file
  if ext == '.out' and os.path.exists(stem + '.cpp') or os.path.exists(stem + '.c'):
    return

  # Files under 'core' can be live if they are find in a `do_core_test` call.
  parts = relpath.split(os.path.sep)
  if parts[0] == 'core':
    pattern = "do_core_test('" + os.path.join(*parts[1:]) + "'"
    if grep(pattern, 'test_core.py'):
      return
    pattern = "'" + os.path.basename(stem) + "'"
    if grep(pattern, 'test_core.py'):
      return

  # Files under 'other' can be live if they are find in a `do_other_test` call.
  if parts[0] == 'other':
    pattern = "do_other_test('" + os.path.join(*parts[1:]) + "'"
    if grep(pattern, 'test_other.py'):
      return

  # Files under 'code_size' are live if the stem can be found quoted in test code.
  if parts[0] == 'code_size':
    if ext == '.json' and grep("'" + os.path.basename(stem) + "'"):
      return

  # test_asan builds it pathnames programatically based on the basename, so just
  # search for the basename.
  if filename.startswith('test_asan_'):
    relpath = os.path.basename(relpath)

  if grep(relpath):
    return

  print('Unreferenced: ' + os.path.relpath(normpath, root_dir))


def main(args):
  if len(args):
    for arg in args:
      check_file(test_dir, arg)
    return 0

  for dirpath, dirnames, filenames in os.walk(test_dir):
    if os.path.basename(dirpath) in ignore_dirs:
      dirnames.clear()
      filenames.clear()
      continue
    for filename in filenames:
      check_file(dirpath, filename)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
