# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import logging

VERSION = '3.2.0'
HASH = '2e5ab5ad83a0d8801abd3f82a276f776a0ad330edc0ab843f879dd7ad3fd2e0dc0e9a3efbb6c5f2e67d14c0e37f0d9abdb40c5e25d8231a357c0025669f219c3'

deps = ['freetype']
variants = {'harfbuzz-mt': {'USE_PTHREADS': 1}}

srcs = '''
hb-aat-layout.cc
hb-aat-map.cc
hb-blob.cc
hb-buffer-serialize.cc
hb-buffer.cc
hb-common.cc
hb-draw.cc
hb-face.cc
hb-fallback-shape.cc
hb-font.cc
hb-map.cc
hb-number.cc
hb-ot-cff1-table.cc
hb-ot-cff2-table.cc
hb-ot-color.cc
hb-ot-face.cc
hb-ot-font.cc
hb-ot-layout.cc
hb-ot-map.cc
hb-ot-math.cc
hb-ot-meta.cc
hb-ot-metrics.cc
hb-ot-name.cc
hb-ot-shape-complex-arabic.cc
hb-ot-shape-complex-default.cc
hb-ot-shape-complex-hangul.cc
hb-ot-shape-complex-hebrew.cc
hb-ot-shape-complex-indic-table.cc
hb-ot-shape-complex-indic.cc
hb-ot-shape-complex-khmer.cc
hb-ot-shape-complex-myanmar.cc
hb-ot-shape-complex-syllabic.cc
hb-ot-shape-complex-thai.cc
hb-ot-shape-complex-use.cc
hb-ot-shape-complex-vowel-constraints.cc
hb-ot-shape-fallback.cc
hb-ot-shape-normalize.cc
hb-ot-shape.cc
hb-ot-tag.cc
hb-ot-var.cc
hb-set.cc
hb-shape-plan.cc
hb-shape.cc
hb-shaper.cc
hb-static.cc
hb-style.cc
hb-ucd.cc
hb-unicode.cc
hb-glib.cc
hb-ft.cc
hb-graphite2.cc
hb-uniscribe.cc
hb-gdi.cc
hb-directwrite.cc
hb-coretext.cc
'''.split()


def needed(settings):
  return settings.USE_HARFBUZZ


def get_lib_name(settings):
  return 'libharfbuzz' + ('-mt' if settings.USE_PTHREADS else '') + '.a'


def get(ports, settings, shared):
  # Harfbuzz only published `.xz` packages, but not all python builds support
  # unpacking lzma archives, so we mirror a `.gz` version:
  # See https://github.com/emscripten-core/emsdk/issues/982
  ports.fetch_project('harfbuzz', f'https://storage.googleapis.com/webassembly/emscripten-ports/harfbuzz-{VERSION}.tar.gz', sha512hash=HASH)

  def create(final):
    logging.info('building port: harfbuzz')

    source_path = os.path.join(ports.get_dir(), 'harfbuzz', 'harfbuzz-' + VERSION)
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
