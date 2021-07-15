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
  return settings.USE_GIFLIB


def get(ports, settings, shared):
  ports.fetch_project('giflib', f'https://storage.googleapis.com/webassembly/emscripten-ports/giflib-{VERSION}.tar.gz', f'giflib-{VERSION}', sha512hash=HASH)

  def create(final):
    logging.info('building port: giflib')

    source_path = os.path.join(ports.get_dir(), 'giflib', f'giflib-{VERSION}')
    dest_path = os.path.join(ports.get_build_dir(), 'giflib')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    ports.install_headers(dest_path)

    ports.build_port(dest_path, final)

  return [shared.Cache.get_lib('libgif.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_lib('libgif.a')


def process_args(ports):
  return []


def show():
  return 'giflib (USE_GIFLIB=1; MIT license)'
