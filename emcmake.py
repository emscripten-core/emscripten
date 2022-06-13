#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import sys
from tools import shared
from tools import config
from tools import utils
from subprocess import CalledProcessError


#
# Main run() function
#
def run():
  if len(sys.argv) < 2 or sys.argv[1] in ('--version', '--help'):
    print('''\
emcmake is a helper for cmake, setting various environment
variables so that emcc etc. are used. Typical usage:

  emcmake cmake [FLAGS]
''', file=sys.stderr)
    return 1

  args = sys.argv[1:]

  def has_substr(args, substr):
    return any(substr in s for s in args)

  # Append the Emscripten toolchain file if the user didn't specify one.
  if not has_substr(args, '-DCMAKE_TOOLCHAIN_FILE'):
    args.append('-DCMAKE_TOOLCHAIN_FILE=' + utils.path_from_root('cmake/Modules/Platform/Emscripten.cmake'))

  if not has_substr(args, '-DCMAKE_CROSSCOMPILING_EMULATOR'):
    node_js = config.NODE_JS[0]
    # In order to allow cmake to run code built with pthreads we need to pass some extra flags to node.
    # Note that we also need --experimental-wasm-bulk-memory which is true by default and hence not added here
    # See https://github.com/emscripten-core/emscripten/issues/15522
    args.append(f'-DCMAKE_CROSSCOMPILING_EMULATOR={node_js};--experimental-wasm-threads')

  # On Windows specify MinGW Makefiles or ninja if we have them and no other
  # toolchain was specified, to keep CMake from pulling in a native Visual
  # Studio, or Unix Makefiles.
  if utils.WINDOWS and not any(arg.startswith('-G') for arg in args):
    if utils.which('mingw32-make'):
      args += ['-G', 'MinGW Makefiles']
    elif utils.which('ninja'):
      args += ['-G', 'Ninja']
    else:
      print('emcmake: no compatible cmake generator found; Please install ninja or mingw32-make, or specify a generator explicitly using -G', file=sys.stderr)
      return 1

  print('configure: ' + shared.shlex_join(args), file=sys.stderr)
  try:
    shared.check_call(args)
    return 0
  except CalledProcessError as e:
    return e.returncode


if __name__ == '__main__':
  sys.exit(run())
