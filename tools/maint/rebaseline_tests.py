#!/usr/bin/env python3
# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Automatically rebaseline tests that have codesize expectations.

This script can also create a git commit containing the resulting changes
along with some details of the changes.
"""

import argparse
import json
import os
import statistics
import subprocess
import sys
import urllib.request

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.insert(0, root_dir)
from tools import utils


def run(cmd, **args):
  return subprocess.check_output(cmd, text=True, cwd=root_dir, **args)


all_deltas = []


def read_size_from_json(content):
  json_data = json.loads(content)
  if 'total' in json_data:
    return json_data['total']
  # If `total` if not in the json dict then just use the first key.  This happens when only one
  # file size is reported (in this case we don't calculate or store the `total`).
  first_key = list(json_data.keys())[0]
  return json_data[first_key]


def get_installed_emsdk_sha():
  emsdk = os.environ.get('EMSDK')
  if not emsdk:
    return None
  version_file = os.path.join(emsdk, 'upstream', '.emsdk_version')
  if not os.path.exists(version_file):
    return None
  version = utils.read_file(version_file).strip()
  parts = version.split('-')
  if len(parts) < 2 or len(parts[1]) != 40:
    utils.exit_with_error(f'failed to parse .emsdk_version: {version}')
  return parts[1]


def format_emsdk_version_update(old_sha, new_sha, limit=30):
  assert old_sha != 'none'
  assert old_sha != new_sha
  log_url = f'https://chromium.googlesource.com/emscripten-releases/+log/{old_sha}..{new_sha}'
  url = f'{log_url}?format=JSON&n={limit + 1}'
  data = urllib.request.urlopen(url, timeout=5).read().decode('utf-8')
  if data.startswith(")]}'"):
    data = data[4:]
  j = json.loads(data)
  entries = j.get('log', [])
  lines = [f'emsdk version updated: {old_sha} => {new_sha}']
  lines.append('')
  lines.append('The following revisions were included in this update:')
  lines.append('')
  lines.append('```')
  for entry in entries[:limit]:
    short_sha = entry['commit'][:8]
    subject = entry['message'].splitlines()[0]
    lines.append(f'- {short_sha} {subject}')
  if len(entries) > limit:
    lines.append('  ... (and more, see full log)')
  lines.append('```')
  lines.append("")
  lines.append(f"Full log: {log_url}")

  return '\n'.join(lines)


def process_changed_file(filename):
  content = utils.read_file(filename)
  old_content = run(['git', 'show', f'HEAD:{filename}'])
  print(f'processing {filename}')

  ext = os.path.splitext(filename)[1]
  if ext == '.size':
    size = int(content.strip())
    old_size = int(old_content.strip())
  elif ext == '.json':
    size = read_size_from_json(content)
    old_size = read_size_from_json(old_content)
  else:
    # Unhandled file type
    return f'{filename} updated\n'

  filename = filename.removeprefix('test/')
  delta = size - old_size
  percent_delta = delta * 100 / old_size
  all_deltas.append(percent_delta)
  return f'{filename}: {old_size} => {size} [{delta:+} bytes / {percent_delta:+.2f}%]\n'


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-s', '--skip-tests', action='store_true', help="Don't actually run the tests, just analyze the existing results")
  parser.add_argument('-b', '--new-branch', action='store_true', help='Create a new branch containing the updates')
  parser.add_argument('-c', '--clear-cache', action='store_true', help='Clear the cache before rebaselining (useful when working with llvm changes)')
  parser.add_argument('-n', '--check-only', dest='check_only', action='store_true', help='Return non-zero if test expectations are out of date, and skip creating a git commit')
  parser.add_argument('--update-emsdk', action='store_true', help='Update test/emsdk_version.txt to match the currently installed emsdk version')
  args = parser.parse_args()

  if args.clear_cache:
    run(['./emcc', '--clear-cache'])

  current_sha = None
  installed_sha = None
  if not args.skip_tests:
    if not args.check_only and run(['git', 'status', '-uno', '--porcelain']).strip():
      print('tree is not clean')
      return 1

    installed_sha = get_installed_emsdk_sha()
    emsdk_version_file = os.path.join(root_dir, 'test', 'emsdk_version.txt')
    current_sha = utils.read_file(emsdk_version_file).strip()
    if args.update_emsdk:
      if not installed_sha:
        utils.exit_with_error('--update-emsdk specified but could not detect installed emsdk version')
      if installed_sha == current_sha:
        print(f'emsdk version is up-to-date ({current_sha})')
        return 0
      print(f'update emsdk: {current_sha} => {installed_sha}')
      utils.write_file(emsdk_version_file, installed_sha + '\n')
    elif installed_sha and installed_sha != current_sha:
      utils.exit_with_error(f'installed emsdk version ({installed_sha}) does not match test/emsdk_version.txt ({current_sha}). Pass --update-emsdk to update it.')

    if installed_sha:
      installed_sha = installed_sha[:8]
    current_sha = current_sha[:8]

    subprocess.check_call([utils.exe_path_from_root('test/runner'), '--rebaseline', 'codesize'], cwd=root_dir)

  output = run(['git', 'status', '-uno', '--porcelain'])
  filenames = []
  for line in output.splitlines():
    filename = line.strip().rsplit(' ', 1)[1]
    if filename.startswith('test') and os.path.isfile(filename) and filename != 'test/emsdk_version.txt':
      filenames.append(filename)

  if not filenames and not args.update_emsdk:
    print('test expectations are up-to-date')
    return 0

  if args.check_only:
    message = 'Test expectations are out-of-date\n'
  elif args.update_emsdk:
    message = f'''Update emsdk version from ${current_sha} => ${installed_sha}

This is an automatic change generated by tools/maint/rebaseline_tests.py --update-emsdk.

'''
    message += format_emsdk_version_update(current_sha, installed_sha) + '\n'
  else:
    message = '''Automatic rebaseline of codesize expectations. NFC

This is an automatic change generated by tools/maint/rebaseline_tests.py.
'''

  if filenames:
    message += f'''
The following ({len(filenames)}) test expectation files were updated by
running the tests with `--rebaseline`:

```
'''
    for file in sorted(filenames):
      message += process_changed_file(file)

    if all_deltas:
      message += f'\nAverage change: {statistics.mean(all_deltas):+.2f}% ({min(all_deltas):+.2f}% - {max(all_deltas):+.2f}%)\n'

    message += '```\n'

  print(message)
  if args.check_only:
    return 1

  if args.new_branch:
    branch_name = 'update_emsdk' if args.update_emsdk else 'rebaseline_tests'
    run(['git', 'checkout', '-b', branch_name])
  run(['git', 'add', '-u', '.'])
  run(['git', 'commit', '-F', '-'], input=message)

  return 2


if __name__ == '__main__':
  sys.exit(main())
