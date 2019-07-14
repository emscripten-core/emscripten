# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_1'
HASH = '13828c09ebea9dd416a587fbe5e481f626073cb173a137fbcb85044c7bde1249b28af9980da0d2445b8bd1a8387e66a4629103f89031beb90a6c6226c96bc74d'

def get(ports, settings, shared):
  if settings.USE_BOOST_HEADERS != 1:
    return []

  ports.fetch_project('boost_headers', 'https://github.com/gabrielcuvillier/libboost-includes/archive/' + TAG + '.zip',
                      'libboost-includes-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libboost_headers')

  def create():
    logging.info('building port: boost_headers')
    ports.clear_project_build('boost_headers')

    # includes
    source_path_include = os.path.join(ports.get_dir(), 'boost_headers', 'libboost-includes-' + TAG, 'boost')
    dest_path_include = os.path.join(ports.get_build_dir(), 'boost_headers', 'boost')
    shutil.copytree(source_path_include, dest_path_include)

    final = os.path.join(ports.get_build_dir(), 'boost_headers', libname)
    # create a dummy empty library
    # this is needed as emscripted ports expect this, even if it is not used
    f = open(final, 'a').close()
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libboost_headers'))

def process_args(ports, args, settings, shared):
  if settings.USE_BOOST_HEADERS == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(),'boost_headers'), '-DBOOST_ALL_NO_LIB']
  return args

def show():
  return 'Boost headers v1.69.0 (USE_BOOST_HEADERS=1; Boost license)'
