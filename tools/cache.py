import os.path, sys, shutil, time, logging
import tempfiles, filelock

# Permanent cache for dlmalloc and stdlibc++
class Cache:
  def __init__(self, dirname=None, debug=False, use_subdir=True):
    if dirname is None:
      dirname = os.environ.get('EM_CACHE')
    if not dirname:
      dirname = os.path.expanduser(os.path.join('~', '.emscripten_cache'))
    if use_subdir:
      if os.environ.get('EMCC_WASM_BACKEND') and os.environ.get('EMCC_WASM_BACKEND') != '0':
        dirname = os.path.join(dirname, 'wasm')
      else:
        dirname = os.path.join(dirname, 'asmjs')
    self.dirname = dirname
    self.debug = debug

    def try_remove_ending(thestring, ending):
      if thestring.endswith(ending):
        return thestring[:-len(ending)]
      return thestring

    self.filelock_name = try_remove_ending(try_remove_ending(self.dirname, '/'), '\\') + '.lock'
    self.filelock = filelock.FileLock(self.filelock_name)

  def ensure(self):
    with self.filelock:
      shared.safe_ensure_dirs(self.dirname)

  def erase(self):
    tempfiles.try_delete(self.dirname)
    try:
      open(self.dirname + '__last_clear', 'w').write('last clear: ' + time.asctime() + '\n')
    except Exception, e:
      print >> sys.stderr, 'failed to save last clear time: ', e
    self.filelock = None
    tempfiles.try_delete(self.filelock_name)
    self.filelock = filelock.FileLock(self.filelock_name)

  def get_path(self, shortname):
    return os.path.join(self.dirname, shortname)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, extension='.bc', what=None, force=False):
    if not shortname.endswith(extension): shortname += extension
    cachename = os.path.join(self.dirname, shortname)

    with self.filelock:
      if os.path.exists(cachename) and not force:
        return cachename
      if what is None:
        if shortname.endswith(('.bc', '.so', '.a')): what = 'system library'
        else: what = 'system asset'
      message = 'generating ' + what + ': ' + shortname + '...'
      logging.warn(message)
      self.ensure()
      temp = creator()
      if temp != cachename:
        shutil.copyfile(temp, cachename)
      logging.warn(' '*len(message) + 'ok')

    return cachename

# Given a set of functions of form (ident, text), and a preferred chunk size,
# generates a set of chunks for parallel processing and caching.
def chunkify(funcs, chunk_size, DEBUG=False):
  chunks = []
  # initialize reasonably, the rest of the funcs we need to split out
  curr = []
  total_size = 0
  for i in range(len(funcs)):
    func = funcs[i]
    curr_size = len(func[1])
    if total_size + curr_size < chunk_size:
      curr.append(func)
      total_size += curr_size
    else:
      chunks.append(curr)
      curr = [func]
      total_size = curr_size
  if curr:
    chunks.append(curr)
    curr = None
  return [''.join([func[1] for func in chunk]) for chunk in chunks] # remove function names

import shared
