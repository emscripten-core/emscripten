# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Enables colored logger just by importing this module
"""

import ctypes
import sys
import logging


def add_coloring_to_emit_windows(fn):
  # Constants from the Windows API
  STD_OUTPUT_HANDLE = -11

  def _get_color():
    SHORT = ctypes.c_short
    WORD = ctypes.c_ushort

    class COORD(ctypes.Structure):
      _fields_ = [
        ("X", SHORT),
        ("Y", SHORT)]

    class SMALL_RECT(ctypes.Structure):
      _fields_ = [
        ("Left", SHORT),
        ("Top", SHORT),
        ("Right", SHORT),
        ("Bottom", SHORT)]

    class CONSOLE_SCREEN_BUFFER_INFO(ctypes.Structure):
      _fields_ = [
        ("dwSize", COORD),
        ("dwCursorPosition", COORD),
        ("wAttributes", WORD),
        ("srWindow", SMALL_RECT),
        ("dwMaximumWindowSize", COORD)]

    hdl = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    csbi = CONSOLE_SCREEN_BUFFER_INFO()
    ctypes.windll.kernel32.GetConsoleScreenBufferInfo(hdl, ctypes.byref(csbi))
    return csbi.wAttributes

  def _set_color(code):
    hdl = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, code)

  def new(*args):
    # wincon.h
    FOREGROUND_BLACK     = 0x0000 # noqa
    FOREGROUND_BLUE      = 0x0001 # noqa
    FOREGROUND_GREEN     = 0x0002 # noqa
    FOREGROUND_CYAN      = 0x0003 # noqa
    FOREGROUND_RED       = 0x0004 # noqa
    FOREGROUND_MAGENTA   = 0x0005 # noqa
    FOREGROUND_YELLOW    = 0x0006 # noqa
    FOREGROUND_GREY      = 0x0007 # noqa
    FOREGROUND_INTENSITY = 0x0008 # foreground color is intensified.

    FOREGROUND_WHITE     = FOREGROUND_BLUE|FOREGROUND_GREEN |FOREGROUND_RED # noqa

    BACKGROUND_BLACK     = 0x0000 # noqa
    BACKGROUND_BLUE      = 0x0010 # noqa
    BACKGROUND_GREEN     = 0x0020 # noqa
    BACKGROUND_CYAN      = 0x0030 # noqa
    BACKGROUND_RED       = 0x0040 # noqa
    BACKGROUND_MAGENTA   = 0x0050 # noqa
    BACKGROUND_YELLOW    = 0x0060 # noqa
    BACKGROUND_GREY      = 0x0070 # noqa
    BACKGROUND_INTENSITY = 0x0080 # background color is intensified.
    levelno = args[1].levelno
    if (levelno >= 50):
        color = BACKGROUND_YELLOW | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY
    elif (levelno >= 40):
        color = FOREGROUND_RED | FOREGROUND_INTENSITY
    elif (levelno >= 30):
        color = FOREGROUND_YELLOW | FOREGROUND_INTENSITY
    elif (levelno >= 20):
        color = FOREGROUND_GREEN
    elif (levelno >= 10):
        color = FOREGROUND_MAGENTA
    else:
        color = FOREGROUND_WHITE

    old_color = _get_color()
    _set_color(color)
    ret = fn(*args)
    _set_color(old_color)
    return ret

  new.orig_func = fn
  return new


def add_coloring_to_emit_ansi(fn):
  # add methods we need to the class
  def new(*args):
    levelno = args[1].levelno
    if levelno >= 50:
      color = '\x1b[31m' # red
    elif levelno >= 40:
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
  if sys.stderr.isatty():
    if sys.platform.startswith('win'):
      logging.StreamHandler.emit = add_coloring_to_emit_windows(logging.StreamHandler.emit)
    else:
      logging.StreamHandler.emit = add_coloring_to_emit_ansi(logging.StreamHandler.emit)


def disable():
  if hasattr(logging.StreamHandler.emit, 'orig_func'):
    logging.StreamHandler.emit = logging.StreamHandler.emit.orig_func
