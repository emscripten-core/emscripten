#!/usr/bin/env python3
# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from datetime import datetime
import os
import subprocess
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.insert(0, root_dir)
from tools import utils


def main(argv):
  if subprocess.check_output(['git', 'status', '-uno', '--porcelain'], cwd=root_dir).strip():
    print('tree is not clean')
    return 1

  is_github_runner = len(argv) > 1 and argv[1] == '--action'

  utils.set_version_globals()

  release_version = [utils.EMSCRIPTEN_VERSION_MAJOR, utils.EMSCRIPTEN_VERSION_MINOR,
                     utils.EMSCRIPTEN_VERSION_TINY]
  new_dev_version = list(release_version)
  new_dev_version[2] += 1

  release_version = '.'.join(str(v) for v in release_version)
  new_dev_version = '.'.join(str(v) for v in new_dev_version)

  print('Creating new release: %s' % release_version)

  version_file = os.path.join(root_dir, 'emscripten-version.txt')
  changelog_file = os.path.join(root_dir, 'ChangeLog.md')

  old_content = utils.read_file(version_file)
  utils.write_file(version_file, old_content.replace(release_version, new_dev_version))

  changelog = utils.read_file(changelog_file)
  marker = f'{release_version} (in development)'
  pos = changelog.find(marker)
  assert pos != -1
  pos += 2 * len(marker) + 1

  # Add new entry
  today = datetime.now().strftime('%m/%d/%y')
  new_entry = f'{release_version} - {today}'
  new_entry = '\n\n' + new_entry + '\n' + ('-' * len(new_entry))
  changelog = changelog[:pos] + new_entry + changelog[pos:]

  # Update the "in development" entry
  changelog = changelog.replace(f'{release_version} (in development)', f'{new_dev_version} (in development)')

  utils.write_file(changelog_file, changelog)

  branch_name = 'version_' + release_version

  if is_github_runner: # For GitHub Actions workflows
    with open(os.environ['GITHUB_ENV'], 'a') as f:
      f.write(f'RELEASE_VERSION={release_version}')
  else: # Local use
    # Create a new git branch
    subprocess.check_call(['git', 'checkout', '-b', branch_name, 'upstream/main'], cwd=root_dir)

    # Create auto-generated changes to the new git branch
    subprocess.check_call(['git', 'add', '-u', '.'], cwd=root_dir)
    subprocess.check_call(['git', 'commit', '-m', f'Mark {release_version} as released'], cwd=root_dir)
    print('New release created in branch: `%s`' % branch_name)

    # Push new branch to upstream
    subprocess.check_call(['git', 'push', 'upstream', branch_name], cwd=root_dir)

  # TODO(sbc): Maybe create the tag too
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
