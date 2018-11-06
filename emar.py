#!/usr/bin/env python2
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
emar - ar helper script
=======================

This script acts as a frontend replacement for ar. See emcc.
'''

from __future__ import print_function
from tools.toolchain_profiler import ToolchainProfiler
if __name__ == '__main__':
  ToolchainProfiler.record_process_start()

import os, subprocess, sys
from tools import shared

#
# Main run() function
#
def run():
  DEBUG = os.environ.get('EMCC_DEBUG')
  if DEBUG == "0":
    DEBUG = None

  newargs = [shared.LLVM_AR] + sys.argv[1:]

  if DEBUG:
    print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)

  to_delete = []
  if len(newargs) > 2:
    if 'r' in newargs[1]:
      # we are adding files to the archive.
      # find the .a; everything after it is an input file.
      # we add a hash to each input, to make them unique as
      # possible, as llvm-ar cannot extract duplicate names
      # (and only the basename is used!)
      i = 1
      while i < len(newargs):
        if newargs[i].endswith('.a'):
          import hashlib, shutil
          for j in range(i+1, len(newargs)):
            orig_name = newargs[j]
            full_name = os.path.abspath(orig_name)
            dir_name = os.path.dirname(full_name)
            base_name = os.path.basename(full_name)
            h = hashlib.md5(full_name.encode('utf-8')).hexdigest()[:8]
            parts = base_name.split('.')
            parts[0] += '_' + h
            newname = '.'.join(parts)
            full_newname = os.path.join(dir_name, newname)
            if not os.path.exists(full_newname):
              try: # it is ok to fail here, we just don't get hashing
                shutil.copyfile(orig_name, full_newname)
                newargs[j] = full_newname
                to_delete.append(full_newname)
              except:
                pass
          break
        i += 1

  if DEBUG:
    print('Invoking ' + str(newargs), file=sys.stderr)
  try:
    return subprocess.call(newargs, stdin=sys.stdin)
  finally:
    for d in to_delete:
      shared.try_delete(d)

if __name__ == '__main__':
  sys.exit(run())
