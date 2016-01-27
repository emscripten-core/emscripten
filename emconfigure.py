#!/usr/bin/env python2

'''
This is a helper script. It runs ./configure (or cmake,
etc.) for you, setting the environment variables to use
emcc and so forth. Usage:

  emconfigure ./configure [FLAGS]

You can also use this for cmake and other configure-like
stages. What happens is that all compilations done during
this command are to native code, not JS, so that configure
tests will work properly.

Relevant defines:

  CONFIGURE_CC - see emcc
'''

import os, sys
from tools import shared
from subprocess import CalledProcessError

#
# Main run() function
#
def run():
  if len(sys.argv) < 2 or ('configure' not in sys.argv[1] and 'cmake' not in sys.argv[1]):
    print >> sys.stderr, '''
  emconfigure is a helper for configure, setting various environment
  variables so that emcc etc. are used. Typical usage:

    emconfigure ./configure [FLAGS]

  (but you can run any command instead of configure)

  '''
  elif 'cmake' in sys.argv[1]:
    node_js = shared.NODE_JS
    if type(node_js) is list: node_js = node_js[0]
    node_js = shared.Building.which(node_js)
    node_js = node_js.replace('"', '\"')
    sys.argv = sys.argv[:2] + ['-DCMAKE_CROSSCOMPILING_EMULATOR="' + node_js +'"'] + sys.argv[2:]

  try:
    shared.Building.configure(sys.argv[1:])
  except CalledProcessError, e:
    sys.exit(e.returncode)

if __name__ == '__main__':
  run()
