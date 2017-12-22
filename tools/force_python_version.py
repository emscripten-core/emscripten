'''
Provides a way to run a script on the preferred Python version
'''

import os
import sys

'''
Runs filename+'.py' on Python 2 subprocess if required, or returns False
'''
def run(filename):
  major = sys.version_info.major
  if major == 2:
    return False
  if os.environ.get('EMSCRIPTEN_ALLOW_NEWER_PYTHON'):
    import logging
    from tools import colored_logger
    logging.warning('Running on Python %s which is not officially supported yet', major)
    return False

  # Python on Windows does not provide `python2` but instead `py` that receives version parameter
  py2 = ['py', '-2'] if sys.platform.startswith('win') else ['python2']
  import subprocess
  sys.exit(subprocess.call(py2 + [os.path.realpath(filename) + '.py'] + sys.argv[1:]))
