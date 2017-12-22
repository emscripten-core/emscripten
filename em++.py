#!/usr/bin/env python

# This script should work in python 2 *or* 3. It loads emcc.py, which needs python 2.
# It also tells emcc.py that we want C++ and not C by default

import sys

sys.argv += ['--emscripten-cxx']

import emcc
if __name__ == '__main__':
  emcc.run()
