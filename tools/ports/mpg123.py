# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = '1.26.2'
HASH = 'aa63fcb08b243a1e09f7701b3d84a19d7412a87253d54d49f014fdb9e75bbc81d152a41ed750fccde901453929b2a001585a7645351b41845ad205c17a73dcc9'


def needed(settings):
  return settings.USE_MPG123


def get(ports, settings, shared):
  ports.fetch_project('mpg123', f'https://www.mpg123.de/download/mpg123-{TAG}.tar.bz2', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('mpg123', 'mpg123-' + TAG)

    src_path = os.path.join(source_path, 'src')
    libmpg123_path = os.path.join(src_path, 'libmpg123')
    compat_path = os.path.join(src_path, 'compat')

    config_h = os.path.join(os.path.dirname(__file__), 'mpg123/config.h')
    mpg123_h = os.path.join(os.path.dirname(__file__), 'mpg123/mpg123.h')
    shutil.copyfile(config_h, os.path.join(src_path, 'config.h'))
    shutil.copyfile(mpg123_h, os.path.join(libmpg123_path, 'mpg123.h'))

    # copy header to a location so it can be used as 'MPG123/'
    ports.install_headers(libmpg123_path, pattern="*123.h", target='')

    flags = [
      '-DOPT_GENERIC',
      '-DREAL_IS_FLOAT',
      '-fomit-frame-pointer',
      '-finline-functions',
      '-ffast-math',
      '-I' + src_path,
      '-I' + compat_path,
      '-I' + libmpg123_path,
    ]

    srcs = [
      os.path.join(libmpg123_path, 'dct64.c'),
      os.path.join(libmpg123_path, 'equalizer.c'),
      os.path.join(libmpg123_path, 'feature.c'),
      os.path.join(libmpg123_path, 'format.c'),
      os.path.join(libmpg123_path, 'frame.c'),
      os.path.join(libmpg123_path, 'icy.c'),
      os.path.join(libmpg123_path, 'icy2utf8.c'),
      os.path.join(libmpg123_path, 'id3.c'),
      os.path.join(libmpg123_path, 'index.c'),
      os.path.join(libmpg123_path, 'layer1.c'),
      os.path.join(libmpg123_path, 'layer2.c'),
      os.path.join(libmpg123_path, 'layer3.c'),
      os.path.join(libmpg123_path, 'lfs_alias.c'),
      os.path.join(libmpg123_path, 'libmpg123.c'),
      os.path.join(libmpg123_path, 'ntom.c'),
      os.path.join(libmpg123_path, 'optimize.c'),
      os.path.join(libmpg123_path, 'parse.c'),
      os.path.join(libmpg123_path, 'readers.c'),
      os.path.join(libmpg123_path, 'stringbuf.c'),
      os.path.join(libmpg123_path, 'synth.c'),
      os.path.join(libmpg123_path, 'synth_8bit.c'),
      os.path.join(libmpg123_path, 'synth_real.c'),
      os.path.join(libmpg123_path, 'synth_s32.c'),
      os.path.join(libmpg123_path, 'tabinit.c'),
      os.path.join(compat_path, 'compat.c'),
      os.path.join(compat_path, 'compat_dl.c'),
      os.path.join(compat_path, 'compat_str.c'),
    ]

    ports.build_port(source_path, final, 'mpg123', flags=flags, srcs=srcs)

  return [shared.cache.get_lib('libmpg123.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libmpg123.a')


def show():
  return 'mpg123 (-sUSE_MPG123=1 or --use-port=mpg123; zlib license)'
