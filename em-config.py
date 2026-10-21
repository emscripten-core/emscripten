#!/usr/bin/env python3
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Display emscripten configure file settings.

Helper script which is designed to make it possible for
other apps to read emscripten's configuration variables
in a unified way.  Usage:

  em-config VAR_NAME

This tool prints the value of the variable to stdout if one
is found, or exits with 1 if the variable does not exist.
"""

import sys

from tools import config, utils


def get_config_setting(key):
  # Special case for EMSCRIPTEN_ROOT, which is not part of `config`
  if key == 'EMSCRIPTEN_ROOT':
    return utils.path_from_root()

  if key not in config.CONFIG_KEYS:
    print(f'Invalid config key: {key}', file=sys.stderr)
    print('Valid keys:')
    for k in config.CONFIG_KEYS:
      print(f'  {k}', file=sys.stderr)
    sys.exit(1)

  return getattr(config, key)


def main():
  if len(sys.argv) != 2:
    print('Usage: em-config VAR_NAME', file=sys.stderr)
    sys.exit(1)

  key = sys.argv[1]
  value = get_config_setting(key)
  print(value)
  return 0


if __name__ == '__main__':
  sys.exit(main())
