# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_1'
HASH = '929e8d6003c06ae09593021b83323c8f1f54532b67b8ba189f4aedce52c25dc182bac474de5392c46ad5b0dea5a24928e4ede1492d52f4dd5cd58eea9be4dba7'


def needed(settings):
  return settings.USE_OGG


def get(ports, settings, shared):
  ports.fetch_project('ogg', f'https://github.com/emscripten-ports/ogg/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('ogg', 'Ogg-' + TAG)
    ports.write_file(os.path.join(source_path, 'include', 'ogg', 'config_types.h'), config_types_h)
    ports.install_headers(os.path.join(source_path, 'include', 'ogg'), target='ogg')
    ports.build_port(os.path.join(source_path, 'src'), final, 'ogg')

  return [shared.cache.get_lib('libogg.a', create)]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libogg.a')


def show():
  return 'ogg (-sUSE_OGG=1 or --use-port=ogg; zlib license)'


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
