# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

TAG = 'release-2.6.0'
HASH = '2175d11a90211871f2289c8d57b31fe830e4b46af7361925c2c30cd521c1c677d2ee244feb682b6d3909cf085129255934751848fc81b480ea410952d990ffe0'

deps = ['sdl2']
variants = {
  'sdl2_image-jpg':    {'SDL2_IMAGE_FORMATS': ["jpg"]},
  'sdl2_image-png':    {'SDL2_IMAGE_FORMATS': ["png"]},
  'sdl2_image-jpg-mt': {'SDL2_IMAGE_FORMATS': ["jpg"], 'PTHREADS': 1},
  'sdl2_image-png-mt': {'SDL2_IMAGE_FORMATS': ["png"], 'PTHREADS': 1},
}

OPTIONS = {
  'formats': 'A comma separated list of formats (ex: --use-port=sdl2_image:formats=png,jpg)',
}

SUPPORTED_FORMATS = {'bmp', 'gif', 'jpg', 'lbm', 'pcx', 'png',
                     'pnm', 'qoi', 'svg', 'tga', 'xcf', 'xpm', 'xv'}

# user options (from --use-port)
opts: dict[str, set] = {
  'formats': set(),
}


def needed(settings):
  return settings.USE_SDL_IMAGE == 2


def get_formats(settings):
  return opts['formats'].union(settings.SDL2_IMAGE_FORMATS)


def get_lib_name(settings):
  formats = '-'.join(sorted(get_formats(settings)))

  libname = 'libSDL2_image'
  if formats != '':
    libname += '-' + formats
  if settings.PTHREADS:
    libname += '-mt'
  if settings.SUPPORT_LONGJMP == 'wasm':
    libname += '-wasm-sjlj'
  return libname + '.a'


def get(ports, settings, shared):
  ports.fetch_project('sdl2_image', f'https://github.com/libsdl-org/SDL_image/archive/refs/tags/{TAG}.zip', sha512hash=HASH)
  libname = get_lib_name(settings)

  def create(final):
    src_dir = ports.get_dir('sdl2_image', 'SDL_image-' + TAG)
    ports.install_headers(src_dir, target='SDL2')
    srcs = '''IMG.c IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c IMG_tga.c
              IMG_tif.c IMG_xcf.c IMG_xpm.c IMG_xv.c IMG_webp.c IMG_ImageIO.m
              IMG_avif.c IMG_jxl.c IMG_svg.c IMG_qoi.c'''.split()

    flags = ['-sUSE_SDL=2', '-Wno-format-security']

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

    ports.build_port(src_dir, final, 'sdl2_image', flags=flags, srcs=srcs)

  return [shared.cache.get_lib(libname, create, what='port')]


def clear(ports, settings, shared):
  shared.cache.erase_lib(get_lib_name(settings))


def process_dependencies(settings):
  settings.USE_SDL = 2
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
  return 'sdl2_image (-sUSE_SDL_IMAGE=2 or --use-port=sdl2_image; zlib license)'
