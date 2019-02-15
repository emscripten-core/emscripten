#!/usr/bin/env python2
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Archive helper script

This script acts as a frontend replacement for `ar`. See emcc.
This is needed because, unlike a traditional linker, emscripten can't handle
archive with duplicate member names.  This is because emscripten extracts
archive to a temporary location and duplicate filenames will clobber each
other in this case.

"""

# TODO(sbc): Implement `ar x` within emscripten, in python, to avoid this issue
# and delete this file.

from __future__ import print_function
import hashlib
import os
import shutil
import sys

from tools.toolchain_profiler import ToolchainProfiler
from tools import shared
from tools.response_file import substitute_response_files, create_response_file

if __name__ == '__main__':
  ToolchainProfiler.record_process_start()


#
# Main run() function
#
def run():
  args = substitute_response_files(sys.argv)
  newargs = [shared.LLVM_AR] + args[1:]

  to_delete = []

  # The 3 argment form of ar doesn't involve other files. For example
  # 'ar x libfoo.a'.
  if len(newargs) > 3:
    cmd = newargs[1]
    if 'r' in cmd:
      # we are adding files to the archive.
      # normally the output file is then arg 2, except in the case were the
      # a or b modifiers are used in which case its arg 3.
      if 'a' in cmd or 'b' in cmd:
        new_member_args_start = 4
      else:
        new_member_args_start = 3

      # we add a hash to each input, to make them unique as
      # possible, as llvm-ar cannot extract duplicate names
      # (and only the basename is used!)
      for j in range(new_member_args_start, len(newargs)):
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

    if shared.DEBUG:
      print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)

    response_filename = create_response_file(newargs[3:], shared.get_emscripten_temp_dir())
    to_delete += [response_filename]
    newargs = newargs[:3] + ['@' + response_filename]

  if shared.DEBUG:
    print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)

  try:
    return shared.run_process(newargs, stdin=sys.stdin, check=False).returncode
  finally:
    for d in to_delete:
      shared.try_delete(d)


if __name__ == '__main__':
  sys.exit(run())
