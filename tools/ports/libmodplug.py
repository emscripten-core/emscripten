# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = '11022021'
HASH = 'f770031ad6c2152cbed8c8eab8edf2be1d27f9e74bc255a9930c17019944ee5fdda5308ea992c66a78af9fe1d8dca090f6c956910ce323f8728247c10e44036b'


def needed(settings):
  return settings.USE_MODPLUG


def get(ports, settings, shared):
  ports.fetch_project('libmodplug', f'https://github.com/jancc/libmodplug/archive/v{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('libmodplug', 'libmodplug-' + TAG)
    src_dir = os.path.join(source_path, 'src')
    libmodplug_path = os.path.join(src_dir, 'libmodplug')

    ports.write_file(os.path.join(source_path, 'config.h'), config_h)

    flags = [
      '-Wno-deprecated-register',
      '-DOPT_GENERIC',
      '-DREAL_IS_FLOAT',
      '-DHAVE_CONFIG_H',
      '-DSYM_VISIBILITY',
      '-std=gnu++14',
      '-O2',
      '-fno-exceptions',
      '-ffast-math',
      '-fno-common',
      '-fvisibility=hidden',
      '-I' + source_path,
      '-I' + libmodplug_path,
    ]
    srcs = [
      os.path.join(src_dir, 'fastmix.cpp'),
      os.path.join(src_dir, 'load_669.cpp'),
      os.path.join(src_dir, 'load_abc.cpp'),
      os.path.join(src_dir, 'load_amf.cpp'),
      os.path.join(src_dir, 'load_ams.cpp'),
      os.path.join(src_dir, 'load_dbm.cpp'),
      os.path.join(src_dir, 'load_dmf.cpp'),
      os.path.join(src_dir, 'load_dsm.cpp'),
      os.path.join(src_dir, 'load_far.cpp'),
      os.path.join(src_dir, 'load_it.cpp'),
      os.path.join(src_dir, 'load_j2b.cpp'),
      os.path.join(src_dir, 'load_mdl.cpp'),
      os.path.join(src_dir, 'load_med.cpp'),
      os.path.join(src_dir, 'load_mid.cpp'),
      os.path.join(src_dir, 'load_mod.cpp'),
      os.path.join(src_dir, 'load_mt2.cpp'),
      os.path.join(src_dir, 'load_mtm.cpp'),
      os.path.join(src_dir, 'load_okt.cpp'),
      os.path.join(src_dir, 'load_pat.cpp'),
      os.path.join(src_dir, 'load_psm.cpp'),
      os.path.join(src_dir, 'load_ptm.cpp'),
      os.path.join(src_dir, 'load_s3m.cpp'),
      os.path.join(src_dir, 'load_stm.cpp'),
      os.path.join(src_dir, 'load_ult.cpp'),
      os.path.join(src_dir, 'load_umx.cpp'),
      os.path.join(src_dir, 'load_wav.cpp'),
      os.path.join(src_dir, 'load_xm.cpp'),
      os.path.join(src_dir, 'mmcmp.cpp'),
      os.path.join(src_dir, 'modplug.cpp'),
      os.path.join(src_dir, 'snd_dsp.cpp'),
      os.path.join(src_dir, 'sndfile.cpp'),
      os.path.join(src_dir, 'snd_flt.cpp'),
      os.path.join(src_dir, 'snd_fx.cpp'),
      os.path.join(src_dir, 'sndmix.cpp'),
    ]

    ports.build_port(source_path, final, 'libmodplug', flags=flags, srcs=srcs)

    ports.install_headers(libmodplug_path, pattern="*.h", target='libmodplug')
    ports.install_headers(src_dir, pattern="modplug.h", target='libmodplug')

  return [shared.cache.get_lib('libmodplug.a', create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib('libmodplug.a')


def show():
  return 'libmodplug (-sUSE_MODPLUG=1 or --use-port=libmodplug; public domain)'


config_h = '''/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `sinf' function. */
#define HAVE_SINF 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libmodplug"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "libmodplug"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libmodplug 0.8.9.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libmodplug"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.8.9.0"

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.8.9.0"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */
'''
