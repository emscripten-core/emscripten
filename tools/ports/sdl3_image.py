# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

from typing import Dict, Set

TAG = 'release-3.2.4'
HASH = '102b8ea30506a1aa0a23196d807e5cdf1ff9efcbcf9db2518a3fffef82f94bda96891129c24a8008a4ec374c9dfc39d5321cf7e573f3e3b10e85b2cfbbc1ad9b'

deps = ['sdl3']
variants = {
  'sdl3_image-jpg':    {'SDL3_IMAGE_FORMATS': ["jpg"]},
  'sdl3_image-png':    {'SDL3_IMAGE_FORMATS': ["png"]},
  'sdl3_image-jpg-mt': {'SDL3_IMAGE_FORMATS': ["jpg"], 'PTHREADS': 1},
  'sdl3_image-png-mt': {'SDL3_IMAGE_FORMATS': ["png"], 'PTHREADS': 1},
}

OPTIONS = {
  'formats': 'A comma separated list of formats (ex: --use-port=sdl3_image:formats=png,jpg)',
}

SUPPORTED_FORMATS = {'avif', 'bmp', 'gif', 'jpg', 'jxl', 'lbm', 'pcx', 'png',
                     'pnm', 'qoi', 'svg', 'tga', 'tif', 'webp', 'xcf', 'xpm', 'xv'}

# user options (from --use-port)
opts: Dict[str, Set] = {
  'formats': set(),
}


def needed(settings):
  return settings.USE_SDL_IMAGE == 3


def get_formats(settings):
  return opts['formats'].union(settings.SDL3_IMAGE_FORMATS)


def get_lib_name(settings):
  formats = '-'.join(sorted(get_formats(settings)))

  libname = 'libSDL3_image'
  if formats != '':
    libname += '-' + formats
  if settings.PTHREADS:
    libname += '-mt'
  if settings.SUPPORT_LONGJMP == 'wasm':
    libname += '-wasm-sjlj'
  return libname + '.a'


def get(ports, settings, shared):
  ports.fetch_project('sdl3_image', f'https://github.com/libsdl-org/SDL_image/archive/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    src_root = ports.get_dir('sdl3_image', 'SDL_image-' + TAG)
    ports.install_header_dir(os.path.join(src_root, 'include'), target='.')
    srcs = [
      "src/IMG.c",
      "src/IMG_ImageIO.m",
      "src/IMG_WIC.c",
      "src/IMG_avif.c",
      "src/IMG_bmp.c",
      "src/IMG_gif.c",
      "src/IMG_jpg.c",
      "src/IMG_jxl.c",
      "src/IMG_lbm.c",
      "src/IMG_pcx.c",
      "src/IMG_png.c",
      "src/IMG_pnm.c",
      "src/IMG_qoi.c",
      "src/IMG_stb.c",
      "src/IMG_svg.c",
      "src/IMG_tga.c",
      "src/IMG_tif.c",
      "src/IMG_webp.c",
      "src/IMG_xcf.c",
      "src/IMG_xpm.c",
      "src/IMG_xv.c",
      "src/IMG_xxx.c",
      ]

    flags = ['-sUSE_SDL=3', '-Wno-format-security']

    formats = get_formats(settings)

    flags.extend(f'-DLOAD_{fmt.upper()}' for fmt in formats)

    if 'png' in formats:
      flags += ['-sUSE_LIBPNG']

    if 'jpg' in formats:
      flags += ['-sUSE_LIBJPEG']

    if settings.PTHREADS:
      flags += ['-pthread']

    if settings.SUPPORT_LONGJMP == 'wasm':
      flags.append('-sSUPPORT_LONGJMP=wasm')

    ports.build_port(src_root, final, 'sdl3_image', flags=flags, srcs=srcs)

  return [shared.cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 3
  formats = get_formats(settings)
  if 'png' in formats:
    deps.append('libpng')
    settings.USE_LIBPNG = 1
  if 'jpg' in formats:
    deps.append('libjpeg')
    settings.USE_LIBJPEG = 1


def handle_options(options, error_handler):
  formats = options['formats'].split(',')
  for format in formats:
    format = format.lower().strip()
    if format not in SUPPORTED_FORMATS:
      error_handler(f'{format} is not a supported format')
    else:
      opts['formats'].add(format)


def show():
  return 'sdl3_image (-sUSE_SDL_IMAGE=3 or --use-port=sdl3_image; zlib license)'
