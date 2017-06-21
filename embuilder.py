#!/usr/bin/env python2

'''
Tool to manage building of various useful things, such as libc, libc++, native optimizer, as well as fetch and build ports like zlib and sdl2
'''

import os, sys
import tools.shared as shared

if len(sys.argv) < 2 or sys.argv[1] in ['-v', '-help', '--help', '-?', '?']:
  print '''
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
        dlmalloc
        dlmalloc_threadsafe
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

'''
  sys.exit(0)

temp_files = shared.configuration.get_temp_files()

def build(src, result_libs, args=[]):
  with temp_files.get_file('.cpp') as temp:
    open(temp, 'w').write(src)
    temp_js = temp_files.get('.js').name
    shared.Building.emcc(temp, args, output_filename=temp_js)

  assert os.path.exists(temp_js), 'failed to build file'
  if result_libs:
    for lib in result_libs:
      assert os.path.exists(shared.Cache.get_path(lib)), 'not seeing that requested library %s has been built because file %s does not exist' % (lib, shared.Cache.get_path(lib))

def build_port(port_name, lib_name, params):
  build('''
    int main() {}
  ''', [os.path.join('ports-builds', port_name, lib_name)] if lib_name else None, params)

operation = sys.argv[1]

if operation == 'build':
  tasks = sys.argv[2:]
  if 'ALL' in tasks:
    tasks = ['libc', 'libc-mt', 'dlmalloc', 'dlmalloc_threadsafe', 'pthreads', 'libcxx', 'libcxx_noexcept', 'libcxxabi', 'gl', 'binaryen', 'bullet', 'freetype', 'libpng', 'ogg', 'sdl2', 'sdl2-image', 'sdl2-ttf', 'sdl2-net', 'vorbis', 'zlib']
    if shared.Settings.WASM_BACKEND:
      skip_tasks = {'libc-mt', 'dlmalloc_threadsafe', 'pthreads'}
      print('Skipping building of %s, because WebAssembly does not support pthreads.' % ', '.join(skip_tasks))
      tasks = [x for x in tasks if x not in skip_tasks]
    if os.environ.get('EMSCRIPTEN_NATIVE_OPTIMIZER'):
      print 'Skipping building of native-optimizer since environment variable EMSCRIPTEN_NATIVE_OPTIMIZER is present and set to point to a prebuilt native optimizer path.'
    elif hasattr(shared, 'EMSCRIPTEN_NATIVE_OPTIMIZER'):
      print 'Skipping building of native-optimizer since .emscripten config file has set EMSCRIPTEN_NATIVE_OPTIMIZER to point to a prebuilt native optimizer path.'
    else:
      tasks += ['native_optimizer']
  for what in tasks:
    shared.logging.info('building and verifying ' + what)
    if what in ('libc', 'dlmalloc'):
      build('''
        #include <string.h>
        #include <stdlib.h>
        int main() {
          return int(malloc(10)) + int(strchr("str", 'c'));
        }
      ''', ['libc.bc', 'dlmalloc.bc'])
    elif what in 'wasm-libc':
      build('''
        #include <string.h>
        int main() {
          return int(strchr("str", 'c'));
        }
      ''', ['wasm-libc.bc'], ['-s', 'WASM=1'])
    elif what in ('libc-mt', 'pthreads', 'dlmalloc_threadsafe'):
      build('''
        #include <string.h>
        #include <stdlib.h>
        int main() {
          return int(malloc(10)) + int(strchr("str", 'c'));
        }
      ''', ['libc-mt.bc', 'dlmalloc_threadsafe.bc', 'pthreads.bc'], ['-s', 'USE_PTHREADS=1'])
    elif what == 'libcxx':
      build('''
        #include <iostream>
        int main() {
          std::cout << "hello";
          return 0;
        }
      ''', ['libcxx.a'])
    elif what == 'libcxx_noexcept':
      build('''
        #include <iostream>
        int main() {
          std::cout << "hello";
          return 0;
        }
      ''', ['libcxx_noexcept.a'], ['-s', 'DISABLE_EXCEPTION_CATCHING=1'])
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
      build('''
        int main() {}
      ''', ['optimizer.2.exe'], ['-O2'])
    elif what == 'wasm_compiler_rt':
      if shared.Settings.WASM_BACKEND:
        build('''
          int main() {}
        ''', ['wasm_compiler_rt.a'], ['-s', 'WASM=1'])
      else:
        shared.logging.warning('wasm_compiler_rt not built when using JSBackend')
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
    elif what == 'sdl2-image':
      build_port('sdl2-image', 'libsdl2_image.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'])
    elif what == 'sdl2-net':
      build_port('sdl2-net', 'libsdl2_net.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2'])
    elif what == 'freetype':
      build_port('freetype', 'libfreetype.a', ['-s', 'USE_FREETYPE=1'])
    elif what == 'sdl2-ttf':
      build_port('sdl2-ttf', 'libsdl2_ttf.bc', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_TTF=2', '-s', 'USE_FREETYPE=1'])
    elif what == 'binaryen':
      build_port('binaryen', None, ['-s', 'WASM=1'])
    elif what == 'cocos2d':
      build_port('cocos2d', None, ['-s', 'USE_COCOS2D=3', '-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1'])
    else:
      shared.logging.error('unfamiliar build target: ' + what)
      sys.exit(1)

    shared.logging.info('...success')

else:
  shared.logging.error('unfamiliar operation: ' + operation)
  sys.exit(1)
