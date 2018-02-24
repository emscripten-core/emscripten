import os, logging

TAG = '1.7.5'

def get(ports, settings, shared):
  if settings.USE_HARFBUZZ == 1:
    ports.fetch_project('harfbuzz', 'https://github.com/harfbuzz/harfbuzz/releases/download/' +
      TAG + '/harfbuzz-' + TAG + '.tar.bz2', 'harfbuzz-' + TAG, is_tarbz2=True)
    def create():
      logging.info('building port: harfbuzz')
      ports.clear_project_build('harfbuzz')

      source_path = os.path.join(ports.get_dir(), 'harfbuzz', 'harfbuzz-' + TAG)
      dest_path = os.path.join(ports.get_build_dir(), 'harfbuzz')

      freetype_dir = os.path.join(ports.get_build_dir(), 'freetype')
      freetype_lib = os.path.join(freetype_dir, 'libfreetype.a')
      freetype_include = os.path.join(freetype_dir, 'include')
      freetype_include_dirs = freetype_include + ';' + os.path.join(freetype_include, 'config')

      shared.Building.configure(['cmake', '-H' + source_path, '-B' + dest_path,
        '-DHB_HAVE_FREETYPE=ON', '-DFREETYPE_LIBRARY=' + freetype_lib,
        '-DFREETYPE_INCLUDE_DIRS=' + freetype_include_dirs,
        '-DCMAKE_BUILD_TYPE=Release'])
      shared.Building.make(['make', '-C' + dest_path])
      return os.path.join(dest_path, 'libharfbuzz.a')
    return [shared.Cache.get('harfbuzz', create, what='port')]
  else:
    return []

def process_dependencies(settings):
  if settings.USE_HARFBUZZ == 1:
    settings.USE_FREETYPE = 1

def process_args(ports, args, settings, shared):
  if settings.USE_HARFBUZZ == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(), 'harfbuzz')]
  return args

def show():
  return 'harfbuzz (USE_HARFBUZZ=1; MIT license)'
