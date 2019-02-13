import os, shutil, logging
import stat

TAG = 'release-2.0.1'

def get(ports, settings, shared):
  if settings.USE_SDL_MIXER == 2:
    sdl_build = os.path.join(ports.get_build_dir(), 'sdl2')
    assert os.path.exists(sdl_build), 'You must use SDL2 to use SDL2_mixer'
    ports.fetch_project('sdl2-mixer', 'https://github.com/emscripten-ports/SDL2_mixer/archive/' + TAG + '.zip', 'SDL2_mixer-' + TAG)
    def create():
      cwd = os.getcwd()
      commonflags = ['--disable-shared', '--disable-music-cmd', '--enable-sdltest', '--disable-smpegtest']
      formatflags = ['--enable-music-wave', '--disable-music-mod', '--disable-music-midi', '--enable-music-ogg', '--disable-music-ogg-shared', '--disable-music-flac', '--disable-music-mp3']
      configure = os.path.join(ports.get_dir(), 'sdl2-mixer', 'SDL2_mixer-' + TAG, 'configure')
      build_dir = os.path.join(ports.get_build_dir(), 'sdl2-mixer')
      dist_dir = os.path.join(ports.get_build_dir(), 'sdl2-mixer', 'dist')
      out = os.path.join(dist_dir, 'lib', 'libSDL2_mixer.a')
      final = os.path.join(ports.get_build_dir(), 'sdl2-mixer', 'libsdl2_mixer.a')
      shared.safe_ensure_dirs(build_dir)

      try:
        os.chdir(build_dir)
        os.chmod(configure, os.stat(configure).st_mode | stat.S_IEXEC)
        ports.run_commands([[shared.EMCONFIGURE, configure, '--prefix=' + dist_dir] + formatflags + commonflags + ['CFLAGS=-s USE_VORBIS=1']])
        ports.run_commands([[shared.EMMAKE, 'make', 'install']])
        shutil.copyfile(out, final)
      finally:
        os.chdir(cwd)
      return final
    return [shared.Cache.get('sdl2-mixer', create, what='port')]
  else:
    return []

def process_dependencies(settings):
  if settings.USE_SDL_MIXER == 2:
    settings.USE_SDL = 2
    settings.USE_VORBIS = 1

def process_args(ports, args, settings, shared):
  if settings.USE_SDL_MIXER == 2:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'sdl2-mixer', 'dist', 'include')]
  return args

def show():
  return 'SDL2_mixer (USE_SDL_MIXER=2; zlib license)'

