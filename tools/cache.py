# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os
import shutil
import logging
from . import tempfiles, filelock

logger = logging.getLogger('cache')


# Permanent cache for system librarys and ports
class Cache(object):
  # If EM_EXCLUSIVE_CACHE_ACCESS is true, this process is allowed to have direct
  # access to the Emscripten cache without having to obtain an interprocess lock
  # for it. Generally this is false, and this is used in the case that
  # Emscripten process recursively calls to itself when building the cache, in
  # which case the parent Emscripten process has already locked the cache.
  # Essentially the env. var EM_EXCLUSIVE_CACHE_ACCESS signals from parent to
  # child process that the child can reuse the lock that the parent already has
  # acquired.
  EM_EXCLUSIVE_CACHE_ACCESS = int(os.environ.get('EM_EXCLUSIVE_CACHE_ACCESS', '0'))

  def __init__(self, dirname, use_subdir=True):
    # figure out the root directory for all caching
    dirname = os.path.normpath(dirname)
    self.root_dirname = dirname

    # if relevant, use a subdir of the cache
    if use_subdir:
      subdir = 'wasm'
      if shared.Settings.LTO:
        subdir += '-lto'
      if shared.Settings.RELOCATABLE:
        subdir += '-pic'
      dirname = os.path.join(dirname, subdir)

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

  def ensure(self):
    shared.safe_ensure_dirs(self.dirname)

  def erase(self):
    self.acquire_cache_lock()
    try:
      if os.path.exists(self.root_dirname):
        for f in os.listdir(self.root_dirname):
          tempfiles.try_delete(os.path.join(self.root_dirname, f))
    finally:
      self.release_cache_lock()

  def get_path(self, shortname, root=False):
    if root:
      return os.path.join(self.root_dirname, shortname)
    return os.path.join(self.dirname, shortname)

  def erase_file(self, shortname):
    name = os.path.join(self.dirname, shortname)
    if os.path.exists(name):
      logging.info('Cache: deleting cached file: %s', name)
      tempfiles.try_delete(name)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, what=None, force=False, root=False):
    if root:
      cachename = os.path.join(self.root_dirname, shortname)
    else:
      cachename = os.path.join(self.dirname, shortname)
    cachename = os.path.abspath(cachename)

    self.acquire_cache_lock()
    try:
      if os.path.exists(cachename) and not force:
        return cachename
      # it doesn't exist yet, create it
      if shared.FROZEN_CACHE:
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
        shared.safe_ensure_dirs(os.path.dirname(cachename))
        shutil.copyfile(temp, cachename)
      logger.info(' - ok')
    finally:
      self.release_cache_lock()

    return cachename


try:
  from . import shared
except ImportError:
  # Python 2 circular import compatibility
  import shared
