#!/usr/bin/env python2

from __future__ import print_function
import sys
print('\n\nemconfiguren.py is deprecated! use "emconfigure"\n\n', file=sys.stderr)

'''
This is a helper script for emmaken.py. See docs in that file for more info.
'''

import os, sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
from tools.shared import *

Building.configure(sys.argv[1:])

