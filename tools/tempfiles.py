import os
import shutil

def try_delete(filename):
  try:
    os.unlink(filename)
  except:
    shutil.rmtree(filename, ignore_errors=True)
