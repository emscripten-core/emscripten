# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

VERSION = '1.0.6'
HASH = '512cbfde5144067f677496452f3335e9368fd5d7564899cb49e77847b9ae7dca598218276637cbf5ec524523be1e8ace4ad36a148ef7f4badf3f6d5a002a4bb2'


def needed(settings):
  return settings.USE_BZIP2


def get(ports, settings, shared):
  ports.fetch_project('bzip2', f'https://github.com/emscripten-ports/bzip2/archive/{VERSION}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('bzip2', 'bzip2-' + VERSION)
    ports.install_headers(source_path)

    # build
    srcs = [
      'blocksort.c', 'compress.c', 'decompress.c', 'huffman.c',
      'randtable.c', 'bzlib.c', 'crctable.c',
    ]
    ports.build_port(source_path, final, 'bzip2', srcs=srcs)

  return [shared.cache.get_lib('libbz2.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libbz2.a')


def show():
  return 'bzip2 (-sUSE_BZIP2=1 or --use-port=bzip2; BSD license)'
