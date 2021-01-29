# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import logging
import os
from . import tempfiles, filelock, config, utils

logger = logging.getLogger('cache')


# Permanent cache for system librarys and ports
class Cache:
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
    if config.FROZEN_CACHE:
      # Raise an exception here rather than exit_with_error since in practice this
      # should never happen
      raise Exception('Attempt to lock the cache but FROZEN_CACHE is set')

    if self.acquired_count == 0:
      logger.debug('PID %s acquiring multiprocess file lock to Emscripten cache at %s' % (str(os.getpid()), self.dirname))
      # This should never happen because any calls to emcc make when populating the
      # cache should not themselves attempt to acquire the cache lock.  For example,
      # it should never be the case that while holding the cache lock to build libraryA
      # a build of libraryB is needed.  This would be a bug in the dependency graph
      # since if libraryA depended on libraryB it should already have been built and
      # cached before the libraryA build started.
      assert 'EM_PARENT_HOLDS_CACHE_LOCK' not in os.environ
      try:
        self.filelock.acquire(60)
      except filelock.Timeout:
        logger.warning('Accessing the Emscripten cache at "' + self.dirname + '" is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "' + self.filelock_name + '" and try again. If this occurs deterministically, consider filing a bug.')
        self.filelock.acquire()

      os.environ['EM_PARENT_HOLDS_CACHE_LOCK'] = '1'
      logger.debug('done')
    self.acquired_count += 1

  def release_cache_lock(self):
    self.acquired_count -= 1
    assert self.acquired_count >= 0, "Called release more times than acquire"
    if self.acquired_count == 0:
      self.filelock.release()
      del os.environ['EM_PARENT_HOLDS_CACHE_LOCK']
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

  def get_sysroot_dir(self, absolute):
    if absolute:
      return self.get_path('sysroot')
    return 'sysroot'

  def get_include_dir(self):
    return os.path.join(self.get_sysroot_dir(absolute=True), 'include')

  def get_lib_dir(self, absolute):
    path = os.path.join(self.get_sysroot_dir(absolute=absolute), 'lib')
    if shared.Settings.MEMORY64:
      path = os.path.join(path, 'wasm64-emscripten')
    else:
      path = os.path.join(path, 'wasm32-emscripten')
    # if relevant, use a subdir of the cache
    subdir = []
    if shared.Settings.LTO:
      subdir.append('lto')
    if shared.Settings.RELOCATABLE:
      subdir.append('pic')
    if subdir:
      path = os.path.join(path, '-'.join(subdir))
    return path

  def get_lib_name(self, name):
    return os.path.join(self.get_lib_dir(absolute=False), name)

  def erase_lib(self, name):
    self.erase_file(self.get_lib_name(name))

  def erase_file(self, shortname):
    with self.lock():
      name = self.get_path(shortname)
      if os.path.exists(name):
        logger.info('deleting cached file: %s', name)
        tempfiles.try_delete(name)

  def get_lib(self, libname, *args, **kwargs):
    name = self.get_lib_name(libname)
    return self.get(name, *args, **kwargs)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, what=None, force=False):
    cachename = self.get_path(shortname)
    cachename = os.path.abspath(cachename)
    # Check for existence before taking the lock in case we can avoid the
    # lock completely.
    if os.path.exists(cachename) and not force:
      return cachename

    if config.FROZEN_CACHE:
      # Raise an exception here rather than exit_with_error since in practice this
      # should never happen
      raise Exception('FROZEN_CACHE is set, but cache file is missing: %s' % shortname)

    with self.lock():
      if os.path.exists(cachename) and not force:
        return cachename
      if what is None:
        if shortname.endswith(('.bc', '.so', '.a')):
          what = 'system library'
        else:
          what = 'system asset'
      message = 'generating ' + what + ': ' + shortname + '... (this will be cached in "' + cachename + '" for subsequent builds)'
      logger.info(message)
      utils.safe_ensure_dirs(os.path.dirname(cachename))
      creator(cachename)
      assert os.path.exists(cachename)
      logger.info(' - ok')

    return cachename


from . import shared
