#!/usr/bin/env python3
"""Search the test directory for un-referenced files.

This might be slow because it runs a `git grep` for every
file in the test directory.
"""

import os
import sys
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

ignore_files = {'runner.bat'}


def clear_line():
  if sys.stdout.isatty():
    sys.stdout.write('\x1b[2K\r')


def main():
  all_files = subprocess.check_output(['git', 'ls-files', 'test'], encoding='utf-8', cwd=root_dir).split('\n')
  for i, filename in enumerate(all_files):
    if sys.stdout.isatty():
      clear_line()
      sys.stdout.write(f'checking [{i}/{len(all_files)}] ({filename})')
    sys.stdout.flush()
    dirname, basename = os.path.split(filename)
    if basename in ignore_files:
      continue
    ext = os.path.splitext(filename)[1]
    if basename.startswith('test_') and ext == '.py':
      continue
    lookfor = basename
    if ext == '.out':
      lookfor = os.path.splitext(basename)[1]
    rtn = subprocess.call(['git', 'grep', '--quiet', lookfor, 'test'], cwd=root_dir)
    if rtn != 0:
      if sys.stdout.isatty():
        clear_line()
      print(f'Not found: {basename}')


if __name__ == '__main__':
  sys.exit(main())
