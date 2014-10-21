import os, shutil

VERSION = 1

def get(ports, settings, shared):
  if settings.USE_SDL == 2:
    ports.fetch_project('sdl2', 'https://github.com/emscripten-ports/SDL2/archive/master.zip', VERSION)
    def setup_includes():
      # copy includes to a location so they can be used as 'SDL2/'
      include_path = os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')
      shared.try_delete(os.path.join(include_path, 'SDL2'))
      shutil.copytree(include_path, os.path.join(include_path, 'SDL2'))
    ret = [ports.build_project('sdl2', 'SDL2-master',
                               ['sh', './configure', '--host=asmjs-unknown-emscripten', '--disable-assembly', '--disable-threads', '--enable-cpuinfo=false', 'CFLAGS=-O2'],
                               [os.path.join('build', '.libs', 'libSDL2.a')],
                               setup_includes)]
    return ret
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_SDL == 1: args += ['-Xclang', '-isystem' + shared.path_from_root('system', 'include', 'SDL')]
  elif settings.USE_SDL == 2:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')]
  return args

