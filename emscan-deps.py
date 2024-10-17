#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emscan-deps - clang-scan-deps helper script

This script acts as a frontend replacement for clang-scan-deps
"""

import sys
from tools import shared, cache

args = sys.argv[1:]
args.append('--sysroot=' + cache.get_sysroot(absolute=True))
shared.exec_process([shared.CLANG_SCAN_DEPS] + args)
