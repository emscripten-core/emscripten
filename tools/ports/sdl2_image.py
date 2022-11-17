# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_4'
HASH = '30a7b04652239bccff3cb1fa7cd8ae602791b5f502a96df39585c13ebc4bb2b64ba1598c0d1f5382028d94e04a5ca02185ea06bf7f4b3520f6df4cc253f9dd24'

deps = ['sdl2']
variants = {
  'sdl2_image_jpg':  {'SDL2_IMAGE_FORMATS': ["jpg"]},
  'sdl2_image_png': {'SDL2_IMAGE_FORMATS': ["png"]},
}


def needed(settings):
  return settings.USE_SDL_IMAGE == 2


def get_lib_name(settings):
  settings.SDL2_IMAGE_FORMATS.sort()
  formats = '-'.join(settings.SDL2_IMAGE_FORMATS)

  libname = 'libSDL2_image'
  if formats != '':
    libname += '_' + formats
  return libname + '.a'


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_image'
  ports.fetch_project('sdl2_image', f'https://github.com/emscripten-ports/SDL2_image/archive/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    src_dir = os.path.join(ports.get_dir(), 'sdl2_image', 'SDL2_image-' + TAG)
    ports.install_headers(src_dir, target='SDL2')
    srcs = '''IMG.c IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c IMG_tga.c
              IMG_tif.c IMG_xcf.c IMG_xpm.c IMG_xv.c IMG_webp.c IMG_ImageIO.m'''.split()

    defs = ['-O2', '-sUSE_SDL=2', '-Wno-format-security']

    for fmt in settings.SDL2_IMAGE_FORMATS:
      defs.append('-DLOAD_' + fmt.upper())

    if 'png' in settings.SDL2_IMAGE_FORMATS:
      defs += ['-sUSE_LIBPNG']

    if 'jpg' in settings.SDL2_IMAGE_FORMATS:
      defs += ['-sUSE_LIBJPEG']

    ports.build_port(src_dir, final, 'sdl2_image', flags=defs, srcs=srcs)

  return [shared.cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 2
  if 'png' in settings.SDL2_IMAGE_FORMATS:
    deps.append('libpng')
    settings.USE_LIBPNG = 1
  if 'jpg' in settings.SDL2_IMAGE_FORMATS:
    deps.append('libjpeg')
    settings.USE_LIBJPEG = 1


def process_args(ports):
  return []


def show():
  return 'SDL2_image (USE_SDL_IMAGE=2; zlib license)'
