import os, shutil, logging, subprocess, sys, stat, ogg

TAG = 'version_1'

def get(ports, settings, shared):
  if settings.USE_VORBIS == 1:
    ports.fetch_project('vorbis', 'https://github.com/emscripten-ports/vorbis/archive/' + TAG + '.zip', 'Vorbis-' + TAG)
    def create():
      logging.info('building port: vorbis')
     
      source_path = os.path.join(ports.get_dir(), 'vorbis', 'Vorbis-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'vorbis')
     
      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)

      final = os.path.join(dest_path, 'libvorbis.bc')
      ports.build_port(os.path.join(dest_path, 'lib'), final, [os.path.join(dest_path, 'include')],
                       ['-s', 'USE_OGG=1'], ['psytune', 'barkmel', 'tone', 'misc'])
      return final
    return [shared.Cache.get('vorbis', create)]
  else:
    return []

def process_dependencies(settings):
  if settings.USE_VORBIS == 1:
    settings.USE_OGG = 1

def process_args(ports, args, settings, shared):
  if settings.USE_VORBIS == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'vorbis', 'include')]
  return args

def show():
  return 'vorbis (USE_VORBIS=1; zlib license)'
