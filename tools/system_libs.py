# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import hashlib
import itertools
import json
import logging
import os
import re
import shutil
import sys
import tarfile
import zipfile
from glob import iglob

from . import ports
from . import shared
from tools.shared import check_call, mangle_c_symbol_name, demangle_c_symbol_name

stdout = None
stderr = None

logger = logging.getLogger('system_libs')


def files_in_path(path_components, filenames):
  srcdir = shared.path_from_root(*path_components)
  return [os.path.join(srcdir, f) for f in filenames]


def glob_in_path(path_components, glob_pattern, excludes=()):
  srcdir = shared.path_from_root(*path_components)
  return [f for f in iglob(os.path.join(srcdir, glob_pattern)) if os.path.basename(f) not in excludes]


def get_cflags(force_object_files=False):
  flags = []
  if force_object_files:
    flags += ['-s', 'WASM_OBJECT_FILES=1']
  elif not shared.Settings.WASM_OBJECT_FILES:
    flags += ['-s', 'WASM_OBJECT_FILES=0']
  if shared.Settings.RELOCATABLE:
    flags += ['-s', 'RELOCATABLE']
  return flags


def run_build_command(cmd):
  # this must only be called on a standard build command
  assert cmd[0] == shared.PYTHON and cmd[1] in (shared.EMCC, shared.EMXX)
  # add standard cflags, but also allow the cmd to override them
  cmd = cmd[:2] + get_cflags() + cmd[2:]
  shared.run_process(cmd, stdout=stdout, stderr=stderr)


def run_commands(commands):
  cores = min(len(commands), shared.Building.get_num_cores())
  if cores <= 1:
    for command in commands:
      run_build_command(command)
  else:
    pool = shared.Building.get_multiprocessing_pool()
    # https://stackoverflow.com/questions/1408356/keyboard-interrupts-with-pythons-multiprocessing-pool
    # https://bugs.python.org/issue8296
    # 999999 seconds (about 11 days) is reasonably huge to not trigger actual timeout
    # and is smaller than the maximum timeout value 4294967.0 for Python 3 on Windows (threading.TIMEOUT_MAX)
    pool.map_async(run_build_command, commands, chunksize=1).get(999999)


def create_lib(libname, inputs):
  """Create a library from a set of input objects."""
  if libname.endswith('.bc'):
    shared.Building.link_to_object(inputs, libname)
  elif libname.endswith('.a'):
    shared.Building.emar('cr', libname, inputs)
  else:
    raise Exception('unknown suffix ' + libname)


def read_symbols(path):
  with open(path) as f:
    content = f.read()

    # Require that Windows newlines should not be present in a symbols file, if running on Linux or macOS
    # This kind of mismatch can occur if one copies a zip file of Emscripten cloned on Windows over to
    # a Linux or macOS system. It will result in Emscripten linker getting confused on stray \r characters,
    # and be unable to link any library symbols properly. We could harden against this by .strip()ping the
    # opened files, but it is possible that the mismatching line endings can cause random problems elsewhere
    # in the toolchain, hence abort execution if so.
    if os.name != 'nt' and '\r\n' in content:
      raise Exception('Windows newlines \\r\\n detected in symbols file "' + path + '"! This could happen for example when copying Emscripten checkout from Windows to Linux or macOS. Please use Unix line endings on checkouts of Emscripten on Linux and macOS!')

    return shared.Building.parse_symbols(content).defs


def get_wasm_libc_rt_files():
  # Static linking is tricky with LLVM, since e.g. memset might not be used
  # from libc, but be used as an intrinsic, and codegen will generate a libc
  # call from that intrinsic *after* static linking would have thought it is
  # all in there. In asm.js this is not an issue as we do JS linking anyhow,
  # and have asm.js-optimized versions of all the LLVM intrinsics. But for
  # wasm, we need a better solution. For now, make another archive that gets
  # included at the same time as compiler-rt.
  # Note that this also includes things that may be depended on by those
  # functions - fmin uses signbit, for example, so signbit must be here (so if
  # fmin is added by codegen, it will have all it needs).
  math_files = files_in_path(
    path_components=['system', 'lib', 'libc', 'musl', 'src', 'math'],
    filenames=[
      'fmin.c', 'fminf.c', 'fminl.c',
      'fmax.c', 'fmaxf.c', 'fmaxl.c',
      'fmod.c', 'fmodf.c', 'fmodl.c',
      'log2.c', 'log2f.c', 'log10.c', 'log10f.c',
      'exp2.c', 'exp2f.c', 'exp10.c', 'exp10f.c',
      'scalbn.c', '__fpclassifyl.c',
      '__signbitl.c', '__signbitf.c', '__signbit.c'
    ])
  other_files = files_in_path(
    path_components=['system', 'lib', 'libc'],
    filenames=['emscripten_memcpy.c', 'emscripten_memset.c',
               'emscripten_memmove.c'])
  return math_files + other_files


class Library(object):
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
      return super().get_default_variation(my_parameter=shared.Settings.MY_PARAMETER, **kwargs)

  This allows the correct variation of the library to be selected when building
  code with Emscripten.
  """

  # The simple name of the library. When linking, this is the name to use to
  # automatically get the correct version of the library.
  # This should only be overridden in a concrete library class, e.g. libc,
  # and left as None in an abstract library class, e.g. MTLibrary.
  name = None

  # A list of simple names of other libraries that this one depends on.
  # For dynamic values, override `get_depends()` instead.
  depends = []

  # A set of symbols that this library exports. If this library is intended to
  # pulled in via undefined symbols, this should be overridden with a set
  # returned by `read_symbols`.
  # If this library is to be linked in manually, leave this empty.
  # For dynamic values, override `get_symbols()` instead.
  symbols = set()

  # A list of symbols that must be exported to keep the JavaScript
  # dependencies of this library working.
  js_depends = []

  # Set to true to prevent EMCC_FORCE_STDLIBS from linking this library.
  never_force = False

  # The C compile executable to use. You can override this to shared.EMXX for C++.
  emcc = shared.EMCC

  # A list of flags to pass to emcc.
  # The flags for the parent class is automatically inherited.
  cflags = ['-Werror']

  # A list of directories to put in the include path when building.
  # This is a list of tuples of path components.
  # For example, to put system/lib/a and system/lib/b under the emscripten
  # directory into the include path, you would write:
  #    includes = [('system', 'lib', 'a'), ('system', 'lib', 'b')]
  # The include path of the parent class is automatically inherited.
  includes = []

  # By default, `get_files` look for source files for this library under `src_dir`.
  # It will either use the files listed in `src_files`, or use the glob pattern in
  # `src_glob`. You may not specify both `src_files` and `src_glob`.
  # When using `src_glob`, you can specify a list of files in `src_glob_exclude`
  # to be excluded from the library.
  # Alternatively, you can override `get_files` to use your own logic.
  src_dir = None
  src_files = None
  src_glob = None
  src_glob_exclude = None

  # Whether to always generate WASM object files, even though WASM_OBJECT_FILES=0.
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

  def in_temp(cls, *args):
    """Gets the path of a file in our temporary directory."""
    return os.path.join(shared.get_emscripten_temp_dir(), *args)

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

  def get_path(self):
    """
    Gets the cached path of this library.

    This will trigger a build if this library is not in the cache.
    """
    return shared.Cache.get(self.get_name(), self.build)

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

  def build_objects(self):
    """
    Returns a list of compiled object files for this library.

    By default, this builds all the source files returned by `self.get_files()`,
    with the `cflags` returned by `self.get_cflags()`.
    """
    commands = []
    objects = []
    cflags = self.get_cflags()
    for src in self.get_files():
      o = self.in_temp(os.path.basename(src) + '.o')
      commands.append([shared.PYTHON, self.emcc, src, '-o', o] + cflags)
      objects.append(o)
    run_commands(commands)
    return objects

  def build(self):
    """Builds the library and returns the path to the file."""
    lib_filename = self.in_temp(self.get_name())
    create_lib(lib_filename, self.build_objects())
    return lib_filename

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
    cflags += get_cflags(force_object_files=self.force_object_files)

    if self.includes:
      cflags += ['-I' + shared.path_from_root(*path) for path in self._inherit_list('includes')]

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
    return '.a' if shared.Settings.WASM_BACKEND and shared.Settings.WASM_OBJECT_FILES else '.bc'

  def get_name(self):
    """
    Return the full name of the library file, including the file extension.
    """
    return self.get_base_name() + self.get_ext()

  def get_symbols(self):
    """
    Return a list of symbols this library exports. See `symbols`.

    This is the dynamic version of `symbols`.
    """
    return self.symbols.copy()

  def get_depends(self):
    """
    Return a list of simple names of libraries that this library depends on.

    This is the dynamic version of `depends`.
    """
    return self.depends

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
      for subclass in subclass.get_inheritance_tree():
        yield subclass

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
    result = {}
    for subclass in cls.get_inheritance_tree():
      if subclass.name:
        library = subclass.get_default_variation()
        if library.can_build() and library.can_use():
          result[subclass.name] = library
    return result


class MTLibrary(Library):
  def __init__(self, **kwargs):
    self.is_mt = kwargs.pop('is_mt')
    super(MTLibrary, self).__init__(**kwargs)

  def get_cflags(self):
    cflags = super(MTLibrary, self).get_cflags()
    if self.is_mt:
      cflags += ['-s', 'USE_PTHREADS=1', '-DUSE_THREADS']
    return cflags

  def get_base_name(self):
    name = super(MTLibrary, self).get_base_name()
    if self.is_mt:
      name += '-mt'
    return name

  @classmethod
  def vary_on(cls):
    return super(MTLibrary, cls).vary_on() + ['is_mt']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(MTLibrary, cls).get_default_variation(is_mt=shared.Settings.USE_PTHREADS, **kwargs)


class NoExceptLibrary(Library):
  def __init__(self, **kwargs):
    self.is_noexcept = kwargs.pop('is_noexcept')
    super(NoExceptLibrary, self).__init__(**kwargs)

  def get_cflags(self):
    cflags = super(NoExceptLibrary, self).get_cflags()
    if self.is_noexcept:
      cflags += ['-fno-exceptions']
    else:
      cflags += ['-s', 'DISABLE_EXCEPTION_CATCHING=0']
    return cflags

  def get_base_name(self):
    name = super(NoExceptLibrary, self).get_base_name()
    if self.is_noexcept:
      name += '-noexcept'
    return name

  @classmethod
  def vary_on(cls):
    return super(NoExceptLibrary, cls).vary_on() + ['is_noexcept']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(NoExceptLibrary, cls).get_default_variation(is_noexcept=shared.Settings.DISABLE_EXCEPTION_CATCHING, **kwargs)


class MuslInternalLibrary(Library):
  includes = [
    ['system', 'lib', 'libc', 'musl', 'src', 'internal'],
    ['system', 'lib', 'libc', 'musl', 'arch', 'js'],
  ]


class AsanInstrumentedLibrary(Library):
  def __init__(self, **kwargs):
    self.is_asan = kwargs.pop('is_asan', False)
    super(AsanInstrumentedLibrary, self).__init__(**kwargs)

  def get_cflags(self):
    cflags = super(AsanInstrumentedLibrary, self).get_cflags()
    if self.is_asan:
      cflags += ['-fsanitize=address']
    return cflags

  def get_base_name(self):
    name = super(AsanInstrumentedLibrary, self).get_base_name()
    if self.is_asan:
      name += '-asan'
    return name

  @classmethod
  def vary_on(cls):
    vary_on = super(AsanInstrumentedLibrary, cls).vary_on()
    if shared.Settings.WASM_BACKEND:
      vary_on += ['is_asan']
    return vary_on

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(AsanInstrumentedLibrary, cls).get_default_variation(is_asan=shared.Settings.USE_ASAN, **kwargs)


class CXXLibrary(Library):
  emcc = shared.EMXX


class NoBCLibrary(Library):
  # Some libraries cannot be compiled as .bc files. This is because .bc files will link in every object in the library.
  # While the optimizer will readily optimize out most of the unused functions, things like global constructors that
  # are linked in cannot be optimized out, even though they are not actually needed. If we use .a files for such
  # libraries, only the object files, and by extension, their contained global constructors, that are actually needed
  # will be linked in.
  def get_ext(self):
    return '.a'


class libcompiler_rt(Library):
  name = 'libcompiler_rt'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcompiler_rt.symbols'))
  depends = ['libc']

  cflags = ['-O2']
  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'builtins']
  src_files = ['divdc3.c', 'divsc3.c', 'muldc3.c', 'mulsc3.c']


class libc(AsanInstrumentedLibrary, MuslInternalLibrary, MTLibrary):
  name = 'libc'

  # XXX We also need to add libc symbols that use malloc, for example strdup. It's very rare to use just them and not
  #     a normal malloc symbol (like free, after calling strdup), so we haven't hit this yet, but it is possible.
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libc.symbols'))

  depends = ['libcompiler_rt']

  # Without -fno-builtin, LLVM can optimize away or convert calls to library
  # functions to something else based on assumptions that they behave exactly
  # like the standard library. This can cause unexpected bugs when we use our
  # custom standard library. The same for other libc/libm builds.
  cflags = ['-Os', '-fno-builtin']

  # Hide several musl warnings that produce a lot of spam to unit test build server logs.
  # TODO: When updating musl the next time, feel free to recheck which of their warnings might have been fixed, and which ones of these could be cleaned up.
  cflags += ['-Wno-return-type', '-Wno-parentheses', '-Wno-ignored-attributes',
             '-Wno-shift-count-overflow', '-Wno-shift-negative-value',
             '-Wno-dangling-else', '-Wno-unknown-pragmas',
             '-Wno-shift-op-parentheses', '-Wno-string-plus-int',
             '-Wno-logical-op-parentheses', '-Wno-bitwise-op-parentheses',
             '-Wno-visibility', '-Wno-pointer-sign', '-Wno-absolute-value',
             '-Wno-empty-body']

  def get_files(self):
    libc_files = []
    musl_srcdir = shared.path_from_root('system', 'lib', 'libc', 'musl', 'src')

    # musl modules
    blacklist = [
        'ipc', 'passwd', 'thread', 'signal', 'sched', 'ipc', 'time', 'linux',
        'aio', 'exit', 'legacy', 'mq', 'process', 'search', 'setjmp', 'env',
        'ldso', 'conf'
    ]

    # individual files
    blacklist += [
        'memcpy.c', 'memset.c', 'memmove.c', 'getaddrinfo.c', 'getnameinfo.c',
        'inet_addr.c', 'res_query.c', 'res_querydomain.c', 'gai_strerror.c',
        'proto.c', 'gethostbyaddr.c', 'gethostbyaddr_r.c', 'gethostbyname.c',
        'gethostbyname2_r.c', 'gethostbyname_r.c', 'gethostbyname2.c',
        'usleep.c', 'alarm.c', 'syscall.c', '_exit.c', 'popen.c',
        'getgrouplist.c', 'initgroups.c', 'wordexp.c', 'timer_create.c',
        'faccessat.c',
    ]

    # individual math files
    blacklist += [
        'abs.c', 'cos.c', 'cosf.c', 'cosl.c', 'sin.c', 'sinf.c', 'sinl.c',
        'tan.c', 'tanf.c', 'tanl.c', 'acos.c', 'acosf.c', 'acosl.c', 'asin.c',
        'asinf.c', 'asinl.c', 'atan.c', 'atanf.c', 'atanl.c', 'atan2.c',
        'atan2f.c', 'atan2l.c', 'exp.c', 'expf.c', 'expl.c', 'log.c', 'logf.c',
        'logl.c', 'sqrtl.c', 'round.c', 'roundf.c',
        'fabsl.c', 'ceill.c', 'floorl.c', 'pow.c', 'powf.c', 'powl.c',
    ]

    if self.is_asan:
      # With ASan, we need to use specialized implementations of certain libc
      # functions that do not rely on undefined behavior, for example, reading
      # multiple bytes at once as an int and overflowing a buffer.
      # Otherwise, ASan will catch these errors and terminate the program.
      blacklist += ['strcpy.c', 'memchr.c', 'strchrnul.c', 'strlen.c',
                    'aligned_alloc.c', 'fcntl.c']
      libc_files += [
        shared.path_from_root('system', 'lib', 'libc', 'emscripten_asan_strcpy.c'),
        shared.path_from_root('system', 'lib', 'libc', 'emscripten_asan_memchr.c'),
        shared.path_from_root('system', 'lib', 'libc', 'emscripten_asan_strchrnul.c'),
        shared.path_from_root('system', 'lib', 'libc', 'emscripten_asan_strlen.c'),
        shared.path_from_root('system', 'lib', 'libc', 'emscripten_asan_fcntl.c'),
      ]

    if shared.Settings.WASM_BACKEND:
      # With the wasm backend these are included in wasm_libc_rt instead
      blacklist += [os.path.basename(f) for f in get_wasm_libc_rt_files()]
    else:
      blacklist += ['rintf.c', 'ceil.c', 'ceilf.c', 'floor.c', 'floorf.c',
                    'fabs.c', 'fabsf.c', 'sqrt.c', 'sqrtf.c']

    blacklist = set(blacklist)
    # TODO: consider using more math code from musl, doing so makes box2d faster
    for dirpath, dirnames, filenames in os.walk(musl_srcdir):
      for f in filenames:
        if f.endswith('.c'):
          if f in blacklist:
            continue
          dir_parts = os.path.split(dirpath)
          cancel = False
          for part in dir_parts:
            if part in blacklist:
              cancel = True
              break
          if not cancel:
            libc_files.append(os.path.join(musl_srcdir, dirpath, f))

    return libc_files

  def get_depends(self):
    depends = super(libc, self).get_depends()
    if shared.Settings.WASM:
      return depends + ['libc-wasm']
    return depends


class libc_wasm(MuslInternalLibrary):
  name = 'libc-wasm'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libc-wasm.symbols'))

  cflags = ['-O2', '-fno-builtin']
  src_dir = ['system', 'lib', 'libc', 'musl', 'src', 'math']
  src_files = ['cos.c', 'cosf.c', 'cosl.c', 'sin.c', 'sinf.c', 'sinl.c',
               'tan.c', 'tanf.c', 'tanl.c', 'acos.c', 'acosf.c', 'acosl.c',
               'asin.c', 'asinf.c', 'asinl.c', 'atan.c', 'atanf.c', 'atanl.c',
               'atan2.c', 'atan2f.c', 'atan2l.c', 'exp.c', 'expf.c', 'expl.c',
               'log.c', 'logf.c', 'logl.c', 'pow.c', 'powf.c', 'powl.c']

  def can_use(self):
    # if building to wasm, we need more math code, since we have fewer builtins
    return shared.Settings.WASM


class libc_extras(MuslInternalLibrary):
  name = 'libc-extras'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libc-extras.symbols'))

  src_dir = ['system', 'lib', 'libc']
  src_files = ['extras.c']


class libcxxabi(CXXLibrary, MTLibrary):
  name = 'libc++abi'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcxxabi.symbols'))
  depends = ['libc']
  cflags = ['-std=c++11', '-Oz', '-D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS']

  def get_cflags(self):
    cflags = super(libcxxabi, self).get_cflags()
    cflags.append('-DNDEBUG')
    if not self.is_mt:
      cflags.append('-D_LIBCXXABI_HAS_NO_THREADS')
    return cflags

  src_dir = ['system', 'lib', 'libcxxabi', 'src']
  src_files = [
    'abort_message.cpp',
    'cxa_aux_runtime.cpp',
    'cxa_default_handlers.cpp',
    'cxa_demangle.cpp',
    'cxa_exception_storage.cpp',
    'cxa_guard.cpp',
    'cxa_handlers.cpp',
    'fallback_malloc.cpp',
    'stdlib_new_delete.cpp',
    'stdlib_exception.cpp',
    'stdlib_stdexcept.cpp',
    'stdlib_typeinfo.cpp',
    'private_typeinfo.cpp'
  ]


class libcxx(NoBCLibrary, CXXLibrary, NoExceptLibrary, MTLibrary):
  name = 'libc++'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcxx.symbols'))
  depends = ['libc++abi']

  cflags = ['-std=c++11', '-DLIBCXX_BUILDING_LIBCXXABI=1', '-D_LIBCPP_BUILDING_LIBRARY', '-Oz',
            '-D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS']

  src_dir = ['system', 'lib', 'libcxx']
  src_files = [
    'algorithm.cpp',
    'any.cpp',
    'bind.cpp',
    'chrono.cpp',
    'condition_variable.cpp',
    'debug.cpp',
    'exception.cpp',
    'future.cpp',
    'functional.cpp',
    'hash.cpp',
    'ios.cpp',
    'iostream.cpp',
    'locale.cpp',
    'memory.cpp',
    'mutex.cpp',
    'new.cpp',
    'optional.cpp',
    'random.cpp',
    'regex.cpp',
    'shared_mutex.cpp',
    'stdexcept.cpp',
    'string.cpp',
    'strstream.cpp',
    'system_error.cpp',
    'thread.cpp',
    'typeinfo.cpp',
    'utility.cpp',
    'valarray.cpp',
    'variant.cpp',
    'vector.cpp',
    os.path.join('experimental', 'memory_resource.cpp'),
    os.path.join('filesystem', 'directory_iterator.cpp'),
    os.path.join('filesystem', 'int128_builtins.cpp'),
    os.path.join('filesystem', 'operations.cpp')
  ]


class libmalloc(MTLibrary, NoBCLibrary):
  name = 'libmalloc'

  cflags = ['-O2', '-fno-builtin']

  def __init__(self, **kwargs):
    self.malloc = kwargs.pop('malloc')
    if self.malloc not in ('dlmalloc', 'emmalloc', 'none'):
      raise Exception('malloc must be one of "emmalloc", "dlmalloc" or "none", see settings.js')

    self.is_debug = kwargs.pop('is_debug')
    self.use_errno = kwargs.pop('use_errno')
    self.is_tracing = kwargs.pop('is_tracing')

    super(libmalloc, self).__init__(**kwargs)

    if self.malloc != 'dlmalloc':
      assert not self.is_mt
      assert not self.is_tracing

  def get_files(self):
    return [shared.path_from_root('system', 'lib', {
      'dlmalloc': 'dlmalloc.c', 'emmalloc': 'emmalloc.cpp'
    }[self.malloc])]

  def get_cflags(self):
    cflags = super(libmalloc, self).get_cflags()
    if self.is_debug:
      cflags += ['-UNDEBUG', '-DDLMALLOC_DEBUG']
      # TODO: consider adding -DEMMALLOC_DEBUG, but that is quite slow
    else:
      cflags += ['-DNDEBUG']
    if not self.use_errno:
      cflags += ['-DMALLOC_FAILURE_ACTION=']
    if self.is_tracing:
      cflags += ['--tracing']
    return cflags

  def get_base_name_prefix(self):
    return 'lib%s' % self.malloc

  def get_base_name(self):
    name = super(libmalloc, self).get_base_name()
    if self.is_debug:
      name += '-debug'
    if not self.use_errno:
      # emmalloc doesn't actually use errno, but it's easier to build it again
      name += '-noerrno'
    if self.is_tracing:
      name += '-tracing'
    return name

  def can_use(self):
    return shared.Settings.MALLOC != 'none'

  @classmethod
  def vary_on(cls):
    return super(libmalloc, cls).vary_on() + ['is_debug', 'use_errno', 'is_tracing']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(libmalloc, cls).get_default_variation(
      malloc=shared.Settings.MALLOC,
      is_debug=shared.Settings.DEBUG_LEVEL >= 3,
      use_errno=shared.Settings.SUPPORT_ERRNO,
      is_tracing=shared.Settings.EMSCRIPTEN_TRACING,
      **kwargs
    )

  @classmethod
  def variations(cls):
    combos = super(libmalloc, cls).variations()
    return ([dict(malloc='dlmalloc', **combo) for combo in combos] +
            [dict(malloc='emmalloc', **combo) for combo in combos
             if not combo['is_mt'] and not combo['is_tracing']])


class libal(Library):
  name = 'libal'
  depends = ['libc']
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'al.symbols'))

  cflags = ['-Os']
  src_dir = ['system', 'lib']
  src_files = ['al.c']


class libgl(MTLibrary):
  name = 'libgl'
  depends = ['libc']
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'gl.symbols'))

  src_dir = ['system', 'lib', 'gl']
  src_glob = '*.c'

  cflags = ['-Oz']

  def __init__(self, **kwargs):
    self.is_legacy = kwargs.pop('is_legacy')
    self.is_webgl2 = kwargs.pop('is_webgl2')
    self.is_ofb = kwargs.pop('is_ofb')
    super(libgl, self).__init__(**kwargs)

  def get_base_name(self):
    name = super(libgl, self).get_base_name()
    if self.is_legacy:
      name += '-emu'
    if self.is_webgl2:
      name += '-webgl2'
    if self.is_ofb:
      name += '-ofb'
    return name

  def get_cflags(self):
    cflags = super(libgl, self).get_cflags()
    if self.is_legacy:
      cflags += ['-DLEGACY_GL_EMULATION=1']
    if self.is_webgl2:
      cflags += ['-DUSE_WEBGL2=1', '-s', 'USE_WEBGL2=1']
    if self.is_ofb:
      cflags += ['-D__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__']
    return cflags

  @classmethod
  def vary_on(cls):
    return super(libgl, cls).vary_on() + ['is_legacy', 'is_webgl2', 'is_ofb']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(libgl, cls).get_default_variation(
      is_legacy=shared.Settings.LEGACY_GL_EMULATION,
      is_webgl2=shared.Settings.USE_WEBGL2,
      is_ofb=shared.Settings.OFFSCREEN_FRAMEBUFFER,
      **kwargs
    )


class libembind(CXXLibrary):
  name = 'libembind'
  cflags = ['-std=c++11']
  depends = ['libc++abi']
  never_force = True

  def __init__(self, **kwargs):
    self.with_rtti = kwargs.pop('with_rtti', False)
    super(libembind, self).__init__(**kwargs)

  def get_cflags(self):
    cflags = super(libembind, self).get_cflags()
    if not self.with_rtti:
      cflags += ['-fno-rtti', '-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0']
    return cflags

  @classmethod
  def vary_on(cls):
    return super(libembind, cls).vary_on() + ['with_rtti']

  def get_base_name(self):
    name = super(libembind, self).get_base_name()
    if self.with_rtti:
      name += '-rtti'
    return name

  def get_files(self):
    return [shared.path_from_root('system', 'lib', 'embind', 'bind.cpp')]

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super(libembind, cls).get_default_variation(with_rtti=shared.Settings.USE_RTTI, **kwargs)


class libfetch(CXXLibrary, MTLibrary):
  name = 'libfetch'
  depends = ['libc++abi']
  never_force = True

  def get_files(self):
    return [shared.path_from_root('system', 'lib', 'fetch', 'emscripten_fetch.cpp')]


class libasmfs(CXXLibrary, MTLibrary):
  name = 'libasmfs'
  depends = ['libc++abi']
  never_force = True

  def get_files(self):
    return [shared.path_from_root('system', 'lib', 'fetch', 'asmfs.cpp')]


class libhtml5(Library):
  name = 'libhtml5'
  symbols = read_symbols(shared.path_from_root('system', 'lib', 'html5.symbols'))

  cflags = ['-Oz']
  src_dir = ['system', 'lib', 'html5']
  src_glob = '*.c'


class libpthreads(AsanInstrumentedLibrary, MuslInternalLibrary, MTLibrary):
  name = 'libpthreads'
  depends = ['libc']
  cflags = ['-O2']

  def get_files(self):
    if self.is_mt:
      files = files_in_path(
        path_components=['system', 'lib', 'libc', 'musl', 'src', 'thread'],
        filenames=[
          'pthread_attr_destroy.c', 'pthread_condattr_setpshared.c',
          'pthread_mutex_lock.c', 'pthread_spin_destroy.c', 'pthread_attr_get.c',
          'pthread_cond_broadcast.c', 'pthread_mutex_setprioceiling.c',
          'pthread_spin_init.c', 'pthread_attr_init.c', 'pthread_cond_destroy.c',
          'pthread_mutex_timedlock.c', 'pthread_spin_lock.c',
          'pthread_attr_setdetachstate.c', 'pthread_cond_init.c',
          'pthread_mutex_trylock.c', 'pthread_spin_trylock.c',
          'pthread_attr_setguardsize.c', 'pthread_cond_signal.c',
          'pthread_mutex_unlock.c', 'pthread_spin_unlock.c',
          'pthread_attr_setinheritsched.c', 'pthread_cond_timedwait.c',
          'pthread_once.c', 'sem_destroy.c', 'pthread_attr_setschedparam.c',
          'pthread_cond_wait.c', 'pthread_rwlockattr_destroy.c', 'sem_getvalue.c',
          'pthread_attr_setschedpolicy.c', 'pthread_equal.c', 'pthread_rwlockattr_init.c',
          'sem_init.c', 'pthread_attr_setscope.c', 'pthread_getspecific.c',
          'pthread_rwlockattr_setpshared.c', 'sem_open.c', 'pthread_attr_setstack.c',
          'pthread_key_create.c', 'pthread_rwlock_destroy.c', 'sem_post.c',
          'pthread_attr_setstacksize.c', 'pthread_mutexattr_destroy.c',
          'pthread_rwlock_init.c', 'sem_timedwait.c', 'pthread_barrierattr_destroy.c',
          'pthread_mutexattr_init.c', 'pthread_rwlock_rdlock.c', 'sem_trywait.c',
          'pthread_barrierattr_init.c', 'pthread_mutexattr_setprotocol.c',
          'pthread_rwlock_timedrdlock.c', 'sem_unlink.c',
          'pthread_barrierattr_setpshared.c', 'pthread_mutexattr_setpshared.c',
          'pthread_rwlock_timedwrlock.c', 'sem_wait.c', 'pthread_barrier_destroy.c',
          'pthread_mutexattr_setrobust.c', 'pthread_rwlock_tryrdlock.c',
          '__timedwait.c', 'pthread_barrier_init.c', 'pthread_mutexattr_settype.c',
          'pthread_rwlock_trywrlock.c', 'vmlock.c', 'pthread_barrier_wait.c',
          'pthread_mutex_consistent.c', 'pthread_rwlock_unlock.c', '__wait.c',
          'pthread_condattr_destroy.c', 'pthread_mutex_destroy.c',
          'pthread_rwlock_wrlock.c', 'pthread_condattr_init.c',
          'pthread_mutex_getprioceiling.c', 'pthread_setcanceltype.c',
          'pthread_condattr_setclock.c', 'pthread_mutex_init.c',
          'pthread_setspecific.c', 'pthread_setcancelstate.c'
        ])
      files += [shared.path_from_root('system', 'lib', 'pthread', 'library_pthread.c')]
      if shared.Settings.WASM_BACKEND:
        files += [shared.path_from_root('system', 'lib', 'pthread', 'library_pthread_wasm.c')]
      else:
        files += [shared.path_from_root('system', 'lib', 'pthread', 'library_pthread_asmjs.c')]
      return files
    else:
      return [shared.path_from_root('system', 'lib', 'pthread', 'library_pthread_stub.c')]

  def get_base_name_prefix(self):
    return 'libpthreads' if self.is_mt else 'libpthreads_stub'

  pthreads_symbols = read_symbols(shared.path_from_root('system', 'lib', 'pthreads.symbols'))
  asmjs_pthreads_symbols = read_symbols(shared.path_from_root('system', 'lib', 'pthreads_asmjs.symbols'))
  pthreads_stub_symbols = read_symbols(shared.path_from_root('system', 'lib', 'pthreads_stub.symbols'))

  def get_symbols(self):
    symbols = self.pthreads_symbols if self.is_mt else self.pthreads_stub_symbols
    if self.is_mt and not shared.Settings.WASM_BACKEND:
      symbols += self.asmjs_pthreads_symbols
    return symbols


class CompilerRTWasmLibrary(NoBCLibrary):
  cflags = ['-O2', '-fno-builtin']
  force_object_files = True

  def can_build(self):
    return shared.Settings.WASM_BACKEND


class libcompiler_rt_wasm(CompilerRTWasmLibrary):
  name = 'libcompiler_rt_wasm'

  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'builtins']
  src_files = ['addtf3.c', 'ashlti3.c', 'ashrti3.c', 'atomic.c', 'comparetf2.c',
               'divtf3.c', 'divti3.c', 'udivmodti4.c',
               'extenddftf2.c', 'extendsftf2.c',
               'fixdfti.c', 'fixsfti.c', 'fixtfdi.c', 'fixtfsi.c', 'fixtfti.c',
               'fixunsdfti.c', 'fixunssfti.c', 'fixunstfdi.c', 'fixunstfsi.c', 'fixunstfti.c',
               'floatditf.c', 'floatsitf.c', 'floattidf.c', 'floattisf.c',
               'floatunditf.c', 'floatunsitf.c', 'floatuntidf.c', 'floatuntisf.c', 'lshrti3.c',
               'modti3.c', 'multc3.c', 'multf3.c', 'multi3.c', 'subtf3.c', 'udivti3.c', 'umodti3.c', 'ashrdi3.c',
               'ashldi3.c', 'fixdfdi.c', 'floatdidf.c', 'lshrdi3.c', 'moddi3.c',
               'trunctfdf2.c', 'trunctfsf2.c', 'umoddi3.c', 'fixunsdfdi.c', 'muldi3.c',
               'divdi3.c', 'divmoddi4.c', 'udivdi3.c', 'udivmoddi4.c']

  def get_files(self):
    return super(libcompiler_rt_wasm, self).get_files() + [shared.path_from_root('system', 'lib', 'compiler-rt', 'extras.c')]


class libc_rt_wasm(AsanInstrumentedLibrary, CompilerRTWasmLibrary, MuslInternalLibrary):
  name = 'libc_rt_wasm'

  def get_files(self):
    return get_wasm_libc_rt_files()


class libubsan_minimal_rt_wasm(CompilerRTWasmLibrary, MTLibrary):
  name = 'libubsan_minimal_rt_wasm'
  never_force = True

  includes = [['system', 'lib', 'compiler-rt', 'lib']]
  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'ubsan_minimal']
  src_files = ['ubsan_minimal_handlers.cpp']


class libsanitizer_common_rt_wasm(CompilerRTWasmLibrary, MTLibrary):
  name = 'libsanitizer_common_rt_wasm'
  depends = ['libc++abi']
  js_depends = ['memalign', 'emscripten_builtin_memalign', '__data_end', '__heap_base']
  never_force = True

  cflags = ['-std=c++11']
  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'sanitizer_common']
  src_glob = '*.cc'
  src_glob_exclude = ['sanitizer_common_nolibc.cc']


class SanitizerLibrary(CompilerRTWasmLibrary, MTLibrary):
  depends = ['libsanitizer_common_rt_wasm']
  never_force = True

  includes = [['system', 'lib', 'compiler-rt', 'lib']]
  cflags = ['-std=c++11']
  src_glob = '*.cc'


class libubsan_rt_wasm(SanitizerLibrary):
  name = 'libubsan_rt_wasm'
  js_depends = ['emscripten_builtin_malloc', 'emscripten_builtin_free']

  cflags = ['-DUBSAN_CAN_USE_CXXABI']
  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'ubsan']


class liblsan_common_rt_wasm(SanitizerLibrary):
  name = 'liblsan_common_rt_wasm'
  js_depends = ['__global_base']

  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'lsan']
  src_glob = 'lsan_common*.cc'


class liblsan_rt_wasm(SanitizerLibrary):
  name = 'liblsan_rt_wasm'
  depends = ['liblsan_common_rt_wasm']
  js_depends = ['emscripten_builtin_malloc', 'emscripten_builtin_free']

  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'lsan']
  src_glob_exclude = ['lsan_common.cc', 'lsan_common_mac.cc', 'lsan_common_linux.cc',
                      'lsan_common_emscripten.cc']


class libasan_rt_wasm(SanitizerLibrary):
  name = 'libasan_rt_wasm'
  depends = ['liblsan_common_rt_wasm', 'libubsan_rt_wasm']

  src_dir = ['system', 'lib', 'compiler-rt', 'lib', 'asan']


# If main() is not in EXPORTED_FUNCTIONS, it may be dce'd out. This can be
# confusing, so issue a warning.
def warn_on_unexported_main(symbolses):
  if '_main' not in shared.Settings.EXPORTED_FUNCTIONS:
    for symbols in symbolses:
      if 'main' in symbols.defs:
        logger.warning('main() is in the input files, but "_main" is not in EXPORTED_FUNCTIONS, which means it may be eliminated as dead code. Export it if you want main() to run.')
        return


def calculate(temp_files, in_temp, stdout_, stderr_, forced=[]):
  global stdout, stderr
  stdout = stdout_
  stderr = stderr_

  # Set of libraries to include on the link line, as opposed to `force` which
  # is the set of libraries to force include (with --whole-archive).
  always_include = set()

  # Setting this will only use the forced libs in EMCC_FORCE_STDLIBS. This avoids spending time checking
  # for unresolved symbols in your project files, which can speed up linking, but if you do not have
  # the proper list of actually needed libraries, errors can occur. See below for how we must
  # export all the symbols in deps_info when using this option.
  only_forced = os.environ.get('EMCC_ONLY_FORCED_STDLIBS')
  if only_forced:
    temp_files = []

  # Add in some hacks for js libraries. If a js lib depends on a symbol provided by a C library, it must be
  # added to here, because our deps go only one way (each library here is checked, then we check the next
  # in order - libc++, libcxextra, etc. - and then we run the JS compiler and provide extra symbols from
  # library*.js files. But we cannot then go back to the C libraries if a new dep was added!
  # TODO: Move all __deps from src/library*.js to deps_info.json, and use that single source of info
  #       both here and in the JS compiler.
  deps_info = json.loads(open(shared.path_from_root('src', 'deps_info.json')).read())
  added = set()

  def add_back_deps(need):
    more = False
    for ident, deps in deps_info.items():
      if ident in need.undefs and ident not in added:
        added.add(ident)
        more = True
        for dep in deps:
          need.undefs.add(dep)
          if shared.Settings.VERBOSE:
            logger.debug('adding dependency on %s due to deps-info on %s' % (dep, ident))
          shared.Settings.EXPORTED_FUNCTIONS.append(mangle_c_symbol_name(dep))
    if more:
      add_back_deps(need) # recurse to get deps of deps

  # Scan symbols
  symbolses = shared.Building.parallel_llvm_nm([os.path.abspath(t) for t in temp_files])

  warn_on_unexported_main(symbolses)

  if len(symbolses) == 0:
    class Dummy(object):
      defs = set()
      undefs = set()
    symbolses.append(Dummy())

  # depend on exported functions
  for export in shared.Settings.EXPORTED_FUNCTIONS:
    if shared.Settings.VERBOSE:
      logger.debug('adding dependency on export %s' % export)
    symbolses[0].undefs.add(demangle_c_symbol_name(export))

  for symbols in symbolses:
    add_back_deps(symbols)

  # If we are only doing forced stdlibs, then we don't know the actual symbols we need,
  # and must assume all of deps_info must be exported. Note that this might cause
  # warnings on exports that do not exist.
  if only_forced:
    for key, value in deps_info.items():
      for dep in value:
        shared.Settings.EXPORTED_FUNCTIONS.append(mangle_c_symbol_name(dep))

  always_include.add('libpthreads')
  if shared.Settings.MALLOC != 'none':
    always_include.add('libmalloc')
  if shared.Settings.WASM_BACKEND:
    always_include.add('libcompiler_rt')

  libs_to_link = []
  already_included = set()
  system_libs_map = Library.get_usable_variations()
  system_libs = sorted(system_libs_map.values(), key=lambda lib: lib.name)

  # Setting this in the environment will avoid checking dependencies and make
  # building big projects a little faster 1 means include everything; otherwise
  # it can be the name of a lib (libc++, etc.).
  # You can provide 1 to include everything, or a comma-separated list with the
  # ones you want
  force = os.environ.get('EMCC_FORCE_STDLIBS')
  if force == '1':
    force = ','.join(name for name, lib in system_libs_map.items() if not lib.never_force)
  force_include = set((force.split(',') if force else []) + forced)
  if force_include:
    logger.debug('forcing stdlibs: ' + str(force_include))

  for lib in always_include:
    assert lib in system_libs_map

  for lib in force_include:
    if lib not in system_libs_map:
      shared.exit_with_error('invalid forced library: %s', lib)

  def add_library(lib):
    if lib.name in already_included:
      return
    already_included.add(lib.name)

    logger.debug('including %s (%s)' % (lib.name, lib.get_name()))

    need_whole_archive = lib.name in force_include and lib.get_ext() == '.a'
    libs_to_link.append((lib.get_path(), need_whole_archive))

    # Recursively add dependencies
    for d in lib.get_depends():
      add_library(system_libs_map[d])

    for d in lib.js_depends:
      d = '_' + d
      if d not in shared.Settings.EXPORTED_FUNCTIONS:
        shared.Settings.EXPORTED_FUNCTIONS.append(d)

  # Go over libraries to figure out which we must include
  for lib in system_libs:
    if lib.name in already_included:
      continue
    force_this = lib.name in force_include
    if not force_this and only_forced:
      continue
    include_this = force_this or lib.name in always_include

    if not include_this:
      need_syms = set()
      has_syms = set()
      for symbols in symbolses:
        if shared.Settings.VERBOSE:
          logger.debug('undefs: ' + str(symbols.undefs))
        for library_symbol in lib.get_symbols():
          if library_symbol in symbols.undefs:
            need_syms.add(library_symbol)
          if library_symbol in symbols.defs:
            has_syms.add(library_symbol)
      for haz in has_syms:
        if haz in need_syms:
          # remove symbols that are supplied by another of the inputs
          need_syms.remove(haz)
      if shared.Settings.VERBOSE:
        logger.debug('considering %s: we need %s and have %s' % (lib.name, str(need_syms), str(has_syms)))
      if not len(need_syms):
        continue

    # We need to build and link the library in
    add_library(lib)

  if shared.Settings.WASM_BACKEND:
    add_library(system_libs_map['libcompiler_rt_wasm'])
    add_library(system_libs_map['libc_rt_wasm'])

  if shared.Settings.UBSAN_RUNTIME == 1:
    add_library(system_libs_map['libubsan_minimal_rt_wasm'])
  elif shared.Settings.UBSAN_RUNTIME == 2:
    add_library(system_libs_map['libubsan_rt_wasm'])

  if shared.Settings.USE_LSAN:
    force_include.add('liblsan_rt_wasm')
    add_library(system_libs_map['liblsan_rt_wasm'])

  if shared.Settings.USE_ASAN:
    force_include.add('libasan_rt_wasm')
    add_library(system_libs_map['libasan_rt_wasm'])

  libs_to_link.sort(key=lambda x: x[0].endswith('.a')) # make sure to put .a files at the end.

  # libc++abi and libc++ *static* linking is tricky. e.g. cxa_demangle.cpp disables c++
  # exceptions, but since the string methods in the headers are *weakly* linked, then
  # we might have exception-supporting versions of them from elsewhere, and if libc++abi
  # is first then it would "win", breaking exception throwing from those string
  # header methods. To avoid that, we link libc++abi last.
  libs_to_link.sort(key=lambda x: x[0].endswith('libc++abi.bc'))

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


class Ports(object):
  """emscripten-ports library management (https://github.com/emscripten-ports).
  """

  @staticmethod
  def get_lib_name(name):
    return shared.static_library_name(name)

  @staticmethod
  def build_port(src_path, output_path, includes=[], flags=[], exclude_files=[], exclude_dirs=[]):
    srcs = []
    for root, dirs, files in os.walk(src_path, topdown=False):
      if any((excluded in root) for excluded in exclude_dirs):
        continue
      for f in files:
        ext = os.path.splitext(f)[1]
        if ext in ('.c', '.cpp') and not any((excluded in f) for excluded in exclude_files):
            srcs.append(os.path.join(root, f))
    include_commands = ['-I' + src_path]
    for include in includes:
      include_commands.append('-I' + include)

    commands = []
    objects = []
    for src in srcs:
      obj = src + '.o'
      commands.append([shared.PYTHON, shared.EMCC, '-c', src, '-O2', '-o', obj, '-w'] + include_commands + flags)
      objects.append(obj)

    run_commands(commands)
    print('create_lib', output_path)
    create_lib(output_path, objects)
    return output_path

  @staticmethod
  def run_commands(commands): # make easily available for port objects
    run_commands(commands)

  @staticmethod
  def create_lib(libname, inputs): # make easily available for port objects
    create_lib(libname, inputs)

  @staticmethod
  def get_dir():
    dirname = os.environ.get('EM_PORTS') or os.path.expanduser(os.path.join('~', '.emscripten_ports'))
    shared.safe_ensure_dirs(dirname)
    return dirname

  @staticmethod
  def erase():
    dirname = Ports.get_dir()
    shared.try_delete(dirname)
    if os.path.exists(dirname):
      logger.warning('could not delete ports dir %s - try to delete it manually' % dirname)

  @staticmethod
  def get_build_dir():
    return shared.Cache.get_path('ports-builds')

  name_cache = set()

  @staticmethod
  def fetch_project(name, url, subdir, is_tarbz2=False, sha512hash=None):
    # To compute the sha512 hash, run `curl URL | sha512sum`.
    fullname = os.path.join(Ports.get_dir(), name)

    # if EMCC_LOCAL_PORTS is set, we use a local directory as our ports. This is useful
    # for testing. This env var should be in format
    #     name=dir,name=dir
    # e.g.
    #     sdl2=/home/username/dev/ports/SDL2
    # so you could run
    #     EMCC_LOCAL_PORTS="sdl2=/home/alon/Dev/ports/SDL2" ./tests/runner.py browser.test_sdl2_mouse
    # this will simply copy that directory into the ports directory for sdl2, and use that. It also
    # clears the build, so that it is rebuilt from that source.
    local_ports = os.environ.get('EMCC_LOCAL_PORTS')
    if local_ports:
      shared.Cache.acquire_cache_lock()
      logger.warning('using local ports: %s' % local_ports)
      local_ports = [pair.split('=', 1) for pair in local_ports.split(',')]
      try:
        for local in local_ports:
          if name == local[0]:
            path = local[1]
            if name not in ports.ports_by_name:
              shared.exit_with_error('%s is not a known port' % name)
            port = ports.ports_by_name[name]
            if not hasattr(port, 'SUBDIR'):
              logger.error('port %s lacks .SUBDIR attribute, which we need in order to override it locally, please update it' % name)
              sys.exit(1)
            subdir = port.SUBDIR
            logger.warning('grabbing local port: ' + name + ' from ' + path + ' to ' + fullname + ' (subdir: ' + subdir + ')')
            shared.try_delete(fullname)
            shutil.copytree(path, os.path.join(fullname, subdir))
            Ports.clear_project_build(name)
      finally:
        shared.Cache.release_cache_lock()
      return

    if is_tarbz2:
      fullpath = fullname + '.tar.bz2'
    elif url.endswith('.tar.gz'):
      fullpath = fullname + '.tar.gz'
    else:
      fullpath = fullname + '.zip'

    if name not in Ports.name_cache: # only mention each port once in log
      logger.debug('including port: ' + name)
      logger.debug('    (at ' + fullname + ')')
      Ports.name_cache.add(name)

    class State(object):
      retrieved = False
      unpacked = False

    def retrieve():
      # retrieve from remote server
      logger.warning('retrieving port: ' + name + ' from ' + url)
      try:
        import requests
        response = requests.get(url)
        data = response.content
      except ImportError:
        try:
          from urllib.request import urlopen
          f = urlopen(url)
          data = f.read()
        except ImportError:
          # Python 2 compatibility
          from urllib2 import urlopen
          f = urlopen(url)
          data = f.read()

      if sha512hash:
        actual_hash = hashlib.sha512(data).hexdigest()
        if actual_hash != sha512hash:
          raise RuntimeError('Unexpected hash: ' + actual_hash + '\n'
                             'If you are updating the port, please update the hash in the port module.')
      open(fullpath, 'wb').write(data)
      State.retrieved = True

    def check_tag():
      if is_tarbz2:
        names = tarfile.open(fullpath, 'r:bz2').getnames()
      elif url.endswith('.tar.gz'):
        names = tarfile.open(fullpath, 'r:gz').getnames()
      else:
        names = zipfile.ZipFile(fullpath, 'r').namelist()

      # check if first entry of the archive is prefixed with the same
      # tag as we need so no longer download and recompile if so
      return bool(re.match(subdir + r'(\\|/|$)', names[0]))

    def unpack():
      logger.warning('unpacking port: ' + name)
      shared.safe_ensure_dirs(fullname)

      # TODO: Someday when we are using Python 3, we might want to change the
      # code below to use shlib.unpack_archive
      # e.g.: shutil.unpack_archive(filename=fullpath, extract_dir=fullname)
      # (https://docs.python.org/3/library/shutil.html#shutil.unpack_archive)
      if is_tarbz2:
        z = tarfile.open(fullpath, 'r:bz2')
      elif url.endswith('.tar.gz'):
        z = tarfile.open(fullpath, 'r:gz')
      else:
        z = zipfile.ZipFile(fullpath, 'r')
      try:
        cwd = os.getcwd()
        os.chdir(fullname)
        z.extractall()
      finally:
        os.chdir(cwd)

      State.unpacked = True

    # main logic. do this under a cache lock, since we don't want multiple jobs to
    # retrieve the same port at once

    shared.Cache.acquire_cache_lock()
    try:
      if not os.path.exists(fullpath):
        retrieve()

      if not os.path.exists(fullname):
        unpack()

      if not check_tag():
        logger.warning('local copy of port is not correct, retrieving from remote server')
        shared.try_delete(fullname)
        shared.try_delete(fullpath)
        retrieve()
        unpack()

      if State.unpacked:
        # we unpacked a new version, clear the build in the cache
        Ports.clear_project_build(name)
    finally:
      shared.Cache.release_cache_lock()

  @staticmethod
  def clear_project_build(name):
    port = ports.ports_by_name[name]
    port.clear(Ports, shared)
    shared.try_delete(os.path.join(Ports.get_build_dir(), name))

  @staticmethod
  def build_native(subdir):
    shared.Building.ensure_no_emmake('We cannot build the native system library in "%s" when under the influence of emmake/emconfigure. To avoid this, create system dirs beforehand, so they are not auto-built on demand. For example, for binaryen, do "python embuilder.py build binaryen"' % subdir)

    old = os.getcwd()

    try:
      os.chdir(subdir)

      cmake_build_type = 'Release'

      # Configure
      check_call(['cmake', '-DCMAKE_BUILD_TYPE=' + cmake_build_type, '.'])

      # Check which CMake generator CMake used so we know which form to pass parameters to make/msbuild/etc. build tool.
      generator = re.search('CMAKE_GENERATOR:INTERNAL=(.*)$', open('CMakeCache.txt', 'r').read(), re.MULTILINE).group(1)

      # Make variants support '-jX' for number of cores to build, MSBuild does /maxcpucount:X
      num_cores = str(shared.Building.get_num_cores())
      make_args = []
      if 'Makefiles' in generator and 'NMake' not in generator:
        make_args = ['--', '-j', num_cores]
      elif 'Visual Studio' in generator:
        make_args = ['--config', cmake_build_type, '--', '/maxcpucount:' + num_cores]

      # Kick off the build.
      check_call(['cmake', '--build', '.'] + make_args)
    finally:
      os.chdir(old)


# get all ports
def get_ports(settings):
  ret = []

  try:
    process_dependencies(settings)
    for port in ports.ports:
      # ports return their output files, which will be linked, or a txt file
      ret += [f for f in port.get(Ports, settings, shared) if not f.endswith('.txt')]
  except Exception:
    logger.error('a problem occurred when using an emscripten-ports library.  try to run `emcc --clear-ports` and then run this command again')
    raise

  ret.reverse()
  return ret


def process_dependencies(settings):
  for port in reversed(ports.ports):
    if hasattr(port, "process_dependencies"):
      port.process_dependencies(settings)


def process_args(args, settings):
  process_dependencies(settings)
  for port in ports.ports:
    args = port.process_args(Ports, args, settings, shared)
  return args


# get a single port
def get_port(name, settings):
  port = ports.ports_by_name[name]
  if hasattr(port, "process_dependencies"):
    port.process_dependencies(settings)
  # ports return their output files, which will be linked, or a txt file
  return [f for f in port.get(Ports, settings, shared) if not f.endswith('.txt')]


def show_ports():
  print('Available ports:')
  for port in ports.ports:
    print('   ', port.show())
