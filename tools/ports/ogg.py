import os, shutil, logging, subprocess, sys, stat

TAG = 'version_1'

def get(ports, settings, shared):
  if settings.USE_OGG == 1:
    ports.fetch_project('ogg', 'https://github.com/emscripten-ports/ogg/archive/' + TAG + '.zip', 'Ogg-' + TAG)
    def create():
      logging.info('building port: ogg')
      ports.clear_project_build('vorbis')
     
      source_path = os.path.join(ports.get_dir(), 'ogg', 'Ogg-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'ogg')
     
      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)

      open(os.path.join(dest_path, 'include', 'ogg', 'config_types.h'), 'w').write(config_types_h)

      final = os.path.join(dest_path, 'libogg.bc')
      ports.build_port(os.path.join(dest_path, 'src'), final, [os.path.join(dest_path, 'include')])
      return final

      return build(ports, shared, dest_path)
    return [shared.Cache.get('ogg', create)]
  else:
    return []


def process_args(ports, args, settings, shared):
  if settings.USE_OGG == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'ogg', 'include')]
  return args

def show():
  return 'ogg (USE_OGG=1; zlib license)'

config_types_h = '''
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
