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


acquired_count = {}
cachedir = None
cache_file_locks = {}
global_cachelock = 'cache'


def is_writable(path):
  return os.access(path, os.W_OK)


def acquire_cache_lock(reason, cachefile):
  global acquired_count
  if config.FROZEN_CACHE:
    # Raise an exception here rather than exit_with_error since in practice this
    # should never happen
    raise Exception('Attempt to lock the cache but FROZEN_CACHE is set')

  if not is_writable(cachedir):
    utils.exit_with_error(f'cache directory "{cachedir}" is not writable while accessing cache for: {reason} (see https://emscripten.org/docs/tools_reference/emcc.html for info on setting the cache directory)')

  # TODO: create a name-mangling scheme so we don't have to keep the .lock files next to the real files
  with cache_file_locks[global_cachelock].acquire():
    setup_file(cachefile)
  # TODO: is aqcuired_count even necessary? filelock.py seems to have similar logic inside.
  if acquired_count[cachefile] == 0:
    logger.debug(f'PID {os.getpid()} acquiring multiprocess file lock to Emscripten cache at {cachedir} for {cachefile}')
    #assert 'EM_CACHE_IS_LOCKED' not in os.environ, f'attempt to lock the cache while a parent process is holding the lock ({reason})'
    try:
      cache_file_locks[cachefile].acquire(60)
    except filelock.Timeout:
      logger.warning(f'Accessing the Emscripten cache at "{cachedir}" (for "{reason}") is taking a long time, another process should be writing to it. If there are none and you suspect this process has deadlocked, try deleting the lock file "{global_cachelock}" and try again. If this occurs deterministically, consider filing a bug.')
      cache_file_locks[cachefile].acquire()

    #os.environ['EM_CACHE_IS_LOCKED'] = '1'
    logger.debug('done')
  acquired_count[cachefile] += 1


def release_cache_lock(cachefile):
  global acquired_count
  acquired_count[cachefile] -= 1
  assert acquired_count[cachefile] >= 0, "Called release more times than acquire"
  if acquired_count[cachefile] == 0:
    #assert os.environ['EM_CACHE_IS_LOCKED'] == '1'
    #del os.environ['EM_CACHE_IS_LOCKED']
    cache_file_locks[cachefile].release()
    logger.debug(f'PID {os.getpid()} released multiprocess file lock to Emscripten cache at {cachedir} for {cachefile}')


@contextlib.contextmanager
def lock(reason, cachefile=global_cachelock):
  """A context manager that performs actions in the given directory."""
  acquire_cache_lock(reason, cachefile)
  try:
    yield
  finally:
    release_cache_lock(cachefile)


def ensure():
  ensure_setup()
  if not os.path.isdir(cachedir):
    try:
      utils.safe_ensure_dirs(cachedir)
    except Exception as e:
      utils.exit_with_error(f'unable to create cache directory "{cachedir}": {e} (see https://emscripten.org/docs/tools_reference/emcc.html for info on setting the cache directory)')


def erase():
  ensure_setup()
  with lock('erase', global_cachelock):
    # Delete everything except the lockfile itself
    utils.delete_contents(cachedir, exclude=[os.path.basename(global_cachelock)])


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
  name = Path(cachedir, shortname)
  with lock('erase: ' + shortname, shortname):
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

  with lock(shortname, shortname):
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


def setup_file(cache_file):
  global cachedir, cache_file_locks, acquired_count
  utils.safe_ensure_dirs(Path(cachedir, cache_file).parent)
  filename = Path(cachedir, str(cache_file) + '.lock')
  cache_file_locks[cache_file] = filelock.FileLock(filename)
  acquired_count[cache_file] = 0


def setup():
  global cachedir, global_cachelock
  # figure out the root directory for all caching
  cachedir = Path(config.CACHE).resolve()

  # since the lock itself lives inside the cache directory we need to ensure it
  # exists.
  ensure()
  setup_file(global_cachelock)


def ensure_setup():
  if not cachedir:
    setup()
