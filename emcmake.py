#!/usr/bin/env python2

import os, subprocess, sys
from tools import shared

#
# Main run() function
#
def run():
  configure_path = shared.path_from_root('emconfigure')

  exit(subprocess.call([shared.PYTHON, configure_path] + sys.argv[1:]))

if __name__ == '__main__':
  run()
