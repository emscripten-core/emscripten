# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = '1.1.0'
HASH = '3ba0180a4a3c20d64727750a3233c82aadba95f265a45052297b955902741edac1befd963400958d6915e5b8d9ade48195eeaf8524f06fdb4cfe43b98677f196'


def get(ports, settings, shared):
  if settings.USE_RAPIDJSON != 1:
    return []

  ports.fetch_project('rapidjson', 'https://github.com/Tencent/rapidjson/archive/v' + TAG + '.zip',
                      'rapidjson', sha512hash=HASH)
  libname = ports.get_lib_name('librapidjson')

  def create():
    logging.info('building port: rapidjson')
    ports.clear_project_build('rapidjson')

    # includes
    source_path_include = os.path.join(ports.get_dir(), 'rapidjson', 'include')
    dest_path_include = os.path.join(ports.get_build_dir(), 'rapidjson', 'include')
    shutil.copytree(source_path_include, dest_path_include)

    # write out a dummy cpp file, to create an empty library
    # this is needed as emscripted ports expect this, even if it is not used
    open(os.path.join(ports.get_build_dir(), 'rapidjson', 'dummy.cpp'), 'w').write('static void dummy() {}')

    commands = []
    o_s = []
    o = os.path.join(ports.get_build_dir(), 'rapidjson', 'dummy.cpp.o')
    command = [shared.PYTHON, shared.EMCC, os.path.join(ports.get_build_dir(), 'rapidjson', 'dummy.cpp'), '-o', o]
    commands.append(command)
    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'rapidjson', libname)
    o_s.append(o)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('librapidjson'))


def process_args(ports, args, settings, shared):
  if settings.USE_RAPIDJSON == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(), 'rapidjson', 'include')]
  return args


def show():
  return 'Rapidjson v1.1.0 (USE_RAPIDJSON=1; MIT license)'
