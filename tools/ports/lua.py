# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil


VERSION = '5.4.3'
HASH = '3a1a3ee8694b72b4ec9d3ce76705fe179328294353604ca950c53f41b41161b449877d43318ef4501fee44ecbd6c83314ce7468d7425ba9b2903c9c32a28bbc0'


def needed(settings):
  return settings.USE_LUA


def get_lib_name(base_name, settings):
  return base_name + ('-mt' if settings.USE_PTHREADS else '') + '.a'


def get(ports, settings, shared):
  url = 'https://www.lua.org/ftp/lua-%s.tar.gz' % VERSION
  ports.fetch_project('lua', url, 'lua', sha512hash=HASH)

  def create(final):
    logging.info('building port: lua')

    source_path = os.path.join(ports.get_dir(), 'lua', 'lua-' + VERSION, 'src')
    dest_path = os.path.join(ports.get_build_dir(), 'lua')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)
    ports.build_port(dest_path, final, [dest_path], exclude_dirs=[], flags=[])

    ports.install_headers(source_path)

  return [shared.Cache.get_lib('liblua.a', create)]


def clear(ports, settings, shared):
  shared.Cache.erase_lib('liblua.a')


def process_args(ports):
  return []


def show():
  return 'lua (USE_LUA=1; MIT License)'
