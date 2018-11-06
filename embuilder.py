#!/usr/bin/env python2
# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Tool to manage building of various useful things, such as libc, libc++, native optimizer, as well as fetch and build ports like zlib and sdl2
'''

from __future__ import print_function
import logging
import os
import sys

from tools import shared

if len(sys.argv) < 2 or sys.argv[1] in ['-v', '-help', '--help', '-?', '?']:
  print('''
Emscripten System Builder Tool
==============================

You can use this tool to manually build parts of the emscripten system
environment. In general emcc will build them automatically on demand, so
you do not strictly need to use this tool, but it gives you more control
over the process (in particular, if emcc does this automatically, and you
are running multiple build commands in parallel, confusion can occur).

Usage:

  embuilder.py OPERATION TASK1 [TASK2..]

Available operations and tasks:

  build libc
        libc-mt
        libc-extras
        struct_info
        emmalloc
        emmalloc_debug
        dlmalloc
        dlmalloc_debug
        dlmalloc_threadsafe
        dlmalloc_threadsafe_debug
        pthreads
        libcxx
        libcxx_noexcept
        libcxxabi
        gl
        native_optimizer
        binaryen
        bullet
        freetype
        libpng
        ogg
        sdl2
        sdl2-image
        sdl2-ttf
        sdl2-net
        vorbis
        zlib
        cocos2d
        wasm-libc
        wasm_compiler_rt

Issuing 'embuilder.py build ALL' causes each task to be built.

It is also possible to build native_optimizer manually by using CMake. To
do that, run

   1. cd $EMSCRIPTEN/tools/optimizer
   2. cmake . -DCMAKE_BUILD_TYPE=Release
   3. make (or mingw32-make/vcbuild/msbuild on Windows)

and set up the location to the native optimizer in ~/.emscripten

''')
  sys.exit(0)

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

SYSTEM_TASKS = ['compiler-rt', 'libc', 'libc-mt', 'libc-extras', 'emmalloc', 'emmalloc_debug', 'dlmalloc', 'dlmalloc_threadsafe', 'pthreads', 'dlmalloc_debug', 'dlmalloc_threadsafe_debug', 'libcxx', 'libcxx_noexcept', 'libcxxabi', 'html5']
USER_TASKS = ['al', 'gl', 'binaryen', 'bullet', 'freetype', 'icu', 'libpng', 'ogg', 'sdl2', 'sdl2-gfx', 'sdl2-image', 'sdl2-ttf', 'sdl2-net', 'vorbis', 'zlib']

temp_files = shared.configuration.get_temp_files()
logger = logging.getLogger(__file__)

def build(src, result_libs, args=[]):
  # build in order to generate the libraries
  # do it all in a temp dir where everything will be cleaned up
  temp_dir = temp_files.get_dir()
  cpp = os.path.join(temp_dir, 'src.cpp')
  open(cpp, 'w').write(src)
  temp_js = os.path.join(temp_dir, 'out.js')
  shared.Building.emcc(cpp, args, output_filename=temp_js)

  # verify
  assert os.path.exists(temp_js), 'failed to build file'
  if result_libs:
    for lib in result_libs:
      assert os.path.exists(shared.Cache.get_path(lib)), 'not seeing that requested library %s has been built because file %s does not exist' % (lib, shared.Cache.get_path(lib))


def build_port(port_name, lib_name, params):
  build(C_BARE, [os.path.join('ports-builds', port_name, lib_name)] if lib_name else None, params)


def main():
  operation = sys.argv[1]
  if operation != 'build':
    logger.error('unfamiliar operation: ' + operation)
    return 1

  auto_tasks = False
  tasks = sys.argv[2:]
  if 'SYSTEM' in tasks:
    tasks = SYSTEM_TASKS
    auto_tasks = True
  if 'ALL' in tasks:
    tasks = SYSTEM_TASKS + USER_TASKS
    auto_tasks = True
  if auto_tasks:
    if shared.Settings.WASM_BACKEND:
      skip_tasks = {'libc-mt', 'dlmalloc_threadsafe', 'dlmalloc_threadsafe_debug', 'pthreads'}
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
      ''', ['compiler-rt.a'])
    elif what == 'libc':
      build(C_WITH_MALLOC, ['libc.bc'])
    elif what == 'libc-extras':
      build('''
        extern char **environ;
        int main() {
          return (int)environ;
        }
      ''', ['libc-extras.bc'])
    elif what == 'struct_info':
      build(C_BARE, ['generated_struct_info.json'])
    elif what == 'emmalloc':
      build(C_WITH_MALLOC, ['emmalloc.bc'], ['-s', 'MALLOC="emmalloc"'])
    elif what == 'emmalloc_debug':
      build(C_WITH_MALLOC, ['emmalloc_debug.bc'], ['-s', 'MALLOC="emmalloc"', '-g'])
    elif what == 'dlmalloc':
      build(C_WITH_MALLOC, ['dlmalloc.bc'], ['-s', 'MALLOC="dlmalloc"'])
    elif what == 'dlmalloc_debug':
      build(C_WITH_MALLOC, ['dlmalloc_debug.bc'], ['-g', '-s', 'MALLOC="dlmalloc"'])
    elif what == 'dlmalloc_threadsafe_debug':
      build(C_WITH_MALLOC, ['dlmalloc_threadsafe_debug.bc'], ['-g', '-s', 'USE_PTHREADS=1', '-s', 'MALLOC="dlmalloc"'])
    elif what in ('dlmalloc_threadsafe', 'libc-mt', 'pthreads'):
      build(C_WITH_MALLOC, ['libc-mt.bc', 'dlmalloc_threadsafe.bc', 'pthreads.bc'], ['-s', 'USE_PTHREADS=1', '-s', 'MALLOC="dlmalloc"'])
    elif what == 'wasm-libc':
      build(C_WITH_STDLIB, ['wasm-libc.bc'], ['-s', 'WASM=1'])
    elif what == 'libcxx':
      build(CXX_WITH_STDLIB, ['libcxx.a'], ['-s', 'DISABLE_EXCEPTION_CATCHING=0'])
    elif what == 'libcxx_noexcept':
      build(CXX_WITH_STDLIB, ['libcxx_noexcept.a'])
    elif what == 'libcxxabi':
      build('''
        struct X { int x; virtual void a() {} };
        struct Y : X { int y; virtual void a() { y = 10; }};
        int main(int argc, char **argv) {
          Y* y = dynamic_cast<Y*>((X*)argv[1]);
          y->a();
          return y->y;
        }
      ''', ['libcxxabi.bc'])
    elif what == 'gl':
      build('''
        extern "C" { extern void* emscripten_GetProcAddress(const char *x); }
        int main() {
          return int(emscripten_GetProcAddress("waka waka"));
        }
      ''', ['gl.bc'])
    elif what == 'native_optimizer':
      build(C_BARE, ['optimizer.2.exe'], ['-O2', '-s', 'WASM=0'])
    elif what == 'wasm_compiler_rt':
      if shared.Settings.WASM_BACKEND:
        build(C_BARE, ['wasm_compiler_rt.a'], ['-s', 'WASM=1'])
      else:
        logger.warning('wasm_compiler_rt not built when using JSBackend')
    elif what == 'html5':
      build('''
        #include <stdlib.h>
        #include "emscripten/key_codes.h"
        int main() {
          return emscripten_compute_dom_pk_code(NULL);
        }

      ''', ['html5.bc'])
    elif what == 'al':
      build('''
        #include "AL/al.h"
        int main() {
          alGetProcAddress(0);
          return 0;
        }
      ''', ['al.bc'])
    elif what == 'icu':
      build_port('icu', 'libicuuc.bc', ['-s', 'USE_ICU=1'])
    elif what == 'zlib':
      build_port('zlib', 'libz.a', ['-s', 'USE_ZLIB=1'])
    elif what == 'bullet':
      build_port('bullet', 'libbullet.bc', ['-s', 'USE_BULLET=1'])
    elif what == 'vorbis':
      build_port('vorbis', 'libvorbis.bc', ['-s', 'USE_VORBIS=1'])
    elif what == 'ogg':
      build_port('ogg', 'libogg.bc', ['-s', 'USE_OGG=1'])
    elif what == 'libpng':
      build_port('libpng', 'libpng.bc', ['-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1'])
    elif what == 'sdl2':
      build_port('sdl2', 'libsdl2.bc', ['-s', 'USE_SDL=2'])
    elif what == 'sdl2-gfx':
      build_port('sdl2-gfx', 'libsdl2_gfx.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2', '-s', 'USE_SDL_GFX=2'])
    elif what == 'sdl2-image':
      build_port('sdl2-image', 'libsdl2_image.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'])
    elif what == 'sdl2-net':
      build_port('sdl2-net', 'libsdl2_net.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2'])
    elif what == 'freetype':
      build_port('freetype', 'libfreetype.a', ['-s', 'USE_FREETYPE=1'])
    elif what == 'harfbuzz':
      build_port('harfbuzz', 'libharfbuzz.a', ['-s', 'USE_HARFBUZZ=1'])
    elif what == 'sdl2-ttf':
      build_port('sdl2-ttf', 'libsdl2_ttf.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_TTF=2', '-s', 'USE_FREETYPE=1'])
    elif what == 'binaryen':
      build_port('binaryen', None, ['-s', 'WASM=1'])
    elif what == 'cocos2d':
      build_port('cocos2d', None, ['-s', 'USE_COCOS2D=3', '-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=0'])
    else:
      logger.error('unfamiliar build target: ' + what)
      sys.exit(1)

    logger.info('...success')
  return 0


if __name__ == '__main__':
  try:
    sys.exit(main())
  except KeyboardInterrupt:
    logger.warning("KeyboardInterrupt")
    sys.exit(1)
