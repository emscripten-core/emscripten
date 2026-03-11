# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Simple color-enabled diagnostics reporting functions.
"""

import logging
import os
import sys

from . import colored_logger

logger = logging.getLogger('diagnostics')
tool_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

# diagnostic levels
WARN = 1
ERROR = 2

# color for use for each diagnostic level
level_colors = {
    WARN: colored_logger.MAGENTA,
    ERROR: colored_logger.RED,
}

level_prefixes = {
    WARN: 'warning: ',
    ERROR: 'error: ',
}


def diag(level, msg, *args):
  # Format output message as:
  # <tool>: <level>: msg
  # With the `<level>:` part being colored accordingly, and the message itself in bold.
  prefix = level_prefixes[level]
  color = level_colors[level]
  if args:
    msg = msg % args

  # Add colors
  prefix = colored_logger.with_bold_color(color, prefix)
  msg = colored_logger.with_bold(msg)
  sys.stderr.write(f'{tool_name}: {prefix}{msg}\n')


def error(msg, *args):
  diag(ERROR, msg, *args)
  sys.exit(1)


def warn(msg, *args):
  diag(WARN, msg, *args)


class WarningManager:
  warnings: dict[str, dict] = {}

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

      warning_name = cmd_args[i].removeprefix('-Wno-').removeprefix('-W')
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
