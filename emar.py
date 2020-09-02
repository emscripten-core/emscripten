#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Archive helper script

This script acts as a frontend replacement for `ar`. See emcc.
This is needed because, unlike a traditional linker, emscripten can't handle
archive with duplicate member names.  This is because emscripten extracts
archive to a temporary location and duplicate filenames will clobber each
other in this case.
"""

# TODO(sbc): Implement `ar x` within emscripten, in python, to avoid this issue
# and delete this file.

from __future__ import print_function
import sys

from tools.toolchain_profiler import ToolchainProfiler
from tools import shared

if __name__ == '__main__':
  ToolchainProfiler.record_process_start()


#
# Main run() function
#
def run():
  # The wasm backend doesn't suffer from the same problem as fastcomp so it
  # doesn't need the filename hashing.
  cmd = [shared.LLVM_AR] + sys.argv[1:]
  return shared.run_process(cmd, stdin=sys.stdin, check=False).returncode


if __name__ == '__main__':
  sys.exit(run())
