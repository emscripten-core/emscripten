# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# sqlite amalgamation download URL uses relase year and tag
# 2022  and (3, 38, 5) -> '/2022/sqlite-amalgamation-3380500.zip'
VERSION = (3, 39, 0)
VERSION_YEAR = 2022
HASH = 'cbaf4adb3e404d9aa403b34f133c5beca5f641ae1e23f84dbb021da1fb9efdc7c56b5922eb533ae5cb6d26410ac60cb3f026085591bc83ebc1c225aed0cf37ca'

variants = {'sqlite3-mt': {'PTHREADS': 1}}


def needed(settings):
  return settings.USE_SQLITE3


def get_lib_name(settings):
  return 'libsqlite3' + ('-mt' if settings.PTHREADS else '') + '.a'


def get(ports, settings, shared):
  release = f'sqlite-amalgamation-{VERSION[0]}{VERSION[1]:02}{VERSION[2]:02}00'
  # TODO: Fetch the file from an emscripten-hosted mirror.
  ports.fetch_project('sqlite3', f'https://www.sqlite.org/{VERSION_YEAR}/{release}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('sqlite3', release)

    ports.install_headers(source_path)

    # flags are based on sqlite-autoconf output.
    # SQLITE_HAVE_ZLIB is only used by shell.c
    flags = [
      '-DSTDC_HEADERS=1',
      '-DHAVE_SYS_TYPES_H=1',
      '-DHAVE_SYS_STAT_H=1',
      '-DHAVE_STDLIB_H=1',
      '-DHAVE_STRING_H=1',
      '-DHAVE_MEMORY_H=1',
      '-DHAVE_STRINGS_H=1',
      '-DHAVE_INTTYPES_H=1',
      '-DHAVE_STDINT_H=1',
      '-DHAVE_UNISTD_H=1',
      '-DHAVE_FDATASYNC=1',
      '-DHAVE_USLEEP=1',
      '-DHAVE_LOCALTIME_R=1',
      '-DHAVE_GMTIME_R=1',
      '-DHAVE_DECL_STRERROR_R=1',
      '-DHAVE_STRERROR_R=1',
      '-DHAVE_POSIX_FALLOCATE=1',
      '-DSQLITE_OMIT_LOAD_EXTENSION=1',
      '-DSQLITE_ENABLE_MATH_FUNCTIONS=1',
      '-DSQLITE_ENABLE_FTS4=1',
      '-DSQLITE_ENABLE_FTS5=1',
      '-DSQLITE_ENABLE_RTREE=1',
      '-DSQLITE_ENABLE_GEOPOLY=1',
      '-DSQLITE_OMIT_POPEN=1',
    ]
    if settings.PTHREADS:
      flags += [
        '-pthread',
        '-DSQLITE_THREADSAFE=1',
      ]
    else:
      flags += ['-DSQLITE_THREADSAFE=0']

    ports.build_port(source_path, final, 'sqlite3', flags=flags, exclude_files=['shell.c'])

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def show():
  return 'sqlite3 (-sUSE_SQLITE3=1 or --use-port=sqlite3); public domain)'
