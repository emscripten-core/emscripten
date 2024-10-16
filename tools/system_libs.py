# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import re
from time import time
from .toolchain_profiler import ToolchainProfiler

import itertools
import logging
import os
import shutil
import textwrap
import shlex
from enum import IntEnum, auto
from glob import iglob
from typing import List, Optional

from . import shared, building, utils
from . import diagnostics
from . import cache
from .settings import settings
from .utils import read_file

logger = logging.getLogger('system_libs')

# Files that are part of libsockets.a and so should be excluded from libc.a
LIBC_SOCKETS = ['socket.c', 'socketpair.c', 'shutdown.c', 'bind.c', 'connect.c',
                'listen.c', 'accept.c', 'getsockname.c', 'getpeername.c', 'send.c',
                'recv.c', 'sendto.c', 'recvfrom.c', 'sendmsg.c', 'recvmsg.c',
                'getsockopt.c', 'setsockopt.c', 'freeaddrinfo.c',
                'gethostbyaddr.c', 'gethostbyaddr_r.c', 'gethostbyname.c',
                'gethostbyname_r.c', 'gethostbyname2.c', 'gethostbyname2_r.c',
                'in6addr_any.c', 'in6addr_loopback.c', 'accept4.c']

# Experimental: Setting EMCC_USE_NINJA will cause system libraries to get built with ninja rather
# than simple subprocesses.  The primary benefit here is that we get accurate dependency tracking.
# This means we can avoid completely rebuilding a library and just rebuild based on what changed.
#
# Setting EMCC_USE_NINJA=2 means that ninja will automatically be run for each library needed at
# link time.
USE_NINJA = int(os.environ.get('EMCC_USE_NINJA', '0'))


def files_in_path(path, filenames):
  srcdir = utils.path_from_root(path)
  return [os.path.join(srcdir, f) for f in filenames]


def glob_in_path(path, glob_pattern, excludes=()):
  srcdir = utils.path_from_root(path)
  files = iglob(os.path.join(srcdir, glob_pattern), recursive=True)
  return sorted(f for f in files if os.path.basename(f) not in excludes)


def get_base_cflags(force_object_files=False, preprocess=True):
  # Always build system libraries with debug information.  Non-debug builds
  # will ignore this at link time because we link with `-strip-debug`.
  flags = ['-g', '-sSTRICT', '-Werror']
  if settings.LTO and not force_object_files:
    flags += ['-flto=' + settings.LTO]
  if settings.RELOCATABLE:
    flags += ['-sRELOCATABLE']
    if preprocess:
      flags += ['-DEMSCRIPTEN_DYNAMIC_LINKING']
  if settings.MEMORY64:
    flags += ['-Wno-experimental', '-sMEMORY64=' + str(settings.MEMORY64)]
  return flags


def clean_env():
  # building system libraries and ports should be hermetic in that it is not
  # affected by things like EMCC_CFLAGS which the user may have set.
  # At least one port also uses autoconf (harfbuzz) so we also need to clear
  # CFLAGS/LDFLAGS which we don't want to effect the inner call to configure.
  safe_env = os.environ.copy()
  for opt in ['CFLAGS', 'CXXFLAGS', 'LDFLAGS',
              'EMCC_CFLAGS',
              'EMCC_AUTODEBUG',
              'EMCC_FORCE_STDLIBS',
              'EMCC_ONLY_FORCED_STDLIBS',
              'EMMAKEN_JUST_CONFIGURE']:
    if opt in safe_env:
      del safe_env[opt]
  return safe_env


def run_build_commands(commands, num_inputs, build_dir=None):
  # Before running a set of build commands make sure the common sysroot
  # headers are installed.  This prevents each sub-process from attempting
  # to setup the sysroot itself.
  ensure_sysroot()
  start_time = time()
  shared.run_multiple_processes(commands, env=clean_env(), cwd=build_dir)
  logger.info(f'compiled {num_inputs} inputs in {time() - start_time:.2f}s')


def objectfile_sort_key(filename):
  """Sort object files that we pass to llvm-ar."""
  # In general, we simply use alphabetical order, but we special case certain
  # object files such they come first.  For example, `vmlock.o` contains the
  # definition of `__vm_wait`, but `mmap.o` also contains a dummy/weak definition
  # for use by `mmap.o` when `vmlock.o` is not otherwise included.
  #
  # When another object looks for `__vm_wait` we prefer that it always find the
  # real definition first and not refer to the dummy one (which is really
  # intended to be local to `mmap.o` but due to the fact the weak aliases can't
  # have internal linkage).
  #
  # The reason we care is that once an object file is pulled into certain aspects
  # of it cannot be undone/removed by the linker.  For example, static
  # constructors or stub library dependencies.
  #
  # In the case of `mmap.o`, once it get included by the linker, it pulls in the
  # the reverse dependencies of the mmap syscall (memalign).  If we don't do this
  # sorting we see a slight regression in test_metadce_minimal_pthreads due to
  # memalign being included.
  basename = os.path.basename(filename)
  if basename in {'vmlock.o'}:
    return 'AAA_' + basename
  else:
    return basename


def create_lib(libname, inputs):
  """Create a library from a set of input objects."""
  suffix = shared.suffix(libname)

  inputs = sorted(inputs, key=objectfile_sort_key)
  if suffix in ('.bc', '.o'):
    if len(inputs) == 1:
      if inputs[0] != libname:
        shutil.copyfile(inputs[0], libname)
    else:
      building.link_to_object(inputs, libname)
  else:
    assert suffix == '.a'
    building.emar('cr', libname, inputs)


def get_top_level_ninja_file():
  return os.path.join(cache.get_path('build'), 'build.ninja')


def run_ninja(build_dir):
  cmd = ['ninja', '-C', build_dir, f'-j{shared.get_num_cores()}']
  if shared.PRINT_SUBPROCS:
    cmd.append('-v')
  shared.check_call(cmd, env=clean_env())


def ensure_target_in_ninja_file(ninja_file, target):
  if os.path.isfile(ninja_file) and target in read_file(ninja_file):
    return
  with open(ninja_file, 'a') as f:
    f.write(target + '\n')


def escape_ninja_path(path):
  """Escape a path to be used in a ninja file."""
  # Replace Windows backslashes with forward slashes.
  path = path.replace('\\', '/')
  # Escape special Ninja chars.
  return re.sub(r'([ :$])', r'$\1', path)


def create_ninja_file(input_files, filename, libname, cflags, asflags=None, customize_build_flags=None):
  if asflags is None:
    asflags = []

  out = f'''\
# Automatically generated by tools/system_libs.py.  DO NOT EDIT

ninja_required_version = 1.5

ASFLAGS = {shlex.join(asflags)}
CFLAGS = {shlex.join(cflags)}
EMCC = {shared.EMCC}
EMXX = {shared.EMXX}
EMAR = {shared.EMAR}

rule cc
  depfile = $out.d
  command = $EMCC -MD -MF $out.d $CFLAGS -c $in -o $out
  description = CC $out

rule cxx
  depfile = $out.d
  command = $EMXX -MD -MF $out.d $CFLAGS -c $in -o $out
  description = CXX $out

rule asm
  command = $EMCC $ASFLAGS -c $in -o $out
  description = ASM $out

rule asm_cpp
  depfile = $out.d
  command = $EMCC -MD -MF $out.d $CFLAGS -c $in -o $out
  description = ASM $out

rule direct_cc
  depfile = $with_depfile
  command = $EMCC -MD -MF $with_depfile $CFLAGS -c $in -o $out
  description = CC $out

rule archive
  # Workaround command line too long issue (https://github.com/ninja-build/ninja/pull/217) by using a response file.
  rspfile = $out.rsp
  rspfile_content = $in
  command = $EMAR cr $out @$rspfile
  description = AR $out

'''
  suffix = shared.suffix(libname)
  build_dir = os.path.dirname(filename)

  case_insensitive = is_case_insensitive(os.path.dirname(filename))
  if suffix == '.o':
    assert len(input_files) == 1
    input_file = escape_ninja_path(input_files[0])
    depfile = shared.unsuffixed_basename(input_file) + '.d'
    out += f'build {escape_ninja_path(libname)}: direct_cc {input_file}\n'
    out += f'  with_depfile = {depfile}\n'
  else:
    objects = []
    for src in input_files:
      # Resolve duplicates by appending unique.
      # This is needed on case insensitive filesystem to handle,
      # for example, _exit.o and _Exit.o.
      object_basename = shared.unsuffixed_basename(src)
      if case_insensitive:
        object_basename = object_basename.lower()
      o = os.path.join(build_dir, object_basename + '.o')
      object_uuid = 0
      # Find a unique basename
      while o in objects:
        object_uuid += 1
        o = os.path.join(build_dir, f'{object_basename}__{object_uuid}.o')
      objects.append(o)
      ext = shared.suffix(src)
      if ext == '.s':
        cmd = 'asm'
        flags = asflags
      elif ext == '.S':
        cmd = 'asm_cpp'
        flags = cflags
      elif ext == '.c':
        cmd = 'cc'
        flags = cflags
      else:
        cmd = 'cxx'
        flags = cflags
      out += f'build {escape_ninja_path(o)}: {cmd} {escape_ninja_path(src)}\n'
      if customize_build_flags:
        custom_flags = customize_build_flags(flags, src)
        if custom_flags != flags:
          out += f'  CFLAGS = {shlex.join(custom_flags)}'
      out += '\n'

    objects = sorted(objects, key=objectfile_sort_key)
    objects = ' '.join(escape_ninja_path(o) for o in objects)
    out += f'build {escape_ninja_path(libname)}: archive {objects}\n'

  utils.write_file(filename, out)
  ensure_target_in_ninja_file(get_top_level_ninja_file(), f'subninja {escape_ninja_path(filename)}')


def is_case_insensitive(path):
  """Returns True if the filesystem at `path` is case insensitive."""
  utils.write_file(os.path.join(path, 'test_file'), '')
  case_insensitive = os.path.exists(os.path.join(path, 'TEST_FILE'))
  os.remove(os.path.join(path, 'test_file'))
  return case_insensitive


class Library:
  """
  `Library` is the base class of all system libraries.

  There are two types of libraries: abstract and concrete.
    * An abstract library, e.g. MTLibrary, is a subclass of `Library` that
      implements certain behaviour common to multiple libraries. The features
      of multiple abstract libraries can be used through multiple inheritance.
    * A concrete library, e.g. libc, is a subclass of `Library` that describes
      how to build a particular library, and its properties, such as name and
      dependencies.

  This library system is meant to handle having many versions of the same library,
  which we call *variations*. For example, some libraries (those that inherit
  from MTLibrary), have both single-threaded and multi-threaded versions.

  An instance of a `Library` subclass represents a specific variation of the
  library. Instance methods perform operations relating to this variation.
  For example, `get_cflags()` would return the emcc flags needed to build this
  variation, and `build()` would generate the library file for this variation.
  The constructor takes keyword arguments that defines the variation.

  Class methods perform tasks relating to all variations. For example,
  `variations()` returns a list of all variations that exists for this library,
  and `get_default_variation()` returns the variation suitable for the current
  environment.

  Other class methods act upon a group of libraries. For example,
  `Library.get_all_variations()` returns a mapping of all variations of
  existing libraries.

  To add a new type of variation, you must add an parameter to `__init__` that
  selects the variant. Then, override one of `vary_on` or `variations`, as well
  as `get_default_variation`.

  If the parameter is boolean, overriding `vary_on` to add the parameter name
  to the returned list is sufficient:

    @classmethod
    def vary_on(cls):
      return super().vary_on() + ['my_parameter']

  Otherwise, you must override `variations`:

    @classmethod
    def variations(cls):
      return [{'my_parameter': value, **other} for value, other in
              itertools.product([1, 2, 3], super().variations())]

  Overriding either `vary_on` or `variations` allows `embuilder.py` to know all
  possible variations so it can build all of them.

  You then need to modify `get_default_variation` to detect the correct value
  for your new parameter based on the settings:

    @classmethod
    def get_default_variation(cls, **kwargs):
      return super().get_default_variation(my_parameter=settings.MY_PARAMETER, **kwargs)

  This allows the correct variation of the library to be selected when building
  code with Emscripten.
  """

  # The simple name of the library. When linking, this is the name to use to
  # automatically get the correct version of the library.
  # This should only be overridden in a concrete library class, e.g. libc,
  # and left as None in an abstract library class, e.g. MTLibrary.
  name: Optional[str] = None

  # Set to true to prevent EMCC_FORCE_STDLIBS from linking this library.
  never_force = False

  # A list of flags to pass to emcc.
  # The flags for the parent class is automatically inherited.
  # TODO: Investigate whether perf gains from loop unrolling would be worth the
  # extra code size. The -fno-unroll-loops flags was added here when loop
  # unrolling landed upstream in LLVM to avoid changing behavior but was not
  # specifically evaluated.
  cflags = ['-O2', '-Wall', '-fno-unroll-loops']

  # A list of directories to put in the include path when building.
  # This is a list of tuples of path components.
  # For example, to put system/lib/a and system/lib/b under the emscripten
  # directory into the include path, you would write:
  #    includes = [('system', 'lib', 'a'), ('system', 'lib', 'b')]
  # The include path of the parent class is automatically inherited.
  includes: List[str] = []

  # By default, `get_files` look for source files for this library under `src_dir`.
  # It will either use the files listed in `src_files`, or use the glob pattern in
  # `src_glob`. You may not specify both `src_files` and `src_glob`.
  # When using `src_glob`, you can specify a list of files in `src_glob_exclude`
  # to be excluded from the library.
  # Alternatively, you can override `get_files` to use your own logic.
  src_dir: Optional[str] = None
  src_files: Optional[List[str]] = []
  src_glob: Optional[str] = None
  src_glob_exclude: Optional[List[str]] = None

  # Whether to always generate WASM object files, even when LTO is set
  force_object_files = False

  def __init__(self):
    """
    Creates a variation of this library.

    A variation is a specific combination of settings a library can have.
    For example, libc++-mt-noexcept is a variation of libc++.
    There might be only one variation of a library.

    The constructor keyword arguments will define what variation to use.

    Use the `variations` classmethod to get the list of all possible constructor
    arguments for this library.

    Use the `get_default_variation` classmethod to construct the variation
    suitable for the current invocation of emscripten.
    """
    if not self.name:
      raise NotImplementedError('Cannot instantiate an abstract library')

  def can_use(self):
    """
    Whether this library can be used in the current environment.

    For example, libmalloc would override this and return False
    if the user requested no malloc.
    """
    return True

  def can_build(self):
    """
    Whether this library can be built in the current environment.

    Override this if, for example, the library can only be built on WASM backend.
    """
    return True

  def erase(self):
    cache.erase_file(self.get_path())

  def get_path(self, absolute=False):
    return cache.get_lib_name(self.get_filename(), absolute=absolute)

  def build(self, deterministic_paths=False):
    """
    Gets the cached path of this library.

    This will trigger a build if this library is not in the cache.
    """
    self.deterministic_paths = deterministic_paths
    return cache.get(self.get_path(), self.do_build, force=USE_NINJA == 2, quiet=USE_NINJA)

  def generate(self):
    self.deterministic_paths = False
    return cache.get(self.get_path(), self.do_generate, force=USE_NINJA == 2, quiet=USE_NINJA,
                     deferred=True)

  def get_link_flag(self):
    """
    Gets the link flags needed to use the library.

    This will trigger a build if this library is not in the cache.
    """
    fullpath = self.build()
    # For non-libraries (e.g. crt1.o) we pass the entire path to the linker
    if self.get_ext() != '.a':
      return fullpath
    # For libraries (.a) files, we pass the abbreviated `-l` form.
    base = shared.unsuffixed_basename(fullpath)
    return '-l' + utils.removeprefix(base, 'lib')

  def get_files(self):
    """
    Gets a list of source files for this library.

    Typically, you will use `src_dir`, `src_files`, `src_glob` and `src_glob_exclude`.
    If those are insufficient to describe the files needed, you can override this method.
    """
    if self.src_dir:
      if self.src_files and self.src_glob:
        raise Exception('Cannot use src_files and src_glob together')

      if self.src_files:
        return files_in_path(self.src_dir, self.src_files)
      elif self.src_glob:
        return glob_in_path(self.src_dir, self.src_glob, self.src_glob_exclude or ())

    raise NotImplementedError()

  def generate_ninja(self, build_dir, libname):
    ensure_sysroot()
    utils.safe_ensure_dirs(build_dir)

    cflags = self.get_cflags()
    if self.deterministic_paths:
      source_dir = utils.path_from_root()
      relative_source_dir = os.path.relpath(source_dir, build_dir)
      cflags += [f'-ffile-prefix-map={source_dir}=/emsdk/emscripten',
                 f'-ffile-prefix-map={relative_source_dir}/=',
                 '-fdebug-compilation-dir=/emsdk/emscripten']
    asflags = get_base_cflags(preprocess=False)
    input_files = self.get_files()
    ninja_file = os.path.join(build_dir, 'build.ninja')
    create_ninja_file(input_files, ninja_file, libname, cflags, asflags=asflags, customize_build_flags=self.customize_build_cmd)

  def build_objects(self, build_dir):
    """
    Returns a list of compiled object files for this library.

    By default, this builds all the source files returned by `self.get_files()`,
    with the `cflags` returned by `self.get_cflags()`.
    """
    batch_inputs = int(os.environ.get('EMCC_BATCH_BUILD', '1'))
    batches = {}
    commands = []
    objects = set()
    cflags = self.get_cflags()
    if self.deterministic_paths:
      source_dir = utils.path_from_root()
      if batch_inputs:
        relative_source_dir = os.path.relpath(source_dir, build_dir)
        cflags += [f'-ffile-prefix-map={relative_source_dir}/=']
      cflags += [f'-ffile-prefix-map={source_dir}=/emsdk/emscripten',
                 '-fdebug-compilation-dir=/emsdk/emscripten']
    case_insensitive = is_case_insensitive(build_dir)
    for src in self.get_files():
      ext = shared.suffix(src)
      if ext in ('.s', '.S', '.c'):
        cmd = shared.EMCC
      else:
        cmd = shared.EMXX
      cmd = [cmd, '-c']
      if ext == '.s':
        # .s files are processed directly by the assembler.  In this case we can't pass
        # pre-processor flags such as `-I` and `-D` but we still want core flags such as
        # `-sMEMORY64`.
        cmd += get_base_cflags(preprocess=False)
      else:
        cmd += cflags
      cmd = self.customize_build_cmd(cmd, src)

      object_basename = shared.unsuffixed_basename(src)
      if case_insensitive:
        object_basename = object_basename.lower()
      o = os.path.join(build_dir, object_basename + '.o')
      if o in objects:
        # If we have seen a file with the same name before, we are on a case-insensitive
        # filesystem and need a separate command to compile this file with a
        # custom unique output object filename, as batch compile doesn't allow
        # such customization.
        #
        # This is needed to handle, for example, _exit.o and _Exit.o.
        object_uuid = 0
        # Find a unique basename
        while o in objects:
          object_uuid += 1
          o = os.path.join(build_dir, f'{object_basename}__{object_uuid}.o')
        commands.append(cmd + [src, '-o', o])
      elif batch_inputs:
        # Use relative paths to reduce the length of the command line.
        # This allows to avoid switching to a response file as often.
        src = os.path.relpath(src, build_dir)
        src = utils.normalize_path(src)
        batches.setdefault(tuple(cmd), []).append(src)
      else:
        commands.append(cmd + [src, '-o', o])
      objects.add(o)

    if batch_inputs:
      # Choose a chunk size that is large enough to avoid too many subprocesses
      # but not too large to avoid task starvation.
      # For now the heuristic is to split inputs by 2x number of cores.
      chunk_size = max(1, len(objects) // (2 * shared.get_num_cores()))
      # Convert batches to commands.
      for cmd, srcs in batches.items():
        cmd = list(cmd)
        for i in range(0, len(srcs), chunk_size):
          chunk_srcs = srcs[i:i + chunk_size]
          commands.append(building.get_command_with_possible_response_file(cmd + chunk_srcs))

    run_build_commands(commands, num_inputs=len(objects), build_dir=build_dir)
    return objects

  def customize_build_cmd(self, cmd, _filename):
    """Allows libraries to customize the build command used on per-file basis.

    For example, libc uses this to replace -Oz with -O2 for some subset of files."""
    return cmd

  def do_build(self, out_filename, generate_only=False):
    """Builds the library and returns the path to the file."""
    assert out_filename == self.get_path(absolute=True)
    build_dir = os.path.join(cache.get_path('build'), self.get_base_name())
    if USE_NINJA:
      self.generate_ninja(build_dir, out_filename)
      if not generate_only:
        run_ninja(build_dir)
    else:
      # Use a separate build directory to the ninja flavor so that building without
      # EMCC_USE_NINJA doesn't clobber the ninja build tree
      build_dir += '-tmp'
      utils.safe_ensure_dirs(build_dir)
      create_lib(out_filename, self.build_objects(build_dir))
      if not shared.DEBUG:
        utils.delete_dir(build_dir)

  def do_generate(self, out_filename):
    self.do_build(out_filename, generate_only=True)

  @classmethod
  def _inherit_list(cls, attr):
    # Some properties, like cflags and includes, makes more sense to inherit
    # via concatenation than replacement.
    result = []
    for item in cls.__mro__[::-1]:
      # Using  __dict__ to avoid inheritance
      result += item.__dict__.get(attr, [])
    return result

  def get_cflags(self):
    """
    Returns the list of flags to pass to emcc when building this variation
    of the library.

    Override and add any flags as needed to handle new variations.
    """
    cflags = self._inherit_list('cflags')
    cflags += get_base_cflags(force_object_files=self.force_object_files)

    if self.includes:
      cflags += ['-I' + utils.path_from_root(i) for i in self._inherit_list('includes')]

    return cflags

  def get_base_name_prefix(self):
    """
    Returns the base name of the library without any suffixes.
    """
    return self.name

  def get_base_name(self):
    """
    Returns the base name of the library file.

    This will include suffixes such as -mt, but will not include a file extension.
    """
    return self.get_base_name_prefix()

  def get_ext(self):
    """
    Return the appropriate file extension for this library.
    """
    return '.a'

  def get_filename(self):
    """
    Return the full name of the library file, including the file extension.
    """
    return self.get_base_name() + self.get_ext()

  @classmethod
  def vary_on(cls):
    """
    Returns a list of strings that are the names of boolean constructor
    arguments that defines the variations of this library.

    This is used by the default implementation of `cls.variations()` to generate
    every possible combination of boolean values to pass to these arguments.
    """
    return []

  @classmethod
  def variations(cls):
    """
    Returns a list of keyword arguments to pass to the constructor to create
    every possible variation of this library.

    By default, this is every possible combination of boolean values to pass
    to the list of arguments returned by `vary_on`, but you can override
    the behaviour.
    """
    vary_on = cls.vary_on()
    return [dict(zip(vary_on, toggles)) for toggles in
            itertools.product([False, True], repeat=len(vary_on))]

  @classmethod
  def get_default_variation(cls, **kwargs):
    """
    Construct the variation suitable for the current invocation of emscripten.

    Subclasses should pass the keyword arguments they introduce to the
    superclass version, and propagate **kwargs. The base class collects
    all the keyword arguments and creates the instance.
    """
    return cls(**kwargs)

  @classmethod
  def get_inheritance_tree(cls):
    """Returns all the classes in the inheritance tree of the current class."""
    yield cls
    for subclass in cls.__subclasses__():
      for cls in subclass.get_inheritance_tree():
        yield cls

  @classmethod
  def get_all_variations(cls):
    """
    Gets all the variations of libraries in the inheritance tree of the current
    library.

    Calling Library.get_all_variations() returns the variations of ALL libraries
    that can be built as a dictionary of variation names to Library objects.
    """
    result = {}
    for library in cls.get_inheritance_tree():
      if library.name:
        for flags in library.variations():
          variation = library(**flags)
          if variation.can_build():
            result[variation.get_base_name()] = variation
    return result

  @classmethod
  def get_usable_variations(cls):
    """
    Gets all libraries suitable for the current invocation of emscripten.

    This returns a dictionary of simple names to Library objects.
    """
    if not hasattr(cls, 'useable_variations'):
      cls.useable_variations = {}
      for subclass in cls.get_inheritance_tree():
        if subclass.name:
          library = subclass.get_default_variation()
          if library.can_build() and library.can_use():
            cls.useable_variations[subclass.name] = library
    return cls.useable_variations


class MTLibrary(Library):
  def __init__(self, **kwargs):
    self.is_mt = kwargs.pop('is_mt')
    self.is_ww = kwargs.pop('is_ww') and not self.is_mt
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_mt:
      cflags += ['-pthread', '-sWASM_WORKERS']
    if self.is_ww:
      cflags += ['-sWASM_WORKERS']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_mt:
      name += '-mt'
    if self.is_ww:
      name += '-ww'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_mt', 'is_ww']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      is_mt=settings.PTHREADS,
      is_ww=settings.SHARED_MEMORY and not settings.PTHREADS,
      **kwargs
    )

  @classmethod
  def variations(cls):
    combos = super(MTLibrary, cls).variations()

    # These are mutually exclusive, only one flag will be set at any give time.
    return [combo for combo in combos if not combo['is_mt'] or not combo['is_ww']]


class DebugLibrary(Library):
  def __init__(self, **kwargs):
    self.is_debug = kwargs.pop('is_debug')
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if not self.is_debug:
      cflags += ['-DNDEBUG']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_debug:
      name += '-debug'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_debug']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(is_debug=settings.ASSERTIONS, **kwargs)


class Exceptions(IntEnum):
  """
  This represents exception handling mode of Emscripten. Currently there are
  three modes of exception handling:
  - None: Does not handle exceptions. This includes -fno-exceptions, which
    prevents both throwing and catching, and -fignore-exceptions, which only
    allows throwing, but library-wise they use the same version.
  - Emscripten: Emscripten provides exception handling capability using JS
    emulation. This causes code size increase and performance degradation.
  - Wasm: Wasm native exception handling support uses Wasm EH instructions and
    is meant to be fast. You need to use a VM that has the EH support to use
    this. This is not fully working yet and still experimental.
  """
  NONE = auto()
  EMSCRIPTEN = auto()
  WASM = auto()


class NoExceptLibrary(Library):
  def __init__(self, **kwargs):
    self.eh_mode = kwargs.pop('eh_mode')
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.eh_mode == Exceptions.NONE:
      cflags += ['-fno-exceptions']
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      cflags += ['-sDISABLE_EXCEPTION_CATCHING=0']
    elif self.eh_mode == Exceptions.WASM:
      cflags += ['-fwasm-exceptions']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    # TODO Currently emscripten-based exception is the default mode, thus no
    # suffixes. Change the default to wasm exception later.
    if self.eh_mode == Exceptions.NONE:
      name += '-noexcept'
    elif self.eh_mode == Exceptions.WASM:
      name += '-except'
    return name

  @classmethod
  def variations(cls, **kwargs):  # noqa
    combos = super().variations()
    return ([dict(eh_mode=Exceptions.NONE, **combo) for combo in combos] +
            [dict(eh_mode=Exceptions.EMSCRIPTEN, **combo) for combo in combos] +
            [dict(eh_mode=Exceptions.WASM, **combo) for combo in combos])

  @classmethod
  def get_default_variation(cls, **kwargs):
    if settings.WASM_EXCEPTIONS:
      eh_mode = Exceptions.WASM
    elif settings.DISABLE_EXCEPTION_CATCHING == 1:
      eh_mode = Exceptions.NONE
    else:
      eh_mode = Exceptions.EMSCRIPTEN
    return super().get_default_variation(eh_mode=eh_mode, **kwargs)


class SjLjLibrary(Library):
  def __init__(self, **kwargs):
    # Whether we use Wasm EH instructions for SjLj support
    self.is_wasm = kwargs.pop('is_wasm')
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_wasm:
      # DISABLE_EXCEPTION_THROWING=0 is the default, which is for Emscripten
      # EH/SjLj, so we should reverse it.
      cflags += ['-sSUPPORT_LONGJMP=wasm',
                 '-sDISABLE_EXCEPTION_THROWING',
                 '-D__USING_WASM_SJLJ__']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    # TODO Currently emscripten-based SjLj is the default mode, thus no
    # suffixes. Change the default to wasm exception later.
    if self.is_wasm:
      name += '-wasm-sjlj'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_wasm']

  @classmethod
  def get_default_variation(cls, **kwargs):
    is_wasm = settings.SUPPORT_LONGJMP == 'wasm'
    return super().get_default_variation(is_wasm=is_wasm, **kwargs)


class MuslInternalLibrary(Library):
  includes = [
    'system/lib/libc/musl/src/internal',
    'system/lib/libc/musl/src/include',
    'system/lib/libc/musl/include',
    'system/lib/libc',
    'system/lib/pthread',
  ]

  cflags = [
    '-std=c99',
    '-D_XOPEN_SOURCE=700',
    '-Wno-unused-result',  # system call results are often ignored in musl, and in wasi that warns
  ]


class AsanInstrumentedLibrary(Library):
  def __init__(self, **kwargs):
    self.is_asan = kwargs.pop('is_asan', False)
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_asan:
      cflags += ['-fsanitize=address']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_asan:
      name += '-asan'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_asan']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(is_asan=settings.USE_ASAN, **kwargs)


# Subclass of SjLjLibrary because emscripten_setjmp.c uses SjLj support
class libcompiler_rt(MTLibrary, SjLjLibrary):
  name = 'libcompiler_rt'
  # compiler_rt files can't currently be part of LTO although we are hoping to remove this
  # restriction soon: https://reviews.llvm.org/D71738
  force_object_files = True

  cflags = ['-fno-builtin', '-DNDEBUG']
  src_dir = 'system/lib/compiler-rt/lib/builtins'
  includes = ['system/lib/libc']
  excludes = [
    # gcc_personality_v0.c depends on libunwind, which don't include by default.
    'gcc_personality_v0.c',
    # bfloat16
    'extendbfsf2.c',
    'truncdfbf2.c',
    'truncsfbf2.c',
    # We provide our own crt
    'crtbegin.c',
    'crtend.c',
    # 80-bit long double (xf_float)
    'divxc3.c',
    'extendxftf2.c',
    'fixxfdi.c',
    'fixxfti.c',
    'fixunsxfdi.c',
    'fixunsxfsi.c',
    'fixunsxfti.c',
    'floatdixf.c',
    'floattixf.c',
    'floatundixf.c',
    'floatuntixf.c',
    'mulxc3.c',
    'powixf2.c',
    'trunctfxf2.c',
  ]
  src_files = glob_in_path(src_dir, '*.c', excludes=excludes)
  src_files += files_in_path(
      path='system/lib/compiler-rt',
      filenames=[
        'stack_ops.S',
        'stack_limits.S',
        'emscripten_setjmp.c',
        'emscripten_exception_builtins.c',
        'emscripten_tempret.s',
        '__trap.c',
      ])


class libnoexit(Library):
  name = 'libnoexit'
  src_dir = 'system/lib/libc'
  src_files = ['atexit_dummy.c']


class libc(MuslInternalLibrary,
           DebugLibrary,
           AsanInstrumentedLibrary,
           MTLibrary):
  name = 'libc'

  # Without -fno-builtin, LLVM can optimize away or convert calls to library
  # functions to something else based on assumptions that they behave exactly
  # like the standard library. This can cause unexpected bugs when we use our
  # custom standard library. The same for other libc/libm builds.
  # We use -fno-inline-functions because it can produce slightly smaller
  # (and slower) code in some cases.  TODO(sbc): remove this and let llvm weight
  # the cost/benefit of inlining.
  cflags = ['-Os', '-fno-inline-functions', '-fno-builtin']

  # Disable certain warnings for code patterns that are contained in upstream musl
  cflags += ['-Wno-ignored-attributes',
             # tre.h defines NDEBUG internally itself
             '-Wno-macro-redefined',
             '-Wno-shift-op-parentheses',
             '-Wno-string-plus-int',
             '-Wno-missing-braces',
             '-Wno-logical-op-parentheses',
             '-Wno-bitwise-op-parentheses',
             '-Wno-unused-but-set-variable',
             '-Wno-unused-variable',
             '-Wno-unused-label',
             '-Wno-pointer-sign']

  def __init__(self, **kwargs):
    self.non_lto_files = self.get_libcall_files()
    super().__init__(**kwargs)

  def get_libcall_files(self):
    # Combining static linking with LTO is tricky under LLVM.  The codegen that
    # happens during LTO can generate references to new symbols that didn't exist
    # in the linker inputs themselves.
    # These symbols are called libcalls in LLVM and are the result of intrinsics
    # and builtins at the LLVM level.  These libcalls cannot themselves be part
    # of LTO because once the linker is running the LTO phase new bitcode objects
    # cannot be added to link.  Another way of putting it: by the time LTO happens
    # the decision about which bitcode symbols to compile has already been made.
    # See: https://bugs.llvm.org/show_bug.cgi?id=44353.
    # To solve this we force certain parts of libc to never be compiled as LTO/bitcode.
    # Note that this also includes things that may be depended on by those
    # functions - fmin uses signbit, for example, so signbit must be here (so if
    # fmin is added by codegen, it will have all it needs).
    math_files = [
      'fmin.c', 'fminf.c', 'fminl.c',
      'fmax.c', 'fmaxf.c', 'fmaxl.c',
      'fmod.c', 'fmodf.c', 'fmodl.c',
      'logf.c', 'logf_data.c',
      'log2f.c', 'log2f_data.c',
      'log10.c', 'log10f.c',
      'exp.c', 'exp_data.c',
      'exp2.c',
      'exp2f.c', 'exp2f_data.c',
      'exp10.c', 'exp10f.c',
      'ldexp.c', 'ldexpf.c', 'ldexpl.c',
      'scalbn.c', '__fpclassifyl.c',
      '__signbitl.c', '__signbitf.c', '__signbit.c',
      '__math_divzero.c', '__math_divzerof.c',
      '__math_oflow.c', '__math_oflowf.c',
      '__math_uflow.c', '__math_uflowf.c',
      '__math_invalid.c', '__math_invalidf.c', '__math_invalidl.c',
      'pow.c', 'pow_data.c', 'log.c', 'log_data.c', 'log2.c', 'log2_data.c',
      'scalbnf.c',
    ]
    math_files = files_in_path(path='system/lib/libc/musl/src/math', filenames=math_files)

    exit_files = files_in_path(
        path='system/lib/libc/musl/src/exit',
        filenames=['atexit.c'])

    other_files = files_in_path(
      path='system/lib/libc',
      filenames=['emscripten_memcpy.c', 'emscripten_memset.c',
                 'emscripten_scan_stack.c',
                 'emscripten_get_heap_size.c',  # needed by malloc
                 'sbrk.c',  # needed by malloc
                 'emscripten_memmove.c'])
    # Calls to iprintf can be generated during codegen. Ideally we wouldn't
    # compile these with -O2 like we do the rest of compiler-rt since its
    # probably not performance sensitive.  However we don't currently have
    # a way to set per-file compiler flags.  And hopefully we should be able
    # move all this stuff back into libc once we it LTO compatible.
    iprintf_files = files_in_path(
      path='system/lib/libc/musl/src/stdio',
      filenames=['__towrite.c', '__overflow.c', 'fwrite.c', 'fputs.c',
                 'getc.c',
                 'fputc.c',
                 'fgets.c',
                 'putc.c', 'putc_unlocked.c',
                 'putchar.c', 'putchar_unlocked.c',
                 'printf.c', 'puts.c', '__lockfile.c'])
    iprintf_files += files_in_path(
      path='system/lib/libc/musl/src/string',
      filenames=['strlen.c'])

    # Transitively required by many system call imports
    errno_files = files_in_path(
      path='system/lib/libc/musl/src/errno',
      filenames=['__errno_location.c', 'strerror.c'])

    return math_files + exit_files + other_files + iprintf_files + errno_files

  def get_files(self):
    libc_files = []
    musl_srcdir = utils.path_from_root('system/lib/libc/musl/src')

    # musl modules
    ignore = [
        'ipc', 'passwd', 'signal', 'sched', 'time', 'linux',
        'aio', 'exit', 'legacy', 'mq', 'setjmp',
        'ldso', 'malloc'
    ]

    # individual files
    ignore += [
        'memcpy.c', 'memset.c', 'memmove.c', 'getaddrinfo.c', 'getnameinfo.c',
        'res_query.c', 'res_querydomain.c',
        'proto.c',
        'ppoll.c',
        'syscall.c', 'popen.c', 'pclose.c',
        'getgrouplist.c', 'initgroups.c', 'wordexp.c', 'timer_create.c',
        'getentropy.c',
        'getauxval.c',
        'lookup_name.c',
        # 'process' exclusion
        'fork.c', 'vfork.c', 'posix_spawn.c', 'posix_spawnp.c', 'execve.c', 'waitid.c', 'system.c',
        '_Fork.c',
        # 'env' exclusion
        '__reset_tls.c', '__init_tls.c', '__libc_start_main.c',
    ]

    ignore += LIBC_SOCKETS

    if self.is_mt:
      ignore += [
        'clone.c',
        'pthread_create.c',
        'pthread_kill.c', 'pthread_sigmask.c',
        '__set_thread_area.c', 'synccall.c',
        '__syscall_cp.c', '__tls_get_addr.c',
        '__unmapself.c',
        # Empty files, simply ignore them.
        'syscall_cp.c', 'tls.c',
        # TODO: Support these. See #12216.
        'pthread_setname_np.c',
        'pthread_getname_np.c',
      ]
      libc_files += files_in_path(
        path='system/lib/pthread',
        filenames=[
          'library_pthread.c',
          'em_task_queue.c',
          'proxying.c',
          'proxying_legacy.c',
          'thread_mailbox.c',
          'pthread_create.c',
          'pthread_kill.c',
          'emscripten_thread_init.c',
          'emscripten_thread_state.S',
          'emscripten_futex_wait.c',
          'emscripten_futex_wake.c',
          'emscripten_yield.c',
        ])
    else:
      ignore += ['thread']
      libc_files += files_in_path(
        path='system/lib/libc/musl/src/thread',
        filenames=[
          'pthread_self.c',
          'pthread_cleanup_push.c',
          'pthread_attr_init.c',
          'pthread_attr_destroy.c',
          'pthread_attr_get.c',
          'pthread_attr_setdetachstate.c',
          'pthread_attr_setguardsize.c',
          'pthread_attr_setinheritsched.c',
          'pthread_attr_setschedparam.c',
          'pthread_attr_setschedpolicy.c',
          'pthread_attr_setscope.c',
          'pthread_attr_setstack.c',
          'pthread_attr_setstacksize.c',
          'pthread_getconcurrency.c',
          'pthread_getcpuclockid.c',
          'pthread_getschedparam.c',
          'pthread_setschedprio.c',
          'pthread_setconcurrency.c',
          'default_attr.c',
          # C11 thread library functions
          'call_once.c',
          'tss_create.c',
          'tss_delete.c',
          'tss_set.c',
          'cnd_broadcast.c',
          'cnd_destroy.c',
          'cnd_init.c',
          'cnd_signal.c',
          'cnd_timedwait.c',
          'cnd_wait.c',
          'mtx_destroy.c',
          'mtx_init.c',
          'mtx_lock.c',
          'mtx_timedlock.c',
          'mtx_trylock.c',
          'mtx_unlock.c',
          'thrd_create.c',
          'thrd_exit.c',
          'thrd_join.c',
          'thrd_sleep.c',
          'thrd_yield.c',
        ])
      libc_files += files_in_path(
        path='system/lib/pthread',
        filenames=[
          'library_pthread_stub.c',
          'pthread_self_stub.c',
          'proxying_stub.c',
        ])

    # These files are in libc directories, but only built in libc_optz.
    ignore += [
      'pow_small.c', 'log_small.c', 'log2_small.c'
    ]

    ignore = set(ignore)
    for dirpath, dirnames, filenames in os.walk(musl_srcdir):
      # Don't recurse into ignored directories
      remove = [d for d in dirnames if d in ignore]
      for r in remove:
        dirnames.remove(r)

      for f in filenames:
        if f.endswith('.c') and f not in ignore:
          libc_files.append(os.path.join(musl_srcdir, dirpath, f))

    # Allowed files from ignored modules
    libc_files += files_in_path(
        path='system/lib/libc/musl/src/time',
        filenames=[
          'clock_settime.c',
          'asctime_r.c',
          'asctime.c',
          'ctime.c',
          'difftime.c',
          'ftime.c',
          'gmtime.c',
          'localtime.c',
          'nanosleep.c',
          'clock_nanosleep.c',
          'ctime_r.c',
          'timespec_get.c',
          'utime.c',
          '__map_file.c',
          'strftime.c',
          '__tz.c',
          '__tm_to_secs.c',
          '__year_to_secs.c',
          '__month_to_secs.c',
        ])
    libc_files += files_in_path(
        path='system/lib/libc/musl/src/legacy',
        filenames=['getpagesize.c', 'err.c', 'euidaccess.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/linux',
        filenames=['getdents.c', 'gettid.c', 'utimes.c', 'statx.c', 'wait4.c', 'wait3.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/sched',
        filenames=['sched_yield.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/exit',
        filenames=['abort.c', '_Exit.c', 'atexit.c', 'at_quick_exit.c', 'quick_exit.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/ldso',
        filenames=['dladdr.c', 'dlerror.c', 'dlsym.c', 'dlclose.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/signal',
        filenames=[
          'block.c',
          'getitimer.c',
          'killpg.c',
          'setitimer.c',
          'sigorset.c',
          'sigandset.c',
          'sigaddset.c',
          'sigdelset.c',
          'sigemptyset.c',
          'sigisemptyset.c',
          'sigfillset.c',
          'sigismember.c',
          'siginterrupt.c',
          'signal.c',
          'sigprocmask.c',
          'sigrtmax.c',
          'sigrtmin.c',
          'sigwait.c',
          'sigwaitinfo.c',
        ])

    libc_files += files_in_path(
        path='system/lib/libc',
        filenames=[
          'emscripten_console.c',
          'emscripten_fiber.c',
          'emscripten_get_heap_size.c',
          'emscripten_memcpy.c',
          'emscripten_memmove.c',
          'emscripten_memset.c',
          'emscripten_mmap.c',
          'emscripten_scan_stack.c',
          'emscripten_time.c',
          'mktime.c',
          'kill.c',
          'lookup_name.c',
          'pthread_sigmask.c',
          'raise.c',
          'sigaction.c',
          'sigtimedwait.c',
          'wasi-helpers.c',
          'sbrk.c',
          'system.c',
        ])

    if settings.RELOCATABLE:
      libc_files += files_in_path(path='system/lib/libc', filenames=['dynlink.c'])

    libc_files += files_in_path(
        path='system/lib/pthread',
        filenames=['thread_profiler.c'])

    libc_files += glob_in_path('system/lib/libc/compat', '*.c')

    # Check for missing file in non_lto_files list.  Do this here
    # rather than in the constructor so it only happens when the
    # library is actually built (not when its instantiated).
    for f in self.non_lto_files:
      assert os.path.exists(f), f

    return libc_files

  def customize_build_cmd(self, cmd, filename):
    if filename in self.non_lto_files:
      # These files act more like the part of compiler-rt in that
      # references to them can be generated at compile time.
      # Treat them like compiler-rt in as much as never compile
      # them as LTO and build them with -O2 rather then -Os (which
      # use used for the rest of libc) because this set of files
      # also contains performance sensitive math functions.
      cmd = [a for a in cmd if not a.startswith('-flto')]
      cmd = [a for a in cmd if not a.startswith('-O')]
      cmd += ['-O2']
    return cmd


# Contains the files from libc that are optimized differently in -Oz mode, where
# we want to aggressively optimize them for size. This is linked in before libc
# so we can override those specific files, when in -Oz.
class libc_optz(libc):
  name = 'libc_optz'

  cflags = ['-Os', '-fno-inline-functions', '-fno-builtin', '-DEMSCRIPTEN_OPTIMIZE_FOR_OZ']

  def __init__(self, **kwargs):
    super().__init__(**kwargs)
    self.non_lto_files = self.get_libcall_files()

  def get_libcall_files(self):
    # see comments in libc.customize_build_cmd

    # some files also appear in libc, and a #define affects them
    mem_files = files_in_path(
      path='system/lib/libc',
      filenames=['emscripten_memcpy.c', 'emscripten_memset.c',
                 'emscripten_memmove.c'])

    # some functions have separate files
    math_files = files_in_path(
      path='system/lib/libc/musl/src/math',
      filenames=['pow_small.c', 'log_small.c', 'log2_small.c'])

    return mem_files + math_files

  def get_files(self):
    libcall_files = self.get_libcall_files()

    # some files also appear in libc, and a #define affects them
    mem_files = files_in_path(
      path='system/lib/libc/musl/src/string',
      filenames=['memcmp.c'])

    return libcall_files + mem_files

  def customize_build_cmd(self, cmd, filename):
    if filename in self.non_lto_files:
      # see comments in libc.customize_build_cmd
      cmd = [a for a in cmd if not a.startswith('-flto')]
      cmd = [a for a in cmd if not a.startswith('-O')]
      cmd += ['-O2']
    return cmd

  def can_use(self):
    # Because libc_optz overrides parts of libc, it is not compatible with
    # dynamic linking which uses --whole-archive. In addition,
    # EMCC_FORCE_STDLIBS can have a similar effect of forcing all libraries.
    # In both cases, the build is not one that is hyper-focused on code size,
    # and so optz is not that important.
    return super(libc_optz, self).can_use() and settings.SHRINK_LEVEL >= 2 and \
        not settings.LINKABLE and not os.environ.get('EMCC_FORCE_STDLIBS')


class libbulkmemory(MuslInternalLibrary, AsanInstrumentedLibrary):
  name = 'libbulkmemory'
  src_dir = 'system/lib/libc'
  src_files = ['emscripten_memcpy.c', 'emscripten_memset.c',
               'emscripten_memcpy_bulkmem.S', 'emscripten_memset_bulkmem.S']
  cflags = ['-mbulk-memory']

  def can_use(self):
    return super(libbulkmemory, self).can_use() and settings.BULK_MEMORY


class libprintf_long_double(libc):
  name = 'libprintf_long_double'
  cflags = ['-DEMSCRIPTEN_PRINTF_LONG_DOUBLE']

  def get_files(self):
    return files_in_path(
        path='system/lib/libc/musl/src/stdio',
        filenames=['vfprintf.c'])

  def can_use(self):
    return super(libprintf_long_double, self).can_use() and settings.PRINTF_LONG_DOUBLE


class libwasm_workers(DebugLibrary):
  name = 'libwasm_workers'
  includes = ['system/lib/libc']

  def __init__(self, **kwargs):
    self.is_stub = kwargs.pop('stub')
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_debug:
      cflags += ['-D_DEBUG']
      # library_wasm_worker.c contains an assert that a nonnull parameter
      # is no NULL, which llvm now warns is redundant/tautological.
      cflags += ['-Wno-tautological-pointer-compare']
      # Override the `-O2` default.  Building library_wasm_worker.c with
      # `-O1` or `-O2` currently causes tests to fail.
      # https://github.com/emscripten-core/emscripten/issues/19331
      cflags += ['-O0']
    else:
      cflags += ['-DNDEBUG', '-Oz']
    if settings.MAIN_MODULE:
      cflags += ['-fPIC']
    if not self.is_stub:
      cflags += ['-sWASM_WORKERS']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_stub:
      name += '-stub'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['stub']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(stub=not settings.WASM_WORKERS, **kwargs)

  def get_files(self):
    files = []
    if self.is_stub:
      files = [
        'library_wasm_worker_stub.c'
      ]
    else:
      files = [
        'library_wasm_worker.c',
        'wasm_worker_initialize.S',
      ]
    return files_in_path(
        path='system/lib/wasm_worker',
        filenames=files)

  def can_use(self):
    # see src/library_wasm_worker.js
    return super().can_use() and not settings.SINGLE_FILE \
      and not settings.RELOCATABLE and not settings.PROXY_TO_WORKER


class libsockets(MuslInternalLibrary, MTLibrary):
  name = 'libsockets'

  cflags = ['-Os', '-fno-inline-functions', '-fno-builtin', '-Wno-shift-op-parentheses']

  def get_files(self):
    return files_in_path(
      path='system/lib/libc/musl/src/network',
      filenames=LIBC_SOCKETS)

  def can_use(self):
    return super(libsockets, self).can_use() and not settings.PROXY_POSIX_SOCKETS


class libsockets_proxy(MTLibrary):
  name = 'libsockets_proxy'

  cflags = ['-Os', '-fno-inline-functions']

  def get_files(self):
    return [utils.path_from_root('system/lib/websocket/websocket_to_posix_socket.c')]

  def can_use(self):
    return super(libsockets_proxy, self).can_use() and settings.PROXY_POSIX_SOCKETS


class crt1(MuslInternalLibrary):
  name = 'crt1'
  src_dir = 'system/lib/libc'
  src_files = ['crt1.c']

  force_object_files = True

  def get_ext(self):
    return '.o'

  def can_use(self):
    return super().can_use() and settings.STANDALONE_WASM


class crt1_reactor(MuslInternalLibrary):
  name = 'crt1_reactor'
  src_dir = 'system/lib/libc'
  src_files = ['crt1_reactor.c']

  force_object_files = True

  def get_ext(self):
    return '.o'

  def can_use(self):
    return super().can_use() and settings.STANDALONE_WASM


class crt1_proxy_main(MuslInternalLibrary):
  name = 'crt1_proxy_main'
  src_dir = 'system/lib/libc'
  src_files = ['crt1_proxy_main.c']

  force_object_files = True

  def get_ext(self):
    return '.o'

  def can_use(self):
    return super().can_use() and settings.PROXY_TO_PTHREAD


class crtbegin(MuslInternalLibrary):
  name = 'crtbegin'
  cflags = ['-pthread']
  src_dir = 'system/lib/pthread'
  src_files = ['emscripten_tls_init.c']

  force_object_files = True

  def get_ext(self):
    return '.o'

  def can_use(self):
    return super().can_use() and settings.SHARED_MEMORY


class libcxxabi(NoExceptLibrary, MTLibrary, DebugLibrary):
  name = 'libc++abi'
  cflags = [
      '-Oz',
      '-fno-inline-functions',
      '-D_LIBCPP_BUILDING_LIBRARY',
      '-D_LIBCXXABI_BUILDING_LIBRARY',
      '-DLIBCXXABI_NON_DEMANGLING_TERMINATE',
      '-std=c++20',
    ]
  includes = ['system/lib/libcxx/src']

  def __init__(self, **kwargs):
    super().__init__(**kwargs)
    # TODO EXCEPTION_STACK_TRACES currently requires the debug version of
    # libc++abi, causing the debug version of libc++abi to be linked, which
    # increases code size. libc++abi is not a big library to begin with, but if
    # this becomes a problem, consider making EXCEPTION_STACK_TRACES work with
    # the non-debug version of libc++abi.
    self.is_debug |= settings.EXCEPTION_STACK_TRACES

  def get_cflags(self):
    cflags = super().get_cflags()
    if not self.is_mt and not self.is_ww:
      cflags.append('-D_LIBCXXABI_HAS_NO_THREADS')
    if self.eh_mode == Exceptions.NONE:
      cflags.append('-D_LIBCXXABI_NO_EXCEPTIONS')
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      cflags.append('-D__EMSCRIPTEN_EXCEPTIONS__')
      # The code used to interpret exceptions during terminate
      # is not compatible with emscripten exceptions.
      cflags.append('-DLIBCXXABI_SILENT_TERMINATE')
    elif self.eh_mode == Exceptions.WASM:
      cflags.append('-D__WASM_EXCEPTIONS__')
    return cflags

  def get_files(self):
    filenames = [
      'abort_message.cpp',
      'cxa_aux_runtime.cpp',
      'cxa_default_handlers.cpp',
      'cxa_demangle.cpp',
      'cxa_guard.cpp',
      'cxa_handlers.cpp',
      'cxa_virtual.cpp',
      'cxa_thread_atexit.cpp',
      'fallback_malloc.cpp',
      'stdlib_new_delete.cpp',
      'stdlib_exception.cpp',
      'stdlib_stdexcept.cpp',
      'stdlib_typeinfo.cpp',
      'private_typeinfo.cpp',
      'cxa_exception_js_utils.cpp',
    ]
    if self.eh_mode == Exceptions.NONE:
      filenames += ['cxa_noexception.cpp']
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      filenames += ['cxa_exception_emscripten.cpp']
    elif self.eh_mode == Exceptions.WASM:
      filenames += [
        'cxa_exception_storage.cpp',
        'cxa_exception.cpp',
        'cxa_personality.cpp'
      ]
    else:
      assert False

    return files_in_path(
        path='system/lib/libcxxabi/src',
        filenames=filenames)


class libcxx(NoExceptLibrary, MTLibrary):
  name = 'libc++'

  cflags = [
    '-Oz',
    '-fno-inline-functions',
    '-DLIBCXX_BUILDING_LIBCXXABI=1',
    '-D_LIBCPP_BUILDING_LIBRARY',
    '-D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS',
    # TODO(sbc): clang recently introduced this new warning which is triggered
    # by `filesystem/directory_iterator.cpp`: https://reviews.llvm.org/D119670
    '-Wno-unqualified-std-cast-call',
    '-Wno-unknown-warning-option',
    '-std=c++20',
  ]

  includes = ['system/lib/libcxx/src']

  src_dir = 'system/lib/libcxx/src'
  src_glob = '**/*.cpp'
  src_glob_exclude = [
    'xlocale_zos.cpp',
    'mbsnrtowcs.cpp',
    'wcsnrtombs.cpp',
    'locale_win32.cpp',
    'thread_win32.cpp',
    'support.cpp',
    'int128_builtins.cpp',
    'libdispatch.cpp',
    # Emscripten does not have C++20's time zone support which requires access
    # to IANA Time Zone Database. TODO Implement this using JS timezone
    'tz.cpp',
    'tzdb_list.cpp',
  ]

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.eh_mode == Exceptions.WASM:
      cflags.append('-D__WASM_EXCEPTIONS__')
    return cflags


class libunwind(NoExceptLibrary, MTLibrary):
  name = 'libunwind'
  # Because calls to _Unwind_CallPersonality are generated during LTO, libunwind
  # can't currently be part of LTO.
  # See https://bugs.llvm.org/show_bug.cgi?id=44353
  force_object_files = True

  cflags = ['-Oz', '-fno-inline-functions', '-D_LIBUNWIND_HIDE_SYMBOLS']
  src_dir = 'system/lib/libunwind/src'
  # Without this we can't build libunwind since it will pickup the unwind.h
  # that is part of llvm (which is not compatible for some reason).
  includes = ['system/lib/libunwind/include']
  src_files = ['Unwind-wasm.c']

  def __init__(self, **kwargs):
    super().__init__(**kwargs)

  def can_use(self):
    return super().can_use() and self.eh_mode == Exceptions.WASM

  def get_cflags(self):
    cflags = super().get_cflags()
    cflags.append('-DNDEBUG')
    if not self.is_mt and not self.is_ww:
      cflags.append('-D_LIBUNWIND_HAS_NO_THREADS')
    if self.eh_mode == Exceptions.NONE:
      cflags.append('-D_LIBUNWIND_HAS_NO_EXCEPTIONS')
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      cflags.append('-D__EMSCRIPTEN_EXCEPTIONS__')
    elif self.eh_mode == Exceptions.WASM:
      cflags.append('-D__WASM_EXCEPTIONS__')
    return cflags


class libmalloc(MTLibrary):
  name = 'libmalloc'

  cflags = ['-fno-builtin', '-Wno-unused-function', '-Wno-unused-but-set-variable', '-Wno-unused-variable']
  # malloc/free/calloc are runtime functions and can be generated during LTO
  # Therefor they cannot themselves be part of LTO.
  force_object_files = True

  def __init__(self, **kwargs):
    self.malloc = kwargs.pop('malloc')
    if self.malloc not in ('dlmalloc', 'emmalloc', 'emmalloc-debug', 'emmalloc-memvalidate', 'emmalloc-verbose', 'emmalloc-memvalidate-verbose', 'mimalloc', 'none'):
      raise Exception('malloc must be one of "emmalloc[-debug|-memvalidate][-verbose]", "dlmalloc" or "none", see settings.js')

    self.is_tracing = kwargs.pop('is_tracing')
    self.memvalidate = kwargs.pop('memvalidate')
    self.verbose = kwargs.pop('verbose')
    self.is_debug = kwargs.pop('is_debug') or self.memvalidate or self.verbose

    super().__init__(**kwargs)

  def get_files(self):
    malloc_base = self.malloc.replace('-memvalidate', '').replace('-verbose', '').replace('-debug', '')
    malloc = utils.path_from_root('system/lib', {
      'dlmalloc': 'dlmalloc.c', 'emmalloc': 'emmalloc.c',
    }[malloc_base])
    return [malloc]

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.memvalidate:
      cflags += ['-DEMMALLOC_MEMVALIDATE']
    if self.verbose:
      cflags += ['-DEMMALLOC_VERBOSE']
    if self.is_debug:
      cflags += ['-UNDEBUG', '-DDLMALLOC_DEBUG']
    else:
      cflags += ['-DNDEBUG']
    if self.is_tracing:
      cflags += ['--tracing']
    return cflags

  def get_base_name_prefix(self):
    return 'lib%s' % self.malloc

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_debug and not self.memvalidate and not self.verbose:
      name += '-debug'
    if self.is_tracing:
      name += '-tracing'
    return name

  def can_use(self):
    return super().can_use() and settings.MALLOC != 'none' and settings.MALLOC != 'mimalloc'

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_debug', 'is_tracing', 'memvalidate', 'verbose']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      malloc=settings.MALLOC,
      is_debug=settings.ASSERTIONS >= 2,
      is_tracing=settings.EMSCRIPTEN_TRACING,
      memvalidate='memvalidate' in settings.MALLOC,
      verbose='verbose' in settings.MALLOC,
      **kwargs
    )

  @classmethod
  def variations(cls):
    combos = super().variations()
    return ([dict(malloc='dlmalloc', **combo) for combo in combos if not combo['memvalidate'] and not combo['verbose']] +
            [dict(malloc='emmalloc', **combo) for combo in combos if not combo['memvalidate'] and not combo['verbose']] +
            [dict(malloc='emmalloc-memvalidate-verbose', **combo) for combo in combos if combo['memvalidate'] and combo['verbose']] +
            [dict(malloc='emmalloc-memvalidate', **combo) for combo in combos if combo['memvalidate'] and not combo['verbose']] +
            [dict(malloc='emmalloc-verbose', **combo) for combo in combos if combo['verbose'] and not combo['memvalidate']])


class libmimalloc(MTLibrary):
  name = 'libmimalloc'

  cflags = [
    '-fno-builtin',
    '-Wno-deprecated-pragma',
    # build emmalloc as only a system allocator, without exporting itself onto
    # malloc/free in the global scope
    '-DEMMALLOC_NO_STD_EXPORTS',
    # build mimalloc with an override of malloc/free
    '-DMI_MALLOC_OVERRIDE',
    # TODO: add build modes that include debug checks 1,2,3
    '-DMI_DEBUG=0',
  ]

  # malloc/free/calloc are runtime functions and can be generated during LTO
  # Therefor they cannot themselves be part of LTO.
  force_object_files = True

  includes = ['system/lib/mimalloc/include']

  # Build all of mimalloc, and also emmalloc which is used as the system
  # allocator underneath it.
  src_dir = 'system/lib/'
  src_files = glob_in_path(
    path='system/lib/mimalloc/src',
    glob_pattern='*.c',
    # mimalloc includes some files at the source level, so exclude them here.
    excludes=['alloc-override.c', 'page-queue.c', 'static.c']
  )
  src_files += files_in_path(
    path='system/lib/mimalloc/src/prim',
    filenames=['prim.c']
  )
  src_files += files_in_path(
    path='system/lib/',
    filenames=['emmalloc.c'])

  def can_use(self):
    return super().can_use() and settings.MALLOC == 'mimalloc'


class libal(Library):
  name = 'libal'

  cflags = ['-Os', '-fno-inline-functions']
  src_dir = 'system/lib'
  src_files = ['al.c']


class libGL(MTLibrary):
  name = 'libGL'

  src_dir = 'system/lib/gl'
  src_files = ['gl.c', 'webgl1.c', 'libprocaddr.c', 'webgl2.c']

  cflags = ['-Oz', '-fno-inline-functions']

  def __init__(self, **kwargs):
    self.is_legacy = kwargs.pop('is_legacy')
    self.is_webgl2 = kwargs.pop('is_webgl2')
    self.is_ofb = kwargs.pop('is_ofb')
    self.is_full_es3 = kwargs.pop('is_full_es3')
    self.is_enable_get_proc_address = kwargs.pop('is_enable_get_proc_address')
    super().__init__(**kwargs)

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_legacy:
      name += '-emu'
    if self.is_webgl2:
      name += '-webgl2'
    if self.is_ofb:
      name += '-ofb'
    if self.is_full_es3:
      name += '-full_es3'
    if self.is_enable_get_proc_address:
      name += '-getprocaddr'
    return name

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_legacy:
      cflags += ['-DLEGACY_GL_EMULATION=1']
    cflags += [f'-DMAX_WEBGL_VERSION={2 if self.is_webgl2 else 1}']
    if self.is_ofb:
      cflags += ['-D__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__']
    if self.is_full_es3:
      cflags += ['-D__EMSCRIPTEN_FULL_ES3__']
    if self.is_enable_get_proc_address:
      cflags += ['-DGL_ENABLE_GET_PROC_ADDRESS=1']
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_legacy', 'is_webgl2', 'is_ofb', 'is_full_es3', 'is_enable_get_proc_address']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      is_legacy=settings.LEGACY_GL_EMULATION,
      is_webgl2=settings.MAX_WEBGL_VERSION >= 2,
      is_ofb=settings.OFFSCREEN_FRAMEBUFFER,
      is_full_es3=settings.FULL_ES3,
      is_enable_get_proc_address=settings.GL_ENABLE_GET_PROC_ADDRESS,
      **kwargs
    )


class libwebgpu(MTLibrary):
  name = 'libwebgpu'

  src_dir = 'system/lib/webgpu'
  src_files = ['webgpu.cpp']


class libwebgpu_cpp(MTLibrary):
  name = 'libwebgpu_cpp'

  cflags = ['-std=c++11']
  src_dir = 'system/lib/webgpu'
  src_files = ['webgpu_cpp.cpp']


class libembind(Library):
  name = 'libembind'
  never_force = True

  def __init__(self, **kwargs):
    self.with_rtti = kwargs.pop('with_rtti', False)
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    cflags.append('-std=c++20')
    if not self.with_rtti:
      cflags += ['-fno-rtti', '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0']
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['with_rtti']

  def get_base_name(self):
    name = super().get_base_name()
    if self.with_rtti:
      name += '-rtti'
    return name

  def get_files(self):
    return [utils.path_from_root('system/lib/embind/bind.cpp')]

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(with_rtti=settings.USE_RTTI, **kwargs)


class libfetch(MTLibrary):
  name = 'libfetch'
  never_force = True
  includes = ['system/lib/libc']

  def get_files(self):
    return [utils.path_from_root('system/lib/fetch/emscripten_fetch.c')]


class libstb_image(Library):
  name = 'libstb_image'
  never_force = True
  includes = ['third_party']

  def get_files(self):
    return [utils.path_from_root('system/lib/stb_image.c')]


class libwasmfs(DebugLibrary, AsanInstrumentedLibrary, MTLibrary):
  name = 'libwasmfs'

  cflags = ['-fno-exceptions', '-std=c++17']

  includes = ['system/lib/wasmfs', 'system/lib/pthread']

  def __init__(self, **kwargs):
    self.ignore_case = kwargs.pop('ignore_case')
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.ignore_case:
      cflags += ['-DWASMFS_CASE_INSENSITIVE']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.ignore_case:
      name += '-icase'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['ignore_case']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(ignore_case=settings.CASE_INSENSITIVE_FS, **kwargs)

  def get_files(self):
    backends = files_in_path(
        path='system/lib/wasmfs/backends',
        filenames=['fetch_backend.cpp',
                   'ignore_case_backend.cpp',
                   'js_file_backend.cpp',
                   'memory_backend.cpp',
                   'node_backend.cpp',
                   'opfs_backend.cpp'])
    return backends + files_in_path(
        path='system/lib/wasmfs',
        filenames=['file.cpp',
                   'file_table.cpp',
                   'js_api.cpp',
                   'emscripten.cpp',
                   'paths.cpp',
                   'special_files.cpp',
                   'support.cpp',
                   'syscalls.cpp',
                   'wasmfs.cpp'])

  def can_use(self):
    return settings.WASMFS


# Minimal syscall implementation, enough for printf. If this can be linked in
# instead of the full WasmFS then it saves a lot of code size for simple
# programs that don't need a full FS implementation.
class libwasmfs_no_fs(Library):
  name = 'libwasmfs_no_fs'

  src_dir = 'system/lib/wasmfs'
  src_files = ['no_fs.c']

  def can_use(self):
    # If the filesystem is forced then we definitely do not need this library.
    return settings.WASMFS and not settings.FORCE_FILESYSTEM


class libwasmfs_noderawfs(Library):
  name = 'libwasmfs_noderawfs'

  cflags = ['-fno-exceptions', '-std=c++17']

  includes = ['system/lib/wasmfs']

  def get_files(self):
    return files_in_path(
        path='system/lib/wasmfs/backends',
        filenames=['noderawfs_root.cpp'])

  def can_use(self):
    return settings.WASMFS and settings.NODERAWFS


class libhtml5(Library):
  name = 'libhtml5'

  includes = ['system/lib/libc']
  cflags = ['-Oz', '-fno-inline-functions']
  src_dir = 'system/lib/html5'
  src_glob = '*.c'


class CompilerRTLibrary(Library):
  cflags = ['-fno-builtin']
  # compiler_rt files can't currently be part of LTO although we are hoping to remove this
  # restriction soon: https://reviews.llvm.org/D71738
  force_object_files = True


class libubsan_minimal_rt(CompilerRTLibrary, MTLibrary):
  name = 'libubsan_minimal_rt'
  never_force = True

  includes = ['system/lib/compiler-rt/lib']
  src_dir = 'system/lib/compiler-rt/lib/ubsan_minimal'
  src_files = ['ubsan_minimal_handlers.cpp']


class libsanitizer_common_rt(CompilerRTLibrary, MTLibrary):
  name = 'libsanitizer_common_rt'
  # TODO(sbc): We should not need musl-internal headers here.
  includes = ['system/lib/libc/musl/src/internal',
              'system/lib/compiler-rt/lib',
              'system/lib/libc']
  never_force = True
  cflags = [
    '-D_LARGEFILE64_SOURCE',
    # The upstream code has many format violations and suppresses it with
    # -Wno-format, so we match that.
    # https://github.com/llvm/llvm-project/blob/da675b922cca3dc9a76642d792e882979a3d8c82/compiler-rt/lib/sanitizer_common/CMakeLists.txt#L225-L226
    # TODO Remove this when the issues are resolved.
    '-Wno-format',
  ]

  src_dir = 'system/lib/compiler-rt/lib/sanitizer_common'
  src_glob = '*.cpp'
  src_glob_exclude = ['sanitizer_common_nolibc.cpp']


class SanitizerLibrary(CompilerRTLibrary, MTLibrary):
  never_force = True

  includes = ['system/lib/compiler-rt/lib']
  src_glob = '*.cpp'


class libubsan_rt(SanitizerLibrary):
  name = 'libubsan_rt'

  includes = ['system/lib/libc']
  cflags = ['-DUBSAN_CAN_USE_CXXABI']
  src_dir = 'system/lib/compiler-rt/lib/ubsan'
  src_glob_exclude = ['ubsan_diag_standalone.cpp']


class liblsan_common_rt(SanitizerLibrary):
  name = 'liblsan_common_rt'

  src_dir = 'system/lib/compiler-rt/lib/lsan'
  src_glob = 'lsan_common*.cpp'


class liblsan_rt(SanitizerLibrary):
  name = 'liblsan_rt'

  includes = ['system/lib/libc']
  src_dir = 'system/lib/compiler-rt/lib/lsan'
  src_glob_exclude = ['lsan_common.cpp', 'lsan_common_mac.cpp', 'lsan_common_linux.cpp',
                      'lsan_common_emscripten.cpp']


class libasan_rt(SanitizerLibrary):
  name = 'libasan_rt'

  includes = ['system/lib/libc']
  src_dir = 'system/lib/compiler-rt/lib/asan'


class libasan_js(Library):
  name = 'libasan_js'
  never_force = True

  cflags = ['-fsanitize=address']

  src_dir = 'system/lib'
  src_files = ['asan_js.c']


# This library is used when STANDALONE_WASM is set. In that mode, we don't
# want to depend on JS, and so this library contains implementations of
# things that we'd normally do in JS. That includes some general things
# as well as some additional musl components (that normally we reimplement
# in JS as it's more efficient that way).
class libstandalonewasm(MuslInternalLibrary):
  name = 'libstandalonewasm'
  # LTO defeats the weak linking trick used in __original_main.c
  force_object_files = True

  cflags = ['-Os', '-fno-inline-functions', '-fno-builtin']
  src_dir = 'system/lib'

  def __init__(self, **kwargs):
    self.is_mem_grow = kwargs.pop('is_mem_grow')
    self.is_pure = kwargs.pop('is_pure')
    self.nocatch = kwargs.pop('nocatch')
    super().__init__(**kwargs)

  def get_base_name(self):
    name = super().get_base_name()
    if self.nocatch:
      name += '-nocatch'
    if self.is_mem_grow:
      name += '-memgrow'
    if self.is_pure:
      name += '-pure'
    return name

  def get_cflags(self):
    cflags = super().get_cflags()
    cflags += ['-DNDEBUG', '-DEMSCRIPTEN_STANDALONE_WASM']
    if self.is_mem_grow:
      cflags += ['-DEMSCRIPTEN_MEMORY_GROWTH']
    if self.is_pure:
      cflags += ['-DEMSCRIPTEN_PURE_WASI']
    if self.nocatch:
      cflags.append('-DEMSCRIPTEN_NOCATCH')
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_mem_grow', 'is_pure', 'nocatch']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      is_mem_grow=settings.ALLOW_MEMORY_GROWTH,
      is_pure=settings.PURE_WASI,
      nocatch=settings.DISABLE_EXCEPTION_CATCHING and not settings.WASM_EXCEPTIONS,
      **kwargs
    )

  def get_files(self):
    files = files_in_path(
        path='system/lib/standalone',
        filenames=['standalone.c',
                   'standalone_wasm_stdio.c',
                   '__main_void.c'])
    files += files_in_path(
        path='system/lib/libc',
        filenames=['emscripten_memcpy.c', 'emscripten_memset.c'])
    # It is more efficient to use JS methods for time, normally.
    files += files_in_path(
        path='system/lib/libc/musl/src/time',
        filenames=['__secs_to_tm.c',
                   '__tz.c',
                   'clock.c',
                   'clock_gettime.c',
                   'gettimeofday.c',
                   'localtime_r.c',
                   'gmtime_r.c',
                   'mktime.c',
                   'strptime.c',
                   'timegm.c',
                   'time.c'])
    # It is more efficient to use JS for __assert_fail, as it avoids always
    # including fprintf etc.
    files += files_in_path(
        path='system/lib/libc/musl/src/exit',
        filenames=['assert.c', 'exit.c'])
    return files

  def can_use(self):
    return super(libstandalonewasm, self).can_use() and settings.STANDALONE_WASM


class libjsmath(Library):
  name = 'libjsmath'
  cflags = ['-Os', '-fno-inline-functions']
  src_dir = 'system/lib'
  src_files = ['jsmath.c']

  def can_use(self):
    return super(libjsmath, self).can_use() and settings.JS_MATH


class libstubs(DebugLibrary):
  name = 'libstubs'
  src_dir = 'system/lib/libc'
  includes = ['system/lib/libc/musl/src/include']
  src_files = ['emscripten_syscall_stubs.c', 'emscripten_libc_stubs.c']


def get_libs_to_link(args):
  libs_to_link = []

  if '-nostdlib' in args:
    return libs_to_link

  already_included = set()
  system_libs_map = Library.get_usable_variations()

  # Setting this in the environment will avoid checking dependencies and make
  # building big projects a little faster 1 means include everything; otherwise
  # it can be the name of a lib (libc++, etc.).
  # You can provide 1 to include everything, or a comma-separated list with the
  # ones you want
  force_include = []
  force = os.environ.get('EMCC_FORCE_STDLIBS')
  # Setting this will only use the forced libs in EMCC_FORCE_STDLIBS. This
  # avoids spending time checking for unresolved symbols in your project files,
  # which can speed up linking, but if you do not have the proper list of
  # actually needed libraries, errors can occur.
  only_forced = os.environ.get('EMCC_ONLY_FORCED_STDLIBS')
  if only_forced:
    # One of the purposes EMCC_ONLY_FORCED_STDLIBS was to skip the scanning
    # of the input files for reverse dependencies.
    diagnostics.warning('deprecated', 'EMCC_ONLY_FORCED_STDLIBS is deprecated.  Use `-nostdlib` to avoid linking standard libraries')
  if force == '1':
    force_include = [name for name, lib in system_libs_map.items() if not lib.never_force]
  elif force is not None:
    force_include = force.split(',')
  if force_include:
    logger.debug(f'forcing stdlibs: {force_include}')

  def add_library(libname):
    lib = system_libs_map[libname]
    if lib.name in already_included:
      return
    already_included.add(lib.name)

    logger.debug('including %s (%s)' % (lib.name, lib.get_filename()))

    need_whole_archive = lib.name in force_include and lib.get_ext() == '.a'
    libs_to_link.append((lib.get_link_flag(), need_whole_archive))

  if '-nostartfiles' not in args:
    if settings.SHARED_MEMORY:
      add_library('crtbegin')

    if not settings.SIDE_MODULE:
      if settings.STANDALONE_WASM:
        if settings.EXPECT_MAIN:
          add_library('crt1')
        else:
          add_library('crt1_reactor')
      elif settings.PROXY_TO_PTHREAD:
        add_library('crt1_proxy_main')

  if settings.SIDE_MODULE:
    return libs_to_link

  # We add the forced libs last so that any libraries that are added in the normal
  # sequence below are added in the correct order even when they are also part of
  # EMCC_FORCE_STDLIBS.
  def add_forced_libs():
    for forced in force_include:
      if forced not in system_libs_map:
        shared.exit_with_error('invalid forced library: %s', forced)
      add_library(forced)

  if '-nodefaultlibs' in args:
    add_forced_libs()
    return libs_to_link

  sanitize = settings.USE_LSAN or settings.USE_ASAN or settings.UBSAN_RUNTIME

  def add_sanitizer_libs():
    if settings.USE_ASAN:
      force_include.append('libasan_rt')
      add_library('libasan_rt')
      add_library('libasan_js')
    elif settings.USE_LSAN:
      force_include.append('liblsan_rt')
      add_library('liblsan_rt')

    if settings.UBSAN_RUNTIME == 1:
      add_library('libubsan_minimal_rt')
    elif settings.UBSAN_RUNTIME == 2:
      add_library('libubsan_rt')

    if settings.USE_LSAN or settings.USE_ASAN:
      add_library('liblsan_common_rt')

    if sanitize:
      add_library('libsanitizer_common_rt')

  if only_forced:
    add_library('libcompiler_rt')
    add_sanitizer_libs()
    add_forced_libs()
    return libs_to_link

  if settings.AUTO_NATIVE_LIBRARIES:
    add_library('libGL')
    add_library('libal')
    add_library('libhtml5')

  # JS math must come before anything else, so that it overrides the normal
  # libc math.
  if settings.JS_MATH:
    add_library('libjsmath')

  # C libraries that override libc must come before it
  if settings.PRINTF_LONG_DOUBLE:
    add_library('libprintf_long_double')
  # See comment in libc_optz itself
  if settings.SHRINK_LEVEL >= 2 and not settings.LINKABLE and \
     not os.environ.get('EMCC_FORCE_STDLIBS'):
    add_library('libc_optz')
  if settings.BULK_MEMORY:
    add_library('libbulkmemory')
  if settings.STANDALONE_WASM:
    add_library('libstandalonewasm')
  if settings.ALLOW_UNIMPLEMENTED_SYSCALLS:
    add_library('libstubs')
  if '-nolibc' not in args:
    if not settings.EXIT_RUNTIME:
      add_library('libnoexit')
    add_library('libc')
    if settings.MALLOC == 'mimalloc':
      add_library('libmimalloc')
    elif settings.MALLOC != 'none':
      add_library('libmalloc')
  add_library('libcompiler_rt')
  if settings.LINK_AS_CXX:
    add_library('libc++')
  if settings.LINK_AS_CXX or sanitize:
    add_library('libc++abi')
    if settings.WASM_EXCEPTIONS:
      add_library('libunwind')

  if settings.PROXY_POSIX_SOCKETS:
    add_library('libsockets_proxy')
  else:
    add_library('libsockets')

  if settings.USE_WEBGPU:
    add_library('libwebgpu')
    if settings.LINK_AS_CXX:
      add_library('libwebgpu_cpp')

  if settings.WASM_WORKERS and (not settings.SINGLE_FILE and
                                not settings.RELOCATABLE and
                                not settings.PROXY_TO_WORKER):
    add_library('libwasm_workers')

  if settings.WASMFS:
    # Link in the no-fs version first, so that if it provides all the needed
    # system libraries then WasmFS is not linked in at all. (We only do this if
    # the filesystem is not forced; if it is then we know we definitely need the
    # whole thing, and it would be unnecessary work to try to link in the no-fs
    # version).
    if not settings.FORCE_FILESYSTEM:
      add_library('libwasmfs_no_fs')
    add_library('libwasmfs')

  add_sanitizer_libs()
  add_forced_libs()
  return libs_to_link


def calculate(args):

  libs_to_link = get_libs_to_link(args)

  # When LINKABLE is set the entire link command line is wrapped in --whole-archive by
  # building.link_ldd.  And since --whole-archive/--no-whole-archive processing does not nest we
  # shouldn't add any extra `--no-whole-archive` or we will undo the intent of building.link_ldd.
  if settings.LINKABLE or settings.SIDE_MODULE:
    return [l[0] for l in libs_to_link]

  # Wrap libraries in --whole-archive, as needed.  We need to do this last
  # since otherwise the abort sorting won't make sense.
  ret = []
  in_group = False
  for name, need_whole_archive in libs_to_link:
    if need_whole_archive and not in_group:
      ret.append('--whole-archive')
      in_group = True
    if in_group and not need_whole_archive:
      ret.append('--no-whole-archive')
      in_group = False
    ret.append(name)
  if in_group:
    ret.append('--no-whole-archive')

  return ret


# Once we require python 3.8 we can use shutil.copytree with
# dirs_exist_ok=True and remove this function.
def copytree_exist_ok(src, dst):
  os.makedirs(dst, exist_ok=True)
  for entry in os.scandir(src):
    srcname = os.path.join(src, entry.name)
    dstname = os.path.join(dst, entry.name)
    if entry.is_dir():
      copytree_exist_ok(srcname, dstname)
    else:
      shared.safe_copy(srcname, dstname)


def install_system_headers(stamp):
  install_dirs = {
    ('include',): '',
    ('lib', 'compiler-rt', 'include'): '',
    ('lib', 'libunwind', 'include'): '',
    # Copy the generic arch files first then
    ('lib', 'libc', 'musl', 'arch', 'generic'): '',
    # Then overlay the emscripten directory on top.
    # This mimics how musl itself installs its headers.
    ('lib', 'libc', 'musl', 'arch', 'emscripten'): '',
    ('lib', 'libc', 'musl', 'include'): '',
    ('lib', 'libcxx', 'include'): os.path.join('c++', 'v1'),
    ('lib', 'libcxxabi', 'include'): os.path.join('c++', 'v1'),
    ('lib', 'mimalloc', 'include'): '',
  }

  target_include_dir = cache.get_include_dir()
  for src, dest in install_dirs.items():
    src = utils.path_from_root('system', *src)
    dest = os.path.join(target_include_dir, dest)
    copytree_exist_ok(src, dest)

  pkgconfig_src = utils.path_from_root('system/lib/pkgconfig')
  pkgconfig_dest = cache.get_sysroot_dir('lib/pkgconfig')
  copytree_exist_ok(pkgconfig_src, pkgconfig_dest)

  bin_src = utils.path_from_root('system/bin')
  bin_dest = cache.get_sysroot_dir('bin')
  copytree_exist_ok(bin_src, bin_dest)

  cmake_src = utils.path_from_root('system/lib/cmake')
  cmake_dest = cache.get_sysroot_dir('lib/cmake')
  copytree_exist_ok(cmake_src, cmake_dest)

  # Create a version header based on the emscripten-version.txt
  version_file = cache.get_include_dir('emscripten/version.h')
  utils.write_file(version_file, textwrap.dedent(f'''\
  /* Automatically generated by tools/system_libs.py */
  #define __EMSCRIPTEN_major__ {utils.EMSCRIPTEN_VERSION_MAJOR}
  #define __EMSCRIPTEN_minor__ {utils.EMSCRIPTEN_VERSION_MINOR}
  #define __EMSCRIPTEN_tiny__ {utils.EMSCRIPTEN_VERSION_TINY}
  '''))

  # Create a stamp file that signal that the headers have been installed
  # Removing this file, or running `emcc --clear-cache` or running
  # `./embuilder build sysroot --force` will cause the re-installation of
  # the system headers.
  utils.write_file(stamp, 'x')
  return stamp


@ToolchainProfiler.profile()
def ensure_sysroot():
  cache.get('sysroot_install.stamp', install_system_headers, what='system headers')


def build_deferred():
  assert USE_NINJA
  top_level_ninja = get_top_level_ninja_file()
  if os.path.isfile(top_level_ninja):
    run_ninja(os.path.dirname(top_level_ninja))
