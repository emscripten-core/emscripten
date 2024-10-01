# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import os
import shutil
import sys
from pathlib import Path

from . import diagnostics

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
WINDOWS = sys.platform.startswith('win')
MACOS = sys.platform == 'darwin'
LINUX = sys.platform.startswith('linux')


def exit_with_error(msg, *args):
  diagnostics.error(msg, *args)


def path_from_root(*pathelems):
  return str(Path(__rootpath__, *pathelems))


def normalize_path(path):
  """Normalize path separators to UNIX-style forward slashes.

  This can be useful when converting paths to URLs or JS strings,
  or when trying to generate consistent output file contents
  across all platforms.  In most cases UNIX-style separators work
  fine on windows.
  """
  return path.replace('\\', '/').replace('//', '/')


def safe_ensure_dirs(dirname):
  os.makedirs(dirname, exist_ok=True)


# TODO(sbc): Replace with str.removeprefix once we update to python3.9
def removeprefix(string, prefix):
  if string.startswith(prefix):
    return string[len(prefix):]
  return string


@contextlib.contextmanager
def chdir(dir):
  """A context manager that performs actions in the given directory."""
  orig_cwd = os.getcwd()
  os.chdir(dir)
  try:
    yield
  finally:
    os.chdir(orig_cwd)


def read_file(file_path):
  """Read from a file opened in text mode"""
  with open(file_path, encoding='utf-8') as fh:
    return fh.read()


def read_binary(file_path):
  """Read from a file opened in binary mode"""
  with open(file_path, 'rb') as fh:
    return fh.read()


def write_file(file_path, text):
  """Write to a file opened in text mode"""
  with open(file_path, 'w', encoding='utf-8') as fh:
    fh.write(text)


def write_binary(file_path, contents):
  """Write to a file opened in binary mode"""
  with open(file_path, 'wb') as fh:
    fh.write(contents)


def delete_file(filename):
  """Delete a file (if it exists)."""
  if os.path.lexists(filename):
    os.remove(filename)


def delete_dir(dirname):
  """Delete a directory (if it exists)."""
  if not os.path.exists(dirname):
    return
  shutil.rmtree(dirname)


def delete_contents(dirname, exclude=None):
  """Delete the contents of a directory without removing
  the directory itself."""
  if not os.path.exists(dirname):
    return
  for entry in os.listdir(dirname):
    if exclude and entry in exclude:
      continue
    entry = os.path.join(dirname, entry)
    if os.path.isdir(entry):
      delete_dir(entry)
    else:
      delete_file(entry)


# TODO: Move this back to shared.py once importing that file becoming side effect free (i.e. it no longer requires a config).
def set_version_globals():
  global EMSCRIPTEN_VERSION, EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY
  filename = path_from_root('emscripten-version.txt')
  EMSCRIPTEN_VERSION = read_file(filename).strip().strip('"')
  parts = [int(x) for x in EMSCRIPTEN_VERSION.split('-')[0].split('.')]
  EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY = parts
