#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Builds the emscripten website from source and creates a new commit & branch
in the emscripten-site repository containing the changes."""

import os
import subprocess
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
site_dir = os.path.join(root_dir, 'site')


def check_git_clean(dirname):
  if subprocess.check_output(['git', 'status', '-uno', '--porcelain'], text=True, cwd=dirname).strip():
    print(f'{dirname}: tree is not clean')
    sys.exit(1)


def main(args):
  if args:
    site_out = args[0]
  else:
    site_out = os.path.join(os.path.dirname(root_dir), 'emscripten-site')

  assert os.path.isdir(site_out)
  print(f'Updating docs in: {site_out}')
  check_git_clean(site_out)
  check_git_clean(root_dir)

  # Ensure the -site checkout is up-to-date
  subprocess.check_call(['git', 'fetch', 'origin'], cwd=site_out)
  subprocess.check_call(['git', 'checkout', 'origin/gh-pages'], cwd=site_out)

  # Build and install the docs
  subprocess.check_call(['make', 'install', f'EMSCRIPTEN_SITE={site_out}'], cwd=site_dir)

  # Create a new branch and commit the changes.
  subprocess.check_call(['git', 'checkout', '-b', 'update'], cwd=site_out)
  subprocess.check_call(['git', 'add', '.'], cwd=site_out)

  hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], text=True, cwd=root_dir).strip()
  message = 'Update emscripten website\n\n'
  message += f'These docs were generated based on git revision {hash}'
  subprocess.run(['git', 'commit', '-F', '-'], input=message, text=True, cwd=site_out)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
