#!/usr/bin/env python3

import json
import os
import sys
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(os.path.dirname(script_dir))

sys.path.append(root_dir)

import emscripten
from tools.settings import settings


def check_structs():
  for struct in info['structs'].keys():
    key = 'C_STRUCTS.' + struct + '.'
    # grep --quiet ruturns 0 when there is a match
    if subprocess.run(['git', 'grep', '--quiet', key], check=False).returncode != 0:
      print(struct)


def check_defines():
  for define in info['defines'].keys():
    key = 'cDefine(.' + define + '.)'
    # grep --quiet ruturns 0 when there is a match
    if subprocess.run(['git', 'grep', '--quiet', key], check=False).returncode != 0:
      print(define)


emscripten.generate_struct_info()
info = json.loads(open(settings.STRUCT_INFO).read())
check_structs()
check_defines()
