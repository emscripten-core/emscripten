# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '9f'
HASH = '7f733d79cf176c690dcf127352f9aa7ec48000455944f286faae606cdeada6f6865b4a3f9f01bda8947b5b1089bb3e52d2b56879b6e871279ec5cbd1829304dc'


def needed(settings):
  return settings.USE_LIBJPEG


def get(ports, settings, shared):
  # Archive mirrored from http://www.ijg.org/files/jpegsrc.v9f.tar.gz.
  # We have issues where python urllib was not able to load from the www.ijg.org webserver
  # and was resulting in 403: Forbidden.
  ports.fetch_project('libjpeg', f'https://storage.googleapis.com/webassembly/emscripten-ports/jpegsrc.v{VERSION}.tar.gz', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('libjpeg', f'jpeg-{VERSION}')
    jconfig_h = os.path.join(os.path.dirname(__file__), 'libjpeg/jconfig.h')
    shutil.copyfile(jconfig_h, os.path.join(source_path, 'jconfig.h'))

    ports.install_headers(source_path)
    ports.make_pkg_config('libjpeg', VERSION, '-sUSE_LIBJPEG')
    excludes = [
      'ansi2knr.c', 'cjpeg.c', 'cjpegalt.c', 'ckconfig.c', 'djpeg.c', 'djpegalt.c', 'example.c',
      'jmemansi.c', 'jmemdos.c', 'jmemmac.c', 'jmemname.c',
      'jpegtran.c', 'rdjpgcom.c', 'wrjpgcom.c',
    ]
    ports.build_port(source_path, final, 'libjpeg', exclude_files=excludes)

  return [shared.cache.get_lib('libjpeg.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libjpeg.a')


def show():
  return 'libjpeg (-sUSE_LIBJPEG or --use-port=libjpeg; BSD license)'
