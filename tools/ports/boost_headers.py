# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = '1.70.0'
HASH = '3ba0180a4a3c20d64727750a3233c82aadba95f265a45052297b955902741edac1befd963400958d6915e5b8d9ade48195eeaf8524f06fdb4cfe43b98677f196'


def get(ports, settings, shared):
  if settings.USE_BOOST_HEADERS != 1:
    return []

  ports.fetch_project('boost_headers', 'https://github.com/emscripten-ports/boost/releases/download/boost-1.70.0/boost-headers-' + TAG + '.zip',
                      'boost', sha512hash=HASH)
  libname = ports.get_lib_name('libboost_headers')

  def create():
    logging.info('building port: boost_headers')
    ports.clear_project_build('boost_headers')

    # includes
    source_path_include = os.path.join(ports.get_dir(), 'boost_headers', 'boost')
    dest_path_include = os.path.join(ports.get_build_dir(), 'boost_headers', 'boost')
    shutil.copytree(source_path_include, dest_path_include)

    # write out a dummy cpp file, to create an empty library
    # this is needed as emscripted ports expect this, even if it is not used
    open(os.path.join(ports.get_build_dir(), 'boost_headers', 'dummy.cpp'), 'w').write('static void dummy() {}')

    commands = []
    o_s = []
    o = os.path.join(ports.get_build_dir(), 'boost_headers', 'dummy.cpp.o')
    command = [shared.PYTHON, shared.EMCC, os.path.join(ports.get_build_dir(), 'boost_headers', 'dummy.cpp'), '-o', o]
    commands.append(command)
    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'boost_headers', libname)
    o_s.append(o)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libboost_headers'))


def process_args(ports, args, settings, shared):
  if settings.USE_BOOST_HEADERS == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(), 'boost_headers'), '-DBOOST_ALL_NO_LIB']
  return args


def show():
  return 'Boost headers v1.70.0 (USE_BOOST_HEADERS=1; Boost license)'
