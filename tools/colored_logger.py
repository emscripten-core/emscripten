# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Enables colored logger just by importing this module

Also, provides utiliy functions to use ANSI colors in the terminal.
"""

import ctypes
import logging
import sys
from functools import wraps

# ANSI colors
RED = 1
GREEN = 2
YELLOW = 3
BLUE = 4
MAGENTA = 5
CYAN = 6
WHITE = 7

color_enabled = False


def output_color(color):
  if color_enabled:
    return '\033[3%sm' % color
  return ''


def bold():
  if color_enabled:
    return '\033[1m'
  return ''


def reset_color():
  if color_enabled:
    return '\033[0m'
  return ''


def with_bold_color(color, string):
  return output_color(color) + bold() + string + reset_color()


def with_color(color, string):
  return output_color(color) + string + reset_color()


def with_bold(string):
  return bold() + string + reset_color()


def ansi_color_available():
  if not sys.platform.startswith('win'):
    return sys.stderr.isatty()

  # Constants from the Windows API
  STD_OUTPUT_HANDLE = -11
  ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004

  kernel32 = ctypes.windll.kernel32
  stdout_handle = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)

  console_mode = ctypes.c_uint()
  # Attempt to enable ANSI color processing (ENABLE_VIRTUAL_TERMINAL_PROCESSING).
  # Assume that failure of either GetConsoleMode or SetConsoleMode means that stdout
  # is not attached to a terminal or that the terminal does not support this mode.
  if kernel32.GetConsoleMode(stdout_handle, ctypes.byref(console_mode)) == 0:
    return False
  if kernel32.SetConsoleMode(stdout_handle, console_mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0:
    return False

  return True


def add_coloring_to_emit_ansi(fn):
  # add methods we need to the class
  @wraps(fn)
  def new(*args):
    levelno = args[1].levelno
    color = None
    if levelno >= 40:
      color = RED
    elif levelno >= 30:
      color = YELLOW
    elif levelno >= 20:
      color = GREEN
    elif levelno >= 10:
      color = MAGENTA
    if color:
      args[1].msg = with_color(color, args[1].msg)
    return fn(*args)

  new.orig_func = fn
  return new


def enable(force=False):
  global color_enabled
  if not color_enabled:
    if ansi_color_available() or force:
      logging.StreamHandler.emit = add_coloring_to_emit_ansi(logging.StreamHandler.emit)
      color_enabled = True


def disable():
  global color_enabled
  if color_enabled:
    if hasattr(logging.StreamHandler.emit, 'orig_func'):
      logging.StreamHandler.emit = logging.StreamHandler.emit.orig_func
    color_enabled = False
