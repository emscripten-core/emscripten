# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'release-62-1'
HASH = 'a990529dc03901f40424b506b6ad748010db98bbb29249e7106f259cc11e03a17cb3054bd7f7cb86116b169ff39c90f232603d762d39271fb5be94550d03f9fb'


def get(ports, settings, shared):
  if settings.USE_ICU != 1:
    return []

  ports.fetch_project('icu', 'https://github.com/unicode-org/icu/archive/' + TAG + '.zip', 'icu-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libicuuc')

  def create():
    logging.info('building port: icu')

    source_path = os.path.join(ports.get_dir(), 'icu', 'icu-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'icu')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    final = os.path.join(dest_path, libname)
    ports.build_port(os.path.join(dest_path, 'icu4c', 'source', 'common'), final, [os.path.join(dest_path, 'icu4c', 'source', 'common')], ['--std=c++11', '-DU_COMMON_IMPLEMENTATION=1'])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libicuuc'))


def process_args(ports, args, settings, shared):
  if settings.USE_ICU == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'icu', 'icu4c', 'source', 'common')]
  return args


def show():
  return 'icu (USE_ICU=1; Unicode License)'
