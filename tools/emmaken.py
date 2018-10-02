#!/usr/bin/env python2
# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import shutil
import sys

print('\n\nemmaken.py is deprecated! use "emcc"\n\n', file=sys.stderr)

'''
emmaken - the emscripten make proxy tool
========================================

Tell your build system to use this instead of the compiler, linker, ar and
ranlib. All the normal build commands will be sent to this script, which
will proxy them to the appropriate LLVM build commands, in order to
generate proper code for Emscripten to later process.

For example, compilation will be translated into calls to llvm-gcc
with -emit-llvm, and linking will be translated into calls to llvm-link,
and so forth.

emmaken is only meant to *COMPILE* source code into LLVM bitcode. It does
not do optimizations (in fact, it will disable -Ox flags and warn you
about that). The reason is that doing such optimizations early can lead
to bitcode that Emscripten cannot process properly, or can process but
not fully optimize. You can (and probably should) still run LLVM
optimizations though, by telling emscripten.py to do so (or run LLVM
opt yourself, but be careful with the parameters you pass).

Example uses:

 * For configure, instead of ./configure, cmake, etc., run emconfiguren.py
   with that command as an argument, for example

    emconfiguren.py ./configure [options]

   emconfiguren.py is a tiny script that just sets some environment vars
   as a convenience. The command just shown is equivalent to

    EMMAKEN_JUST_CONFIGURE=1 RANLIB=PATH/emmaken.py AR=PATH/emmaken.py CXX=PATH/emmakenxx.py CC=PATH/emmaken.py ./configure [options]

   where PATH is the path to this file.

   EMMAKEN_JUST_CONFIGURE tells emmaken that it is being run in ./configure,
   so it should relay everything to gcc/g++. You should not define that when
   running make, of course.

 * With CMake, the same command will work (with cmake instead of ./configure). You may also be
   able to do the following in your CMakeLists.txt:

    SET(CMAKE_C_COMPILER "PATH/emmaken.py")
    SET(CMAKE_CXX_COMPILER "PATH/emmakenxx.py")
    SET(CMAKE_LINKER "PATH/emmaken.py")
    SET(CMAKE_CXX_LINKER "PATH/emmaken.py")
    SET(CMAKE_C_LINK_EXECUTABLE "PATH/emmaken.py")
    SET(CMAKE_CXX_LINK_EXECUTABLE "PATH/emmaken.py")
    SET(CMAKE_AR "PATH/emmaken.py")
    SET(CMAKE_RANLIB "PATH/emmaken.py")

 * For SCons the shared.py can be imported like so:
    __file__ = str(Dir('#/project_path_to_emscripten/dummy/dummy'))
    __rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    def path_from_root(*pathelems):
      return os.path.join(__rootpath__, *pathelems)
    sys.path += [path_from_root('')]
    from tools import shared

   For using the Emscripten compilers/linkers/etc. you can do:
    env = Environment()
    ...
    env.Append(CCFLAGS = COMPILER_OPTS)
    env.Replace(LINK = LLVM_LD)
    env.Replace(LD   = LLVM_LD)
   TODO: Document all relevant setup changes

After setting that up, run your build system normally. It should generate
LLVM instead of the normal output, and end up with .ll files that you can
give to Emscripten. Note that this tool doesn't run Emscripten itself. Note
also that you may need to do some manual fiddling later, for example to
link files that weren't linked, and them llvm-dis them.

Note the appearance of emmakenxx.py instead of emmaken.py
for the C++ compiler. This is needed for cases where we get
a C++ file with a C extension, in which case CMake can be told
to run g++ on it despite the .c extension, see

  https://github.com/kripken/emscripten/issues/6

(If a similar situation occurs with ./configure, you can do the same there too.)

emmaken can be influenced by a few environment variables:

  EMMAKEN_NO_SDK - Will tell emmaken *not* to use the emscripten headers. Instead
                   your system headers will be used.

  EMMAKEN_COMPILER - The compiler to be used, if you don't want the default clang.
'''

import sys
import os
import subprocess

print('emmaken.py: ', ' '.join(sys.argv), file=sys.stderr)

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

from tools.shared import EMSDK_OPTS, EM_CONFIG, CLANG, COMPILER_OPTS, LLVM_LD, to_cc

# If this is a configure-type thing, just do that
CONFIGURE_CONFIG = os.environ.get('EMMAKEN_JUST_CONFIGURE')
CMAKE_CONFIG = 'CMakeFiles/cmTryCompileExec.dir' in ' '.join(sys.argv)# or 'CMakeCCompilerId' in ' '.join(sys.argv)
if CONFIGURE_CONFIG or CMAKE_CONFIG:
  compiler = 'g++' if 'CXXCompiler' in ' '.join(sys.argv) or os.environ.get('EMMAKEN_CXX') else 'gcc'
  cmd = [compiler] + EMSDK_OPTS + sys.argv[1:]
  print('emmaken.py, just configuring: ', cmd, file=sys.stderr)
  exit(subprocess.call(cmd))

try:
  # f=open('/dev/shm/tmp/waka.txt', 'a')
  # f.write('Args: ' + ' '.join(sys.argv) + '\nCMake? ' + str(CMAKE_CONFIG) + '\n')
  # f.close()

  if os.environ.get('EMMAKEN_COMPILER'):
    CXX = os.environ['EMMAKEN_COMPILER']
  else:
    CXX = CLANG

  CC = to_cc(CXX)

  # If we got here from a redirection through emmakenxx.py, then force a C++ compiler here
  if os.environ.get('EMMAKEN_CXX'):
    CC = CXX

  CC_ADDITIONAL_ARGS = COMPILER_OPTS # + ['-g']?
  ALLOWED_LINK_ARGS = ['-f', '-help', '-o', '-print-after', '-print-after-all', '-print-before',
                       '-print-before-all', '-time-passes', '-v', '-verify-dom-info', '-version']
  TWO_PART_DISALLOWED_LINK_ARGS = ['-L'] # Ignore thingsl like |-L .|

  EMMAKEN_CFLAGS = os.environ.get('EMMAKEN_CFLAGS')
  if EMMAKEN_CFLAGS:
    CC_ADDITIONAL_ARGS += EMMAKEN_CFLAGS.split(' ')

  # ----------------  End configs -------------

  if len(sys.argv) == 2 and 'conftest' not in ' '.join(sys.argv): # Avoid messing with configure, see below too
    # ranlib
    sys.exit(0)
  if len(sys.argv) == 1 or sys.argv[1] in ['x', 't']:
    # noop ar
    sys.exit(0)

  use_cxx = True
  use_linker = True
  header = False # pre-compiled headers. We fake that by just copying the file

  opts = []
  files = []
  for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if arg.startswith('-'):
      opts.append(arg)
    else:
      files.append(arg)
      if arg.endswith('.c'):
        use_cxx = False
      if arg.endswith(('.c', '.cc', '.cpp', '.dT')):
        use_linker = False
      if arg.endswith('.h') and sys.argv[i - 1] != '-include':
        header = True
        use_linker = False

  if '--version' in opts:
    use_linker = False

  if set(sys.argv[1]).issubset(set('-cruqs')): # ar
    sys.argv = sys.argv[:1] + sys.argv[3:] + ['-o=' + sys.argv[2]]
    assert use_linker, 'Linker should be used in this case'

  if use_linker:
    call = LLVM_LD
    newargs = ['-disable-opt']
    found_o = False
    i = 0
    while i < len(sys.argv) - 1:
      i += 1
      arg = sys.argv[i]
      if found_o:
        newargs.append('-o=%s' % arg)
        found_o = False
        continue
      if arg.startswith('-'):
        if arg == '-o':
          found_o = True
          continue
        prefix = arg.split('=', 1)[0]
        if prefix in ALLOWED_LINK_ARGS:
          newargs.append(arg)
        if arg in TWO_PART_DISALLOWED_LINK_ARGS:
          i += 1
      elif arg.endswith('.so'):
        continue # .so's do not exist yet, in many cases
      else:
        # not option, so just append
        newargs.append(arg)
  elif not header:
    call = CXX if use_cxx else CC
    newargs = sys.argv[1:]
    for i in range(len(newargs)):
      if newargs[i].startswith('-O'):
        print('emmaken.py: WARNING: Optimization flags (-Ox) are ignored in emmaken. Tell emscripten.py to do that, or run LLVM opt.', file=sys.stderr)
        newargs[i] = ''
    newargs = [arg for arg in newargs if arg is not ''] + CC_ADDITIONAL_ARGS
    newargs.append('-emit-llvm')
    if not use_linker:
      newargs.append('-c')
  else:
    print('Just copy.', file=sys.stderr)
    shutil.copy(sys.argv[-1], sys.argv[-2])
    exit(0)

  # f=open('/dev/shm/tmp/waka.txt', 'a')
  # f.write('Calling: ' + ' '.join(newargs) + '\n\n')
  # f.close()

  print("Running:", call, ' '.join(newargs), file=sys.stderr)

  subprocess.call([call] + newargs)
except Exception as e:
  print('Error in emmaken.py. (Is the config file %s set up properly?) Error:' % EM_CONFIG, e)
  raise
