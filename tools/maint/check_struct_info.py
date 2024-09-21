#!/usr/bin/env python3

"""Find entries in struct_info.json that are not needd by
any JS library code and can be removed."""

import json
import os
import sys
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.insert(0, root_dir)

from tools import utils


def check_structs(info):
  for struct, values in info['structs'].items():
    key = 'C_STRUCTS\\.' + struct + '\\.'
    # grep --quiet ruturns 0 when there is a match
    if subprocess.run(['git', 'grep', '--quiet', key], check=False).returncode != 0:
      print(key)
    else:
      for value in values:
        if value != '__size__':
          key = 'C_STRUCTS\\.' + struct + '\\.' + value
          # grep --quiet ruturns 0 when there is a match
          if subprocess.run(['git', 'grep', '--quiet', key], check=False).returncode != 0:
            print(key)


def check_defines(info):
  for define in info['defines'].keys():
    key = r'cDefs\.' + define + r'\>'
    # grep --quiet ruturns 0 when there is a match
    if subprocess.run(['git', 'grep', '--quiet', key], check=False).returncode != 0:
      print(define)


def main():
  json_file = utils.path_from_root('src/struct_info_generated.json')
  info = json.loads(utils.read_file(json_file))
  check_structs(info)
  check_defines(info)
  return 0


if __name__ == '__main__':
  sys.exit(main())
