# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.75.0'
HASH = '8c38be1ebef1b8ada358ad6b7c9ec17f5e0a300e8085db3473a13e19712c95eeb3c3defacd3c53482eb96368987c4b022efa8da2aac2431a154e40153d3c3dcd'


def needed(settings):
  return settings.USE_BOOST_HEADERS == 1


def get(ports, settings, shared):
  ports.fetch_project('boost_headers',
                      f'https://github.com/emscripten-ports/boost/releases/download/boost-{TAG}/boost-headers-{TAG}.zip',
                      sha512hash=HASH)

  def create(final):
    # includes
    source_path = os.path.join(ports.get_dir(), 'boost_headers')
    source_path_include = os.path.join(source_path, 'boost')
    ports.install_header_dir(source_path_include, 'boost')

    # write out a dummy cpp file, to create an empty library
    # this is needed as emscripted ports expect this, even if it is not used
    dummy_file = os.path.join(source_path, 'dummy.cpp')
    shared.safe_ensure_dirs(os.path.dirname(dummy_file))
    ports.write_file(dummy_file, 'static void dummy() {}')

    ports.build_port(source_path, final, 'boost_headers', srcs=['dummy.cpp'])

  return [shared.cache.get_lib('libboost_headers.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libboost_headers.a')


def process_args(ports):
  return ['-DBOOST_ALL_NO_LIB']


def show():
  return 'Boost headers v1.70.0 (USE_BOOST_HEADERS=1; Boost license)'
