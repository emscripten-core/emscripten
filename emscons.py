#!/usr/bin/env python3
"""Wrapping the scons invocation, EMSCRIPTEN_TOOL_PATH is set in the process
environment, and can be used to locate the emscripten SCons Tool.

Example:

# Load emscripten Tool
my_env = Environment(tools=['emscripten'], toolpath=[os.environ['EMSCRIPTEN_TOOL_PATH']])
"""

import os
import subprocess
import sys
from tools import building, utils

tool_path = utils.path_from_root('tools/scons/site_scons/site_tools/emscripten')
building_env = building.get_building_env()

env = os.environ.copy()
env['EMSCRIPTEN_TOOL_PATH'] = tool_path
env['EMSCRIPTEN_ROOT'] = utils.path_from_root()
env['EMSCONS_PKG_CONFIG_LIBDIR'] = building_env['PKG_CONFIG_LIBDIR']
env['EMSCONS_PKG_CONFIG_PATH'] = building_env['PKG_CONFIG_PATH']

sys.exit(subprocess.call(sys.argv[1:], env=env))
