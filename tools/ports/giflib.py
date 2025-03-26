# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

VERSION = '5.2.1'
HASH = '4550e53c21cb1191a4581e363fc9d0610da53f7898ca8320f0d3ef6711e76bdda2609c2df15dc94c45e28bff8de441f1227ec2da7ea827cb3c0405af4faa4736'


def needed(settings):
  return settings.USE_GIFLIB


def get(ports, settings, shared):
  ports.fetch_project('giflib', f'https://storage.googleapis.com/webassembly/emscripten-ports/giflib-{VERSION}.tar.gz', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('giflib', f'giflib-{VERSION}')
    ports.install_headers(source_path)
    exclude_files = [
      'giffix.c', 'gifecho.c', 'giffilter.c', 'gifcolor.c', 'gifecho.c', 'gifinto.c',
      'gifsponge.c', 'gif2rgb.c', 'gifbg.c', 'gifbuild.c', 'gifclrmp.c', 'gifhisto.c',
      'gifbuild.c', 'gifclrmp.c', 'gifhisto.c', 'giftext.c', 'giftool.c', 'gifwedge.c',
    ]
    ports.build_port(source_path, final, 'giflib', exclude_files=exclude_files)

  return [shared.cache.get_lib('libgif.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libgif.a')


def show():
  return 'giflib (-sUSE_GIFLIB=1 or --use-port=giflib; MIT license)'
