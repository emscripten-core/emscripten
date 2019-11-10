#!/usr/bin/env python2
# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

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
