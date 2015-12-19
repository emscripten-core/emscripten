#!/usr/bin/env python

# This script should work in python 2 *or* 3. It loads emcc.py, which needs python 2.
# It also tells emcc.py that we want C++ and not C by default

import sys

sys.argv += ['--emscripten-cxx']

if sys.version_info.major == 2:
  import emcc
  if __name__ == '__main__':
    emcc.run()
else:
  import os, subprocess
  if __name__ == '__main__':
    sys.exit(subprocess.call(['python2', os.path.join(os.path.dirname(__file__), 'emcc.py')] + sys.argv[1:]))
