#!/usr/bin/env python3
# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import argparse
import os
import subprocess
import sys
from datetime import datetime

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.insert(0, root_dir)
from tools import utils

REPO = 'emscripten-core/emscripten'


def get_repo_url():
  # Pick correct URL for pushing to REPO (emscripten-core/emscripten).
  remotes = subprocess.check_output(['git', 'remote', '-v'], text=True, cwd=root_dir)
  for line in remotes.splitlines():
    _name, url, url_type = line.split()
    if REPO in url and url_type == '(push)':
      return url

  # If we can't find a URL to push to then just abort
  raise Exception(f'Unable to determine remote URL for {REPO}')


def update_version_txt(release_version, new_version):
  version_file = os.path.join(root_dir, 'emscripten-version.txt')
  old_content = utils.read_file(version_file)
  utils.write_file(version_file, old_content.replace(release_version, new_version))


def update_changelog(release_version, new_version):
  changelog_file = os.path.join(root_dir, 'ChangeLog.md')
  changelog = utils.read_file(changelog_file)
  marker = f'{release_version} (in development)'
  pos = changelog.find(marker)
  assert pos != -1
  pos += len(marker) + 1
  # Skip the next line which should just be hyphens
  assert changelog[pos] == '-'
  pos = changelog.find('\n', pos)
  assert pos != -1

  # Add new entry
  today = datetime.now().strftime('%m/%d/%y')
  new_entry = f'{release_version} - {today}'
  new_entry = '\n\n' + new_entry + '\n' + ('-' * len(new_entry))
  changelog = changelog[:pos] + new_entry + changelog[pos:]

  # Update the "in development" entry
  changelog = changelog.replace(f'{release_version} (in development)', f'{new_version} (in development)')

  utils.write_file(changelog_file, changelog)


def create_release_pr(release_version, dry_run):
  branch_name = 'version_' + release_version

  print(f'Creating new branch {branch_name}')
  subprocess.check_call(['git', 'checkout', '-b', branch_name], cwd=root_dir)

  # Create auto-generated changes to the new git branch
  subprocess.check_call(['git', 'add', '-u', '.'], cwd=root_dir)
  subprocess.check_call(['git', 'commit', '-m', f'Mark {release_version} as released'], cwd=root_dir)
  print('New release created in branch: `%s`' % branch_name)

  if not dry_run:
    # Push the new branch to emscripten repo and create PR
    repo_url = get_repo_url()
    subprocess.check_call(['git', 'push', repo_url, branch_name], cwd=root_dir)
    subprocess.check_call(['gh', 'pr', 'create', '--fill', '--reviewer', 'release-reviewers', '--head', branch_name, '-R', REPO], cwd=root_dir)


def create_draft_release(version, base_commit):
  print(f'Creating draft release for {version}...', file=sys.stderr)

  # Call gen_release_notes.py
  gen_script = os.path.join(script_dir, 'gen_release_notes.py')
  notes = subprocess.check_output([sys.executable, gen_script, version], text=True)

  # Save notes to out/release-notes-{version}.md
  out_dir = os.path.join(root_dir, 'out')
  os.makedirs(out_dir, exist_ok=True)
  notes_file = os.path.join(out_dir, f'release-notes-{version}.md')
  with open(notes_file, 'w', encoding='utf-8') as f:
    f.write(notes)

  # Create the draft release
  cmd = [
      'gh', 'release', 'create', version,
      '--draft',
      '--title', version,
      '--notes-file', notes_file,
      '-R', REPO,
  ]
  if base_commit:
    cmd.extend(['--target', base_commit])
  subprocess.run(cmd, check=True)
  print(f'Draft release created successfully! Notes saved to {notes_file}', file=sys.stderr)
  return 0


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-n', '--dry-run', action='store_true', help='Dry run (do not push branch/tag/release)')
  parser.add_argument('--release-commit', help='Base commit/SHA for the release')
  args = parser.parse_args()

  if subprocess.check_output(['git', 'status', '-uno', '--porcelain'], cwd=root_dir).strip():
    print('tree is not clean')
    return 1

  utils.set_version_globals()

  release_version = [utils.EMSCRIPTEN_VERSION_MAJOR, utils.EMSCRIPTEN_VERSION_MINOR,
                     utils.EMSCRIPTEN_VERSION_TINY]
  new_dev_version = list(release_version)
  new_dev_version[2] += 1

  release_version = '.'.join(str(v) for v in release_version)
  new_dev_version = '.'.join(str(v) for v in new_dev_version)

  update_version_txt(release_version, new_dev_version)
  update_changelog(release_version, new_dev_version)

  print('Creating new release: %s' % release_version)

  create_release_pr(release_version, args.dry_run)

  if not args.dry_run:
    create_draft_release(release_version, args.release_commit)

  return 0


if __name__ == '__main__':
  sys.exit(main())
