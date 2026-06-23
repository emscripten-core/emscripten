#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Helper script to generate GitHub release notes using ChangeLog.md.

It uses the `gh` CLI to fetch default release notes (to get the contributor list
and changelog link), parses ChangeLog.md for the actual release description,
and combines them into a final release notes draft.
"""

import argparse
import json
import os
import re
import subprocess
import sys
from collections import Counter

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

BOTS = {'dependabot[bot]', 'emscripten-bot', 'github-actions[bot]', 'web-flow'}
REPO = 'emscripten-core/emscripten'


def get_changelog_entry(changelog_path, version):
  if not os.path.exists(changelog_path):
    print(f'Error: Changelog not found at {changelog_path}', file=sys.stderr)
    return None

  with open(changelog_path, encoding='utf-8') as f:
    lines = f.readlines()

  start_idx = -1
  end_idx = -1

  for i, line in enumerate(lines):
    line = line.strip()
    # Match version followed by space, dash, or end of string
    if re.match(rf'^{re.escape(version)}(?:\s|$|-)', line):
      # A header is followed by a line of dashes
      if i + 1 < len(lines) and re.match(r'^--+$', lines[i + 1].strip()):
        start_idx = i + 2
        break

  if start_idx == -1:
    return None

  for i in range(start_idx, len(lines)):
    line = lines[i].strip()
    if i + 1 < len(lines) and re.match(r'^--+$', lines[i + 1].strip()):
      end_idx = i
      break
  else:
    end_idx = len(lines)

  entry_lines = lines[start_idx:end_idx]
  while entry_lines and not entry_lines[0].strip():
    entry_lines.pop(0)
  while entry_lines and not entry_lines[-1].strip():
    entry_lines.pop()

  return ''.join(entry_lines)


def get_latest_version(changelog_path):
  if not os.path.exists(changelog_path):
    return None
  with open(changelog_path, encoding='utf-8') as f:
    lines = f.readlines()
  for i, line in enumerate(lines):
    line = line.strip()
    # A header is followed by a line of dashes
    if i + 1 < len(lines) and re.match(r'^--+$', lines[i + 1].strip()):
      # Skip "in development" versions
      if '(in development)' in line:
        continue
      match = re.match(r'^([0-9.]+)', line)
      if match:
        return match.group(1)
  return None


def get_previous_version(changelog_path, version):
  if not os.path.exists(changelog_path):
    return None

  with open(changelog_path, encoding='utf-8') as f:
    lines = f.readlines()

  # Find the header for the target version
  target_idx = -1
  for i, line in enumerate(lines):
    line = line.strip()
    if re.match(rf'^{re.escape(version)}(?:\s|$|-)', line):
      if i + 1 < len(lines) and re.match(r'^--+$', lines[i + 1].strip()):
        target_idx = i
        break

  if target_idx == -1:
    return None

  # Find the next header after the target version
  for i in range(target_idx + 2, len(lines)):
    line = lines[i].strip()
    if i + 1 < len(lines) and re.match(r'^--+$', lines[i + 1].strip()):
      match = re.match(r'^([0-9.]+)', line)
      if match:
        return match.group(1)
  return None


def generate_default_notes(tag, previous_tag):
  cmd = [
      'gh', 'api',
      f'/repos/{REPO}/releases/generate-notes',
      '-f', f'tag_name={tag}',
      '-f', f'previous_tag_name={previous_tag}',
  ]
  result = subprocess.run(cmd, capture_output=True, text=True, check=True)
  return json.loads(result.stdout)


def parse_default_notes(body):
  sections = {}
  current_header = None
  current_content = []

  # Parse the default notes into sections based on H2 headers (##)
  for line in body.split('\n'):
    if line.startswith('## '):
      if current_header:
        sections[current_header] = '\n'.join(current_content).strip()
      current_header = line[3:].strip()
      current_content = []
    elif line.startswith('**Full Changelog**'):
      if current_header:
        sections[current_header] = '\n'.join(current_content).strip()
      current_header = 'Full Changelog'
      current_content = [line]
    else:
      current_content.append(line)

  if current_header:
    sections[current_header] = '\n'.join(current_content).strip()

  return sections


def extract_pr_authors(whats_changed_content):
  if not whats_changed_content:
    return Counter()
  # Match "by @username"
  matches = re.findall(r'\sby\s@([a-zA-Z0-9-]+(?:\[bot\])?)', whats_changed_content)
  return Counter(matches)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('version', nargs='?', help='Version to generate notes for (e.g. 6.0.0). Defaults to the latest entry in ChangeLog.md.')
  args = parser.parse_args()

  changelog_path = os.path.join(root_dir, 'ChangeLog.md')

  version = args.version
  if not version:
    version = get_latest_version(changelog_path)
    if not version:
      print(f'Error: Could not detect latest version from {changelog_path}', file=sys.stderr)
      return 1

  # 1. Get Changelog Entry
  changelog_entry = get_changelog_entry(changelog_path, version)
  if changelog_entry is None:
    print(f'Error: Could not find changelog entry for version {version} in {changelog_path}', file=sys.stderr)
    return 1

  # 2. Detect Previous Tag
  previous_tag = get_previous_version(changelog_path, version)
  if not previous_tag:
    print(f'Error: Could not detect previous version for {version} from ChangeLog.md.', file=sys.stderr)
    return 1

  # 3. Generate Default Notes from GitHub
  default_notes_json = generate_default_notes(version, previous_tag)
  default_body = default_notes_json.get('body', '')

  # 4. Parse Default Notes
  default_sections = parse_default_notes(default_body)

  # 5. Extract Contributors
  whats_changed = default_sections.get('What\'s Changed', '')
  new_contributors_content = default_sections.get('New Contributors', '')

  pr_authors_counts = extract_pr_authors(whats_changed)
  contributors = {user: count for user, count in pr_authors_counts.items() if user not in BOTS}

  # 6. Assemble Final Notes
  notes = []
  notes.append('## What\'s Changed')
  notes.append(changelog_entry)
  notes.append('')

  if contributors:
    notes.append('## Contributors')
    sorted_contributors = sorted(
        contributors.items(),
        key=lambda item: (-item[1], item[0].lower()),
    )
    mentions = ' '.join(f'@{user} ({count})' for user, count in sorted_contributors)
    notes.append(f'Thanks to the following contributors: {mentions}')
    notes.append('')

  if new_contributors_content:
    notes.append('## New Contributors')
    notes.append(new_contributors_content)
    notes.append('')

  full_changelog = default_sections.get('Full Changelog')
  if full_changelog:
    notes.append(full_changelog)

  # 7. Output Final Notes
  print('\n'.join(notes))
  return 0


if __name__ == '__main__':
  sys.exit(main())
