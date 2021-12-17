# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
from pathlib import Path

TAG = '1.5.0'
HASH = '25b657529a698eec891f92ff4a085d1fd95d2ff938ce52c8a4ff6163eb0b668ec642dd09e0db190652638cd92371006afa01d8e437437762c4097ad301675c33'


def needed(settings):
  return settings.USE_ZSTD


def get(ports, settings, shared):
  ports.fetch_project('zstd', 'https://github.com/facebook/zstd/archive/refs/tags/v%s.tar.gz' % TAG, 'zstd-' + TAG, sha512hash=HASH)

  def create(final):
    ports.clear_project_build('zstd')

    source_path = os.path.join(ports.get_dir(), 'zstd', 'zstd-' + TAG, 'lib')
    dest_path = os.path.join(ports.get_build_dir(), 'zstd')
    shared.try_delete(dest_path)
    os.makedirs(dest_path)
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)
    ports.build_port(dest_path, final, [dest_path], exclude_dirs=[], flags=[])

    ports.install_headers(dest_path)

  return [shared.Cache.get_lib('libzstd.a', create)]


def clear(ports, settings, shared):
  shared.Cache.erase_lib('libzstd.a')


def process_args(ports):
  return []


def show():
  return 'zstd (USE_ZSTD=1; GPL2 license)'
