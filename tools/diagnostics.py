# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Simple color-enabled diagnositics reporting functions.
"""

import ctypes
import logging
import os
import sys
from typing import Dict


WINDOWS = sys.platform.startswith('win')

logger = logging.getLogger('diagnostics')
color_enabled = sys.stderr.isatty()
tool_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

# diagnostic levels
WARN = 1
ERROR = 2
FATAL = 3

# available colors
RED = 1
GREEN = 2
YELLOW = 3
BLUE = 4
MAGENTA = 5
CYAN = 6
WHITE = 7

# color for use for each diagnostic level
level_colors = {
    WARN: MAGENTA,
    ERROR: RED,
}

level_prefixes = {
    WARN: 'warning: ',
    ERROR: 'error: ',
}

# Constants from the Windows API
STD_OUTPUT_HANDLE = -11


def output_color_windows(color):
  # TODO(sbc): This code is duplicated in colored_logger.py.  Refactor.
  # wincon.h
  FOREGROUND_BLACK     = 0x0000 # noqa
  FOREGROUND_BLUE      = 0x0001 # noqa
  FOREGROUND_GREEN     = 0x0002 # noqa
  FOREGROUND_CYAN      = 0x0003 # noqa
  FOREGROUND_RED       = 0x0004 # noqa
  FOREGROUND_MAGENTA   = 0x0005 # noqa
  FOREGROUND_YELLOW    = 0x0006 # noqa
  FOREGROUND_GREY      = 0x0007 # noqa

  color_map = {
    RED: FOREGROUND_RED,
    GREEN: FOREGROUND_GREEN,
    YELLOW: FOREGROUND_YELLOW,
    BLUE: FOREGROUND_BLUE,
    MAGENTA: FOREGROUND_MAGENTA,
    CYAN: FOREGROUND_CYAN,
    WHITE: FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
  }

  sys.stderr.flush()
  hdl = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
  ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, color_map[color])


def get_color_windows():
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


def reset_color_windows():
  sys.stderr.flush()
  hdl = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
  ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, default_color)


def output_color(color):
  if WINDOWS:
    return output_color_windows(color)
  return '\033[3%sm' % color


def reset_color():
  if WINDOWS:
    return reset_color_windows()
  return '\033[0m'


def diag(level, msg, *args):
  # Format output message as:
  # <tool>: <level>: msg
  # With the `<level>:` part being colored accordingly.
  sys.stderr.write(tool_name + ': ')

  if color_enabled:
    output = output_color(level_colors[level])
    if output:
      sys.stderr.write(output)

  sys.stderr.write(level_prefixes[level])

  if color_enabled:
    output = reset_color()
    if output:
      sys.stderr.write(output)

  if args:
    msg = msg % args
  sys.stderr.write(str(msg))
  sys.stderr.write('\n')


def error(msg, *args):
  diag(ERROR, msg, *args)
  sys.exit(1)


def warn(msg, *args):
  diag(WARN, msg, *args)


class WarningManager:
  warnings: Dict[str, Dict] = {}

  def add_warning(self, name, enabled=True, part_of_all=True, shared=False, error=False):
    self.warnings[name] = {
      'enabled': enabled,
      'part_of_all': part_of_all,
      # True for flags that are shared with the underlying clang driver
      'shared': shared,
      'error': error,
    }

  def capture_warnings(self, cmd_args):
    for i in range(len(cmd_args)):
      if cmd_args[i] == '-w':
        for warning in self.warnings.values():
          warning['enabled'] = False
        continue

      if not cmd_args[i].startswith('-W'):
        continue

      if cmd_args[i] == '-Wall':
        for warning in self.warnings.values():
          if warning['part_of_all']:
            warning['enabled'] = True
        continue

      if cmd_args[i] == '-Werror':
        for warning in self.warnings.values():
          warning['error'] = True
        continue

      if cmd_args[i].startswith('-Werror=') or cmd_args[i].startswith('-Wno-error='):
        warning_name = cmd_args[i].split('=', 1)[1]
        if warning_name in self.warnings:
          enabled = not cmd_args[i].startswith('-Wno-')
          self.warnings[warning_name]['error'] = enabled
          if enabled:
            self.warnings[warning_name]['enabled'] = True
          cmd_args[i] = ''
          continue

      warning_name = cmd_args[i].replace('-Wno-', '').replace('-W', '')
      enabled = not cmd_args[i].startswith('-Wno-')

      # special case pre-existing warn-absolute-paths
      if warning_name == 'warn-absolute-paths':
        self.warnings['absolute-paths']['enabled'] = enabled
        cmd_args[i] = ''
        continue

      if warning_name in self.warnings:
        self.warnings[warning_name]['enabled'] = enabled
        if not self.warnings[warning_name]['shared']:
          cmd_args[i] = ''
        continue

    return cmd_args

  def warning(self, warning_type, message, *args):
    warning_info = self.warnings[warning_type]
    msg = (message % args) + ' [-W' + warning_type.lower().replace('_', '-') + ']'
    if warning_info['enabled']:
      if warning_info['error']:
        error(msg + ' [-Werror]')
      else:
        warn(msg)
    else:
      logger.debug('disabled warning: ' + msg)


def add_warning(name, enabled=True, part_of_all=True, shared=False, error=False):
  manager.add_warning(name, enabled, part_of_all, shared, error)


def enable_warning(name, as_error=False):
  manager.warnings[name]['enabled'] = True
  if as_error:
    manager.warnings[name]['error'] = True


def disable_warning(name):
  manager.warnings[name]['enabled'] = False


def is_enabled(name):
  return manager.warnings[name]['enabled']


def warning(warning_type, message, *args):
  manager.warning(warning_type, message, *args)


def capture_warnings(argv):
  return manager.capture_warnings(argv)


if WINDOWS:
  default_color = get_color_windows()

manager = WarningManager()
