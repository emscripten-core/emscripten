#!/usr/bin/env python3
"""Find unused settings from settings.js and settings_internal.js.
"""

import os
import subprocess
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.insert(0, root_dir)

# This avoiding including `LEGACY_SETTINGS`
os.environ['EMCC_STRICT'] = '1'

from tools.settings import settings


def main():
  print(f'Searching {len(settings.attrs)} settings')
  for key in settings.attrs:
    cmd = ['git', 'grep', '-q', f'\\<{key}\\>',  ':(exclude)src/settings.js', ':(exclude)src/settings_internal.js']
    print('CHECKING:', key)
    # git grep returns 0 if there is a match and non-zero when there is not
    if subprocess.run(cmd, check=False).returncode:
      print('NOT FOUND: ', key)


if __name__ == '__main__':
  sys.exit(main())
