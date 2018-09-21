# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging, subprocess, sys, stat

TAG = 'release-62-1'

def get(ports, settings, shared):
  if settings.USE_ICU == 1:
    ports.fetch_project('icu', 'https://github.com/unicode-org/icu/archive/' + TAG + '.zip', 'icu-' + TAG)
    def create():
      logging.info('building port: icu')

      source_path = os.path.join(ports.get_dir(), 'icu', 'icu-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'icu')

      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)

      final = os.path.join(dest_path, 'libicuuc.bc')
      ports.build_port(os.path.join(dest_path, 'icu4c', 'source', 'common'), final, [os.path.join(dest_path, 'icu4c', 'source', 'common')], ['--std=c++11', '-DU_COMMON_IMPLEMENTATION=1'])
      return final
    return [shared.Cache.get('icu', create)]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_ICU == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'icu', 'icu4c', 'source', 'common')]
  return args

def show():
  return 'icu (USE_ICU=1; Unicode License)'
