#!/usr/bin/env python2
"""Wrapping the scons invocation, EMSCRIPTEN_TOOL_PATH is set in the process
environment, and can be used to locate the emscripten SCons Tool.

Example:

# Load emscripten Tool
my_env = Environment(tools=['emscripten'], toolpath=[os.environ['EMSCRIPTEN_TOOL_PATH']])
"""

import os
import subprocess
import sys
from tools import shared

tool_path = os.path.join(shared.path_from_root('tools'), 'scons', 'site_scons', 'site_tools', 'emscripten')

env = os.environ.copy()
env['EMSCRIPTEN_TOOL_PATH'] = tool_path

sys.exit(subprocess.call(sys.argv[1:], env=env))
