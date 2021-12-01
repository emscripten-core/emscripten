# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from .toolchain_profiler import ToolchainProfiler

import itertools
import logging
import os
import shutil
import textwrap
from enum import IntEnum, auto
from glob import iglob

from . import shared, building, utils
from . import deps_info, tempfiles
from . import diagnostics
from tools.shared import demangle_c_symbol_name
from tools.settings import settings

logger = logging.getLogger('system_libs')

# Files that are part of libsockets.a and so should be excluded from libc.a
LIBC_SOCKETS = ['socket.c', 'socketpair.c', 'shutdown.c', 'bind.c', 'connect.c',
                'listen.c', 'accept.c', 'getsockname.c', 'getpeername.c', 'send.c',
                'recv.c', 'sendto.c', 'recvfrom.c', 'sendmsg.c', 'recvmsg.c',
                'getsockopt.c', 'setsockopt.c', 'freeaddrinfo.c',
                'in6addr_any.c', 'in6addr_loopback.c']


def files_in_path(path, filenames):
  srcdir = utils.path_from_root(path)
  return [os.path.join(srcdir, f) for f in filenames]


def glob_in_path(path, glob_pattern, excludes=()):
  srcdir = utils.path_from_root(path)
  files = iglob(os.path.join(srcdir, glob_pattern), recursive=True)
  return sorted(f for f in files if os.path.basename(f) not in excludes)


def get_base_cflags(force_object_files=False):
  # Always build system libraries with debug information.  Non-debug builds
  # will ignore this at link time because we link with `-strip-debug`.
  flags = ['-g']
  if settings.LTO and not force_object_files:
    flags += ['-flto=' + settings.LTO]
  if settings.RELOCATABLE:
    flags += ['-sRELOCATABLE']
  if settings.MEMORY64:
    flags += ['-sMEMORY64=' + str(settings.MEMORY64)]
  return flags


def clean_env():
  # building system libraries and ports should be hermetic in that it is not
  # affected by things like EMCC_CFLAGS which the user may have set.
  # At least one port also uses autoconf (harfbuzz) so we also need to clear
  # CFLAGS/LDFLAGS which we don't want to effect the inner call to configure.
  safe_env = os.environ.copy()
  for opt in ['CFLAGS', 'CXXFLAGS', 'LDFLAGS',
              'EMCC_CFLAGS',
              'EMCC_FORCE_STDLIBS',
              'EMCC_ONLY_FORCED_STDLIBS']:
    if opt in safe_env:
      del safe_env[opt]
  return safe_env


def run_build_commands(commands):
  # Before running a set of build commands make sure the common sysroot
  # headers are installed.  This prevents each sub-process from attempting
  # to setup the sysroot itself.
  ensure_sysroot()
  shared.run_multiple_processes(commands, env=clean_env())


def create_lib(libname, inputs):
  """Create a library from a set of input objects."""
  suffix = shared.suffix(libname)
  inputs = sorted(inputs, key=lambda x: os.path.basename(x))
  if suffix in ('.bc', '.o'):
    if len(inputs) == 1:
      if inputs[0] != libname:
        shutil.copyfile(inputs[0], libname)
    else:
      building.link_to_object(inputs, libname)
  else:
    assert suffix == '.a'
    building.emar('cr', libname, inputs)


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
  name = None

  # Set to true to prevent EMCC_FORCE_STDLIBS from linking this library.
  never_force = False

  # A list of flags to pass to emcc.
  # The flags for the parent class is automatically inherited.
  # TODO: Investigate whether perf gains from loop unrolling would be worth the
  # extra code size. The -fno-unroll-loops flags was added here when loop
  # unrolling landed upstream in LLVM to avoid changing behavior but was not
  # specifically evaluated.
  cflags = ['-O2', '-Werror', '-fno-unroll-loops']

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
    shared.Cache.erase_file(shared.Cache.get_lib_name(self.get_filename()))

  def get_path(self):
    """
    Gets the cached path of this library.

    This will trigger a build if this library is not in the cache.
    """
    return shared.Cache.get_lib(self.get_filename(), self.build)

  def get_link_flag(self):
    """
    Gets the link flags needed to use the library.

    This will trigger a build if this library is not in the cache.
    """
    fullpath = self.get_path()
    # For non-libaries (e.g. crt1.o) we pass the entire path to the linker
    if self.get_ext() != '.a':
      return fullpath
    # For libraries (.a) files, we pass the abbreviated `-l` form.
    base = shared.unsuffixed_basename(fullpath)
    return '-l' + shared.strip_prefix(base, 'lib')

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

  def build_objects(self, build_dir):
    """
    Returns a list of compiled object files for this library.

    By default, this builds all the source files returned by `self.get_files()`,
    with the `cflags` returned by `self.get_cflags()`.
    """
    commands = []
    objects = []
    cflags = self.get_cflags()
    base_flags = get_base_cflags()
    case_insensitive = is_case_insensitive(build_dir)
    for src in self.get_files():
      object_basename = shared.unsuffixed_basename(src)
      # Resolve duplicates by appending unique.
      # This is needed on case insensitve filesystem to handle,
      # for example, _exit.o and _Exit.o.
      if case_insensitive:
        object_basename = object_basename.lower()
      o = os.path.join(build_dir, object_basename + '.o')
      object_uuid = 0
      # Find a unique basename
      while o in objects:
        object_uuid += 1
        o = os.path.join(build_dir, f'{object_basename}__{object_uuid}.o')
      ext = shared.suffix(src)
      if ext in ('.s', '.S', '.c'):
        cmd = [shared.EMCC]
      else:
        cmd = [shared.EMXX]
      if ext in ('.s', '.S'):
        cmd += base_flags
        # TODO(sbc) There is an llvm bug that causes a crash when `-g` is used with
        # assembly files that define wasm globals.
        cmd.remove('-g')
      else:
        cmd += cflags
      cmd = self.customize_build_cmd(cmd, src)
      commands.append(cmd + ['-c', src, '-o', o])
      objects.append(o)
    run_build_commands(commands)
    return objects

  def customize_build_cmd(self, cmd, filename):  # noqa
    """Allows libraries to customize the build command used on per-file basis.

    For example, libc uses this to replace -Oz with -O2 for some subset of files."""
    return cmd

  def build(self, out_filename):
    """Builds the library and returns the path to the file."""
    build_dir = shared.Cache.get_path(os.path.join('build', self.get_base_name()))
    utils.safe_ensure_dirs(build_dir)
    create_lib(out_filename, self.build_objects(build_dir))
    if not shared.DEBUG:
      tempfiles.try_delete(build_dir)

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
    super().__init__(**kwargs)

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_mt:
      cflags += ['-sUSE_PTHREADS']
    return cflags

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_mt:
      name += '-mt'
    return name

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_mt']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(is_mt=settings.USE_PTHREADS, **kwargs)


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


class OptimizedAggressivelyForSizeLibrary(Library):
  def __init__(self, **kwargs):
    self.is_optz = kwargs.pop('is_optz')
    super().__init__(**kwargs)

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_optz:
      name += '-optz'
    return name

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_optz:
      cflags += ['-DEMSCRIPTEN_OPTIMIZE_FOR_OZ']
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_optz']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(is_optz=settings.SHRINK_LEVEL >= 2, **kwargs)


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
    if settings.EXCEPTION_HANDLING:
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
                 '-sDISABLE_EXCEPTION_THROWING=1',
                 '-D__USING_WASM_SJLJ__']
    else:
      cflags += ['-sSUPPORT_LONGJMP=emscripten']
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

  cflags = ['-fno-builtin']
  src_dir = 'system/lib/compiler-rt/lib/builtins'
  # gcc_personality_v0.c depends on libunwind, which don't include by default.
  src_files = glob_in_path(src_dir, '*.c', excludes=['gcc_personality_v0.c'])
  src_files += files_in_path(
      path='system/lib/compiler-rt',
      filenames=[
        'stack_ops.S',
        'stack_limits.S',
        'emscripten_setjmp.c',
        'emscripten_exception_builtins.c'
      ])


class libnoexit(Library):
  name = 'libnoexit'
  src_dir = 'system/lib/libc'
  src_files = ['atexit_dummy.c']


class libc(MuslInternalLibrary,
           DebugLibrary,
           OptimizedAggressivelyForSizeLibrary,
           AsanInstrumentedLibrary,
           MTLibrary):
  name = 'libc'

  # Without -fno-builtin, LLVM can optimize away or convert calls to library
  # functions to something else based on assumptions that they behave exactly
  # like the standard library. This can cause unexpected bugs when we use our
  # custom standard library. The same for other libc/libm builds.
  cflags = ['-Os', '-fno-builtin']

  # Disable certain warnings for code patterns that are contained in upstream musl
  cflags += ['-Wno-ignored-attributes',
             # tre.h defines NDEBUG internally itself
             '-Wno-macro-redefined',
             '-Wno-shift-op-parentheses',
             '-Wno-string-plus-int',
             '-Wno-pointer-sign']

  def __init__(self, **kwargs):
    self.non_lto_files = self.get_non_lto_files()
    super().__init__(**kwargs)

  def get_non_lto_files(self):
    # Combining static linking with LTO is tricky under LLVM.  The codegen that
    # happens during LTO can generate references to new symbols that didn't exist
    # in the linker inputs themselves.
    # These symbols are called libcalls in LLVM and are the result of intrinsics
    # and builtins at the LLVM level.  These libcalls cannot themselves be part
    # of LTO because once the linker is running the LTO phase new bitcode objects
    # cannot be added to link.  Another way of putting it: by the time LTO happens
    # the decision about which bitcode symbols to compile has already been made.
    # See: https://bugs.llvm.org/show_bug.cgi?id=44353.
    # To solve this we put all such libcalls in a separate library that, like
    # compiler-rt, is never compiled as LTO/bitcode (see force_object_files in
    # CompilerRTLibrary).
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
      'pow_small.c', 'log_small.c', 'log2_small.c',
      'pow.c', 'pow_data.c', 'log.c', 'log_data.c', 'log2.c', 'log2_data.c'
    ]
    math_files = files_in_path(path='system/lib/libc/musl/src/math', filenames=math_files)

    other_files = files_in_path(
      path='system/lib/libc',
      filenames=['emscripten_memcpy.c', 'emscripten_memset.c',
                 'emscripten_scan_stack.c',
                 'emscripten_memmove.c'])
    # Calls to iprintf can be generated during codegen. Ideally we wouldn't
    # compile these with -O2 like we do the rest of compiler-rt since its
    # probably not performance sensitive.  However we don't currently have
    # a way to set per-file compiler flags.  And hopefully we should be able
    # move all this stuff back into libc once we it LTO compatible.
    iprintf_files = files_in_path(
      path='system/lib/libc/musl/src/stdio',
      filenames=['__towrite.c', '__overflow.c', 'fwrite.c', 'fputs.c',
                 'printf.c', 'puts.c', '__lockfile.c'])
    iprintf_files += files_in_path(
      path='system/lib/libc/musl/src/string',
      filenames=['strlen.c'])
    return math_files + other_files + iprintf_files

  def get_files(self):
    libc_files = []
    musl_srcdir = utils.path_from_root('system/lib/libc/musl/src')

    # musl modules
    ignore = [
        'ipc', 'passwd', 'signal', 'sched', 'time', 'linux',
        'aio', 'exit', 'legacy', 'mq', 'setjmp', 'env',
        'ldso', 'malloc'
    ]

    # individual files
    ignore += [
        'memcpy.c', 'memset.c', 'memmove.c', 'getaddrinfo.c', 'getnameinfo.c',
        'res_query.c', 'res_querydomain.c',
        'proto.c', 'gethostbyaddr.c', 'gethostbyaddr_r.c', 'gethostbyname.c',
        'gethostbyname2_r.c', 'gethostbyname_r.c', 'gethostbyname2.c',
        'alarm.c', 'syscall.c', 'popen.c', 'pclose.c',
        'getgrouplist.c', 'initgroups.c', 'wordexp.c', 'timer_create.c',
        'getentropy.c',
        # 'process' exclusion
        'fork.c', 'vfork.c', 'posix_spawn.c', 'posix_spawnp.c', 'execve.c', 'waitid.c', 'system.c',
        '_Fork.c',
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
        # TODO: Support this. See #12216.
        'pthread_setname_np.c',
      ]
      libc_files += files_in_path(
        path='system/lib/pthread',
        filenames=[
          'library_pthread.c',
          'proxying.c',
          'pthread_create.c',
          'emscripten_proxy_main.c',
          'emscripten_thread_init.c',
          'emscripten_thread_state.S',
          'emscripten_futex_wait.c',
          'emscripten_futex_wake.c',
        ])
    else:
      ignore += ['thread']
      libc_files += files_in_path(
        path='system/lib/libc/musl/src/thread',
        filenames=[
          'pthread_self.c',
          'pthread_cleanup_push.c',
          'pthread_attr_get.c',
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
          'pthread_self_stub.c'
        ])

    if self.is_optz:
      ignore += ['pow.c', 'pow_data.c', 'log.c', 'log_data.c', 'log2.c', 'log2_data.c']
    else:
      ignore += ['pow_small.c', 'log_small.c', 'log2_small.c']

    ignore = set(ignore)
    for dirpath, dirnames, filenames in os.walk(musl_srcdir):
      # Don't recurse into ingored directories
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
          'gmtime.c',
          'localtime.c',
          'nanosleep.c',
          'clock_nanosleep.c',
          'ctime_r.c',
          'utime.c',
        ])
    libc_files += files_in_path(
        path='system/lib/libc/musl/src/legacy',
        filenames=['getpagesize.c', 'err.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/linux',
        filenames=['getdents.c', 'gettid.c', 'utimes.c'])
    libc_files += files_in_path(
        path='system/lib/libc/musl/src/env',
        filenames=['__environ.c', 'getenv.c', 'putenv.c', 'setenv.c', 'unsetenv.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/sched',
        filenames=['sched_yield.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/exit',
        filenames=['_Exit.c', 'atexit.c'])

    libc_files += files_in_path(
        path='system/lib/libc/musl/src/ldso',
        filenames=['dlerror.c', 'dlsym.c', 'dlclose.c'])

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
          'dynlink.c',
          'wasi-helpers.c',
          'emscripten_get_heap_size.c',
          'raise.c',
          'kill.c',
          'sigaction.c',
          'sigtimedwait.c',
          'pthread_sigmask.c',
          'emscripten_console.c',
          'emscripten_time.c',
        ])

    libc_files += files_in_path(
        path='system/lib/pthread',
        filenames=['emscripten_atomic.c', 'thread_profiler.c'])

    libc_files += files_in_path(
      path='system/lib/libc',
      filenames=['emscripten_memcpy.c',
                 'emscripten_memset.c',
                 'emscripten_scan_stack.c',
                 'emscripten_memmove.c',
                 'emscripten_mmap.c'
                 ])

    libc_files += glob_in_path('system/lib/libc/compat', '*.c')

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


class libprintf_long_double(libc):
  name = 'libprintf_long_double'
  cflags = ['-DEMSCRIPTEN_PRINTF_LONG_DOUBLE']

  def get_files(self):
    return files_in_path(
        path='system/lib/libc/musl/src/stdio',
        filenames=['vfprintf.c'])

  def can_use(self):
    return super(libprintf_long_double, self).can_use() and settings.PRINTF_LONG_DOUBLE


class libsockets(MuslInternalLibrary, MTLibrary):
  name = 'libsockets'

  cflags = ['-Os', '-fno-builtin', '-Wno-shift-op-parentheses']

  def get_files(self):
    return files_in_path(
      path='system/lib/libc/musl/src/network',
      filenames=LIBC_SOCKETS)

  def can_use(self):
    return super(libsockets, self).can_use() and not settings.PROXY_POSIX_SOCKETS


class libsockets_proxy(MTLibrary):
  name = 'libsockets_proxy'

  cflags = ['-Os']

  def get_files(self):
    return [utils.path_from_root('system/lib/websocket/websocket_to_posix_socket.cpp')]

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


class crtbegin(Library):
  name = 'crtbegin'
  cflags = ['-sUSE_PTHREADS']
  src_dir = 'system/lib/pthread'
  src_files = ['emscripten_tls_init.c']

  force_object_files = True

  def get_ext(self):
    return '.o'

  def can_use(self):
    return super().can_use() and settings.USE_PTHREADS


class libcxxabi(NoExceptLibrary, MTLibrary):
  name = 'libc++abi'
  cflags = [
      '-Oz',
      '-D_LIBCXXABI_BUILDING_LIBRARY',
      '-DLIBCXXABI_NON_DEMANGLING_TERMINATE',
    ]
  includes = ['system/lib/libcxx']

  def get_cflags(self):
    cflags = super().get_cflags()
    cflags.append('-DNDEBUG')
    if not self.is_mt:
      cflags.append('-D_LIBCXXABI_HAS_NO_THREADS')
    if self.eh_mode == Exceptions.NONE:
      cflags.append('-D_LIBCXXABI_NO_EXCEPTIONS')
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      cflags.append('-D__USING_EMSCRIPTEN_EXCEPTIONS__')
      # The code used to interpret exceptions during terminate
      # is not compatible with emscripten exceptions.
      cflags.append('-DLIBCXXABI_SILENT_TERMINATE')
    elif self.eh_mode == Exceptions.WASM:
      cflags.append('-D__USING_WASM_EXCEPTIONS__')
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
      'private_typeinfo.cpp'
    ]
    if self.eh_mode == Exceptions.NONE:
      filenames += ['cxa_noexception.cpp']
    elif self.eh_mode == Exceptions.WASM:
      filenames += [
        'cxa_exception_storage.cpp',
        'cxa_exception.cpp',
        'cxa_personality.cpp'
      ]

    return files_in_path(
        path='system/lib/libcxxabi/src',
        filenames=filenames)


class libcxx(NoExceptLibrary, MTLibrary):
  name = 'libc++'

  cflags = ['-DLIBCXX_BUILDING_LIBCXXABI=1', '-D_LIBCPP_BUILDING_LIBRARY', '-Oz',
            '-D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS']

  src_dir = 'system/lib/libcxx/src'
  src_glob = '**/*.cpp'
  src_glob_exclude = [
    'xlocale_zos.cpp',
    'locale_win32.cpp',
    'thread_win32.cpp',
    'support.cpp',
    'int128_builtins.cpp'
  ]


class libunwind(NoExceptLibrary, MTLibrary):
  name = 'libunwind'
  # Because calls to _Unwind_CallPersonality are generated during LTO, libunwind
  # can't currently be part of LTO.
  # See https://bugs.llvm.org/show_bug.cgi?id=44353
  force_object_files = True

  cflags = ['-Oz', '-D_LIBUNWIND_DISABLE_VISIBILITY_ANNOTATIONS']
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
    if not self.is_mt:
      cflags.append('-D_LIBUNWIND_HAS_NO_THREADS')
    if self.eh_mode == Exceptions.NONE:
      cflags.append('-D_LIBUNWIND_HAS_NO_EXCEPTIONS')
    elif self.eh_mode == Exceptions.EMSCRIPTEN:
      cflags.append('-D__USING_EMSCRIPTEN_EXCEPTIONS__')
    elif self.eh_mode == Exceptions.WASM:
      cflags.append('-D__USING_WASM_EXCEPTIONS__')
    return cflags


class libmalloc(MTLibrary):
  name = 'libmalloc'

  cflags = ['-fno-builtin']

  def __init__(self, **kwargs):
    self.malloc = kwargs.pop('malloc')
    if self.malloc not in ('dlmalloc', 'emmalloc', 'emmalloc-debug', 'emmalloc-memvalidate', 'emmalloc-verbose', 'emmalloc-memvalidate-verbose', 'none'):
      raise Exception('malloc must be one of "emmalloc[-debug|-memvalidate][-verbose]", "dlmalloc" or "none", see settings.js')

    self.use_errno = kwargs.pop('use_errno')
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
    sbrk = utils.path_from_root('system/lib/sbrk.c')
    return [malloc, sbrk]

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
    if not self.use_errno:
      cflags += ['-DMALLOC_FAILURE_ACTION=', '-DEMSCRIPTEN_NO_ERRNO']
    if self.is_tracing:
      cflags += ['--tracing']
    return cflags

  def get_base_name_prefix(self):
    return 'lib%s' % self.malloc

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_debug and not self.memvalidate and not self.verbose:
      name += '-debug'
    if not self.use_errno:
      # emmalloc doesn't actually use errno, but it's easier to build it again
      name += '-noerrno'
    if self.is_tracing:
      name += '-tracing'
    return name

  def can_use(self):
    return super().can_use() and settings.MALLOC != 'none'

  def can_build(self):
    # emmalloc is not currently compatible with 64-bit pointers.  See
    # the comment at the top of emmalloc.c.
    return not self.malloc.startswith('emmalloc') or not settings.MEMORY64

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_debug', 'use_errno', 'is_tracing', 'memvalidate', 'verbose']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      malloc=settings.MALLOC,
      is_debug=settings.ASSERTIONS >= 2,
      use_errno=settings.SUPPORT_ERRNO,
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


class libal(Library):
  name = 'libal'

  cflags = ['-Os']
  src_dir = 'system/lib'
  src_files = ['al.c']


class libGL(MTLibrary):
  name = 'libGL'

  src_dir = 'system/lib/gl'
  src_files = ['gl.c', 'webgl1.c', 'libprocaddr.c']

  cflags = ['-Oz']

  def __init__(self, **kwargs):
    self.is_legacy = kwargs.pop('is_legacy')
    self.is_webgl2 = kwargs.pop('is_webgl2')
    self.is_ofb = kwargs.pop('is_ofb')
    self.is_full_es3 = kwargs.pop('is_full_es3')
    if self.is_webgl2 or self.is_full_es3:
      # Don't use append or += here, otherwise we end up adding to
      # the class member.
      self.src_files = self.src_files + ['webgl2.c']
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
    return name

  def get_cflags(self):
    cflags = super().get_cflags()
    if self.is_legacy:
      cflags += ['-DLEGACY_GL_EMULATION=1']
    if self.is_webgl2:
      cflags += ['-DMAX_WEBGL_VERSION=2']
    if self.is_ofb:
      cflags += ['-D__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__']
    if self.is_full_es3:
      cflags += ['-D__EMSCRIPTEN_FULL_ES3__']
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_legacy', 'is_webgl2', 'is_ofb', 'is_full_es3']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      is_legacy=settings.LEGACY_GL_EMULATION,
      is_webgl2=settings.MAX_WEBGL_VERSION >= 2,
      is_ofb=settings.OFFSCREEN_FRAMEBUFFER,
      is_full_es3=settings.FULL_ES3,
      **kwargs
    )


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

  def get_files(self):
    return [utils.path_from_root('system/lib/fetch/emscripten_fetch.c')]


class libstb_image(Library):
  name = 'libstb_image'
  never_force = True
  includes = ['third_party']

  def get_files(self):
    return [utils.path_from_root('system/lib/stb_image.c')]


class libasmfs(MTLibrary):
  name = 'libasmfs'
  never_force = True

  def get_files(self):
    return [utils.path_from_root('system/lib/fetch/asmfs.cpp')]

  def can_build(self):
    # ASMFS is looking for a maintainer
    # https://github.com/emscripten-core/emscripten/issues/9534
    return True


class libwasmfs(MTLibrary, DebugLibrary, AsanInstrumentedLibrary):
  name = 'libwasmfs'

  cflags = ['-fno-exceptions', '-std=c++17']

  includes = ['system/lib/wasmfs']

  def get_files(self):
    backends = files_in_path(
        path='system/lib/wasmfs/backends',
        filenames=['fetch_backend.cpp',
                   'js_file_backend.cpp',
                   'memory_backend.cpp',
                   'proxied_file_backend.cpp'])
    return backends + files_in_path(
        path='system/lib/wasmfs',
        filenames=['file.cpp',
                   'file_table.cpp',
                   'js_api.cpp',
                   'paths.cpp',
                   'streams.cpp',
                   'syscalls.cpp',
                   'wasmfs.cpp'])

  def can_use(self):
    return settings.WASMFS


class libhtml5(Library):
  name = 'libhtml5'

  cflags = ['-Oz']
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
              'system/lib/compiler-rt/lib']
  never_force = True

  src_dir = 'system/lib/compiler-rt/lib/sanitizer_common'
  src_glob = '*.cpp'
  src_glob_exclude = ['sanitizer_common_nolibc.cpp']


class SanitizerLibrary(CompilerRTLibrary, MTLibrary):
  never_force = True

  includes = ['system/lib/compiler-rt/lib']
  src_glob = '*.cpp'


class libubsan_rt(SanitizerLibrary):
  name = 'libubsan_rt'

  cflags = ['-DUBSAN_CAN_USE_CXXABI']
  src_dir = 'system/lib/compiler-rt/lib/ubsan'


class liblsan_common_rt(SanitizerLibrary):
  name = 'liblsan_common_rt'

  src_dir = 'system/lib/compiler-rt/lib/lsan'
  src_glob = 'lsan_common*.cpp'


class liblsan_rt(SanitizerLibrary):
  name = 'liblsan_rt'

  src_dir = 'system/lib/compiler-rt/lib/lsan'
  src_glob_exclude = ['lsan_common.cpp', 'lsan_common_mac.cpp', 'lsan_common_linux.cpp',
                      'lsan_common_emscripten.cpp']


class libasan_rt(SanitizerLibrary):
  name = 'libasan_rt'

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

  cflags = ['-Os', '-fno-builtin']
  src_dir = 'system/lib'

  def __init__(self, **kwargs):
    self.is_mem_grow = kwargs.pop('is_mem_grow')
    super().__init__(**kwargs)

  def get_base_name(self):
    name = super().get_base_name()
    if self.is_mem_grow:
      name += '-memgrow'
    return name

  def get_cflags(self):
    cflags = super().get_cflags()
    cflags += ['-DNDEBUG', '-DEMSCRIPTEN_STANDALONE_WASM']
    if self.is_mem_grow:
      cflags += ['-DEMSCRIPTEN_MEMORY_GROWTH']
    return cflags

  @classmethod
  def vary_on(cls):
    return super().vary_on() + ['is_mem_grow']

  @classmethod
  def get_default_variation(cls, **kwargs):
    return super().get_default_variation(
      is_mem_grow=settings.ALLOW_MEMORY_GROWTH,
      **kwargs
    )

  def get_files(self):
    files = files_in_path(
        path='system/lib/standalone',
        filenames=['standalone.c', 'standalone_wasm_stdio.c', '__original_main.c',
                   '__main_void.c', '__main_argc_argv.c'])
    files += files_in_path(
        path='system/lib/libc',
        filenames=['emscripten_memcpy.c'])
    # It is more efficient to use JS methods for time, normally.
    files += files_in_path(
        path='system/lib/libc/musl/src/time',
        filenames=['strftime.c',
                   '__month_to_secs.c',
                   '__secs_to_tm.c',
                   '__tm_to_secs.c',
                   '__tz.c',
                   '__year_to_secs.c',
                   'clock.c',
                   'clock_gettime.c',
                   'difftime.c',
                   'gettimeofday.c',
                   'localtime_r.c',
                   'gmtime_r.c',
                   'mktime.c',
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
  cflags = ['-Os']
  src_dir = 'system/lib'
  src_files = ['jsmath.c']

  def can_use(self):
    return super(libjsmath, self).can_use() and settings.JS_MATH


class libstubs(DebugLibrary):
  name = 'libstubs'
  src_dir = 'system/lib/libc'
  src_files = ['emscripten_syscall_stubs.c', 'emscripten_libc_stubs.c']


# If main() is not in EXPORTED_FUNCTIONS, it may be dce'd out. This can be
# confusing, so issue a warning.
def warn_on_unexported_main(symbolses):
  # In STANDALONE_WASM we don't expect main to be explictly exported
  if settings.STANDALONE_WASM:
    return
  if '_main' not in settings.EXPORTED_FUNCTIONS:
    for symbols in symbolses:
      if 'main' in symbols['defs']:
        logger.warning('main() is in the input files, but "_main" is not in EXPORTED_FUNCTIONS, which means it may be eliminated as dead code. Export it if you want main() to run.')
        return


def handle_reverse_deps(input_files):
  if settings.REVERSE_DEPS == 'none':
    return
  elif settings.REVERSE_DEPS == 'all':
    # When not optimzing we add all possible reverse dependencies rather
    # than scanning the input files
    for symbols in deps_info.get_deps_info().values():
      for symbol in symbols:
        settings.REQUIRED_EXPORTS.append(symbol)
    return

  if settings.REVERSE_DEPS != 'auto':
    shared.exit_with_error(f'invalid values for REVERSE_DEPS: {settings.REVERSE_DEPS}')

  added = set()

  def add_reverse_deps(need):
    more = False
    for ident, deps in deps_info.get_deps_info().items():
      if ident in need['undefs'] and ident not in added:
        added.add(ident)
        more = True
        for dep in deps:
          need['undefs'].add(dep)
          logger.debug('adding dependency on %s due to deps-info on %s' % (dep, ident))
          settings.REQUIRED_EXPORTS.append(dep)
    if more:
      add_reverse_deps(need) # recurse to get deps of deps

  # Scan symbols
  symbolses = building.llvm_nm_multiple([os.path.abspath(t) for t in input_files])

  warn_on_unexported_main(symbolses)

  if len(symbolses) == 0:
    symbolses.append({'defs': set(), 'undefs': set()})

  # depend on exported functions
  for export in settings.EXPORTED_FUNCTIONS:
    if settings.VERBOSE:
      logger.debug('adding dependency on export %s' % export)
    symbolses[0]['undefs'].add(demangle_c_symbol_name(export))

  for symbols in symbolses:
    add_reverse_deps(symbols)


def get_libs_to_link(args, forced, only_forced):
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
  if force == '1':
    force_include = [name for name, lib in system_libs_map.items() if not lib.never_force]
  elif force is not None:
    force_include = force.split(',')
  force_include += forced
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
    if settings.USE_PTHREADS:
      add_library('crtbegin')

    if settings.STANDALONE_WASM:
      if settings.EXPECT_MAIN:
        add_library('crt1')
      else:
        add_library('crt1_reactor')

  if settings.SIDE_MODULE:
    return libs_to_link

  for forced in force_include:
    if forced not in system_libs_map:
      shared.exit_with_error('invalid forced library: %s', forced)
    add_library(forced)

  if '-nodefaultlibs' in args:
    return libs_to_link

  if only_forced:
    add_library('libcompiler_rt')
    return libs_to_link

  if settings.AUTO_NATIVE_LIBRARIES:
    add_library('libGL')
    add_library('libal')
    add_library('libhtml5')

  sanitize = settings.USE_LSAN or settings.USE_ASAN or settings.UBSAN_RUNTIME

  # JS math must come before anything else, so that it overrides the normal
  # libc math.
  if settings.JS_MATH:
    add_library('libjsmath')

  # to override the normal libc printf, we must come before it
  if settings.PRINTF_LONG_DOUBLE:
    add_library('libprintf_long_double')

  if settings.STANDALONE_WASM:
    add_library('libstandalonewasm')
  if settings.ALLOW_UNIMPLEMENTED_SYSCALLS:
    add_library('libstubs')
  if '-nolibc' not in args:
    if not settings.EXIT_RUNTIME:
      add_library('libnoexit')
    add_library('libc')
    if settings.MALLOC != 'none':
      add_library('libmalloc')
  add_library('libcompiler_rt')
  if settings.LINK_AS_CXX:
    add_library('libc++')
  if settings.LINK_AS_CXX or sanitize:
    add_library('libc++abi')
    if settings.EXCEPTION_HANDLING:
      add_library('libunwind')

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

  if settings.PROXY_POSIX_SOCKETS:
    add_library('libsockets_proxy')
  else:
    add_library('libsockets')

  if settings.USE_WEBGPU:
    add_library('libwebgpu_cpp')

  return libs_to_link


def calculate(input_files, args, forced):
  # Setting this will only use the forced libs in EMCC_FORCE_STDLIBS. This avoids spending time checking
  # for unresolved symbols in your project files, which can speed up linking, but if you do not have
  # the proper list of actually needed libraries, errors can occur. See below for how we must
  # export all the symbols in deps_info when using this option.
  only_forced = os.environ.get('EMCC_ONLY_FORCED_STDLIBS')
  if only_forced:
    # One of the purposes EMCC_ONLY_FORCED_STDLIBS was to skip the scanning
    # of the input files for reverse dependencies.
    diagnostics.warning('deprecated', 'EMCC_ONLY_FORCED_STDLIBS is deprecated.  Use `-nostdlib` and/or `-s REVERSE_DEPS=none` depending on the desired result')
    settings.REVERSE_DEPS = 'all'

  handle_reverse_deps(input_files)

  libs_to_link = get_libs_to_link(args, forced, only_forced)

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
    # This mimicks how musl itself installs its headers.
    ('lib', 'libc', 'musl', 'arch', 'emscripten'): '',
    ('lib', 'libc', 'musl', 'include'): '',
    ('lib', 'libcxx', 'include'): os.path.join('c++', 'v1'),
    ('lib', 'libcxxabi', 'include'): os.path.join('c++', 'v1'),
  }

  target_include_dir = shared.Cache.get_include_dir()
  for src, dest in install_dirs.items():
    src = utils.path_from_root('system', *src)
    dest = os.path.join(target_include_dir, dest)
    copytree_exist_ok(src, dest)

  pkgconfig_src = utils.path_from_root('system/lib/pkgconfig')
  pkgconfig_dest = shared.Cache.get_sysroot_dir('lib', 'pkgconfig')
  copytree_exist_ok(pkgconfig_src, pkgconfig_dest)

  bin_src = utils.path_from_root('system/bin')
  bin_dest = shared.Cache.get_sysroot_dir('bin')
  copytree_exist_ok(bin_src, bin_dest)

  cmake_src = utils.path_from_root('system/lib/cmake')
  cmake_dest = shared.Cache.get_sysroot_dir('lib', 'cmake')
  copytree_exist_ok(cmake_src, cmake_dest)

  # Create a version header based on the emscripten-version.txt
  version_file = os.path.join(shared.Cache.get_include_dir(), 'emscripten/version.h')
  utils.write_file(version_file, textwrap.dedent(f'''\
  /* Automatically generated by tools/system_libs.py */
  #define __EMSCRIPTEN_major__ {shared.EMSCRIPTEN_VERSION_MAJOR}
  #define __EMSCRIPTEN_minor__ {shared.EMSCRIPTEN_VERSION_MINOR}
  #define __EMSCRIPTEN_tiny__ {shared.EMSCRIPTEN_VERSION_TINY}
  '''))

  # Create a stamp file that signal the the header have been installed
  # Removing this file, or running `emcc --clear-cache` or running
  # `./embuilder build sysroot --force` will cause the re-installation of
  # the system headers.
  utils.write_file(stamp, 'x')
  return stamp


@ToolchainProfiler.profile()
def ensure_sysroot():
  shared.Cache.get('sysroot_install.stamp', install_system_headers, what='system headers')
