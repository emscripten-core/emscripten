# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '3.27.2'


def get(ports, settings, shared):
  if settings.USE_SQLITE != 1:
    return []

  ports.fetch_project('sqlite', 'https://sqlite.org/2019/sqlite-amalgamation-3270200.zip', 'sqlite-amalgamation-3270200')

  def create():
    ports.clear_project_build('sqlite')

    source_path = os.path.join(ports.get_dir(), 'sqlite', 'sqlite-amalgamation-3270200')
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sqlite')
    shared.try_delete(dest_path)
    os.makedirs(dest_path)
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    # build
    srcs = ['sqlite3.c']
    commands = []
    o_s = []
    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'sqlite', src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      commands.append([shared.PYTHON, shared.EMCC, os.path.join(dest_path, src), '-O2', '-o', o, '-I' + dest_path, '-w', ])
      o_s.append(o)
    ports.run_commands(commands)

    final = os.path.join(ports.get_build_dir(), 'sqlite', 'libsqlite3.a')
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get('libsqlite3.a', create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file('libsqlite3.a')


def process_args(ports, args, settings, shared):
  if settings.USE_SQLITE == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sqlite')]
  return args


def show():
  return 'sqlite (USE_SQLITE=1; Public domain)'
