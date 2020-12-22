# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import os
import sys

from . import diagnostics

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
WINDOWS = sys.platform.startswith('win')
MACOS = sys.platform == 'darwin'
LINUX = sys.platform.startswith('linux')


def exit_with_error(msg, *args):
  diagnostics.error(msg, *args)


def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)


def safe_ensure_dirs(dirname):
  try:
    os.makedirs(dirname)
  except OSError:
    # Python 2 compatibility: makedirs does not support exist_ok parameter
    # Ignore error for already existing dirname as exist_ok does
    if not os.path.isdir(dirname):
      raise


@contextlib.contextmanager
def chdir(dir):
  """A context manager that performs actions in the given directory."""
  orig_cwd = os.getcwd()
  os.chdir(dir)
  try:
    yield
  finally:
    os.chdir(orig_cwd)


# Finds the given executable 'program' in PATH. Operates like the Unix tool 'which'.
def which(program):
  def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

  if os.path.isabs(program):
    if os.path.isfile(program):
      return program

    if WINDOWS:
      for suffix in ['.exe', '.cmd', '.bat']:
        if is_exe(program + suffix):
          return program + suffix

  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.environ["PATH"].split(os.pathsep):
      path = path.strip('"')
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
      if WINDOWS:
        for suffix in ('.exe', '.cmd', '.bat'):
          if is_exe(exe_file + suffix):
            return exe_file + suffix

  return None
