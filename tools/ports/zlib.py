# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '1.4'
HASH = '67f3ce120210a69aa05cece6f76d5f45b50d947c095169f0d699732d0182d7d04aec1af56a3fc6bbcc2af91a57e099cd73980b17f7d96ad90aeabbeaafbf1cbc'


def needed(settings):
  return settings.USE_ZLIB


def get(ports, settings, shared):
  ports.fetch_project('zlib', f'https://github.com/madler/zlib/archive/refs/tags/v{VERSION}.tar.gz', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('zlib', 'zlib-' + VERSION)
    # Overwrite zconf.h with our own version
    zconf_h = os.path.join(os.path.dirname(__file__), 'zlib/zconf.h')
    shutil.copyfile(zconf_h, os.path.join(source_path, 'zconf.h'))
    ports.install_headers(source_path)
    ports.make_pkg_config('zlib', VERSION, '-sUSE_ZLIB')

    # build
    srcs = 'adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c'.split()
    flags = ['-Wno-deprecated-non-prototype']
    ports.build_port(source_path, final, 'zlib', srcs=srcs, flags=flags)

  return [shared.cache.get_lib('libz.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libz.a')


def show():
  return 'zlib (-sUSE_ZLIB or --use-port=zlib; zlib license)'
