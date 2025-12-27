# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shlex
import tempfile

from . import shared
from .utils import WINDOWS

DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))


def create_response_file_contents(args):
  """Create response file contents based on list of arguments.
  """
  escape_chars = ['\\', '\"']
  # When calling llvm-ar on Linux and macOS, single quote characters ' should be escaped.
  if not WINDOWS:
    escape_chars += ['\'']

  def escape(arg):
    for char in escape_chars:
      arg = arg.replace(char, '\\' + char)
    return arg

  args = [escape(a) for a in args]
  contents = ''

  # Arguments containing spaces need to be quoted.
  for arg in args:
    if ' ' in arg:
      arg = '"%s"' % arg
    contents += arg + '\n'

  return contents


def create_response_file(args, directory):
  """Routes the given cmdline param list in args into a new response file and
  returns the filename to it.
  """
  # Backslashes and other special chars need to be escaped in the response file.
  contents = create_response_file_contents(args)

  response_fd, response_filename = tempfile.mkstemp(prefix='emscripten_', suffix='.rsp.utf-8', dir=directory, text=True)

  with os.fdopen(response_fd, 'w', encoding='utf-8') as f:
    f.write(contents)

  if DEBUG:
    logging.warning(f'Creating response file {response_filename} with following contents: {contents}')

  # Register the created .rsp file to be automatically cleaned up once this
  # process finishes, so that caller does not have to remember to do it.
  shared.get_temp_files().note(response_filename)

  return response_filename


def expand_response_file(arg):
  """Reads a response file, and returns the list of cmdline params found in the
  file.

  The encoding that the response filename should be read with can be specified
  as a suffix to the file, e.g. "foo.rsp.utf-8" or "foo.rsp.cp1252". If not
  specified, first UTF-8 and then Python locale.getpreferredencoding() are
  attempted.

  The parameter `arg` is the command line argument to be expanded."""

  if arg.startswith('@'):
    response_filename = arg[1:]
  elif arg.startswith('-Wl,@'):
    response_filename = arg[5:]
  else:
    response_filename = None

  # Is the argument is not a response file, or if the file does not exist
  # just return original argument.
  if not response_filename or not os.path.exists(response_filename):
    return [arg]

  # Guess encoding based on the file suffix
  components = os.path.basename(response_filename).split('.')
  encoding_suffix = components[-1].lower()
  if len(components) > 1 and (encoding_suffix.startswith(('utf', 'cp', 'iso')) or encoding_suffix in {'ascii', 'latin-1'}):
    guessed_encoding = encoding_suffix
  else:
    # On windows, recent version of CMake emit rsp files containing
    # a BOM.  Using 'utf-8-sig' works on files both with and without
    # a BOM.
    guessed_encoding = 'utf-8-sig'

  try:
    # First try with the guessed encoding
    with open(response_filename, encoding=guessed_encoding) as f:
      args = f.read()
  except (ValueError, LookupError): # UnicodeDecodeError is a subclass of ValueError, and Python raises either a ValueError or a UnicodeDecodeError on decode errors. LookupError is raised if guessed encoding is not an encoding.
    if DEBUG:
      logging.warning(f'failed to parse response file {response_filename} with guessed encoding "{guessed_encoding}". Trying default system encoding...')
    # If that fails, try with the Python default locale.getpreferredencoding()
    with open(response_filename) as f:
      args = f.read()

  args = shlex.split(args)

  if DEBUG:
    logging.warning(f'read response file {response_filename}: {args}')

  # Response file can be recursive so call substitute_response_files on the arguments
  return substitute_response_files(args)


def substitute_response_files(args):
  """Substitute any response files found in args with their contents."""
  new_args = []
  for arg in args:
    new_args += expand_response_file(arg)
  return new_args
