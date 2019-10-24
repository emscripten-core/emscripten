# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_1'
HASH = '3922486816cf7d99ee02c3c1ef63d94290e8ed304016dd9927137d04206e7674d9df8773a4abb7bb57783d0a5107ad0f893aa87acfb34f7b316eec22ca55a536'


def get(ports, settings, shared):
  if settings.USE_BULLET != 1:
    return []

  ports.fetch_project('bullet', 'https://github.com/emscripten-ports/bullet/archive/' + TAG + '.zip', 'Bullet-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libbullet')

  def create():
    logging.info('building port: bullet')

    source_path = os.path.join(ports.get_dir(), 'bullet', 'Bullet-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'bullet')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    src_path = os.path.join(dest_path, 'bullet', 'src')
    includes = []
    for root, dirs, files in os.walk(src_path, topdown=False):
      for dir in dirs:
        includes.append(os.path.join(root, dir))

    final = os.path.join(ports.get_build_dir(), 'bullet', libname)
    ports.build_port(src_path, final, includes=includes, exclude_dirs=['MiniCL'])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libbullet'))


def process_args(ports, args, settings, shared):
  if settings.USE_BULLET == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'bullet', 'bullet', 'src')]
  return args


def show():
  return 'bullet (USE_BULLET=1; zlib license)'
