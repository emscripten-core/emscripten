# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""General purpose utility functions.  The code in this file should mostly be
not emscripten-specific, but general purpose enough to be useful in any command
line utility."""

import functools
import logging
import os
import shlex
import shutil
import stat
import subprocess
import sys
from pathlib import Path

from . import diagnostics

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
WINDOWS = sys.platform.startswith('win')
MACOS = sys.platform == 'darwin'
LINUX = sys.platform.startswith('linux')

logger = logging.getLogger('utils')


def run_process(cmd, check=True, input=None, *args, **kw):
  """Runs a subprocess returning the exit code.

  By default this function will raise an exception on failure.  Therefore this should only be
  used if you want to handle such failures.  For most subprocesses, failures are not recoverable
  and should be fatal.  In those cases the `check_call` wrapper should be preferred.
  """

  # Flush standard streams otherwise the output of the subprocess may appear in the
  # output before messages that we have already written.
  sys.stdout.flush()
  sys.stderr.flush()
  kw.setdefault('text', True)
  kw.setdefault('encoding', 'utf-8')
  ret = subprocess.run(cmd, check=check, input=input, *args, **kw)
  debug_text = '%sexecuted %s' % ('successfully ' if check else '', shlex.join(cmd))
  logger.debug(debug_text)
  return ret


def exec(cmd):
  if WINDOWS:
    rtn = run_process(cmd, stdin=sys.stdin, check=False).returncode
    sys.exit(rtn)
  else:
    sys.stdout.flush()
    sys.stderr.flush()
    os.execvp(cmd[0], cmd)


def exit_with_error(msg, *args):
  diagnostics.error(msg, *args)


def path_from_root(*pathelems):
  return str(Path(__rootpath__, *pathelems))


def suffix(name):
  """Return the file extension"""
  return os.path.splitext(name)[1]


def exe_suffix(cmd):
  return cmd + '.exe' if WINDOWS else cmd


def bat_suffix(cmd):
  return cmd + '.bat' if WINDOWS else cmd


def replace_suffix(filename, new_suffix):
  assert new_suffix[0] == '.'
  return os.path.splitext(filename)[0] + new_suffix


def unsuffixed(name):
  """Return the filename without the extension.

  If there are multiple extensions this strips only the final one.
  """
  return os.path.splitext(name)[0]


def unsuffixed_basename(name):
  return os.path.basename(unsuffixed(name))


def get_file_suffix(filename):
  """Parses the essential suffix of a filename, discarding Unix-style version
  numbers in the name. For example for 'libz.so.1.2.8' returns '.so'"""
  while filename:
    filename, suffix = os.path.splitext(filename)
    if not suffix[1:].isdigit():
      return suffix
  return ''


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


def make_writable(filename):
  assert os.path.exists(filename)
  old_mode = stat.S_IMODE(os.stat(filename).st_mode)
  os.chmod(filename, old_mode | stat.S_IWUSR)


def safe_copy(src, dst):
  logger.debug('copy: %s -> %s', src, dst)
  src = os.path.abspath(src)
  dst = os.path.abspath(dst)
  if os.path.isdir(dst):
    dst = os.path.join(dst, os.path.basename(src))
  if src == dst:
    return
  if dst == os.devnull:
    return
  # Copies data and permission bits, but not other metadata such as timestamp
  shutil.copy(src, dst)
  # We always want the target file to be writable even when copying from
  # read-only source. (e.g. a read-only install of emscripten).
  make_writable(dst)


# TODO(sbc): Replace with str.removeprefix once we update to python3.9
def removeprefix(string, prefix):
  if string.startswith(prefix):
    return string[len(prefix):]
  return string


def convert_line_endings_in_file(filename, to_eol):
  if to_eol == os.linesep:
    assert os.path.exists(filename)
    return # No conversion needed

  text = read_file(filename)
  write_file(filename, text, line_endings=to_eol)


def read_file(file_path):
  """Read from a file opened in text mode"""
  with open(file_path, encoding='utf-8') as fh:
    return fh.read()


def read_binary(file_path):
  """Read from a file opened in binary mode"""
  with open(file_path, 'rb') as fh:
    return fh.read()


def write_file(file_path, text, line_endings=None):
  """Write to a file opened in text mode"""
  if line_endings and line_endings != os.linesep:
    text = text.replace('\n', line_endings)
    write_binary(file_path, text.encode('utf-8'))
  else:
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


def get_num_cores():
  # Prefer `os.process_cpu_count` when available (3.13 and above) since
  # it takes into account thread affinity.
  # Fall back to `os.sched_getaffinity` where available and finally
  # `os.cpu_count`, which should work everywhere.
  if hasattr(os, 'process_cpu_count'):
    cpu_count = os.process_cpu_count()
  elif hasattr(os, 'sched_getaffinity'):
    cpu_count = len(os.sched_getaffinity(0))
  else:
    cpu_count = os.cpu_count()
  return int(os.environ.get('EMCC_CORES', cpu_count))


# TODO(sbc): Replace with functools.cache, once we update to python 3.9
memoize = functools.lru_cache(maxsize=None)


# TODO: Move this back to shared.py once importing that file becoming side effect free (i.e. it no longer requires a config).
def set_version_globals():
  global EMSCRIPTEN_VERSION, EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY
  filename = path_from_root('emscripten-version.txt')
  EMSCRIPTEN_VERSION = read_file(filename).strip().strip('"')
  parts = [int(x) for x in EMSCRIPTEN_VERSION.split('-')[0].split('.')]
  EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY = parts
