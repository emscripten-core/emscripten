# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'VER-2-14-3'
# See version_info in builds/unix/configure.raw or `freetype-config --version`
PKG_VERSION = '26.2.20'
HASH = '460ea4dc9cc879822556d801341a1cf3efcd57a13cd4da64169603ea98873337dd19e3c8c6d9054b27d10acbbaa8d5c59b7f560c697b98dc2640bcd88f57554a'

variants = {
  'freetype-wasmsjlj': {'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 0},
  'freetype-legacysjlj': {'SUPPORT_LONGJMP': 'wasm', 'WASM_LEGACY_EXCEPTIONS': 1},
}
deps = ['zlib']


def needed(settings):
  return settings.USE_FREETYPE


def get_lib_name(settings):
  if settings.SUPPORT_LONGJMP == 'wasm':
    if settings.WASM_LEGACY_EXCEPTIONS:
      return 'libfreetype-legacysjlj.a'
    else:
      return 'libfreetype-wasmsjlj.a'
  else:
    return 'libfreetype.a'


def get(ports, settings, shared):
  ports.fetch_project('freetype', f'https://github.com/freetype/freetype/archive/{TAG}.zip', sha512hash=HASH)

  def create(final):
    source_path = ports.get_dir('freetype', f'freetype-{TAG}')
    # Overwrite the default config file with our own.
    ports.install_header_dir(os.path.join(source_path, 'include'),
                             target=os.path.join('freetype2'))

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
      '-DFT_CONFIG_OPTION_SYSTEM_ZLIB',
      '-DHAVE_UNISTD_H',
      '-DHAVE_FCNTL_H',
      '-I' + source_path + '/include',
      '-I' + source_path + '/truetype',
      '-I' + source_path + '/sfnt',
      '-I' + source_path + '/autofit',
      '-I' + source_path + '/smooth',
      '-I' + source_path + '/raster',
      '-I' + source_path + '/psaux',
      '-I' + source_path + '/psnames',
      '-I' + source_path + '/truetype',
      '-pthread',
    ]

    if settings.SUPPORT_LONGJMP == 'wasm':
      flags.append('-sSUPPORT_LONGJMP=wasm')
      flags.append(f'-sWASM_LEGACY_EXCEPTIONS={settings.WASM_LEGACY_EXCEPTIONS}')

    ports.make_pkg_config('freetype2', PKG_VERSION, '-sUSE_FREETYPE')
    ports.build_port(source_path, final, 'freetype', flags=flags, srcs=srcs)

  return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_args(ports):
  return ['-isystem', ports.get_include_dir('freetype2')]


def show():
  return 'freetype (-sUSE_FREETYPE or --use-port=freetype; freetype license)'
