# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

VERSION = '5.2.1'
HASH = '4550e53c21cb1191a4581e363fc9d0610da53f7898ca8320f0d3ef6711e76bdda2609c2df15dc94c45e28bff8de441f1227ec2da7ea827cb3c0405af4faa4736'


def needed(settings):
  return settings.USE_LIBGIF


def get(ports, settings, shared):
  ports.fetch_project('libgif', 'https://vorboss.dl.sourceforge.net/project/giflib/giflib-5.2.1.tar.gz', 'giflib-5.2.1', sha512hash=HASH)

  libname = ports.get_lib_name('libgif')

  def create():
    logging.info('building port: libgif')

    source_path = os.path.join(ports.get_dir(), 'libgif', 'giflib-5.2.1')
    dest_path = os.path.join(ports.get_build_dir(), 'libgif')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    ports.install_headers(dest_path)

    final = os.path.join(ports.get_build_dir(), 'libgif', libname)
    ports.build_port(dest_path, final)
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file(ports.get_lib_name('libgif'))


def process_args(ports):
  return []


def show():
  return 'libgif (USE_LIBGIF=1; MIT license)'
