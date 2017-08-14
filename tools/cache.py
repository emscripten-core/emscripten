from toolchain_profiler import ToolchainProfiler
import os.path, sys, shutil, time, logging
import tempfiles, filelock

# Permanent cache for dlmalloc and stdlibc++
class Cache(object):

  # If EM_EXCLUSIVE_CACHE_ACCESS is true, this process is allowed to have direct access to
  # the Emscripten cache without having to obtain an interprocess lock for it. Generally this
  # is false, and this is used in the case that Emscripten process recursively calls to itself
  # when building the cache, in which case the parent Emscripten process has already locked
  # the cache. Essentially the env. var EM_EXCLUSIVE_CACHE_ACCESS signals from parent to
  # child process that the child can reuse the lock that the parent already has acquired.
  EM_EXCLUSIVE_CACHE_ACCESS = int(os.environ.get('EM_EXCLUSIVE_CACHE_ACCESS') or 0)

  def __init__(self, dirname=None, debug=False, use_subdir=True):
    if dirname is None:
      dirname = os.environ.get('EM_CACHE')
    if not dirname:
      dirname = os.path.expanduser(os.path.join('~', '.emscripten_cache'))

    def try_remove_ending(thestring, ending):
      if thestring.endswith(ending):
        return thestring[:-len(ending)]
      return thestring

    self.filelock_name = try_remove_ending(try_remove_ending(dirname, '/'), '\\') + '.lock'
    self.filelock = filelock.FileLock(self.filelock_name)

    if use_subdir:
      if shared.Settings.WASM_BACKEND:
        dirname = os.path.join(dirname, 'wasm')
      else:
        dirname = os.path.join(dirname, 'asmjs')
    self.dirname = dirname
    self.debug = debug
    self.acquired_count = 0

  def acquire_cache_lock(self):
    if not self.EM_EXCLUSIVE_CACHE_ACCESS and self.acquired_count == 0:
      logging.debug('Cache: PID %s acquiring multiprocess file lock to Emscripten cache' % str(os.getpid()))
      try:
        self.filelock.acquire(60)
      except filelock.Timeout:
        # The multiprocess cache locking can be disabled altogether by setting EM_EXCLUSIVE_CACHE_ACCESS=1 environment
        # variable before building. (in that case, use "embuilder.py build ALL" to prepopulate the cache)
        logging.warning('Accessing the Emscripten cache at "' + self.dirname + '" is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "' + self.filelock_name + '" and try again. If this occurs deterministically, consider filing a bug.')
        self.filelock.acquire()

      self.prev_EM_EXCLUSIVE_CACHE_ACCESS = os.environ.get('EM_EXCLUSIVE_CACHE_ACCESS')
      os.environ['EM_EXCLUSIVE_CACHE_ACCESS'] = '1'
      logging.debug('Cache: done')
    self.acquired_count += 1

  def release_cache_lock(self):
    self.acquired_count -= 1
    assert self.acquired_count >= 0, "Called release more times than acquire"
    if not self.EM_EXCLUSIVE_CACHE_ACCESS and self.acquired_count == 0:
      if self.prev_EM_EXCLUSIVE_CACHE_ACCESS: os.environ['EM_EXCLUSIVE_CACHE_ACCESS'] = self.prev_EM_EXCLUSIVE_CACHE_ACCESS
      else: del os.environ['EM_EXCLUSIVE_CACHE_ACCESS']
      self.filelock.release()
      logging.debug('Cache: PID %s released multiprocess file lock to Emscripten cache' % str(os.getpid()))

  def ensure(self):
    self.acquire_cache_lock()
    try:
      shared.safe_ensure_dirs(self.dirname)
    finally:
      self.release_cache_lock()

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

    self.acquire_cache_lock()
    try:
      if os.path.exists(cachename) and not force:
        return cachename
      if what is None:
        if shortname.endswith(('.bc', '.so', '.a')): what = 'system library'
        else: what = 'system asset'
      message = 'generating ' + what + ': ' + shortname + '... (this will be cached in "' + cachename + '" for subsequent builds)'
      logging.info(message)
      self.ensure()
      temp = creator()
      if temp != cachename:
        shutil.copyfile(temp, cachename)
      logging.info(' - ok')
    finally:
      self.release_cache_lock()

    return cachename

# Given a set of functions of form (ident, text), and a preferred chunk size,
# generates a set of chunks for parallel processing and caching.
def chunkify(funcs, chunk_size, DEBUG=False):
  with ToolchainProfiler.profile_block('chunkify'):
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
