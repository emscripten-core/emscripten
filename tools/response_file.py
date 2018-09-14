# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shlex
import tempfile


DEBUG = os.environ.get('EMCC_DEBUG')
if DEBUG == "0":
  DEBUG = None


def create_response_file(args, directory):
  """Routes the given cmdline param list in args into a new response file and
  returns the filename to it.

  The returned filename has a suffix '.rsp'.
  """
  (response_fd, response_filename) = tempfile.mkstemp(prefix='emscripten_', suffix='.rsp', dir=directory, text=True)
  response_fd = os.fdopen(response_fd, "w")

  args = [p.replace('\\', '\\\\').replace('"', '\\"') for p in args]
  contents = '"' + '" "'.join(args) + '"'
  if DEBUG:
    logging.warning('Creating response file ' + response_filename + ': ' + contents)
  response_fd.write(contents)
  response_fd.close()

  # Register the created .rsp file to be automatically cleaned up once this
  # process finishes, so that caller does not have to remember to do it.
  from . import shared
  shared.configuration.get_temp_files().note(response_filename)

  return response_filename


def read_response_file(response_filename):
  """Reads a response file, and returns the list of cmdline params found in the
  file.

  The parameter response_filename may start with '@'."""
  if response_filename.startswith('@'):
    response_filename = response_filename[1:]

  if not os.path.exists(response_filename):
    raise Exception("Response file '%s' not found!" % response_filename)

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
