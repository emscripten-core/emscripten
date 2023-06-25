# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import hashlib
import os
import shutil
import glob
from typing import Set
from tools import cache
from tools import config
from tools import shared
from tools import system_libs
from tools import utils
from tools.settings import settings

ports = []

ports_by_name = {}

# Variant builds that we want to support for certain ports
# {variant_name: (port_name, extra_settings)}
port_variants = {}

ports_dir = os.path.dirname(os.path.abspath(__file__))

logger = logging.getLogger('ports')


def read_ports():
  expected_attrs = ['get', 'clear', 'process_args', 'show', 'needed']
  for filename in os.listdir(ports_dir):
    if not filename.endswith('.py') or filename == '__init__.py':
      continue
    filename = os.path.splitext(filename)[0]
    port = __import__(filename, globals(), level=1)
    ports.append(port)
    port.name = filename
    ports_by_name[port.name] = port
    for a in expected_attrs:
      assert hasattr(port, a), 'port %s is missing %s' % (port, a)
    if not hasattr(port, 'process_dependencies'):
      port.process_dependencies = lambda x: 0
    if not hasattr(port, 'linker_setup'):
      port.linker_setup = lambda x, y: 0
    if not hasattr(port, 'deps'):
      port.deps = []
    if not hasattr(port, 'variants'):
      # port variants (default: no variants)
      port.variants = {}

    for variant, extra_settings in port.variants.items():
      if variant in port_variants:
        utils.exit_with_error('duplicate port variant: %s' % variant)
      port_variants[variant] = (port.name, extra_settings)

  for dep in port.deps:
    if dep not in ports_by_name:
      utils.exit_with_error('unknown dependency in port: %s' % dep)


def get_all_files_under(dirname):
  for path, _, files in os.walk(dirname):
    for name in files:
      yield os.path.join(path, name)


def dir_is_newer(dir_a, dir_b):
  assert os.path.exists(dir_a)
  assert os.path.exists(dir_b)
  files_a = [(x, os.path.getmtime(x)) for x in get_all_files_under(dir_a)]
  files_b = [(x, os.path.getmtime(x)) for x in get_all_files_under(dir_b)]
  newest_a = max([f for f in files_a], key=lambda f: f[1])
  newest_b = max([f for f in files_b], key=lambda f: f[1])
  logger.debug('newest_a: %s %s', *newest_a)
  logger.debug('newest_b: %s %s', *newest_b)
  return newest_a[1] > newest_b[1]


def maybe_copy(src, dest):
  """Just like shutil.copyfile, but will do nothing if the destination already
  exists and has the same contents as the source.

  In the case where a library is built in multiple different configurations,
  we want to avoids racing between processes that are reading headers (without
  holding the cache lock) (e.g. normal compile steps) and a process that is
  building/installing a new flavor of a given library.  In this case the
  headers will be "re-installed" but we skip the actual filesystem mods
  to avoid racing with other processes that might be reading these files.
  """
  if os.path.exists(dest) and utils.read_file(src) == utils.read_file(dest):
    return
  shutil.copyfile(src, dest)


class Ports:
  """emscripten-ports library management (https://github.com/emscripten-ports).
  """

  @staticmethod
  def get_include_dir(*parts):
    dirname = cache.get_include_dir(*parts)
    shared.safe_ensure_dirs(dirname)
    return dirname

  @staticmethod
  def install_header_dir(src_dir, target=None):
    if not target:
      target = os.path.basename(src_dir)
    dest = Ports.get_include_dir(target)
    utils.delete_dir(dest)
    logger.debug(f'installing headers: {dest}')
    shutil.copytree(src_dir, dest)

  @staticmethod
  def install_headers(src_dir, pattern='*.h', target=None):
    logger.debug('install_headers')
    dest = Ports.get_include_dir()
    assert os.path.exists(dest)
    if target:
      dest = os.path.join(dest, target)
      shared.safe_ensure_dirs(dest)
    matches = glob.glob(os.path.join(src_dir, pattern))
    assert matches, f'no headers found to install in {src_dir}'
    for f in matches:
      logger.debug('installing: ' + os.path.join(dest, os.path.basename(f)))
      maybe_copy(f, os.path.join(dest, os.path.basename(f)))

  @staticmethod
  def build_port(src_dir, output_path, port_name, includes=[], flags=[], cxxflags=[], exclude_files=[], exclude_dirs=[], srcs=[]):  # noqa
    build_dir = os.path.join(Ports.get_build_dir(), port_name)
    if srcs:
      srcs = [os.path.join(src_dir, s) for s in srcs]
    else:
      srcs = []
      for root, dirs, files in os.walk(src_dir):
        for ex in exclude_dirs:
          if ex in dirs:
            dirs.remove(ex)
        for f in files:
          ext = shared.suffix(f)
          if ext in ('.c', '.cpp') and not any((excluded in f) for excluded in exclude_files):
            srcs.append(os.path.join(root, f))

    cflags = system_libs.get_base_cflags() + ['-Werror', '-O2', '-I' + src_dir] + flags
    for include in includes:
      cflags.append('-I' + include)

    if system_libs.USE_NINJA:
      os.makedirs(build_dir, exist_ok=True)
      ninja_file = os.path.join(build_dir, 'build.ninja')
      system_libs.ensure_sysroot()
      system_libs.create_ninja_file(srcs, ninja_file, output_path, cflags=cflags)
      system_libs.run_ninja(build_dir)
    else:
      commands = []
      objects = []
      for src in srcs:
        relpath = os.path.relpath(src, src_dir)
        obj = os.path.join(build_dir, relpath) + '.o'
        dirname = os.path.dirname(obj)
        os.makedirs(dirname, exist_ok=True)
        cmd = [shared.EMCC, '-c', src, '-o', obj] + cflags
        if shared.suffix(src) in ('.cc', '.cxx', '.cpp'):
          cmd[0] = shared.EMXX
          cmd += cxxflags
        commands.append(cmd)
        objects.append(obj)

      system_libs.run_build_commands(commands)
      system_libs.create_lib(output_path, objects)

    return output_path

  @staticmethod
  def get_dir():
    dirname = config.PORTS
    shared.safe_ensure_dirs(dirname)
    return dirname

  @staticmethod
  def erase():
    dirname = Ports.get_dir()
    utils.delete_dir(dirname)

  @staticmethod
  def get_build_dir():
    return cache.get_path('ports-builds')

  name_cache: Set[str] = set()

  @staticmethod
  def fetch_project(name, url, sha512hash=None):
    # To compute the sha512 hash, run `curl URL | sha512sum`.
    fullname = os.path.join(Ports.get_dir(), name)

    if name not in Ports.name_cache: # only mention each port once in log
      logger.debug(f'including port: {name}')
      logger.debug(f'    (at {fullname})')
      Ports.name_cache.add(name)

    # EMCC_LOCAL_PORTS: A hacky way to use a local directory for a port. This
    #                   is not tested but can be useful for debugging
    #                   changes to a port.
    #
    # if EMCC_LOCAL_PORTS is set, we use a local directory as our ports. This is useful
    # for testing. This env var should be in format
    #     name=dir,name=dir
    # e.g.
    #     sdl2=/home/username/dev/ports/SDL2
    # so you could run
    #     EMCC_LOCAL_PORTS="sdl2=/home/alon/Dev/ports/SDL2" ./test/runner.py browser.test_sdl2_mouse
    # this will simply copy that directory into the ports directory for sdl2, and use that. It also
    # clears the build, so that it is rebuilt from that source.
    local_ports = os.environ.get('EMCC_LOCAL_PORTS')
    if local_ports:
      logger.warning('using local ports: %s' % local_ports)
      local_ports = [pair.split('=', 1) for pair in local_ports.split(',')]
      for local_name, path in local_ports:
        if name == local_name:
          port = ports_by_name.get(name)
          if not port:
            utils.exit_with_error('%s is not a known port' % name)
          if not hasattr(port, 'SUBDIR'):
            utils.exit_with_error(f'port {name} lacks .SUBDIR attribute, which we need in order to override it locally, please update it')
          subdir = port.SUBDIR
          target = os.path.join(fullname, subdir)

          uptodate_message = f'not grabbing local port: {name} from {path} to {fullname} (subdir: {subdir}) as the destination {target} is newer (run emcc --clear-ports if that is incorrect)'
          # before acquiring the lock we have an early out if the port already exists
          if os.path.exists(target) and dir_is_newer(path, target):
            logger.warning(uptodate_message)
            return
          with cache.lock('unpack local port'):
            # Another early out in case another process unpackage the library while we were
            # waiting for the lock
            if os.path.exists(target) and not dir_is_newer(path, target):
              logger.warning(uptodate_message)
              return
            logger.warning(f'grabbing local port: {name} from {path} to {fullname} (subdir: {subdir})')
            utils.delete_dir(fullname)
            shutil.copytree(path, target)
            Ports.clear_project_build(name)
          return

    url_filename = url.rsplit('/')[-1]
    ext = url_filename.split('.', 1)[1]
    fullpath = fullname + '.' + ext

    def retrieve():
      # retrieve from remote server
      logger.info(f'retrieving port: {name} from {url}')
      try:
        import requests
        response = requests.get(url)
        data = response.content
      except ImportError:
        from urllib.request import urlopen
        f = urlopen(url)
        data = f.read()

      if sha512hash:
        actual_hash = hashlib.sha512(data).hexdigest()
        if actual_hash != sha512hash:
          utils.exit_with_error(f'Unexpected hash: {actual_hash}\n'
                                'If you are updating the port, please update the hash.')
      utils.write_binary(fullpath, data)

    marker = os.path.join(fullname, '.emscripten_url')

    def unpack():
      logger.info(f'unpacking port: {name}')
      shared.safe_ensure_dirs(fullname)
      shutil.unpack_archive(filename=fullpath, extract_dir=fullname)
      utils.write_file(marker, url + '\n')

    def up_to_date():
      if os.path.exists(marker):
        if utils.read_file(marker).strip() == url:
          return True
      return False

    # before acquiring the lock we have an early out if the port already exists
    if up_to_date():
      return

    # main logic. do this under a cache lock, since we don't want multiple jobs to
    # retrieve the same port at once
    with cache.lock('unpack port'):
      if os.path.exists(fullpath):
        # Another early out in case another process unpackage the library while we were
        # waiting for the lock
        if up_to_date():
          return
        # file exists but tag is bad
        logger.warning('local copy of port is not correct, retrieving from remote server')
        utils.delete_dir(fullname)
        utils.delete_file(fullpath)

      retrieve()
      unpack()

      # we unpacked a new version, clear the build in the cache
      Ports.clear_project_build(name)

  @staticmethod
  def clear_project_build(name):
    port = ports_by_name[name]
    port.clear(Ports, settings, shared)
    build_dir = os.path.join(Ports.get_build_dir(), name)
    utils.delete_dir(build_dir)
    return build_dir

  @staticmethod
  def write_file(filename, contents):
    if os.path.exists(filename) and utils.read_file(filename) == contents:
      return
    utils.write_file(filename, contents)


def dependency_order(port_list):
  # Perform topological sort of ports according to the dependency DAG
  port_map = {p.name: p for p in port_list}

  # Perform depth first search of dependecy graph adding nodes to
  # the stack only after all children have been explored.
  stack = []
  unsorted = set(port_list)

  def dfs(node):
    for dep in node.deps:
      child = port_map[dep]
      if child in unsorted:
        unsorted.remove(child)
        dfs(child)
    stack.append(node)

  while unsorted:
    dfs(unsorted.pop())

  return stack


def resolve_dependencies(port_set, settings):
  def add_deps(node):
    node.process_dependencies(settings)
    for d in node.deps:
      dep = ports_by_name[d]
      if dep not in port_set:
        port_set.add(dep)
        add_deps(dep)

  for port in port_set.copy():
    add_deps(port)


def get_needed_ports(settings):
  # Start with directly needed ports, and transitively add dependencies
  needed = set(p for p in ports if p.needed(settings))
  resolve_dependencies(needed, settings)
  return needed


def build_port(port_name, settings):
  port = ports_by_name[port_name]
  port_set = {port}
  resolve_dependencies(port_set, settings)
  for port in dependency_order(port_set):
    port.get(Ports, settings, shared)


def clear_port(port_name, settings):
  port = ports_by_name[port_name]
  port.clear(Ports, settings, shared)


def clear():
  Ports.erase()


def get_libs(settings):
  """Called add link time to calculate the list of port libraries.
  Can have the side effect of building and installing the needed ports.
  """
  ret = []
  needed = get_needed_ports(settings)

  for port in dependency_order(needed):
    if port.needed(settings):
      port.linker_setup(Ports, settings)
      # ports return their output files, which will be linked, or a txt file
      ret += [f for f in port.get(Ports, settings, shared) if not f.endswith('.txt')]

  ret.reverse()
  return ret


def add_cflags(args, settings): # noqa: U100
  """Called during compile phase add any compiler flags (e.g -Ifoo) needed
  by the selected ports.  Can also add/change settings.

  Can have the side effect of building and installing the needed ports.
  """

  # Legacy SDL1 port is not actually a port at all but builtin
  if settings.USE_SDL == 1:
    args += ['-I' + Ports.get_include_dir('SDL')]

  needed = get_needed_ports(settings)

  # Now get (i.e. build) the ports in dependency order.  This is important because the
  # headers from one ports might be needed before we can build the next.
  for port in dependency_order(needed):
    port.get(Ports, settings, shared)
    args += port.process_args(Ports)


def show_ports():
  print('Available ports:')
  for port in ports:
    print('   ', port.show())


read_ports()
