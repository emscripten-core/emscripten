# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import tempfile
import atexit
import sys

from . import utils


class TempFiles:
  def __init__(self, tmpdir, save_debug_files):
    self.tmpdir = tmpdir
    self.save_debug_files = save_debug_files
    self.to_clean = []

    atexit.register(self.clean)

  def note(self, filename):
    self.to_clean.append(filename)

  def get(self, suffix, prefix=None):
    """Returns a named temp file with the given prefix."""
    named_file = tempfile.NamedTemporaryFile(dir=self.tmpdir, suffix=suffix, prefix=prefix, delete=False)
    self.note(named_file.name)
    return named_file

  def get_file(self, suffix):
    """Returns an object representing a RAII-like access to a temp file
    that has convenient pythonesque semantics for being used via a construct
      'with TempFiles.get_file(..) as filename:'.
    The file will be deleted immediately once the 'with' block is exited.
    """
    class TempFileObject:
      def __enter__(self_):
        self_.file = tempfile.NamedTemporaryFile(dir=self.tmpdir, suffix=suffix, delete=False)
        self_.file.close() # NamedTemporaryFile passes out open file handles, but callers prefer filenames (and open their own handles manually if needed)
        return self_.file.name

      def __exit__(self_, _type, _value, _traceback):
        if not self.save_debug_files:
          utils.delete_file(self_.file.name)
    return TempFileObject()

  def get_dir(self):
    """Returns a named temp directory with the given prefix."""
    directory = tempfile.mkdtemp(dir=self.tmpdir)
    self.note(directory)
    return directory

  def clean(self):
    if self.save_debug_files:
      print(f'not cleaning up temp files since in debug-save mode, see them in {self.tmpdir}', file=sys.stderr)
      return
    for filename in self.to_clean:
      utils.delete_file(filename)
    self.to_clean = []
