#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Builds the emscripten website from source and creates a new commit & branch
in the emscripten-site repository containing the changes."""

import os
import sys
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
site_dir = os.path.join(root_dir, 'site')


def main(args):
  if args:
    site_out = args[0]
  else:
    site_out = os.path.join(os.path.dirname(root_dir), 'emscripten-site')
  assert os.path.exists(site_out)

  output = subprocess.check_output(['git', 'status', '--short'], cwd=site_out)

  print(f'Updating docs in: {site_out}')

  # Ensure the -site checkout is up-to-date and clean.
  output = subprocess.check_output(['git', 'status', '--short'], cwd=site_out)
  output = output.decode('utf-8').strip()
  if output:
    print('Site tree is not clean')
    return 1

  subprocess.check_call(['git', 'fetch', 'origin'], cwd=site_out)
  subprocess.check_call(['git', 'checkout', 'origin/gh-pages'], cwd=site_out)

  # Build and install the docs
  subprocess.check_call(['make', 'install', f'EMSCRIPTEN_SITE={site_out}'], cwd=site_dir)

  # Create a new branch and commit the changes.
  subprocess.check_call(['git', 'checkout', '-b', 'update'], cwd=site_out)
  subprocess.check_call(['git', 'add', '.'], cwd=site_out)
  subprocess.check_call(['git', 'commit', '-mupdate'], cwd=site_out)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
