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
  ('npm packages', ['package.json'], [shutil.which('npm'), 'ci']),
  ('create entry points', [
     'tools/maint/create_entry_points.py',
     'tools/maint/run_python.bat',
     'tools/maint/run_python.sh',
     'tools/maint/run_python.ps1',
   ],
   [sys.executable, 'tools/maint/create_entry_points.py']),
  ('git submodules', ['test/third_party/posixtestsuite/'], [shutil.which('git'), 'submodule', 'update', '--init']),
]


def get_stamp_file(action_name):
  return os.path.join(STAMP_DIR, action_name.replace(' ', '_') + '.stamp')


def check_deps(name, deps):
  stamp_file = get_stamp_file(name)
  if not os.path.exists(stamp_file):
    return False
  for dep in deps:
    dep = utils.path_from_root(dep)
    if os.path.getmtime(dep) > os.path.getmtime(stamp_file):
      return False
  return True


def check():
  for name, deps, _ in actions:
    if not check_deps(name, deps):
      utils.exit_with_error(f'emscripten setup is not complete ("{name}" is out-of-date). Run `bootstrap` to update')


def main(args):
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose', default=False)
  parser.add_argument('-n', '--dry-run', action='store_true', help='dry run', default=False)
  parser.add_argument('-i', '--install-post-checkout', action='store_true', help='install post checkout script', default=False)
  args = parser.parse_args()

  if args.install_post_checkout:
    if not os.path.exists(utils.path_from_root('.git')):
      print('--install-post-checkout requires git checkout')
      return 1

    src = utils.path_from_root('tools/maint/post-checkout')
    dst = utils.path_from_root('.git/hooks')
    if not os.path.exists(dst):
      os.mkdir(dst)
    shutil.copy(src, dst)
    return 0

  for name, deps, cmd in actions:
    if check_deps(name, deps):
      print('Up-to-date: %s' % name)
      continue
    print('Out-of-date: %s' % name)
    stamp_file = get_stamp_file(name)
    if args.dry_run:
      print(' (skipping: dry run) -> %s' % ' '.join(cmd))
      continue
    print(' -> %s' % ' '.join(cmd))
    shared.run_process(cmd, cwd=utils.path_from_root())
    utils.safe_ensure_dirs(STAMP_DIR)
    utils.write_file(stamp_file, 'Timestamp file created by bootstrap.py')
  return 0


if __name__ == '__main__':
  main(sys.argv[1:])
