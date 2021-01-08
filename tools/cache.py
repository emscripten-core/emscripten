# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import logging
import os
import shutil
from . import tempfiles, filelock, config, utils

logger = logging.getLogger('cache')


# Permanent cache for system librarys and ports
class Cache:
  # If EM_EXCLUSIVE_CACHE_ACCESS is true, this process is allowed to have direct
  # access to the Emscripten cache without having to obtain an interprocess lock
  # for it. Generally this is false, and this is used in the case that
  # Emscripten process recursively calls to itself when building the cache, in
  # which case the parent Emscripten process has already locked the cache.
  # Essentially the env. var EM_EXCLUSIVE_CACHE_ACCESS signals from parent to
  # child process that the child can reuse the lock that the parent already has
  # acquired.
  EM_EXCLUSIVE_CACHE_ACCESS = int(os.environ.get('EM_EXCLUSIVE_CACHE_ACCESS', '0'))

  def __init__(self, dirname):
    # figure out the root directory for all caching
    dirname = os.path.normpath(dirname)
    self.dirname = dirname
    self.acquired_count = 0

    # since the lock itself lives inside the cache directory we need to ensure it
    # exists.
    self.ensure()
    self.filelock_name = os.path.join(dirname, 'cache.lock')
    self.filelock = filelock.FileLock(self.filelock_name)

  def acquire_cache_lock(self):
    if not self.EM_EXCLUSIVE_CACHE_ACCESS and self.acquired_count == 0:
      logger.debug('PID %s acquiring multiprocess file lock to Emscripten cache at %s' % (str(os.getpid()), self.dirname))
      try:
        self.filelock.acquire(60)
      except filelock.Timeout:
        # The multiprocess cache locking can be disabled altogether by setting EM_EXCLUSIVE_CACHE_ACCESS=1 environment
        # variable before building. (in that case, use "embuilder.py build ALL" to prepopulate the cache)
        logger.warning('Accessing the Emscripten cache at "' + self.dirname + '" is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "' + self.filelock_name + '" and try again. If this occurs deterministically, consider filing a bug.')
        self.filelock.acquire()

      self.prev_EM_EXCLUSIVE_CACHE_ACCESS = os.environ.get('EM_EXCLUSIVE_CACHE_ACCESS')
      os.environ['EM_EXCLUSIVE_CACHE_ACCESS'] = '1'
      logger.debug('done')
    self.acquired_count += 1

  def release_cache_lock(self):
    self.acquired_count -= 1
    assert self.acquired_count >= 0, "Called release more times than acquire"
    if not self.EM_EXCLUSIVE_CACHE_ACCESS and self.acquired_count == 0:
      if self.prev_EM_EXCLUSIVE_CACHE_ACCESS:
        os.environ['EM_EXCLUSIVE_CACHE_ACCESS'] = self.prev_EM_EXCLUSIVE_CACHE_ACCESS
      else:
        del os.environ['EM_EXCLUSIVE_CACHE_ACCESS']
      self.filelock.release()
      logger.debug('PID %s released multiprocess file lock to Emscripten cache at %s' % (str(os.getpid()), self.dirname))

  @contextlib.contextmanager
  def lock(self):
    """A context manager that performs actions in the given directory."""
    self.acquire_cache_lock()
    try:
      yield
    finally:
      self.release_cache_lock()

  def ensure(self):
    utils.safe_ensure_dirs(self.dirname)

  def erase(self):
    with self.lock():
      if os.path.exists(self.dirname):
        for f in os.listdir(self.dirname):
          tempfiles.try_delete(os.path.join(self.dirname, f))

  def get_path(self, name):
    return os.path.join(self.dirname, name)

  def get_include_dir(self):
    return os.path.join(self.dirname, 'include')

  def get_lib_dir(self):
    subdir = 'wasm'
    if shared.Settings.LTO:
      subdir += '-lto'
    if shared.Settings.RELOCATABLE:
      subdir += '-pic'
    if shared.Settings.MEMORY64:
      subdir += '-memory64'
    return subdir

  def get_lib_name(self, name):
    return os.path.join(self.get_lib_dir(), name)

  def erase_lib(self, name):
    self.erase_file(self.get_lib_name(name))

  def erase_file(self, shortname):
    name = os.path.join(self.dirname, shortname)
    if os.path.exists(name):
      logging.info('Cache: deleting cached file: %s', name)
      tempfiles.try_delete(name)

  def get_lib(self, libname, *args, **kwargs):
    name = self.get_lib_name(libname)
    return self.get(name, *args, **kwargs)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, what=None, force=False):
    cachename = os.path.join(self.dirname, shortname)
    cachename = os.path.abspath(cachename)
    # Check for existence before taking the lock in case we can avoid the
    # lock completely.
    if os.path.exists(cachename) and not force:
      return cachename

    with self.lock():
      if os.path.exists(cachename) and not force:
        return cachename
      # it doesn't exist yet, create it
      if config.FROZEN_CACHE:
        # it's ok to build small .txt marker files like "vanilla"
        if not shortname.endswith('.txt'):
          raise Exception('FROZEN_CACHE disallows building system libs: %s' % shortname)
      if what is None:
        if shortname.endswith(('.bc', '.so', '.a')):
          what = 'system library'
        else:
          what = 'system asset'
      message = 'generating ' + what + ': ' + shortname + '... (this will be cached in "' + cachename + '" for subsequent builds)'
      logger.info(message)
      self.ensure()
      temp = creator()
      if os.path.normcase(temp) != os.path.normcase(cachename):
        utils.safe_ensure_dirs(os.path.dirname(cachename))
        shutil.copyfile(temp, cachename)
      logger.info(' - ok')

    return cachename


from . import shared
