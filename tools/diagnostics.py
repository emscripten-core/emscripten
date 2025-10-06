# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Simple color-enabled diagnositics reporting functions.
"""

import logging
import os
import sys
from typing import Dict

from . import colored_logger

color_enabled = colored_logger.ansi_color_available()
logger = logging.getLogger('diagnostics')
tool_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

# diagnostic levels
WARN = 1
ERROR = 2

# available (ANSI) colors
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


def output_color(color):
  assert color_enabled
  return '\033[3%sm' % color


def bold():
  assert color_enabled
  return '\033[1m'


def reset_color():
  assert color_enabled
  return '\033[0m'


def with_color(color, text):
  return output_color(color) + text + reset_color()


def diag(level, msg, *args):
  # Format output message as:
  # <tool>: <level>: msg
  # With the `<level>:` part being colored accordingly.
  sys.stderr.write(tool_name + ': ')

  if color_enabled:
    output = output_color(level_colors[level]) + bold()
    if output:
      sys.stderr.write(output)

  sys.stderr.write(level_prefixes[level])

  if color_enabled:
    output = reset_color() + bold()
    if output:
      sys.stderr.write(output)

  if args:
    msg = msg % args
  sys.stderr.write(str(msg))
  sys.stderr.write('\n')

  if color_enabled:
    output = reset_color()
    if output:
      sys.stderr.write(output)


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


def is_enabled(name):
  return manager.warnings[name]['enabled']


def warning(warning_type, message, *args):
  manager.warning(warning_type, message, *args)


def capture_warnings(argv):
  return manager.capture_warnings(argv)


manager = WarningManager()
