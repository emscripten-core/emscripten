#!/usr/bin/env python2
"""This is a helper script for emmaken.py. See docs in that file for more info.
"""

from __future__ import print_function
import os
import sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

from tools.shared import Building  # noqa

print('\n\nemconfiguren.py is deprecated! use "emconfigure"\n\n', file=sys.stderr)

Building.configure(sys.argv[1:])
