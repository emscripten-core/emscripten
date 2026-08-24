#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Synchronize emscripten dependency repositories (llvm-project and binaryen).

Discovers the revisions by parsing test/emsdk_version.txt in this repository
and fetching the corresponding DEPS file from the emscripten-releases repository.

Use this script if you want to build llvm and binaryn from source, and
want to keep them in sync with the versions implied by test/emsdk_version.txt.
"""

import argparse
import base64
import os
import re
import subprocess
import sys
import urllib.request

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))
sys.path.insert(0, root_dir)

from tools import utils

VERBOSE = False
DRY_RUN = False

# Hardcode the URLs here rather than depend on specific named remotes
# (e.g. origin vs upstream)
LLVM_URL = 'https://github.com/llvm/llvm-project.git'
BINARYEN_URL = 'https://github.com/WebAssembly/binaryen.git'


def get_emsdk_version(emsdk_version_file=None):
  if not emsdk_version_file:
    emsdk_version_file = utils.path_from_root('test', 'emsdk_version.txt')
  return utils.read_file(emsdk_version_file).strip()


def fetch_deps(emsdk_hash):
  url = (
      'https://chromium.googlesource.com/emscripten-releases/+/'
      f'{emsdk_hash}/DEPS?format=TEXT'
  )
  if VERBOSE:
    print(f'Fetching DEPS file from {url}...')
  with urllib.request.urlopen(url) as response:
    return base64.b64decode(response.read()).decode('utf-8')


def parse_deps(content):
  revisions = {}
  for key in ('llvm_project_revision', 'binaryen_revision'):
    pattern = rf"['\"]{re.escape(key)}['\"]\s*:\s*['\"]([0-9a-fA-F]{{7,40}})['\"]"
    m = re.search(pattern, content)
    if not m:
      utils.exit_with_error(f"Failed to parse '{key}' from DEPS file")
    revisions[key] = m.group(1)
  return revisions


def has_revision(repo_dir, revision):
  res = subprocess.run(
      ['git', '-C', repo_dir, 'rev-parse', '--verify', f'{revision}^{{commit}}'],
      stdout=subprocess.DEVNULL,
      stderr=subprocess.DEVNULL,
  )
  return res.returncode == 0


def get_current_revision(repo_dir):
  return subprocess.check_output(
      ['git', '-C', repo_dir, 'rev-parse', 'HEAD'], text=True,
  ).strip()


def is_dirty(repo_dir):
  return bool(subprocess.check_output(['git', '-C', repo_dir, 'status', '--porcelain'], text=True).strip())


def sync_repo(name, repo_dir, revision, url):
  if not os.path.isdir(repo_dir):
    utils.exit_with_error(
        f"Directory for {name} does not exist: '{repo_dir}'. "
        f"Use --{name}-dir to specify its location or --no-{name} to skip.",
    )

  print(f'Syncing {name} => {revision}')

  cur_rev = get_current_revision(repo_dir)
  if cur_rev.startswith(revision) or revision.startswith(cur_rev):
    print('  already at the requested revision')
    return

  if is_dirty(repo_dir):
    utils.exit_with_error("Directory for {name} is dirty: '{repo_dir}'")

  if not has_revision(repo_dir, revision):
    print('  Fetching revision')
    if not DRY_RUN:
      subprocess.check_call(['git', '-C', repo_dir, 'fetch', url, revision])

      if not has_revision(repo_dir, revision):
        utils.exit_with_error(
            f"Revision {revision} not found in '{repo_dir}' even after fetching",
        )

  print('  Checking out revision')
  if not DRY_RUN:
    subprocess.check_call(['git', '-C', repo_dir, 'checkout', revision])


def main():
  parent_dir = os.path.dirname(utils.path_from_root())
  default_llvm_dir = os.path.join(parent_dir, 'llvm-project')
  default_binaryen_dir = os.path.join(parent_dir, 'binaryen')

  parser = argparse.ArgumentParser(
      description='Sync emscripten dependency repos (llvm-project and binaryen) '
                  'to the revisions used in test/emsdk_version.txt.',
  )
  parser.add_argument('--llvm-dir', default=default_llvm_dir,
      help='Path to the llvm-project repo (default: %(default)s)',
  )
  parser.add_argument('--binaryen-dir', default=default_binaryen_dir,
      help='Path to the binaryen repo (default: %(default)s)',
  )
  parser.add_argument('-n', '--dry-run', action='store_true',
      help='Print what would be done without modifying repositories',
  )
  parser.add_argument('-v', '--verbose',
      action='store_true',
      help='Print verbose diagnostic output',
  )
  args = parser.parse_args()

  global VERBOSE, DRY_RUN
  VERBOSE = args.verbose
  DRY_RUN = args.dry_run

  emsdk_hash = get_emsdk_version()
  if VERBOSE or args.dry_run:
    print(f'Using emscripten-releases revision: {emsdk_hash}')
  deps_content = fetch_deps(emsdk_hash)

  revisions = parse_deps(deps_content)
  sync_repo('llvm-project', args.llvm_dir, revisions['llvm_project_revision'], LLVM_URL)
  sync_repo('binaryen', args.binaryen_dir, revisions['binaryen_revision'], BINARYEN_URL)
  print('Done.')


if __name__ == '__main__':
  main()
