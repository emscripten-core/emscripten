# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

TAG = 'version_1'
HASH = '3922486816cf7d99ee02c3c1ef63d94290e8ed304016dd9927137d04206e7674d9df8773a4abb7bb57783d0a5107ad0f893aa87acfb34f7b316eec22ca55a536'


def needed(settings):
  return settings.USE_BULLET == 1


def get(ports, settings, shared):
  ports.fetch_project('bullet', f'https://github.com/emscripten-ports/bullet/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('bullet', 'Bullet-' + TAG)
    src_path = os.path.join(source_path, 'bullet', 'src')

    dest_include_path = ports.get_include_dir('bullet')
    for base, _, files in os.walk(src_path):
      for f in files:
        if shared.suffix(f) != '.h':
          continue
        fullpath = os.path.join(base, f)
        relpath = os.path.relpath(fullpath, src_path)
        target = os.path.join(dest_include_path, relpath)
        shared.safe_ensure_dirs(os.path.dirname(target))
        shutil.copyfile(fullpath, target)

    includes = []
    for base, dirs, _ in os.walk(src_path, topdown=False):
      for dir in dirs:
        includes.append(os.path.join(base, dir))

    flags = [
      '-Wno-single-bit-bitfield-constant-conversion',
      '-std=gnu++14'
    ]

    ports.build_port(src_path, final, 'bullet', includes=includes, flags=flags, exclude_dirs=['MiniCL'])

  return [shared.cache.get_lib('libbullet.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libbullet.a')


def process_args(ports):
  return ['-isystem', ports.get_include_dir('bullet')]


def show():
  return 'bullet (-sUSE_BULLET=1 or --use-port=bullet; zlib license)'
