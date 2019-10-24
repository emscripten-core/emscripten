# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_1'
HASH = '929e8d6003c06ae09593021b83323c8f1f54532b67b8ba189f4aedce52c25dc182bac474de5392c46ad5b0dea5a24928e4ede1492d52f4dd5cd58eea9be4dba7'


def get(ports, settings, shared):
  if settings.USE_OGG != 1:
    return []

  ports.fetch_project('ogg', 'https://github.com/emscripten-ports/ogg/archive/' + TAG + '.zip', 'Ogg-' + TAG, sha512hash=HASH)
  libname = ports.get_lib_name('libogg')

  def create():
    logging.info('building port: ogg')
    ports.clear_project_build('vorbis')

    source_path = os.path.join(ports.get_dir(), 'ogg', 'Ogg-' + TAG)
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'ogg')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    open(os.path.join(dest_path, 'include', 'ogg', 'config_types.h'), 'w').write(config_types_h)

    final = os.path.join(dest_path, libname)
    ports.build_port(os.path.join(dest_path, 'src'), final, [os.path.join(dest_path, 'include')])
    return final

  return [shared.Cache.get(libname, create)]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libogg'))


def process_args(ports, args, settings, shared):
  if settings.USE_OGG == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'ogg', 'include')]
  return args


def show():
  return 'ogg (USE_OGG=1; zlib license)'


config_types_h = '''\
#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

/* these are filled in by configure */
#define INCLUDE_INTTYPES_H 1
#define INCLUDE_STDINT_H 1
#define INCLUDE_SYS_TYPES_H 1

#if INCLUDE_INTTYPES_H
#  include <inttypes.h>
#endif
#if INCLUDE_STDINT_H
#  include <stdint.h>
#endif
#if INCLUDE_SYS_TYPES_H
#  include <sys/types.h>
#endif

typedef int16_t ogg_int16_t;
typedef uint16_t ogg_uint16_t;
typedef int32_t ogg_int32_t;
typedef uint32_t ogg_uint32_t;
typedef int64_t ogg_int64_t;

#endif
'''
