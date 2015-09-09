import os
import shutil
import tempfile
import atexit
import stat

def try_delete(filename):
  try:
    os.unlink(filename)
  except:
    pass
  if not os.path.exists(filename): return
  try:
    shutil.rmtree(filename, ignore_errors=True)
  except:
    pass
  if not os.path.exists(filename): return
  try:
    os.chmod(filename, os.stat(filename).st_mode | stat.S_IWRITE)
    def remove_readonly_and_try_again(func, path, exc_info):
      if not (os.stat(path).st_mode & stat.S_IWRITE):
        os.chmod(path, os.stat(path).st_mode | stat.S_IWRITE)
        func(path)
      else:
        raise
    shutil.rmtree(filename, onerror=remove_readonly_and_try_again)
  except:
    pass

class TempFiles:
  def __init__(self, tmp, save_debug_files=False):
    self.tmp = tmp
    self.save_debug_files = save_debug_files
    self.to_clean = []

    atexit.register(self.clean)

  def note(self, filename):
    self.to_clean.append(filename)

  def get(self, suffix):
    """Returns a named temp file  with the given prefix."""
    named_file = tempfile.NamedTemporaryFile(dir=self.tmp, suffix=suffix, delete=False)
    self.note(named_file.name)
    return named_file

  def get_dir(self):
    """Returns a named temp file  with the given prefix."""
    directory = tempfile.mkdtemp(dir=self.tmp)
    self.note(directory)
    return directory

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
