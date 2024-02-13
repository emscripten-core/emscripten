# Copyright 2024 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os


def get_lib_name(settings):
  return 'lib_simple.a'


def get(ports, settings, shared):
  # for simplicity in testing, the source is in the same folder as the port and not fetched as a tarball
  source_dir = os.path.dirname(__file__)

  def create(final):
    ports.install_headers(source_dir)
    ports.build_port(source_dir, final, 'simple')

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))
