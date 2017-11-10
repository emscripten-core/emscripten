#!/usr/bin/env python2

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
import shlex

def handle_args_from_files(argument_input):
  new_arg_list = []
  for elem in argument_input:
    if elem.startswith("@"):
      with open(elem.replace("@","")) as args_file:
        arg_strings = []
        for arg_line in args_file.read().splitlines():
          for arg in shlex.split(arg_line):
            arg_strings.append(arg)
        #recurse
        arg_strings = handle_args_from_files(arg_strings)
        new_arg_list.extend(arg_strings)
    else:
      new_arg_list.append(elem)
  return new_arg_list

#
# Main run() function
#
def run():
  DEBUG = os.environ.get('EMCC_DEBUG')
  if DEBUG == "0":
    DEBUG = None

  #handle reading args from files
  expanded_args = handle_args_from_files(sys.argv)

  newargs = [shared.LLVM_AR] + expanded_args[1:]

  if DEBUG:
    print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)

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
  sys.exit(0)
