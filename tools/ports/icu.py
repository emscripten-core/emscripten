# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'release-68-2'
VERSION = '68_2'
HASH = '12c3db5966c234c94e7918fb8acc8bd0838edc36a620f3faa788e7ff27b06f1aa431eb117401026e3963622b9323212f444b735d5c9dd3d0b82d772a4834b993'


def needed(settings):
  return settings.USE_ICU


def get(ports, settings, shared):
  url = 'https://github.com/unicode-org/icu/releases/download/%s/icu4c-%s-src.zip' % (TAG, VERSION)
  ports.fetch_project('icu', url, 'icu', sha512hash=HASH)

  def create(final):
    logging.info('building port: icu')

    source_path = os.path.join(ports.get_dir(), 'icu', 'icu')
    dest_path = os.path.join(ports.get_build_dir(), 'icu')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    ports.build_port(os.path.join(dest_path, 'source', 'common'), final, [os.path.join(dest_path, 'source', 'common')], ['-DU_COMMON_IMPLEMENTATION=1'])

    ports.install_header_dir(os.path.join(dest_path, 'source', 'common', 'unicode'))

  return [shared.Cache.get_lib('libicuuc.a', create)]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libicuuc.a')


def process_args(ports):
  return []


def show():
  return 'icu (USE_ICU=1; Unicode License)'
