#!/usr/bin/env python2
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Tool to manage building of various useful things, such as libc, libc++,
native optimizer, as well as fetch and build ports like zlib and sdl2
"""

from __future__ import print_function
import logging
import os
import sys

from tools import shared

C_BARE = '''
        int main() {}
      '''
C_WITH_STDLIB = '''
        #include <string.h>
        int main() {
          return int(strchr("str", 'c'));
        }
      '''
C_WITH_MALLOC = '''
        #include <string.h>
        #include <stdlib.h>
        int main() {
          return int(malloc(10)) + int(strchr("str", 'c'));
        }
      '''
CXX_WITH_STDLIB = '''
        #include <iostream>
        int main() {
          std::cout << "hello";
          return 0;
        }
      '''

SYSTEM_TASKS = [
    'al',
    'compiler-rt',
    'emmalloc',
    'emmalloc_debug',
    'gl',
    'gl-emu',
    'gl-emu-webgl2',
    'gl-mt',
    'gl-mt-emu',
    'gl-mt-emu-webgl2',
    'gl-mt-webgl2',
    'gl-webgl2',
    'html5',
    'libc',
    'libc++',
    'libc++_noexcept',
    'libc++abi',
    'libc-extras',
    'libc-mt',
    'pthreads',
    'pthreads_stub',
]

for debug in ['', '_debug']:
  for noerrno in ['', '_noerrno']:
    for threadsafe in ['', '_threadsafe']:
      for tracing in ['', '_tracing']:
        SYSTEM_TASKS += ['dlmalloc' + debug + noerrno + threadsafe + tracing]

USER_TASKS = [
    'binaryen',
    'bullet',
    'bzip2',
    'cocos2d',
    'freetype',
    'harfbuzz',
    'icu',
    'libjpeg',
    'libpng',
    'ogg',
    'regal',
    'sdl2',
    'sdl2-mt',
    'sdl2-gfx',
    'sdl2-image',
    'sdl2-image-png',
    'sdl2-mixer',
    'sdl2-net',
    'sdl2-ttf',
    'vorbis',
    'zlib',
]

temp_files = shared.configuration.get_temp_files()
logger = logging.getLogger('embuilder')


def print_help():
  all_tasks = SYSTEM_TASKS + USER_TASKS
  all_tasks.sort()
  print('''
Emscripten System Builder Tool
==============================

You can use this tool to manually build parts of the emscripten system
environment. In general emcc will build them automatically on demand, so
you do not strictly need to use this tool, but it gives you more control
over the process (in particular, if emcc does this automatically, and you
are running multiple build commands in parallel, confusion can occur).

Usage:

  embuilder.py OPERATION TASK1 [TASK2..] [--pic] [--lto]

Available operations and tasks:

  build %s

Issuing 'embuilder.py build ALL' causes each task to be built.

Flags:

  --lto  Build bitcode files, for LTO with the LLVM wasm backend
  --pic  Build as position independent code (used by MAIN_MODULE/SIDE_MODULE)

It is also possible to build native_optimizer manually by using CMake. To
do that, run

   1. cd $EMSCRIPTEN/tools/optimizer
   2. cmake . -DCMAKE_BUILD_TYPE=Release
   3. make (or mingw32-make/vcbuild/msbuild on Windows)

and set up the location to the native optimizer in ~/.emscripten
''' % '\n        '.join(all_tasks))


def build(src, result_libs, args=[]):
  if not shared.Settings.WASM_OBJECT_FILES:
    args += ['-s', 'WASM_OBJECT_FILES=0']
  if shared.Settings.RELOCATABLE:
    args += ['-s', 'RELOCATABLE']
  # build in order to generate the libraries
  # do it all in a temp dir where everything will be cleaned up
  temp_dir = temp_files.get_dir()
  cpp = os.path.join(temp_dir, 'src.cpp')
  open(cpp, 'w').write(src)
  temp_js = os.path.join(temp_dir, 'out.js')
  shared.Building.emcc(cpp, args, output_filename=temp_js)

  # verify
  if not os.path.exists(temp_js):
    shared.exit_with_error('failed to build file')

  for lib in result_libs:
    if not os.path.exists(shared.Cache.get_path(lib)):
      shared.exit_with_error('not seeing that requested library %s has been built because file %s does not exist' % (lib, shared.Cache.get_path(lib)))


def build_port(port_name, lib_name, params):
  build(C_BARE, [lib_name] if lib_name else [], params)


def main():
  if len(sys.argv) < 2 or sys.argv[1] in ['-v', '-help', '--help', '-?', '?']:
    print_help()
    return 0

  operation = sys.argv[1]
  if operation != 'build':
    shared.exit_with_error('unfamiliar operation: ' + operation)

  # process flags

  args = sys.argv[2:]

  def is_flag(arg):
    return arg.startswith('--')

  for arg in args:
    if is_flag(arg):
      arg = arg[2:]
      if arg == 'lto':
        shared.Settings.WASM_OBJECT_FILES = 0
      elif arg == 'pic':
        shared.Settings.RELOCATABLE = 1
      # Reconfigure the cache dir to reflect the change
      shared.reconfigure_cache()

  args = [a for a in args if not is_flag(a)]

  # process tasks
  libname = shared.static_library_name

  auto_tasks = False
  tasks = args
  if 'SYSTEM' in tasks:
    tasks = SYSTEM_TASKS
    auto_tasks = True
  elif 'USER' in tasks:
    tasks = USER_TASKS
    auto_tasks = True
  elif 'ALL' in tasks:
    tasks = SYSTEM_TASKS + USER_TASKS
    auto_tasks = True
  if auto_tasks:
    if shared.Settings.WASM_BACKEND:
      skip_tasks = [task for task in SYSTEM_TASKS + USER_TASKS if '-mt' in task or 'thread' in task]
      # cocos2d: must be ported, errors on
      # "Cannot recognize the target platform; are you targeting an unsupported platform?"
      skip_tasks += ['cocos2d']
      print('Skipping building of %s, because WebAssembly does not support pthreads.' % ', '.join(skip_tasks))
      tasks = [x for x in tasks if x not in skip_tasks]
    else:
      if os.environ.get('EMSCRIPTEN_NATIVE_OPTIMIZER'):
        print('Skipping building of native-optimizer; EMSCRIPTEN_NATIVE_OPTIMIZER is environment.')
      elif shared.EMSCRIPTEN_NATIVE_OPTIMIZER:
        print('Skipping building of native-optimizer; EMSCRIPTEN_NATIVE_OPTIMIZER set in .emscripten config.')
      else:
        tasks += ['native_optimizer']
    print('Building targets: %s' % ' '.join(tasks))
  for what in tasks:
    logger.info('building and verifying ' + what)
    if what == 'compiler-rt':
      build('''
        int main() {
          double _Complex a, b, c;
          c = a / b;
          return 0;
        }
      ''', ['libcompiler_rt.a'])
    elif what == 'libc':
      build(C_WITH_MALLOC, [libname('libc')])
    elif what == 'libc-extras':
      build('''
        extern char **environ;
        int main() {
          return (int)environ;
        }
      ''', [libname('libc-extras')])
    elif what == 'struct_info':
      build(C_BARE, ['generated_struct_info.json'])
    elif what == 'emmalloc':
      build(C_WITH_MALLOC, [libname('libemmalloc')], ['-s', 'MALLOC="emmalloc"'])
    elif what == 'emmalloc_debug':
      build(C_WITH_MALLOC, [libname('libemmalloc_debug')], ['-s', 'MALLOC="emmalloc"', '-g'])
    elif what.startswith('dlmalloc'):
      cmd = ['-s', 'MALLOC="dlmalloc"']
      if '_debug' in what:
        cmd += ['-g']
      if '_noerrno' in what:
        cmd += ['-s', 'SUPPORT_ERRNO=0']
      if '_threadsafe' in what:
        cmd += ['-s', 'USE_PTHREADS=1']
      if '_tracing' in what:
        cmd += ['-s', 'EMSCRIPTEN_TRACING=1']
      build(C_WITH_MALLOC, [libname('lib' + what)], cmd)
    elif what in ('libc-mt', 'pthreads'):
      build(C_WITH_MALLOC, [libname('libc-mt'), libname('libpthreads')], ['-s', 'USE_PTHREADS=1'])
    elif what == 'libc-wasm':
      build(C_WITH_STDLIB, [libname('libc-wasm')], ['-s', 'WASM=1'])
    elif what == 'libc++':
      build(CXX_WITH_STDLIB, ['libc++.a'], ['-s', 'DISABLE_EXCEPTION_CATCHING=0'])
    elif what == 'libc++_noexcept':
      build(CXX_WITH_STDLIB, ['libc++_noexcept.a'])
    elif what == 'libc++abi':
      build('''
        struct X { int x; virtual void a() {} };
        struct Y : X { int y; virtual void a() { y = 10; }};
        int main(int argc, char **argv) {
          Y* y = dynamic_cast<Y*>((X*)argv[1]);
          y->a();
          return y->y;
        }
      ''', [libname('libc++abi')])
    elif what == 'gl' or what.startswith('gl-'):
      opts = []
      if '-mt' in what:
        opts += ['-s', 'USE_PTHREADS=1']
      if '-emu' in what:
        opts += ['-s', 'LEGACY_GL_EMULATION=1']
      if '-webgl2' in what:
        opts += ['-s', 'USE_WEBGL2=1']
      build('''
        extern "C" { extern void* emscripten_GetProcAddress(const char *x); }
        int main() {
          return int(emscripten_GetProcAddress("waka waka"));
        }
      ''', [libname('lib' + what)], opts)
    elif what == 'native_optimizer':
      build(C_BARE, ['optimizer.2.exe'], ['-O2', '-s', 'WASM=0'])
    elif what == 'compiler_rt_wasm':
      if shared.Settings.WASM_BACKEND:
        build(C_BARE, ['libcompiler_rt_wasm.a'], ['-s', 'WASM=1'])
      else:
        logger.warning('compiler_rt_wasm not built when using JSBackend')
    elif what == 'html5':
      build('''
        #include <stdlib.h>
        #include "emscripten/key_codes.h"
        int main() {
          return emscripten_compute_dom_pk_code(NULL);
        }

      ''', [libname('libhtml5')])
    elif what == 'pthreads_stub':
      build('''
        #include <emscripten/threading.h>
        int main() {
          return emscripten_is_main_runtime_thread();
        }

      ''', [libname('libpthreads_stub')])
    elif what == 'al':
      build('''
        #include "AL/al.h"
        int main() {
          alGetProcAddress(0);
          return 0;
        }
      ''', [libname('libal')])
    elif what == 'icu':
      build_port('icu', libname('libicuuc'), ['-s', 'USE_ICU=1'])
    elif what == 'zlib':
      build_port('zlib', 'libz.a', ['-s', 'USE_ZLIB=1'])
    elif what == 'bzip2':
      build_port('bzip2', 'libbz2.a', ['-s', 'USE_BZIP2=1'])
    elif what == 'bullet':
      build_port('bullet', libname('libbullet'), ['-s', 'USE_BULLET=1'])
    elif what == 'vorbis':
      build_port('vorbis', libname('libvorbis'), ['-s', 'USE_VORBIS=1'])
    elif what == 'ogg':
      build_port('ogg', libname('libogg'), ['-s', 'USE_OGG=1'])
    elif what == 'libjpeg':
      build_port('libjpeg', libname('libjpeg'), ['-s', 'USE_LIBJPEG=1'])
    elif what == 'libpng':
      build_port('libpng', libname('libpng'), ['-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1'])
    elif what == 'sdl2':
      build_port('sdl2', libname('libSDL2'), ['-s', 'USE_SDL=2'])
    elif what == 'sdl2-mt':
      build_port('sdl2', libname('libSDL2-mt'), ['-s', 'USE_SDL=2', '-s', 'USE_PTHREADS=1'])
    elif what == 'sdl2-gfx':
      build_port('sdl2-gfx', libname('libSDL2_gfx'), ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-s', 'USE_SDL_GFX=2'])
    elif what == 'sdl2-image':
      build_port('sdl2-image', libname('libSDL2_image'), ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'])
    elif what == 'sdl2-image-png':
      build_port('sdl2-image', libname('libSDL2_image'), ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-s', 'SDL2_IMAGE_FORMATS=["png"]'])
    elif what == 'sdl2-net':
      build_port('sdl2-net', libname('libSDL2_net'), ['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2'])
    elif what == 'sdl2-mixer':
      build_port('sdl2-mixer', 'libSDL2_mixer.a', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_MIXER=2', '-s', 'USE_VORBIS=1'])
    elif what == 'freetype':
      build_port('freetype', 'libfreetype.a', ['-s', 'USE_FREETYPE=1'])
    elif what == 'harfbuzz':
      build_port('harfbuzz', 'libharfbuzz.a', ['-s', 'USE_HARFBUZZ=1'])
    elif what == 'sdl2-ttf':
      build_port('sdl2-ttf', libname('libSDL2_ttf'), ['-s', 'USE_SDL=2', '-s', 'USE_SDL_TTF=2', '-s', 'USE_FREETYPE=1'])
    elif what == 'binaryen':
      build_port('binaryen', None, ['-s', 'WASM=1'])
    elif what == 'cocos2d':
      build_port('cocos2d', libname('libcocos2d'), ['-s', 'USE_COCOS2D=3', '-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=0'])
    elif what == 'regal':
      build_port('regal', libname('libregal'), ['-s', 'USE_REGAL=1'])
    else:
      logger.error('unfamiliar build target: ' + what)
      return 1

    logger.info('...success')
  return 0


if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
