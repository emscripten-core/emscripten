# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = 'version_1'
HASH = '77f7d8f18fe11bb66a57e358325b7422d721f7b506bd63293cfde74079f958864db66ead5a36c311a76dd8c2b089b7659641a5522de650de0f9e6865782a60dd'


def needed(settings):
  return True


def get(ports, settings, shared):
  ports.fetch_project('libarchive', 'https://github.com/libarchive/libarchive/releases/download/v' + TAG + '/libarchive-' + TAG + '.tar.gz', 'libarchive-' + TAG, sha512hash=HASH)

  def create():
    ports.clear_project_build('libarchive')

    source_path = os.path.join(ports.get_dir(), 'libarchive', 'libarchive-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'libarchive')
    shared.try_delete(dest_path)
    os.makedirs(dest_path)
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)
    ports.install_headers(dest_path)

    # build
    srcs = 'adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c infback.c'
    'inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c'.split()
    commands = []
    o_s = []
    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'libarchive', src + '.o')
      shared.safe_ensure_dirs(os.path.dirname(o))
      commands.append([shared.EMCC, os.path.join(dest_path, src), '-O2', '-o', o, '-I' + dest_path, '-w', '-c'])
      o_s.append(o)
    ports.run_commands(commands)

    final = os.path.join(ports.get_build_dir(), 'libarchive', 'libarchive.a')
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get('libarchive.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libarchive.a')


def process_args(ports):
  return []


def show():
  return 'libarchive'
