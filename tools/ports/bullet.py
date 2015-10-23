import os, shutil, logging, subprocess, sys, stat

TAG = 'version_1'

def build_with_configure(ports, shared, path): # not currently used
  if not sys.platform.startswith('win'): #TODO: test on windows
     autogen_path = os.path.join(path, 'bullet', 'autogen.sh')
     os.chmod(autogen_path, os.stat(autogen_path).st_mode | 0111) #Make executable
     subprocess.Popen(["sh", "autogen.sh"], cwd=os.path.join(path, 'bullet')).wait()
  subprocess.Popen(["python", "make.py"], cwd=path).wait()
  final = os.path.join(path, 'libbullet.bc')
  shutil.copyfile(os.path.join(path, 'bullet', 'build', 'libbullet.bc'), final)
  return final

def get(ports, settings, shared):
  if settings.USE_BULLET == 1:
    ports.fetch_project('bullet', 'https://github.com/emscripten-ports/bullet/archive/' + TAG + '.zip', 'Bullet-' + TAG)
    def create():
      logging.warning('building port: bullet')
     
      source_path = os.path.join(ports.get_dir(), 'bullet', 'Bullet-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'bullet')
     
      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)

      src_path = os.path.join(dest_path, 'bullet', 'src')
      includes = []
      for root, dirs, files in os.walk(src_path, topdown=False):
          for dir in dirs:
              includes.append(os.path.join(root, dir))

      final = os.path.join(ports.get_build_dir(), 'bullet', 'libbullet.bc')
      ports.build_port(src_path, final, includes=includes, exclude_dirs=['MiniCL'])
      return final
    return [shared.Cache.get('bullet', create)]
  else:
    return []


def process_args(ports, args, settings, shared):
  if settings.USE_BULLET == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'bullet', 'bullet', 'src')]
  return args

def show():
  return 'bullet (USE_BULLET=1; zlib license)'
