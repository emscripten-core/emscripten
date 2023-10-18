#!/usr/bin/env python3
"""Bootstrap script for emscripten developers / git users.

After checking out emscripten there are certain steps that need to be
taken before it can be used.  This script enumerates and automates
these steps and is able to run just the steps that are needed based
on the timestamps of various input files (kind of like a dumb version
of a Makefile).
"""
import argparse
import os
import shutil
import sys

__rootdir__ = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, __rootdir__)

STAMP_DIR = os.path.join(__rootdir__, 'out')

from tools import shared, utils

actions = [
  ('npm packages', 'package.json', [shutil.which('npm'), 'ci']),
  # TODO(sbc): Remove the checked in entry point files and have them
  # built on demand by this step.
  ('create entry points', 'tools/maint/create_entry_points.py', [sys.executable, 'tools/maint/create_entry_points.py']),
  ('git submodules', 'test/third_party/posixtestsuite/', [shutil.which('git'), 'submodule', 'update', '--init']),
]


def get_stamp_file(action_name):
  return os.path.join(STAMP_DIR, action_name.replace(' ', '_') + '.stamp')


def check():
  for name, filename, _ in actions:
    stamp_file = get_stamp_file(name)
    filename = utils.path_from_root(filename)
    if not os.path.exists(stamp_file) or os.path.getmtime(filename) > os.path.getmtime(stamp_file):
      utils.exit_with_error(f'emscripten setup is not complete ("{name}" is out-of-date). Run bootstrap.py to update')


def main(args):
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose', default=False)
  parser.add_argument('-n', '--dry-run', action='store_true', help='dry run', default=False)
  args = parser.parse_args()

  for name, filename, cmd in actions:
    stamp_file = get_stamp_file(name)
    filename = utils.path_from_root(filename)
    if os.path.exists(stamp_file) and os.path.getmtime(filename) <= os.path.getmtime(stamp_file):
      print('Up-to-date: %s' % name)
      continue
    print('Out-of-date: %s' % name)
    if args.dry_run:
      print(' (skipping: dry run) -> %s' % ' '.join(cmd))
      return
    print(' -> %s' % ' '.join(cmd))
    shared.run_process(cmd, cwd=utils.path_from_root())
    utils.safe_ensure_dirs(STAMP_DIR)
    utils.write_file(stamp_file, 'Timestamp file created by bootstrap.py')


if __name__ == '__main__':
  main(sys.argv[1:])
