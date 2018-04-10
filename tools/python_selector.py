'''
Provides a way to run a script on the preferred Python version
'''

import os
import sys

'''
Imports the target script by filename and calls run()
'''
def run_by_import(filename, main):
  import importlib
  return getattr(importlib.import_module(os.path.basename(filename)), main)()

'''
Opens Python 2 subprocess to run target script
'''
def run_by_subprocess(filename):
  # Python on Windows does not provide `python2` but instead `py` that receives version parameter
  py2 = ['py', '-2'] if sys.platform.startswith('win') else ['python2']
  import subprocess
  return subprocess.run(py2 + [os.path.realpath(filename) + '.py'] + sys.argv[1:]).returncode

def on_allowed_version():
  # we now allow python 2 or 3. eventually, we will only allow 3, and can change
  # this. when we do that, also note that in shared.get_building_env() we'll need
  # to specify the proper python to be executed, and need to do that carefully.
  return True

'''
Runs filename+'.py' by opening Python 2 subprocess if required, or by importing.
'''
def run(filename, profile=False, main="run"):
  if profile:
    from tools.toolchain_profiler import ToolchainProfiler
    ToolchainProfiler.record_process_start()

  sys.exit(run_by_import(filename, main) if on_allowed_version() else run_by_subprocess(filename))
