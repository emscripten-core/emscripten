import os, logging

TAG = '2.9'

def get(ports, settings, shared):
  if settings.USE_FREETYPE == 1:
    ports.fetch_project('freetype', 'https://download.savannah.gnu.org/releases/freetype/freetype-' +
      TAG + '.tar.bz2', 'freetype-' + TAG, is_tarbz2=True)
    def create():
      logging.info('building port: freetype')
      ports.clear_project_build('freetype')

      source_path = os.path.join(ports.get_dir(), 'freetype', 'freetype-' + TAG)
      dest_path = os.path.join(ports.get_build_dir(), 'freetype')

      shared.Building.configure(['cmake', '-H' + source_path, '-B' + dest_path,
        '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_INSTALL_PREFIX:PATH=' + dest_path])
      shared.Building.make(['make', '-C' + dest_path])
      shared.Building.make(['make', '-C' + dest_path, 'install'])
      return os.path.join(dest_path, 'libfreetype.a')
    return [shared.Cache.get('freetype', create, what='port')]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_FREETYPE == 1:
    get(ports, settings, shared)
    args += ['-Xclang',
      '-isystem' + os.path.join(ports.get_build_dir(), 'freetype', 'include', 'freetype', 'config'),
      '-isystem' + os.path.join(ports.get_build_dir(), 'freetype', 'include', 'freetype2')]
  return args

def show():
  return 'freetype (USE_FREETYPE=1; freetype license)'
