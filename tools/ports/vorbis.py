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
  lib_name = ports.get_lib_name('libvorbis')
  lib_name_file = ports.get_lib_name('libvorbisfile')
  lib_name_enc = ports.get_lib_name('libvorbisenc')

  def create(library):
    def internal_create():
      logging.info('building port: vorbis')

      source_path = os.path.join(ports.get_dir(), 'vorbis', 'Vorbis-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'vorbis')

      if not create.recreated_tree:
        logging.info('recreating tree %s %s %s' % (library, source_path, dest_path))
        shutil.rmtree(dest_path, ignore_errors=True)
        shutil.copytree(source_path, dest_path)
        create.recreated_tree = True

      final = os.path.join(dest_path, library)

      if library == lib_name:
        excluded_files = ['psytune', 'barkmel', 'tone', 'misc']
        ports.build_port(os.path.join(dest_path, 'lib'), final, [os.path.join(dest_path, 'include')],
                         ['-s', 'USE_OGG=1'], excluded_files)
      if library == lib_name_file:
        ports.build_port(os.path.join(dest_path, 'lib', 'vorbisfile.c'), final, [os.path.join(dest_path, 'include')],
                         ['-s', 'USE_OGG=1'])
      if library == lib_name_enc:
        ports.build_port(os.path.join(dest_path, 'lib', 'vorbisenc.c'), final, [os.path.join(dest_path, 'include')],
                         ['-s', 'USE_OGG=1'])

      return final
    return internal_create

  create.recreated_tree = False
  return [shared.Cache.get(lib_name, create(lib_name)), shared.Cache.get(lib_name_file, create(lib_name_file)), shared.Cache.get(lib_name_enc, create(lib_name_enc))]


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
