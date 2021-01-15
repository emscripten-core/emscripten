# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os

TAG = 'version_4'
HASH = '30a7b04652239bccff3cb1fa7cd8ae602791b5f502a96df39585c13ebc4bb2b64ba1598c0d1f5382028d94e04a5ca02185ea06bf7f4b3520f6df4cc253f9dd24'


def needed(settings):
  return settings.USE_SDL_IMAGE == 2


def get(ports, settings, shared):
  sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
  assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_image'
  ports.fetch_project('sdl2_image', 'https://github.com/emscripten-ports/SDL2_image/archive/' + TAG + '.zip', 'SDL2_image-' + TAG, sha512hash=HASH)

  settings.SDL2_IMAGE_FORMATS.sort()
  formats = '-'.join(settings.SDL2_IMAGE_FORMATS)

  libname = 'libSDL2_image'
  if formats != '':
    libname += '_' + formats
  libname += '.a'

  def create():
    src_dir = os.path.join(ports.get_dir(), 'sdl2_image', 'SDL2_image-' + TAG)
    ports.install_headers(src_dir, target='SDL2')
    srcs = '''IMG.c IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c IMG_tga.c
              IMG_tif.c IMG_xcf.c IMG_xpm.c IMG_xv.c IMG_webp.c IMG_ImageIO.m'''.split()
    commands = []
    o_s = []
    defs = []

    for fmt in settings.SDL2_IMAGE_FORMATS:
      defs.append('-DLOAD_' + fmt.upper())

    if 'png' in settings.SDL2_IMAGE_FORMATS:
      defs += ['-s', 'USE_LIBPNG=1']

    if 'jpg' in settings.SDL2_IMAGE_FORMATS:
      defs += ['-s', 'USE_LIBJPEG=1']

    for src in srcs:
      o = os.path.join(ports.get_build_dir(), 'sdl2_image', src + '.o')
      commands.append([shared.EMCC, '-c', os.path.join(src_dir, src),
                       '-O2', '-s', 'USE_SDL=2', '-o', o, '-w'] + defs)
      o_s.append(o)
    shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
    ports.run_commands(commands)
    final = os.path.join(ports.get_build_dir(), 'sdl2_image', libname)
    ports.create_lib(final, o_s)
    return final

  return [shared.Cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.Cache.get_path('libSDL2_image.a')


def process_dependencies(settings):
  global deps
  deps = ['sdl2']
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
