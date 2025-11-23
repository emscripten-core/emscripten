#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emld - emscripten linker.

emld is a drop-in replacement for a link such as ld or wasm-ld.

See emld --help for details.
"""

import argparse
import logging
import sys
import time

from tools import cmdline, link
from tools.settings import settings
from tools.toolchain_profiler import ToolchainProfiler  # ruff: ignore[unused-import]

logger = logging.getLogger('emld')


def parse_args_emld(argv):
  """Parse additional linker-specific arguments.

  In addition to the normal emcc arguments parsing from cmdline.py,
  we need to parse additional linker-specific argument that can come
  from either the clang driver or directly from the user.
  """
  parser = argparse.ArgumentParser('emld', description=__doc__, add_help=False)
  parser.add_argument('-m', metavar='ARCH', help='target architecture, either wasm32 or wasm64')
  options = parser.parse_known_args(argv)[0]
  if options.m and options.m == 'wasm64':
    settings.default_setting('MEMORY64', 1)


def is_wasm_ld_arg(arg):
  ignore_prefixes = ['-O', '-g', '-flto', '-o', '-W']
  ignore_args = ['-fexceptions', '-fwasm-exceptions', '-pthread']

  if arg in ignore_args:
    return False

  if any(arg.startswith(p) for p in ignore_prefixes):
    return False

  return True


def get_wasm_ld_args(args):
  return [a for a in args if is_wasm_ld_arg(a)]


def run(argv):
  ## Process argument and setup the compiler
  parse_args_emld(argv[1:])
  newargs = cmdline.parse_arguments(argv[1:])
  wasm_ld_args = [a for a in newargs if is_wasm_ld_arg(a)]
  return link.run(wasm_ld_args)


def main(args):
  # shared.DEBUG = True
  start_time = time.time()
  ret = run(args)
  logger.debug('total time: %.2f seconds', (time.time() - start_time))
  return ret


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.warning('KeyboardInterrupt')
    sys.exit(1)
