# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import logging

TAG = '7.0.1'
HASH = 'd276f3a8c9db2efccc310747c2eac0b2eed0c8f709e74a6603cffa30847cca449ab3bb42e2cc2cfa3f192306662b901a382ff41df55aac28d1c6e928ff2f83eb'

deps = ['freetype']
variants = {'harfbuzz-mt': {'USE_PTHREADS': 1}}

srcs = ['harfbuzz.cc']


def needed(settings):
  return settings.USE_HARFBUZZ


def get_lib_name(settings):
  return 'libharfbuzz' + ('-mt' if settings.USE_PTHREADS else '') + '.a'


def get(ports, settings, shared):
  ports.fetch_project('harfbuzz', f'https://github.com/harfbuzz/harfbuzz/archive/refs/tags/{TAG}.tar.gz', sha512hash=HASH)

  def create(final):
    logging.info('building port: harfbuzz')

    source_path = os.path.join(ports.get_dir(), 'harfbuzz', 'harfbuzz-' + TAG)
    freetype_include = ports.get_include_dir('freetype2')
    ports.install_headers(os.path.join(source_path, 'src'), target='harfbuzz')

    # TODO(sbc): Look into HB_TINY, HB_LEAN, HB_MINI options.  Remove
    # HAVE_MMAP/HAVE_MPROTECT/HAVE_SYSCONF since we don't really support those?

    # These cflags are the ones that the cmake build selects when running emcmake
    # with harfbuzz
    cflags = '''
    -DHAVE_FREETYPE
    -DHAVE_ATEXIT
    -DHAVE_FALLBACK
    -DHAVE_FT_SET_VAR_BLEND_COORDINATES
    -DHAVE_INTEL_ATOMIC_PRIMITIVES
    -DHAVE_MMAP
    -DHAVE_MPROTECT
    -DHAVE_OT
    -DHAVE_STRTOD_L
    -DHAVE_SYSCONF
    -DHAVE_UCDN
    -DHAVE_UNIST_H
    -DHAVE_XLOCALE_H
    -DHAVE_SYS_MMAN_H
    -DHAVE_UNISTD_H
    -fno-rtti
    -fno-exceptions
    -O3
    -DNDEBUG
    '''.split()

    cflags += ['-I' + freetype_include, '-I' + os.path.join(freetype_include, 'config')]

    if settings.RELOCATABLE:
      cflags.append('-fPIC')

    if settings.USE_PTHREADS:
      cflags.append('-pthread')
      cflags.append('-DHAVE_PTHREAD')
    else:
      cflags.append('-DHB_NO_MT')

    # Letting HarfBuzz enable warnings through pragmas can block compiler
    # upgrades in situations where say a ToT compiler build adds a new
    # stricter warning under -Wfoowarning-subgroup. HarfBuzz pragma-enables
    # -Wfoowarning which default-enables -Wfoowarning-subgroup implicitly but
    # HarfBuzz upstream is not yet clean of warnings produced for
    # -Wfoowarning-subgroup. Hence disabling pragma warning control here.
    # See also: https://github.com/emscripten-core/emscripten/pull/18119
    cflags.append('-DHB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR')
    cflags.append('-DHB_NO_PRAGMA_GCC_DIAGNOSTIC_WARNING')

    ports.build_port(os.path.join(source_path, 'src'), final, 'harfbuzz', flags=cflags, srcs=srcs)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_FREETYPE = 1


def process_args(ports):
  return ['-I' + ports.get_include_dir('harfbuzz')]


def show():
  return 'harfbuzz (USE_HARFBUZZ=1; MIT license)'
