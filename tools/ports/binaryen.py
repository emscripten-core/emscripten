import os, shutil, logging, subprocess, multiprocessing, re

TAG = 'version_1'

def needed(settings, shared):
  if not settings.BINARYEN: return False
  try:
    if shared.BINARYEN_ROOT: # if defined, and not falsey, we don't need the port
      logging.debug('binaryen root already set to ' + shared.BINARYEN_ROOT)
      return False
  except:
    pass
  return True

def get(ports, settings, shared):
  if not needed(settings, shared):
    return []
  ports.fetch_project('binaryen', 'https://github.com/WebAssembly/binaryen/archive/' + TAG + '.zip', 'binaryen-' + TAG)
  def create():
    logging.warning('building port: binaryen')
    # TODO: refactor into Ports.build_native()?
    old = os.getcwd()
    try:
      os.chdir(os.path.join(ports.get_dir(), 'binaryen', 'binaryen-' + TAG))

      # Configure
      subprocess.check_call(['cmake', '.'])

      # Check which CMake generator CMake used so we know which form to pass parameters to make/msbuild/etc. build tool.
      generator = re.search('CMAKE_GENERATOR:INTERNAL=(.*)$', open('CMakeCache.txt', 'r').read(), re.MULTILINE).group(1)

      # Make variants support '-jX' for number of cores to build, MSBuild does /maxcpucount:X
      num_cores = os.environ.get('EMCC_CORES') or str(multiprocessing.cpu_count())
      make_args = []
      if 'Makefiles' in generator: make_args = ['--', '-j', num_cores]
      elif 'Visual Studio' in generator: make_args = ['--', '/maxcpucount:' + num_cores]

      # Kick off the build.
      subprocess.check_call(['cmake', '--build', '.'] + make_args)
    finally:
      os.chdir(old)
    # the "output" of this port build is a tag file, saying which port we have
    tag_file = os.path.join(ports.get_dir(), 'binaryen', 'tag.txt')
    open(tag_file, 'w').write(TAG)
    return tag_file
  return [shared.Cache.get('binaryen-tag', create, what='port', extension='.txt')]

def process_args(ports, args, settings, shared):
  if not needed(settings, shared):
    return args
  get(ports, settings, shared)
  shared.BINARYEN_ROOT = os.path.join(ports.get_dir(), 'binaryen', 'binaryen-' + TAG)
  logging.debug('setting binaryen root to ' + shared.BINARYEN_ROOT)
  return args

def show():
  return 'Binaryen (Apache 2.0 license)'

