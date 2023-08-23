# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Permanent cache for system libraries and ports.
"""

import contextlib
import logging
import os
from pathlib import Path

from . import filelock, config, utils
from .settings import settings

logger = logging.getLogger('cache')


acquired_count = 0
cachedir = None
cachelock = None
cachelock_name = None


def acquire_cache_lock(reason):
  global acquired_count
  if config.FROZEN_CACHE:
    # Raise an exception here rather than exit_with_error since in practice this
    # should never happen
    raise Exception('Attempt to lock the cache but FROZEN_CACHE is set')

  if acquired_count == 0:
    logger.debug(f'PID {os.getpid()} acquiring multiprocess file lock to Emscripten cache at {cachedir}')
    assert 'EM_CACHE_IS_LOCKED' not in os.environ, f'attempt to lock the cache while a parent process is holding the lock ({reason})'
    try:
      cachelock.acquire(60)
    except filelock.Timeout:
      logger.warning(f'Accessing the Emscripten cache at "{cachedir}" (for "{reason}") is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "{cachelock_name}" and try again. If this occurs deterministically, consider filing a bug.')
      cachelock.acquire()

    os.environ['EM_CACHE_IS_LOCKED'] = '1'
    logger.debug('done')
  acquired_count += 1


def release_cache_lock():
  global acquired_count
  acquired_count -= 1
  assert acquired_count >= 0, "Called release more times than acquire"
  if acquired_count == 0:
    assert os.environ['EM_CACHE_IS_LOCKED'] == '1'
    del os.environ['EM_CACHE_IS_LOCKED']
    cachelock.release()
    logger.debug(f'PID {os.getpid()} released multiprocess file lock to Emscripten cache at {cachedir}')


@contextlib.contextmanager
def lock(reason):
  """A context manager that performs actions in the given directory."""
  acquire_cache_lock(reason)
  try:
    yield
  finally:
    release_cache_lock()


def ensure():
  ensure_setup()
  utils.safe_ensure_dirs(cachedir)


def erase():
  ensure_setup()
  with lock('erase'):
    # Delete everything except the lockfile itself
    utils.delete_contents(cachedir, exclude=[os.path.basename(cachelock_name)])


def get_path(name):
  ensure_setup()
  return Path(cachedir, name)


def get_sysroot(absolute):
  ensure_setup()
  if absolute:
    return os.path.join(cachedir, 'sysroot')
  return 'sysroot'


def get_include_dir(*parts):
  return str(get_sysroot_dir('include', *parts))


def get_sysroot_dir(*parts):
  return str(Path(get_sysroot(absolute=True), *parts))


def get_lib_dir(absolute):
  ensure_setup()
  path = Path(get_sysroot(absolute=absolute), 'lib')
  if settings.MEMORY64:
    path = Path(path, 'wasm64-emscripten')
  else:
    path = Path(path, 'wasm32-emscripten')
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


def get_lib_name(name, absolute=False):
  return str(get_lib_dir(absolute=absolute).joinpath(name))


def erase_lib(name):
  erase_file(get_lib_name(name))


def erase_file(shortname):
  with lock('erase: ' + shortname):
    name = Path(cachedir, shortname)
    if name.exists():
      logger.info(f'deleting cached file: {name}')
      utils.delete_file(name)


def get_lib(libname, *args, **kwargs):
  name = get_lib_name(libname)
  return get(name, *args, **kwargs)


# Request a cached file. If it isn't in the cache, it will be created with
# the given creator function
def get(shortname, creator, what=None, force=False, quiet=False, deferred=False):
  ensure_setup()
  cachename = Path(cachedir, shortname)
  # Check for existence before taking the lock in case we can avoid the
  # lock completely.
  if cachename.exists() and not force:
    return str(cachename)

  if config.FROZEN_CACHE:
    # Raise an exception here rather than exit_with_error since in practice this
    # should never happen
    raise Exception(f'FROZEN_CACHE is set, but cache file is missing: "{shortname}" (in cache root path "{cachedir}")')

  with lock(shortname):
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
    if not deferred:
      assert cachename.exists()
    if not quiet:
      logger.info(' - ok')

  return str(cachename)


def setup():
  global cachedir, cachelock, cachelock_name
  # figure out the root directory for all caching
  cachedir = Path(config.CACHE).resolve()

  # since the lock itself lives inside the cache directory we need to ensure it
  # exists.
  ensure()
  cachelock_name = Path(cachedir, 'cache.lock')
  cachelock = filelock.FileLock(cachelock_name)


def ensure_setup():
  if not cachedir:
    setup()
