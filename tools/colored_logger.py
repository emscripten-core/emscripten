# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Enables colored logger just by importing this module
"""

import ctypes
import sys
import logging


def ansi_color_available():
  if not sys.platform.startswith('win'):
    return sys.stderr.isatty()

  # Constants from the Windows API
  STD_OUTPUT_HANDLE = -11
  ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004

  kernel32 = ctypes.windll.kernel32
  stdout_handle = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)

  # Get the current console mode
  console_mode = ctypes.c_uint()
  if not kernel32.GetConsoleMode(stdout_handle, ctypes.byref(console_mode)):
    # Handle error if GetConsoleMode fails
    return False

  # Check if the flag is set in the current console mode
  return (console_mode.value & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0


def add_coloring_to_emit_ansi(fn):
  # add methods we need to the class
  def new(*args):
    levelno = args[1].levelno
    if levelno >= 40:
      color = '\x1b[31m' # red
    elif levelno >= 30:
      color = '\x1b[33m' # yellow
    elif levelno >= 20:
      color = '\x1b[32m' # green
    elif levelno >= 10:
      color = '\x1b[35m' # pink
    else:
      color = '\x1b[0m' # normal
    args[1].msg = color + args[1].msg + '\x1b[0m'  # normal
    return fn(*args)

  new.orig_func = fn
  return new


def enable():
  if ansi_color_available():
    logging.StreamHandler.emit = add_coloring_to_emit_ansi(logging.StreamHandler.emit)


def disable():
  if hasattr(logging.StreamHandler.emit, 'orig_func'):
    logging.StreamHandler.emit = logging.StreamHandler.emit.orig_func
