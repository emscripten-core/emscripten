# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shlex
import tempfile
from .utils import WINDOWS


DEBUG = int(os.environ.get('EMCC_DEBUG', '0'))


def create_response_file(args, directory, suffix='.rsp.utf-8'):
  """Routes the given cmdline param list in args into a new response file and
  returns the filename to it.

  By default the returned filename has a suffix '.rsp.utf-8'. Pass a suffix parameter to override.
  """

  assert suffix.startswith('.')

  response_fd, response_filename = tempfile.mkstemp(prefix='emscripten_', suffix=suffix, dir=directory, text=True)

  # Backslashes and other special chars need to be escaped in the response file.
  escape_chars = ['\\', '\"']
  # When calling llvm-ar on Linux and macOS, single quote characters ' should be escaped.
  if not WINDOWS:
    escape_chars += ['\'']

  def escape(arg):
    for char in escape_chars:
      arg = arg.replace(char, '\\' + char)
    return arg

  args = [escape(a) for a in args]
  contents = ""

  # Arguments containing spaces need to be quoted.
  for arg in args:
    if ' ' in arg:
      arg = '"%s"' % arg
    contents += arg + '\n'

  # Decide the encoding of the generated file based on the requested file suffix
  if suffix.count('.') == 2:
    # Use the encoding specified in the suffix of the response file
    encoding = suffix.split('.')[2]
  else:
    encoding = 'utf-8'

  with os.fdopen(response_fd, 'w', encoding=encoding) as f:
    f.write(contents)

  if DEBUG:
    logging.warning('Creating response file ' + response_filename + ' with following contents: ' + contents)

  # Register the created .rsp file to be automatically cleaned up once this
  # process finishes, so that caller does not have to remember to do it.
  from . import shared
  shared.get_temp_files().note(response_filename)

  return response_filename


def read_response_file(response_filename):
  """Reads a response file, and returns the list of cmdline params found in the
  file.

  The encoding that the response filename should be read with can be specified
  as a suffix to the file, e.g. "foo.rsp.utf-8" or "foo.rsp.cp1252". If not
  specified, first UTF-8 and then Python locale.getpreferredencoding() are
  attempted.

  The parameter response_filename may start with '@'."""
  if response_filename.startswith('@'):
    response_filename = response_filename[1:]

  if not os.path.exists(response_filename):
    raise IOError("response file not found: %s" % response_filename)

  # Guess encoding based on the file suffix
  components = os.path.basename(response_filename).split('.')
  encoding_suffix = components[-1].lower()
  if len(components) > 1 and (encoding_suffix.startswith('utf') or encoding_suffix.startswith('cp') or encoding_suffix.startswith('iso') or encoding_suffix in ['ascii', 'latin-1']):
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
      logging.warning(f'Failed to parse response file {response_filename} with guessed encoding "{guessed_encoding}". Trying default system encoding...')
    # If that fails, try with the Python default locale.getpreferredencoding()
    with open(response_filename) as f:
      args = f.read()

  args = shlex.split(args)

  if DEBUG:
    logging.warning('Read response file ' + response_filename + ': ' + str(args))

  return args


def substitute_response_files(args):
  """Substitute any response files found in args with their contents."""
  new_args = []
  for arg in args:
    if arg.startswith('@'):
      new_args += read_response_file(arg)
    elif arg.startswith('-Wl,@'):
      for a in read_response_file(arg[5:]):
        if a.startswith('-'):
          a = '-Wl,' + a
        new_args.append(a)
    else:
      new_args.append(arg)
  return new_args
