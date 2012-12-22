#!/usr/bin/env python2

'''
see emmaken.py
'''

import os, subprocess, sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
from tools.shared import *

emmaken = path_from_root('tools', 'emmaken.py')
os.environ['EMMAKEN_CXX'] = '1'
exit(subprocess.call([PYTHON, emmaken] + sys.argv[1:]))

