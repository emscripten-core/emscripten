#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Wrapper script around `llvm-ar`.
"""

import sys
from tools import shared

shared.exec_process([shared.LLVM_AR] + sys.argv[1:])
