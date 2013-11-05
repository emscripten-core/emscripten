import os
import shutil
import tempfile

def try_delete(filename):
  try:
    os.unlink(filename)
  except:
    if os.path.exists(filename):
      shutil.rmtree(filename, ignore_errors=True)

class TempFiles:
  def __init__(self, tmp, save_debug_files=False):
    self.tmp = tmp
    self.save_debug_files = save_debug_files
    
    self.to_clean = []

  def note(self, filename):
    self.to_clean.append(filename)

  def get(self, suffix):
    """Returns a named temp file  with the given prefix."""
    named_file = tempfile.NamedTemporaryFile(dir=self.tmp, suffix=suffix, delete=False)
    self.note(named_file.name)
    return named_file

  def clean(self):
    if self.save_debug_files:
      import sys
      print >> sys.stderr, 'not cleaning up temp files since in debug-save mode, see them in %s' % (self.tmp,)
      return
    for filename in self.to_clean:
      try_delete(filename)
    self.to_clean = []

  def run_and_clean(self, func):
    try:
      return func()
    finally:
      self.clean()
