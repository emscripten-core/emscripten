import os

def get(ports, settings, shared):
  if settings.USE_SDL == 2:
    ports.fetch_project('sdl2', 'https://github.com/emscripten-ports/SDL2/archive/master.zip')
    return [ports.build_project('sdl2', 'SDL2-master',
                               ['sh', './configure', '--host=asmjs-unknown-emscripten', '--disable-assembly', '--disable-threads', '--enable-cpuinfo=false', 'CFLAGS=-O2'],
                               [os.path.join('build', '.libs', 'libSDL2.a')])]
  else:
    return []

def process_args(args, settings, shared):
  if settings.USE_SDL == 1: args += ['-Xclang', '-isystem' + shared.path_from_root('system', 'include', 'SDL')]
  elif settings.USE_SDL == 2: args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2', 'include')]
  return args

