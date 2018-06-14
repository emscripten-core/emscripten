#!/usr/bin/env python2
"""See emmaken.py
"""
import os
import subprocess
import sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

from tools import shared  # noqa

emmaken = shared.path_from_root('tools', 'emmaken.py')
os.environ['EMMAKEN_CXX'] = '1'
sys.exit(subprocess.call([shared.PYTHON, emmaken] + sys.argv[1:]))
