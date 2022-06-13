#!/usr/bin/env python3

"""Find references to closed github issues in the source code.

Depends on the `gh` command line tool being installed."""

import json
import os
import subprocess
import re
import sys


script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))


def run(*args, **kwargs):
  kwargs['universal_newlines'] = True
  return subprocess.check_output(*args, **kwargs)


def is_closed(issue_number):
  output = run(['gh', 'issue', 'view', '--json', 'state', issue_number])
  state = json.loads(output)['state']
  assert state in ['OPEN', 'CLOSED']
  return state == 'CLOSED'


def main():
  output = run(['git', 'grep', '-I', '-P', r'github.com/.*/issues/\d'], cwd=root_dir)
  # For now, limit output of lines that have TODO or disabled in them.
  matcher = re.compile(r'(TODO|disabled).*(https?://.*github.com/.*/issues/\d+)')
  for line in output.splitlines():
    match = matcher.search(line)
    if not match:
      continue
    issue = match[2]
    if is_closed(issue):
      print('Reference to closed github issue (#%s):' % issue)
      print(line)


if __name__ == '__main__':
  sys.exit(main())
