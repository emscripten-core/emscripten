# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '1.83.0'
HASH = '82e3a64e55caac0254f9e6b179437ad9421943b8f73957285978dad16c24dcae6372c464909fdc085b9790662b6a3af5163140b1e456705e80fda51c5fe3c243'


def needed(settings):
  return settings.USE_BOOST_HEADERS == 1


def get(ports, settings, shared):
  ports.fetch_project('boost_headers',
                      f'https://github.com/emscripten-ports/boost/releases/download/boost-{TAG}/boost-headers-{TAG}.zip',
                      sha512hash=HASH)

  def create(final):
    # includes
    source_path = ports.get_dir('boost_headers')
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
  return 'boost_headers - Boost headers v1.70.0 (-sUSE_BOOST_HEADERS=1 or --use-port=boost_headers; Boost license)'
