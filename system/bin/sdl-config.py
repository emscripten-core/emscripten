#!/usr/bin/env python3

from __future__ import print_function
import sys

print('emscripten sdl-config called with', ' '.join(sys.argv), file=sys.stderr)

args = sys.argv[1:]

if args[0] == '--cflags':
  print('')
elif '--version' in args:
  print('1.3.0')

