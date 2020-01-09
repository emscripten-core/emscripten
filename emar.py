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
import tempfile

from tools.toolchain_profiler import ToolchainProfiler
from tools import shared
from tools.response_file import substitute_response_files, create_response_file

if __name__ == '__main__':
  ToolchainProfiler.record_process_start()


#
# Main run() function
#
def run():
  if shared.Settings.WASM_BACKEND:
    # The wasm backend does suffer from the same probllem as fastcomp so doesn't
    # need the filename hashing.
    cmd = [shared.LLVM_AR] + sys.argv[1:]
    return shared.run_process(cmd, stdin=sys.stdin, check=False).returncode

  try:
    args = substitute_response_files(sys.argv)
  except IOError as e:
    shared.exit_with_error(e)
  newargs = [shared.LLVM_AR] + args[1:]

  tmpdir = None
  response_filename = None

  # The 3 argmuent form of ar doesn't involve other files. For example
  # 'ar x libfoo.a'.
  if len(newargs) > 3:
    tmpdir = tempfile.mkdtemp(prefix='emar-')
    cmd = newargs[1]
    if 'r' in cmd or 'q' in cmd:
      # We are adding files to the archive.
      # Normally the output file is then arg 2, except in the case were the
      # a or b modifiers are used in which case its arg 3.
      if 'a' in cmd or 'b' in cmd:
        out_arg_index = 3
      else:
        out_arg_index = 2

      # Add a hash to colliding basename, to make them unique.
      for j in range(out_arg_index + 1, len(newargs)):
        orig_name = newargs[j]
        full_name = os.path.abspath(orig_name)
        basename = os.path.basename(full_name)

        h = hashlib.md5(full_name.encode('utf-8')).hexdigest()[:8]
        parts = basename.split('.')
        parts[0] += '_' + h
        newname = '.'.join(parts)
        full_newname = os.path.join(tmpdir, newname)
        shutil.copyfile(orig_name, full_newname)
        newargs[j] = full_newname

    if shared.DEBUG:
      print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)
    response_filename = create_response_file(newargs[3:], shared.get_emscripten_temp_dir())
    newargs = newargs[:3] + ['@' + response_filename]

  if shared.DEBUG:
    print('emar:', sys.argv, '  ==>  ', newargs, file=sys.stderr)

  rtn = shared.run_process(newargs, stdin=sys.stdin, check=False).returncode
  if tmpdir:
    shutil.rmtree(tmpdir)
    shared.try_delete(response_filename)
  return rtn


if __name__ == '__main__':
  sys.exit(run())
