#!/usr/bin/env python3
# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emscan-deps - clang-scan-deps helper script

This script acts as a frontend replacement for clang-scan-deps.
"""

import sys
import emcc
from tools import shared, cmdline

argv = sys.argv[1:]

# Parse and discard any emcc-specific flags (e.g. -sXXX).
newargs = cmdline.parse_arguments(argv)[1]

# Add any clang flags that emcc would add.
newargs += emcc.get_cflags(tuple(argv))

shared.exec_process([shared.CLANG_SCAN_DEPS] + newargs)
