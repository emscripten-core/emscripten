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
        libcxx
        libcxxabi
        gl
        struct_info
        native_optimizer
        zlib
        sdl2
        sdl2-image

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
  temp = temp_files.get('.cpp').name
  open(temp, 'w').write(src)
  temp_js = temp_files.get('.js').name
  shared.Building.emcc(temp, args, output_filename=temp_js)
  assert os.path.exists(temp_js), 'failed to build file'
  for lib in result_libs:
    assert os.path.exists(shared.Cache.get_path(lib)), 'not seeing that requested library %s has been built' % lib

operation = sys.argv[1]

if operation == 'build':
  for what in sys.argv[2:]:
    shared.logging.info('building and verifying ' + what)
    if what == 'libc':
      build('''
        #include <string.h>
        #include <stdlib.h>
        int main() {
          return int(malloc(10)) + int(strchr("str", 'c'));
        }
      ''', ['libc.bc', 'libcextra.bc'])
    elif what == 'libcxx':
      build('''
        #include <iostream>
        int main() {
          std::cout << "hello";
          return 0;
        }
      ''', ['libcxx.bc'])
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
    elif what == 'struct_info':
      build('''
        int main() {}
      ''', ['struct_info.compiled.json'])
    elif what == 'native_optimizer':
      build('''
        int main() {}
      ''', ['optimizer.exe'], ['-O2'])
    elif what == 'zlib':
      build('''
        int main() {}
      ''', [os.path.join('ports-builds', 'zlib', 'libz.a')], ['-s', 'USE_ZLIB=1'])
    elif what == 'sdl2':
      build('''
        int main() {}
      ''', [os.path.join('ports-builds', 'sdl2', 'libsdl2.bc')], ['-s', 'USE_SDL=2'])
    elif what == 'sdl2-image':
      build('''
        int main() {}
      ''', [os.path.join('ports-builds', 'sdl2-image', 'libsdl2_image.bc')], ['-s', 'USE_SDL=2', '-s', 'USE_SDL_IMAGE=2'])
    else:
      shared.logging.error('unfamiliar build target: ' + what)
      sys.exit(1)

    shared.logging.info('...success')

else:
  shared.logging.error('unfamiliar operation: ' + operation)
  sys.exit(1)

