#!/usr/bin/env python2

'''
emar - ar helper script
=======================

This script acts as a frontend replacement for ar. See emcc.
'''

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
    print >> sys.stderr, 'emar:', sys.argv, '  ==>  ', newargs

  if len(newargs) > 2:
    to_delete = []
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
            h = hashlib.md5(full_name).hexdigest()[:8]
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
    subprocess.call(newargs)
    for d in to_delete:
      shared.try_delete(d)

if __name__ == '__main__':
  run()
