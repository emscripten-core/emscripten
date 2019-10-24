# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '1.0.6'
HASH = '00ace5438cfa0c577e5f578d8a808613187eff5217c35164ffe044fbafdfec9e98f4192c02a7d67e01e5a5ccced630583ad1003c37697219b0f147343a3fdd12'


def get(ports, settings, shared):
  if settings.USE_BZIP2 != 1:
    return []

  ports.fetch_project('bzip2', 'https://downloads.sourceforge.net/project/bzip2/bzip2-1.0.6.tar.gz', 'bzip2-1.0.6', sha512hash=HASH)

  def create():
    ports.clear_project_build('bzip2')

    source_path = os.path.join(ports.get_dir(), 'bzip2', 'bzip2-1.0.6')
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'bzip2')
    shared.try_delete(dest_path)
    os.makedirs(dest_path)
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    # build
    srcs = [
      'blocksort.c', 'bzip2recover.c', 'compress.c', 'decompress.c',
      'huffman.c', 'randtable.c', 'unzcrash.c', 'bzip2.c', 'bzlib.c',
      'crctable.c', 'dlltest.c', 'mk251.c', 'spewG.c',
    ]
    commands = []
    o_s = []
    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'bzip2', src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      commands.append([shared.PYTHON, shared.EMCC, os.path.join(dest_path, src), '-O2', '-o', o, '-I' + dest_path, '-w', ])
      o_s.append(o)
    ports.run_commands(commands)

    final = os.path.join(ports.get_build_dir(), 'bzip2', 'libbz2.a')
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get('libbz2.a', create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file('libbz2.a')


def process_args(ports, args, settings, shared):
  if settings.USE_BZIP2 == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'bzip2')]
  return args


def show():
  return 'bzip2 (USE_BZIP2=1; BSD license)'
