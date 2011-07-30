#!/usr/bin/env python

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

Example uses:

 * With configure, do something like

    RANLIB=PATH/emmaken.py AR=PATH/emmaken.py CXX=PATH/emmaken.py CC=PATH/emmaken.py ./configure [options]

   where PATH is the path to this file.

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
'''

import sys
import os
import subprocess

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + list(pathelems)))
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

# If this is a CMake config, just do that
CMAKE_CONFIG = 'CMakeFiles/cmTryCompileExec.dir' in ' '.join(sys.argv)# or 'CMakeCCompilerId' in ' '.join(sys.argv)
if CMAKE_CONFIG:
  compiler = 'g++' if 'CXXCompiler' in ' '.join(sys.argv) else 'gcc'
  exit(os.execvp(compiler, [compiler] + sys.argv[1:]))

try:
  print >> sys.stderr, 'emmaken.py: ', ' '.join(sys.argv)

  #f=open('/dev/shm/tmp/waka.txt', 'a')
  #f.write('Args: ' + ' '.join(sys.argv) + '\nCMake? ' + str(CMAKE_CONFIG) + '\n')
  #f.close()

  # If no provided compiler, use LLVM_GCC from ~/.emscripten.
  # Or, use the provided one; if it is 'clang', then use CLANG from ~/.emscripten
  cxx = os.environ.get('EMMAKEN_COMPILER')
  if cxx and cxx == 'clang':
    cxx = CLANG
  CXX = cxx or LLVM_GCC
  CC = to_cc(CXX)

  # If we got here from a redirection through emmakenxx.py, then force a C++ compiler here
  if sys.argv[-1] == '-EMMAKEN_CXX':
    CC = CXX
    sys.argv = sys.argv[:-1]

  CC_ARG_SKIP = ['-O1', '-O2', '-O3']
  CC_ADDITIONAL_ARGS = ['-m32', '-g', '-U__i386__', '-U__x86_64__', '-U__i386', '-U__x86_64', '-U__SSE__', '-U__SSE2__', '-UX87_DOUBLE_ROUNDING', '-UHAVE_GCC_ASM_FOR_X87']
  ALLOWED_LINK_ARGS = ['-f', '-help', '-o', '-print-after', '-print-after-all', '-print-before',
                       '-print-before-all', '-time-passes', '-v', '-verify-dom-info', '-version' ]
  TWO_PART_DISALLOWED_LINK_ARGS = ['-L'] # Ignore thingsl like |-L .|

  EMMAKEN_CFLAGS = os.environ.get('EMMAKEN_CFLAGS')
  if EMMAKEN_CFLAGS: CC_ADDITIONAL_ARGS += EMMAKEN_CFLAGS.split(' ')

  # ----------------  End configs -------------

  if len(sys.argv) == 2 and 'conftest' not in ' '.join(sys.argv): # Avoid messing with configure, see below too
    # ranlib
    os.execvp(LLVM_DIS, ['-show-annotations', sys.argv[1]])
    sys.exit(0)
  if len(sys.argv) == 1 or sys.argv[1] in ['x', 't']:
    # noop ar
    sys.exit(0)

  use_cxx = True
  use_linker = True
  header = False # pre-compiled headers. We fake that by just copying the file

  opts = []
  files = []
  for arg in sys.argv[1:]:
    if arg.startswith('-'):
      opts.append(arg)
    else:
      files.append(arg)
      if arg.endswith('.c'):
        use_cxx = False
      if arg.endswith(('.c', '.cc', '.cpp')):
        use_linker = False
      if arg.endswith('.h'):
        header = True
        use_linker = False

  if '--version' in opts:
    use_linker = False

  if set(sys.argv[1]).issubset(set('cruqs')): # ar
    sys.argv = sys.argv[:1] + sys.argv[3:] + ['-o='+sys.argv[2]]
    assert use_linker, 'Linker should be used in this case'

  if use_linker:
    call = LLVM_LINK
    newargs = []
    found_o = False
    i = 0
    while i < len(sys.argv)-1:
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
        prefix = arg.split('=')[0]
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
    newargs = [ arg for arg in sys.argv[1:] if arg not in CC_ARG_SKIP ] + CC_ADDITIONAL_ARGS
    if 'conftest.c' not in files:
      newargs.append('-emit-llvm')
      if not use_linker:
        newargs.append('-c') 
  else:
    shutil.copy(sys.argv[-1], sys.argv[-2])
    exit(0)

  #f=open('/dev/shm/tmp/waka.txt', 'a')
  #f.write('Calling: ' + ' '.join(newargs) + '\n\n')
  #f.close()

  print >> sys.stderr, "Running:", call, ' '.join(newargs)

  os.execvp(call, [call] + newargs)
except Exception, e:
  print 'Error in emmaken.py. (Is the config file ~/.emscripten set up properly?) Error:', e
  raise

