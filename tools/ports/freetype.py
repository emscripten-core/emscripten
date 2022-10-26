# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'VER-2-12-1'
HASH = '3ef3e47752b7c3cd158c738d7e0194f1c9f97ac85c754b02be6ee0f7999c3c19050f713d1e975f5310a4689337463e7b54450ef62e02c3f09864f4c6b13740d9'

deps = ['zlib']


def needed(settings):
  return settings.USE_FREETYPE


def get(ports, settings, shared):
  ports.fetch_project('freetype', 'https://github.com/freetype/freetype/archive/refs/tags/' + TAG + '.zip', 'FreeType-' + TAG, sha512hash=HASH)

  def create(final):
    source_path = os.path.join(ports.get_dir(), 'freetype', 'freetype-' + TAG)
    ports.write_file(os.path.join(source_path, 'include/ftconfig.h'), ftconf_h)
    ports.install_header_dir(os.path.join(source_path, 'include'),
                             target=os.path.join('freetype2'))

    # fixes freetype + zlib duplicate symbols errors
    with open(os.path.join(source_path, 'include', 'freetype', 'config', 'ftoption.h'), 'a') as ftheader:
      ftheader.write("#define FT_CONFIG_OPTION_SYSTEM_ZLIB\n")
      ftheader.write("#define HAVE_UNISTD_H\n")
      ftheader.write("#define HAVE_FCNTL_H\n")
      ftheader.write("#define HAVE_STDINT_H\n")

    # build
    srcs = ['builds/unix/ftsystem.c',
            'src/autofit/autofit.c',
            'src/base/ftbase.c',
            'src/base/ftbbox.c',
            'src/base/ftbdf.c',
            'src/base/ftbitmap.c',
            'src/base/ftcid.c',
            'src/base/ftdebug.c',
            'src/base/ftfstype.c',
            'src/base/ftgasp.c',
            'src/base/ftglyph.c',
            'src/base/ftgxval.c',
            'src/base/ftinit.c',
            'src/base/ftmm.c',
            'src/base/ftotval.c',
            'src/base/ftpatent.c',
            'src/base/ftpfr.c',
            'src/base/ftstroke.c',
            'src/base/ftsynth.c',
            'src/base/fttype1.c',
            'src/base/ftwinfnt.c',
            'src/bdf/bdf.c',
            'src/bzip2/ftbzip2.c',
            'src/cache/ftcache.c',
            'src/cff/cff.c',
            'src/cid/type1cid.c',
            'src/gzip/ftgzip.c',
            'src/lzw/ftlzw.c',
            'src/pcf/pcf.c',
            'src/pfr/pfr.c',
            'src/psaux/psaux.c',
            'src/pshinter/pshinter.c',
            'src/psnames/psnames.c',
            'src/raster/raster.c',
            'src/sdf/sdf.c',
            'src/sfnt/sfnt.c',
            'src/smooth/smooth.c',
            'src/svg/svg.c',
            'src/truetype/truetype.c',
            'src/type1/type1.c',
            'src/type42/type42.c',
            'src/winfonts/winfnt.c']

    flags = [
      '-DFT2_BUILD_LIBRARY',
      '-I' + source_path + '/include',
      '-I' + source_path + '/truetype',
      '-I' + source_path + '/sfnt',
      '-I' + source_path + '/autofit',
      '-I' + source_path + '/smooth',
      '-I' + source_path + '/raster',
      '-I' + source_path + '/psaux',
      '-I' + source_path + '/psnames',
      '-I' + source_path + '/truetype',
      '-pthread'
    ]

    ports.build_port(source_path, final, 'freetype', flags=flags, srcs=srcs)

  return [shared.Cache.get_lib('libfreetype.a', create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.erase_lib('libfreetype.a')


def process_args(ports):
  return ['-I' + ports.get_include_dir('freetype2')]


def show():
  return 'freetype (USE_FREETYPE=1; freetype license)'


ftconf_h = r'''/***************************************************************************/
#ifndef FTCONFIG_H_
#define FTCONFIG_H_

#include <ft2build.h>
#include FT_CONFIG_OPTIONS_H
#include FT_CONFIG_STANDARD_LIBRARY_H

#include <freetype/config/integer-types.h>
#include <freetype/config/public-macros.h>
#include <freetype/config/mac-support.h>

#endif /* FTCONFIG_H_ */'''
