# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import os
import shutil

TAG = 'version_1'
HASH = '929e8d6003c06ae09593021b83323c8f1f54532b67b8ba189f4aedce52c25dc182bac474de5392c46ad5b0dea5a24928e4ede1492d52f4dd5cd58eea9be4dba7'


def needed(settings):
  return settings.USE_OGG


def get(ports, settings, shared):
  ports.fetch_project('ogg', 'https://github.com/emscripten-ports/ogg/archive/' + TAG + '.zip', 'Ogg-' + TAG, sha512hash=HASH)
  libname = 'libogg.a'

  def create():
    logging.info('building port: ogg')
    ports.clear_project_build('vorbis')

    source_path = os.path.join(ports.get_dir(), 'ogg', 'Ogg-' + TAG)
    dest_path = os.path.join(ports.get_build_dir(), 'ogg')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    open(os.path.join(dest_path, 'include', 'ogg', 'config_types.h'), 'w').write(config_types_h)

    header_dir = os.path.join(ports.get_include_dir(), 'ogg')
    shutil.rmtree(header_dir, ignore_errors=True)
    shutil.copytree(os.path.join(dest_path, 'include', 'ogg'), header_dir)

    final = os.path.join(dest_path, libname)
    ports.build_port(os.path.join(dest_path, 'src'), final)
    return final

  return [shared.Cache.get_lib(libname, create)]


def clear(ports, settings, shared):
  shared.Cache.erase_file('libogg.a')


def process_args(ports):
  return []


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
