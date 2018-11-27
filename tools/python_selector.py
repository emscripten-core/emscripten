# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Provides a way to run a script on the preferred Python version"""

import os
import subprocess
import sys


def run(filename):
  """Runs filename+'.py' in subprocess of the correct python version
  """
  if os.environ.get('EM_PROFILE_TOOLCHAIN'):
    from tools.toolchain_profiler import ToolchainProfiler
    ToolchainProfiler.record_process_start()

  # In the future we might froce a particular python version here.
  # Python on Windows does not provide `python2` but instead `py` that receives version parameter
  # python = ['py', '-2'] if sys.platform.startswith('win') else ['python2']
  python = sys.executable
  sys.exit(subprocess.call([python, os.path.realpath(filename) + '.py'] + sys.argv[1:]))
