# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import contextlib
import logging
import os
from pathlib import Path

from . import filelock, config, utils
from .settings import settings

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
    self.dirname = Path(dirname).resolve()
    self.acquired_count = 0

    # since the lock itself lives inside the cache directory we need to ensure it
    # exists.
    self.ensure()
    self.filelock_name = Path(dirname, 'cache.lock')
    self.filelock = filelock.FileLock(self.filelock_name)

  def acquire_cache_lock(self):
    if config.FROZEN_CACHE:
      # Raise an exception here rather than exit_with_error since in practice this
      # should never happen
      raise Exception('Attempt to lock the cache but FROZEN_CACHE is set')

    if not self.EM_EXCLUSIVE_CACHE_ACCESS and self.acquired_count == 0:
      logger.debug(f'PID {os.getpid()} acquiring multiprocess file lock to Emscripten cache at {self.dirname}')
      try:
        self.filelock.acquire(60)
      except filelock.Timeout:
        # The multiprocess cache locking can be disabled altogether by setting EM_EXCLUSIVE_CACHE_ACCESS=1 environment
        # variable before building. (in that case, use "embuilder.py build ALL" to prepopulate the cache)
        logger.warning(f'Accessing the Emscripten cache at "{self.dirname}" is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "{self.filelock_name}" and try again. If this occurs deterministically, consider filing a bug.')
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
      logger.debug(f'PID {os.getpid()} released multiprocess file lock to Emscripten cache at {self.dirname}')

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
      # Delete everything except the lockfile itself
      utils.delete_contents(self.dirname, exclude=[os.path.basename(self.filelock_name)])

  def get_path(self, name):
    return Path(self.dirname, name)

  def get_sysroot(self, absolute):
    if absolute:
      return os.path.join(self.dirname, 'sysroot')
    return 'sysroot'

  def get_include_dir(self, *parts):
    return str(self.get_sysroot_dir('include', *parts))

  def get_sysroot_dir(self, *parts):
    return str(Path(self.get_sysroot(absolute=True), *parts))

  def get_lib_dir(self, absolute, varies=True):
    path = Path(self.get_sysroot(absolute=absolute), 'lib')
    if settings.MEMORY64:
      path = Path(path, 'wasm64-emscripten')
    else:
      path = Path(path, 'wasm32-emscripten')
    if not varies:
      return path
    # if relevant, use a subdir of the cache
    subdir = []
    if settings.LTO:
      if settings.LTO == 'thin':
        subdir.append('thinlto')
      else:
        subdir.append('lto')
    if settings.RELOCATABLE:
      subdir.append('pic')
    if subdir:
      path = Path(path, '-'.join(subdir))
    return path

  def get_lib_name(self, name, varies=True):
    return str(self.get_lib_dir(absolute=False, varies=varies).joinpath(name))

  def erase_lib(self, name):
    self.erase_file(self.get_lib_name(name))

  def erase_file(self, shortname):
    with self.lock():
      name = Path(self.dirname, shortname)
      if name.exists():
        logger.info(f'deleting cached file: {name}')
        utils.delete_file(name)

  def get_lib(self, libname, *args, **kwargs):
    name = self.get_lib_name(libname)
    return self.get(name, *args, **kwargs)

  # Request a cached file. If it isn't in the cache, it will be created with
  # the given creator function
  def get(self, shortname, creator, what=None, force=False):
    cachename = Path(self.dirname, shortname)
    # Check for existence before taking the lock in case we can avoid the
    # lock completely.
    if cachename.exists() and not force:
      return str(cachename)

    if config.FROZEN_CACHE:
      # Raise an exception here rather than exit_with_error since in practice this
      # should never happen
      raise Exception(f'FROZEN_CACHE is set, but cache file is missing: "{shortname}" (in cache root path "{self.dirname}")')

    with self.lock():
      if cachename.exists() and not force:
        return str(cachename)
      if what is None:
        if shortname.endswith(('.bc', '.so', '.a')):
          what = 'system library'
        else:
          what = 'system asset'
      message = f'generating {what}: {shortname}... (this will be cached in "{cachename}" for subsequent builds)'
      logger.info(message)
      utils.safe_ensure_dirs(cachename.parent)
      creator(str(cachename))
      assert cachename.exists()
      logger.info(' - ok')

    return str(cachename)
