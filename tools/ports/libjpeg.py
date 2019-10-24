# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import logging

VERSION = '9c'
HASH = '2b581c60ae401a79bbbe748ff2deeda5acd50bfd2ea22e5926e36d34b9ebcffb6580b0ff48e972c1441583e30e21e1ea821ca0423f9c67ce08a31dffabdbe6b7'


def get(ports, settings, shared):
  if settings.USE_LIBJPEG != 1:
    return []

  ports.fetch_project('libjpeg', 'https://dl.bintray.com/homebrew/mirror/jpeg-9c.tar.gz', 'jpeg-9c', sha512hash=HASH)

  libname = ports.get_lib_name('libjpeg')

  def create():
    logging.info('building port: libjpeg')

    source_path = os.path.join(ports.get_dir(), 'libjpeg', 'jpeg-9c')
    dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'libjpeg')

    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    open(os.path.join(dest_path, 'jconfig.h'), 'w').write(jconfig_h)

    final = os.path.join(ports.get_build_dir(), 'libjpeg', libname)
    ports.build_port(
      dest_path, final,
      exclude_files=[
        'ansi2knr.c', 'cjpeg.c', 'ckconfig.c', 'djpeg.c', 'example.c',
        'jmemansi.c', 'jmemdos.c', 'jmemmac.c', 'jmemname.c',
        'jpegtran.c', 'rdjpgcom.c', 'wrjpgcom.c',
      ]
    )
    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, shared):
  shared.Cache.erase_file(ports.get_lib_name('libjpeg'))


def process_args(ports, args, settings, shared):
  if settings.USE_LIBJPEG == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'libjpeg')]
  return args


def show():
  return 'libjpeg (USE_LIBJPEG=1; BSD license)'


jconfig_h = '''/* jconfig.h.  Generated from jconfig.cfg by configure.  */
/* jconfig.cfg --- source file edited by configure script */
/* see jconfig.txt for explanations */

#define HAVE_PROTOTYPES 1
#define HAVE_UNSIGNED_CHAR 1
#define HAVE_UNSIGNED_SHORT 1
/* #undef void */
/* #undef const */
/* #undef CHAR_IS_UNSIGNED */
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_LOCALE_H 1
/* #undef NEED_BSD_STRINGS */
/* #undef NEED_SYS_TYPES_H */
/* #undef NEED_FAR_POINTERS */
/* #undef NEED_SHORT_EXTERNAL_NAMES */
/* Define this if you get warnings about undefined structures. */
/* #undef INCOMPLETE_TYPES_BROKEN */

/* Define "boolean" as unsigned char, not enum, on Windows systems. */
#ifdef _WIN32
#ifndef __RPCNDR_H__		/* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#ifndef FALSE			/* in case these macros already exist */
#define FALSE	0		/* values of boolean */
#endif
#ifndef TRUE
#define TRUE	1
#endif
#define HAVE_BOOLEAN		/* prevent jmorecfg.h from redefining it */
#endif

#ifdef JPEG_INTERNALS

/* #undef RIGHT_SHIFT_IS_UNSIGNED */
#define INLINE __inline__
/* These are for configuring the JPEG memory manager. */
/* #undef DEFAULT_MAX_MEM */
/* #undef NO_MKTEMP */

#endif /* JPEG_INTERNALS */

#ifdef JPEG_CJPEG_DJPEG

#define BMP_SUPPORTED		/* BMP image file format */
#define GIF_SUPPORTED		/* GIF image file format */
#define PPM_SUPPORTED		/* PBMPLUS PPM/PGM image file format */
/* #undef RLE_SUPPORTED */
#define TARGA_SUPPORTED		/* Targa image file format */

/* #undef TWO_FILE_COMMANDLINE */
/* #undef NEED_SIGNAL_CATCHER */
/* #undef DONT_USE_B_MODE */

/* Define this if you want percent-done progress reports from cjpeg/djpeg. */
/* #undef PROGRESS_REPORT */

#endif /* JPEG_CJPEG_DJPEG */
'''
