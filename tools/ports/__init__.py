# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import glob
import hashlib
import importlib.util
import logging
import os
import shutil
import subprocess
import sys
from inspect import signature
from pathlib import Path
from urllib.request import urlopen

from tools import cache, config, shared, system_libs, utils
from tools.settings import settings
from tools.toolchain_profiler import ToolchainProfiler

ports = []

ports_by_name: dict[str, object] = {}

ports_needed = set()

# Variant builds that we want to support for certain ports
# {variant_name: (port_name, extra_settings)}
port_variants = {}

ports_dir = os.path.dirname(os.path.abspath(__file__))

logger = logging.getLogger('ports')


def get_port_by_name(name):
  port = ports_by_name[name]
  if port.is_external:
    load_external_port(port)
    return ports_by_name[name]
  else:
    return port


def init_port(name, port):
  ports.append(port)
  port.is_contrib = name.startswith('contrib.')
  port.is_external = hasattr(port, 'EXTERNAL_PORT')
  port.name = name
  ports_by_name[port.name] = port
  if port.is_external:
    init_external_port(name, port)
  else:
    init_local_port(name, port)


def init_local_port(name, port):
  if not hasattr(port, 'needed'):
    port.needed = lambda s: name in ports_needed
  else:
    needed = port.needed
    port.needed = lambda s: needed(s) or name in ports_needed
  if not hasattr(port, 'process_dependencies'):
    port.process_dependencies = lambda x: 0
  if not hasattr(port, 'linker_setup'):
    port.linker_setup = lambda x, y: 0
  if not hasattr(port, 'deps'):
    port.deps = []
  if not hasattr(port, 'process_args'):
    port.process_args = lambda x: []
  if not hasattr(port, 'variants'):
    # port variants (default: no variants)
    port.variants = {}
  if not hasattr(port, 'show'):
    port.show = lambda: f'{port.name} (--use-port={port.name}; {port.LICENSE})'

  for variant, extra_settings in port.variants.items():
    if variant in port_variants:
      utils.exit_with_error('duplicate port variant: `%s`' % variant)
    port_variants[variant] = (port.name, extra_settings)

  validate_port(port)


def load_port_module(module_name, port_file):
  spec = importlib.util.spec_from_file_location(module_name, port_file)
  port = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(port)
  return port


def load_external_port(external_port):
  name = external_port.name
  up_to_date = Ports.fetch_port_artifact(name, external_port.EXTERNAL_PORT, external_port.SHA512)
  port_file = os.path.join(Ports.get_dir(), name, external_port.PORT_FILE)
  local_port = load_port_module(f'tools.ports.external.{name}', port_file)
  ports.remove(external_port)
  for a in ['URL', 'DESCRIPTION', 'LICENSE']:
    if not hasattr(local_port, a):
      setattr(local_port, a, getattr(external_port, a))
  init_port(name, local_port)
  if not up_to_date:
    Ports.clear_project_build(name)


def init_external_port(name, port):
  expected_attrs = ['SHA512', 'PORT_FILE', 'URL', 'DESCRIPTION', 'LICENSE']
  for a in expected_attrs:
    assert hasattr(port, a), 'port %s is missing %s' % (port, a)
  port.needed = lambda s: name in ports_needed
  port.show = lambda: f'{port.name} (--use-port={port.name}; {port.LICENSE})'


def load_port(path, name=None):
  if not name:
    name = utils.unsuffixed_basename(path)
  if name in ports_by_name:
    utils.exit_with_error(f'port path [`{path}`] is invalid: duplicate port name `{name}`')
  port = load_port_module(f'tools.ports.{name}', path)
  init_port(name, port)
  return name


def validate_port(port):
  expected_attrs = ['get', 'clear', 'show']
  if port.is_contrib:
    expected_attrs += ['URL', 'DESCRIPTION', 'LICENSE']
  if hasattr(port, 'handle_options'):
    expected_attrs += ['OPTIONS']
  for a in expected_attrs:
    assert hasattr(port, a), 'port %s is missing %s' % (port, a)


@ToolchainProfiler.profile()
def read_ports():
  for filename in os.listdir(ports_dir):
    if not filename.endswith('.py') or filename == '__init__.py':
      continue
    load_port(os.path.join(ports_dir, filename))

  contrib_dir = os.path.join(ports_dir, 'contrib')
  for filename in os.listdir(contrib_dir):
    if not filename.endswith('.py') or filename == '__init__.py':
      continue
    name = 'contrib.' + utils.unsuffixed(filename)
    load_port(os.path.join(contrib_dir, filename), name)


def get_all_files_under(dirname):
  for path, _, files in os.walk(dirname):
    for name in files:
      yield os.path.join(path, name)


def dir_is_newer(dir_a, dir_b):
  assert os.path.exists(dir_a)
  assert os.path.exists(dir_b)
  files_a = ((x, os.path.getmtime(x)) for x in get_all_files_under(dir_a))
  files_b = ((x, os.path.getmtime(x)) for x in get_all_files_under(dir_b))
  newest_a = max(files_a, key=lambda f: f[1])
  newest_b = max(files_b, key=lambda f: f[1])
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
  if os.path.exists(dest) and utils.read_binary(src) == utils.read_binary(dest):
    return
  shutil.copyfile(src, dest)


class Ports:
  """emscripten-ports library management (https://github.com/emscripten-ports).
  """

  @staticmethod
  def get_include_dir(*parts):
    dirname = cache.get_include_dir(*parts)
    utils.safe_ensure_dirs(dirname)
    return dirname

  @staticmethod
  def install_header_dir(src_dir, target=None):
    """Like install_headers but recursively copied all files in a directory"""
    if not target:
      target = os.path.basename(src_dir)
    dest = Ports.get_include_dir(target)
    logger.debug(f'installing headers: {dest}')
    shutil.copytree(src_dir, dest, dirs_exist_ok=True, copy_function=maybe_copy)

  @staticmethod
  def install_file(filename, target):
    sysroot = cache.get_sysroot_dir()
    target_dir = os.path.join(sysroot, os.path.dirname(target))
    os.makedirs(target_dir, exist_ok=True)
    maybe_copy(filename, os.path.join(sysroot, target))

  @staticmethod
  def install_headers(src_dir, pattern='*.h', target=None):
    logger.debug('install_headers')
    dest = Ports.get_include_dir()
    assert os.path.exists(dest)
    if target:
      dest = os.path.join(dest, target)
      utils.safe_ensure_dirs(dest)
    matches = glob.glob(os.path.join(src_dir, pattern))
    assert matches, f'no headers found to install in {src_dir}'
    for f in matches:
      logger.debug('installing: ' + os.path.join(dest, os.path.basename(f)))
      maybe_copy(f, os.path.join(dest, os.path.basename(f)))

  @staticmethod
  def build_port(src_dir, output_path, port_name, includes=[], flags=[], cxxflags=[], exclude_files=[], exclude_dirs=[], srcs=[]):  # noqa
    mangled_name = str(Path(output_path).relative_to(Path(cache.get_sysroot(True)) / 'lib'))
    mangled_name = mangled_name.replace(os.sep, '_').replace('.a', '').replace('-emscripten', '')
    build_dir = os.path.join(Ports.get_build_dir(), port_name, mangled_name)
    logger.debug(f'build_port: {port_name} {output_path} in {build_dir}')
    if srcs:
      srcs = [os.path.join(src_dir, s) for s in srcs]
    else:
      srcs = []
      for root, dirs, files in os.walk(src_dir):
        for ex in exclude_dirs:
          if ex in dirs:
            dirs.remove(ex)
        for f in files:
          ext = utils.suffix(f)
          if ext in ('.c', '.cpp') and not any((excluded in f) for excluded in exclude_files):
            srcs.append(os.path.join(root, f))

    cflags = system_libs.get_base_cflags(build_dir) + ['-O2', '-I' + src_dir] + flags
    for include in includes:
      cflags.append('-I' + include)

    if system_libs.USE_NINJA:
      os.makedirs(build_dir, exist_ok=True)
      ninja_file = os.path.join(build_dir, 'build.ninja')
      system_libs.ensure_sysroot()
      system_libs.create_ninja_file(srcs, ninja_file, output_path, cflags=cflags)
      if not os.getenv('EMBUILDER_PORT_BUILD_DEFERRED'):
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
        if utils.suffix(src) in ('.cc', '.cxx', '.cpp'):
          cmd[0] = shared.EMXX
          cmd += cxxflags
        commands.append(cmd)
        objects.append(obj)

      system_libs.run_build_commands(commands, num_inputs=len(srcs))
      system_libs.create_lib(output_path, objects)

    return output_path

  @staticmethod
  def get_dir(*parts):
    dirname = os.path.join(config.PORTS, *parts)
    utils.safe_ensure_dirs(dirname)
    return dirname

  @staticmethod
  def erase():
    dirname = Ports.get_dir()
    utils.delete_dir(dirname)

  @staticmethod
  def get_build_dir():
    return system_libs.get_build_dir()

  name_cache: set[str] = set()

  @staticmethod
  def fetch_port_artifact(name, url, sha512hash=None):
    """This function only fetches the port and returns True when the port is up to date, False otherwise"""
    # To compute the sha512 hash, run `curl URL | sha512sum`.
    fullname = Ports.get_dir(name)

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
            return True
          with cache.lock('unpack local port'):
            # Another early out in case another process unpackage the library while we were
            # waiting for the lock
            if os.path.exists(target) and not dir_is_newer(path, target):
              logger.warning(uptodate_message)
              return True
            logger.warning(f'grabbing local port: {name} from {path} to {fullname} (subdir: {subdir})')
            utils.delete_dir(fullname)
            shutil.copytree(path, target)
            return False

    url_filename = url.rsplit('/')[-1]
    ext = url_filename.split('.', 1)[1]
    fullpath = fullname + '.' + ext

    def retrieve():
      # retrieve from remote server
      logger.info(f'retrieving port: {name} from {url}')

      if utils.MACOS:
        # Use `curl` over `urllib` on macOS to avoid issues with
        # certificate verification.
        # https://stackoverflow.com/questions/40684543/how-to-make-python-use-ca-certificates-from-mac-os-truststore
        # Unlike on Windows or Linux, curl is guaranteed to always be
        # available on macOS.
        data = subprocess.check_output(['curl', '-sSL', url])
      else:
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
      utils.safe_ensure_dirs(fullname)
      shutil.unpack_archive(filename=fullpath, extract_dir=fullname)
      utils.write_file(marker, url + '\n')

    def up_to_date():
      return os.path.exists(marker) and utils.read_file(marker).strip() == url

    # before acquiring the lock we have an early out if the port already exists
    if up_to_date():
      return True

    # main logic. do this under a cache lock, since we don't want multiple jobs to
    # retrieve the same port at once
    cache.ensure() # TODO: find a better place for this (necessary at the moment)
    with cache.lock('unpack port'):
      if os.path.exists(fullpath):
        # Another early out in case another process unpackage the library while we were
        # waiting for the lock
        if up_to_date():
          return True
        # file exists but tag is bad
        logger.warning('local copy of port is not correct, retrieving from remote server')
        utils.delete_dir(fullname)
        utils.delete_file(fullpath)

      retrieve()
      unpack()

      return False

  @staticmethod
  def fetch_project(name, url, sha512hash=None):
    if not Ports.fetch_port_artifact(name, url, sha512hash):
      # we unpacked a new version, clear the build in the cache
      Ports.clear_project_build(name)

  @staticmethod
  def clear_project_build(name):
    port = get_port_by_name(name)
    port.clear(Ports, settings, shared)
    build_dir = os.path.join(Ports.get_build_dir(), name)
    logger.debug(f'clearing port build: {name} {build_dir}')
    utils.delete_dir(build_dir)
    return build_dir

  @staticmethod
  def write_file(filename, contents):
    if os.path.exists(filename) and utils.read_file(filename) == contents:
      return
    utils.write_file(filename, contents)

  @staticmethod
  def make_pkg_config(name, version, flags):
    pkgconfig_dir = cache.get_sysroot_dir('lib/pkgconfig')
    filename = os.path.join(pkgconfig_dir, name + '.pc')
    Ports.write_file(filename, f'''
Name: {name}
Description: {name} port from emscripten
Version: {version}
Libs: {flags}
Cflags: {flags}
''')


class OrderedSet:
  """Partial implementation of OrderedSet.  Just enough for what we need here."""
  def __init__(self, items):
    self.dict = {}
    for i in items:
      self.dict[i] = True

  def __repr__(self):
    return f"OrderedSet({list(self.dict.keys())})"

  def __len__(self):
    return len(self.dict.keys())

  def copy(self):
    return OrderedSet(self.dict.keys())

  def __iter__(self):
    return iter(self.dict.keys())

  def pop(self, index=-1):
    key = list(self.dict.keys())[index]
    self.dict.pop(key)
    return key

  def add(self, item):
    self.dict[item] = True

  def remove(self, item):
    del self.dict[item]


def dependency_order(port_list):
  # Perform topological sort of ports according to the dependency DAG
  port_map = {p.name: p for p in port_list}

  # Perform depth first search of dependency graph adding nodes to
  # the stack only after all children have been explored.
  stack = []
  unsorted = OrderedSet(port_list)

  def dfs(node):
    for dep in node.deps:
      dep, _ = split_port_options(dep)
      child = port_map[dep]
      if child in unsorted:
        unsorted.remove(child)
        dfs(child)
    stack.append(node)

  while unsorted:
    dfs(unsorted.pop())

  return stack


def resolve_dependencies(port_set, settings, cflags_only=False):
  def add_deps(node):
    sig = signature(node.process_dependencies)
    if len(sig.parameters) == 2:
      # The optional second parameter here is useful for ports that want
      # to mutate linker-only settings.  Modifying these settings during the
      # compile phase (or in a compile-only) command generates errors.
      node.process_dependencies(settings, cflags_only)
    else:
      node.process_dependencies(settings)
    for d in node.deps:
      d, _ = split_port_options(d)
      if d not in ports_by_name:
        utils.exit_with_error(f'unknown dependency `{d}` for port `{node.name}`')
      dep = get_port_by_name(d)
      if dep not in port_set:
        port_set.add(dep)
        add_deps(dep)

  for port in port_set.copy():
    add_deps(port)


def handle_use_port_error(arg, message):
  utils.exit_with_error(f'error with `--use-port={arg}` | {message}')


def show_port_help_and_exit(port):
  print(port.show())
  if hasattr(port, 'DESCRIPTION'):
    print(port.DESCRIPTION)
  if hasattr(port, 'OPTIONS'):
    print("Options:")
    for option, desc in port.OPTIONS.items():
      print(f'* {option}: {desc}')
  else:
    print("No options.")
  if hasattr(port, 'URL'):
    print(f'More info: {port.URL}')
  sys.exit(0)


# extract dict and delegate to port.handle_options for handling (format is 'option1=value1:option2=value2')
def handle_port_options(name, options, error_handler):
  port = get_port_by_name(name)
  if options == 'help':
    show_port_help_and_exit(port)
  if not hasattr(port, 'handle_options'):
    error_handler(f'no options available for port `{name}`')
  else:
    options_dict = {}
    for name_value in options.replace('::', '\0').split(':'):
      name_value = name_value.replace('\0', ':')
      nv = name_value.split('=', 1)
      if len(nv) != 2:
        error_handler(f'`{name_value}` is missing a value')
      if nv[0] not in port.OPTIONS:
        error_handler(f'`{nv[0]}` is not supported; available options are {port.OPTIONS}')
      if nv[0] in options_dict:
        error_handler(f'duplicate option `{nv[0]}`')
      options_dict[nv[0]] = nv[1]
    port.handle_options(options_dict, error_handler)


# handle port dependencies (ex: deps=['sdl2_image:formats=jpg'])
def handle_port_deps(name, error_handler):
  port = get_port_by_name(name)
  for dep in port.deps:
    dep_name, dep_options = split_port_options(dep)
    if dep_name not in ports_by_name:
      error_handler(f'unknown dependency `{dep_name}`')
    if dep_options:
      handle_port_options(dep_name, dep_options, error_handler)
    handle_port_deps(dep_name, error_handler)


def split_port_options(arg):
  # Ignore ':' in first or second char of string since we could be dealing with a windows drive separator
  pos = arg.find(':', 2)
  if pos != -1:
    return arg[:pos], arg[pos + 1:]
  else:
    return arg, None


def handle_use_port_arg(settings, arg, error_handler=None):
  if not error_handler:
    def error_handler(message):
      handle_use_port_error(arg, message)
  name, options = split_port_options(arg)
  if name.endswith('.py'):
    port_file_path = name
    if not os.path.isfile(port_file_path):
      error_handler(f'not a valid port path: {port_file_path}')
    name = load_port(port_file_path)
  elif name not in ports_by_name:
    error_handler(f'invalid port name: `{name}`')
  ports_needed.add(name)
  if options:
    handle_port_options(name, options, error_handler)
  handle_port_deps(name, error_handler)
  return name


def get_needed_ports(settings, cflags_only=False):
  # Start with directly needed ports, and transitively add dependencies
  needed = OrderedSet(get_port_by_name(p.name) for p in ports if p.needed(settings))
  resolve_dependencies(needed, settings, cflags_only)
  return needed


def build_port(port_name, settings):
  port = get_port_by_name(port_name)
  port_set = OrderedSet([port])
  resolve_dependencies(port_set, settings)
  for port in dependency_order(port_set):
    port.get(Ports, settings, shared)


def clear_port(port_name, settings):
  port = get_port_by_name(port_name)
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
    port.linker_setup(Ports, settings)
    # port.get returns a list of libraries to link
    ret += port.get(Ports, settings, shared)

  ret.reverse()
  return ret


def add_cflags(args, settings):
  """Called during compile phase add any compiler flags (e.g -Ifoo) needed
  by the selected ports.  Can also add/change settings.

  Can have the side effect of building and installing the needed ports.
  """

  # Legacy SDL1 port is not actually a port at all but builtin
  if settings.USE_SDL == 1:
    args += ['-I' + Ports.get_include_dir('SDL')]

  needed = get_needed_ports(settings, cflags_only=True)

  # Now get (i.e. build) the ports in dependency order.  This is important because the
  # headers from one port might be needed before we can build the next.
  for port in dependency_order(needed):
    # When using embuilder, don't build the dependencies
    if not os.getenv('EMBUILDER_PORT_BUILD_DEFERRED'):
      port.get(Ports, settings, shared)
    args += port.process_args(Ports)


def show_ports():
  sorted_ports = sorted(ports, key=lambda p: p.name)
  print('Available official ports:')
  for port in sorted_ports:
    if not port.is_contrib:
      print('   ', port.show())
  print('Available contrib ports:')
  for port in sorted_ports:
    if port.is_contrib:
      print('   ', port.show())


read_ports()
