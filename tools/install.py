#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Install the parts of emscripten needed for end users. This works like
a traditional `make dist` target but is written in python so it can be portable
and run on non-unix platforms (basically windows).
"""

import argparse
import fnmatch
import logging
import os
import shutil
import subprocess
import sys

EXCLUDES = [os.path.normpath(x) for x in '''
test/third_party
tools/maint
site
node_modules
Makefile
.git
cache
cache.lock
bootstrap.py
'''.split()]

EXCLUDE_PATTERNS = '''
*.pyc
.*
__pycache__
'''.split()

logger = logging.getLogger('install')


def add_revision_file(target):
  # text=True would be better than encoding here, but it's only supported in 3.7+
  git_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD'], encoding='utf-8').strip()
  with open(os.path.join(target, 'emscripten-revision.txt'), 'w') as f:
    f.write(git_hash + '\n')


def copy_emscripten(target):
  script_dir = os.path.dirname(os.path.abspath(__file__))
  emscripten_root = os.path.dirname(script_dir)
  os.chdir(emscripten_root)
  for root, dirs, files in os.walk('.'):
    # Handle the case where the target directory is underneath emscripten_root
    if os.path.abspath(root) == os.path.abspath(target):
      dirs.clear()
      continue

    remove_dirs = []
    for d in dirs:
      if d in EXCLUDE_PATTERNS:
        remove_dirs.append(d)
        continue
      fulldir = os.path.normpath(os.path.join(root, d))
      if fulldir in EXCLUDES:
        remove_dirs.append(d)
        continue
      os.makedirs(os.path.join(target, fulldir))

    for d in remove_dirs:
      # Prevent recursion in excluded dirs
      logger.debug('skipping dir: ' + os.path.join(root, d))
      dirs.remove(d)

    for f in files:
      if any(fnmatch.fnmatch(f, pat) for pat in EXCLUDE_PATTERNS):
        logger.debug('skipping file: ' + os.path.join(root, f))
        continue
      full = os.path.normpath(os.path.join(root, f))
      if full in EXCLUDES:
        logger.debug('skipping file: ' + os.path.join(root, f))
        continue
      logger.debug('installing file: ' + os.path.join(root, f))
      shutil.copy2(full, os.path.join(target, root, f), follow_symlinks=False)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose',
                      default=int(os.environ.get('EMCC_DEBUG', '0')))
  parser.add_argument('target', help='target directory')
  args = parser.parse_args()
  target = os.path.abspath(args.target)
  if os.path.exists(target):
    print('target directory already exists: %s' % target)
    return 1
  logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)
  os.makedirs(target)
  copy_emscripten(target)
  if os.path.isdir('.git'):
    # Add revision flag only if the source directory is a Git repository
    # and not a source archive
    add_revision_file(target)
  return 0


if __name__ == '__main__':
  sys.exit(main())
