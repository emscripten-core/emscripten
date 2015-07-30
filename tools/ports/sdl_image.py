import os, shutil, logging

TAG = 'version_3'

def get(ports, settings, shared):
  if settings.USE_SDL_IMAGE == 2:
    sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
    assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_image'
    ports.fetch_project('sdl2-image', 'https://github.com/emscripten-ports/SDL2_image/archive/' + TAG + '.zip', 'SDL2_image-' + TAG)
    def create():
      logging.warning('building port: sdl2-image')
      shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2-image', 'SDL2_image-' + TAG, 'SDL_image.h'), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL_image.h'))
      shutil.copyfile(os.path.join(ports.get_dir(), 'sdl2-image', 'SDL2_image-' + TAG, 'SDL_image.h'), os.path.join(ports.get_build_dir(), 'sdl2', 'include', 'SDL2', 'SDL_image.h'))
      srcs = 'IMG.c IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c IMG_tga.c IMG_tif.c IMG_xcf.c IMG_xpm.c IMG_xv.c IMG_webp.c IMG_ImageIO.m'.split(' ')
      commands = []
      o_s = []
      for src in srcs:
        o = os.path.join(ports.get_build_dir(), 'sdl2-image', src + '.o')
        commands.append([shared.PYTHON, shared.EMCC, os.path.join(ports.get_dir(), 'sdl2-image', 'SDL2_image-' + TAG, src), '-O2', '-s', 'USE_SDL=2', '-o', o, '-w'])
        o_s.append(o)
      shared.safe_ensure_dirs(os.path.dirname(o_s[0]))
      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'sdl2-image', 'libsdl2_image.bc')
      shared.Building.link(o_s, final)
      return final
    return [shared.Cache.get('sdl2-image', create)]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_SDL_IMAGE == 2:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2-image', 'include')]
  return args

def show():
  return 'SDL2_image (zlib license)'

