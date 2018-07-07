#!/usr/bin/env python

# This script should work in python 2 *or* 3. It loads the main code using
# python_selector, which may pick a different version.
# It also tells emcc.py that we want C++ and not C by default

import os
import sys
from tools import python_selector

sys.argv += ['--emscripten-cxx']

emcc = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'emcc')
if __name__ == '__main__':
  python_selector.run(emcc)
