#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emranlib - ranlib helper script

This script acts as a frontend replacement for ranlib, and simply invokes
llvm-ranlib internally.
"""

import sys
from tools import shared

shared.exec_process([shared.LLVM_RANLIB] + sys.argv[1:])
