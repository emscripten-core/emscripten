# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_1'
HASH = '99bee75beb662f8520bbb18ad6dbf8590d30eb3a7360899f0ac4764ca72fe8013da37c9df21e525f9d2dc5632827d4b4cea558cbc938e7fbed0c41a29a7a2dc5'


def get(ports, settings, shared):
  if settings.USE_VORBIS != 1:
    return []

  ports.fetch_project('vorbis', 'https://github.com/emscripten-ports/vorbis/archive/' + TAG + '.zip', 'Vorbis-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libvorbis')

  def create():
    logging.info('building port: vorbis')

    source_path = os.path.join(ports.get_dir(), 'vorbis', 'Vorbis-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'vorbis')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    final = os.path.join(dest_path, libname)
    ports.build_port(os.path.join(dest_path, 'lib'), final, [os.path.join(dest_path, 'include')],
                     ['-s', 'USE_OGG=1'], ['psytune', 'barkmel', 'tone', 'misc'])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libvorbis'))


def process_dependencies(settings):
  if settings.USE_VORBIS == 1:
    settings.USE_OGG = 1


def process_args(ports, args, settings, shared):
  if settings.USE_VORBIS == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'vorbis', 'include')]
  return args


def show():
  return 'vorbis (USE_VORBIS=1; zlib license)'
