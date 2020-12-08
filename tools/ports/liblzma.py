# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

VERSION = '5.2.5'
HASH = '7443674247deda2935220fbc4dfc7665e5bb5a260be8ad858c8bd7d7b9f0f868f04ea45e62eb17c0a5e6a2de7c7500ad2d201e2d668c48ca29bd9eea5a73a3ce'


def needed(settings):
  return settings.USE_LIBLZMA


def get(ports, settings, shared):
  libname = ports.get_lib_name('liblzma')
  ports.fetch_project('liblzma', 'https://tukaani.org/xz/xz-' + VERSION + '.tar.gz', 'xz-' + VERSION, sha512hash=HASH)

  def create():
    ports.clear_project_build('liblzma')

    source_path = os.path.join(ports.get_dir(), 'liblzma', 'xz-' + VERSION)
    dest_path = os.path.join(ports.get_build_dir(), 'liblzma')

    shared.try_delete(dest_path)
    os.makedirs(dest_path)
    shutil.rmtree(dest_path, ignore_errors=True)
    shutil.copytree(source_path, dest_path)

    build_flags = ['-DHAVE_CONFIG_H', '-DTUKLIB_SYMBOL_PREFIX=lzma_', '-fvisibility=hidden']
    exclude_dirs = ['xzdec', 'xz', 'lzmainfo']
    exclude_files = ['crc32_small.c', 'crc64_small.c', 'crc32_tablegen.c', 'crc64_tablegen.c', 'price_tablegen.c', 'fastpos_tablegen.c'
                     'tuklib_exit.c', 'tuklib_mbstr_fw.c', 'tuklib_mbstr_width.c', 'tuklib_open_stdxxx.c', 'tuklib_progname.c']
    include_dirs_rel = ['../common', 'api', 'common', 'check', 'lz', 'rangecoder', 'lzma', 'delta', 'simple']

    open(os.path.join(dest_path, 'src', 'config.h'), 'w').write(config_h)

    final = os.path.join(dest_path, libname)
    include_dirs = [os.path.join(dest_path, 'src', 'liblzma', p) for p in include_dirs_rel]
    ports.build_port(os.path.join(dest_path, 'src'), final, flags=build_flags, exclude_dirs=exclude_dirs, exclude_files=exclude_files, includes=include_dirs)

    ports.install_headers(os.path.join(dest_path, 'src', 'liblzma', 'api'), 'lzma.h')
    ports.install_headers(os.path.join(dest_path, 'src', 'liblzma', 'api', 'lzma'), '*.h', 'lzma')

    return final

  return [shared.Cache.get(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_file(ports.get_lib_name('liblzma'))


def process_args(ports):
  return []


def show():
  return 'liblzma (USE_LIBLZMA=1; public domain)'


config_h = r'''
#define ASSUME_RAM 128
#define ENABLE_NLS 1
#define HAVE_CHECK_CRC32 1
#define HAVE_CHECK_CRC64 1
#define HAVE_CHECK_SHA256 1
#define HAVE_CLOCK_GETTIME 1
#define HAVE_DCGETTEXT 1
#define HAVE_DECL_CLOCK_MONOTONIC 1
#define HAVE_DECL_PROGRAM_INVOCATION_NAME 1
#define HAVE_DECODERS 1
#define HAVE_DECODER_ARM 1
#define HAVE_DECODER_ARMTHUMB 1
#define HAVE_DECODER_DELTA 1
#define HAVE_DECODER_IA64 1
#define HAVE_DECODER_LZMA1 1
#define HAVE_DECODER_LZMA2 1
#define HAVE_DECODER_POWERPC 1
#define HAVE_DECODER_SPARC 1
#define HAVE_DECODER_X86 1
#define HAVE_DLFCN_H 1
#define HAVE_ENCODERS 1
#define HAVE_ENCODER_ARM 1
#define HAVE_ENCODER_ARMTHUMB 1
#define HAVE_ENCODER_DELTA 1
#define HAVE_ENCODER_IA64 1
#define HAVE_ENCODER_LZMA1 1
#define HAVE_ENCODER_LZMA2 1
#define HAVE_ENCODER_POWERPC 1
#define HAVE_ENCODER_SPARC 1
#define HAVE_ENCODER_X86 1
#define HAVE_FCNTL_H 1
#define HAVE_FUTIMENS 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_LONG 1
#define HAVE_GETTEXT 1
#define HAVE_IMMINTRIN_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MBRTOWC 1
#define HAVE_MEMORY_H 1
#define HAVE_MF_BT2 1
#define HAVE_MF_BT3 1
#define HAVE_MF_BT4 1
#define HAVE_MF_HC3 1
#define HAVE_MF_HC4 1
#define HAVE_OPTRESET 1
#define HAVE_POSIX_FADVISE 1
#define HAVE_PTHREAD_CONDATTR_SETCLOCK 1
#define HAVE_PTHREAD_PRIO_INHERIT 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRUCT_STAT_ST_ATIM_TV_NSEC 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UINTPTR_T 1
#define HAVE_UNISTD_H 1
#define HAVE_VISIBILITY 1
#define HAVE_WCWIDTH 1
#define HAVE__BOOL 1
#define HAVE___BUILTIN_ASSUME_ALIGNED 1
#define HAVE___BUILTIN_BSWAPXX 1
#define MYTHREAD_POSIX 1
#define NDEBUG 1
#define PACKAGE "xz"
#define PACKAGE_BUGREPORT "lasse.collin@tukaani.org"
#define PACKAGE_NAME "XZ Utils"
#define PACKAGE_STRING "XZ Utils 5.2.5"
#define PACKAGE_TARNAME "xz"
#define PACKAGE_VERSION "5.2.5"
#define SIZEOF_SIZE_T 4
#define STDC_HEADERS 1
#define TUKLIB_CPUCORES_SYSCONF 1
#define TUKLIB_FAST_UNALIGNED_ACCESS 1
#define TUKLIB_PHYSMEM_SYSCONF 1
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif
#define VERSION "5.2.5"
'''
