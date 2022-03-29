#!/usr/bin/env python3
# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Wrapper script around `llvm-strip`.
"""

import sys
from tools import shared

cmd = [shared.LLVM_STRIP] + sys.argv[1:]
sys.exit(shared.run_process(cmd, stdin=sys.stdin, check=False).returncode)
