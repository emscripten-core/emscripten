# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import shutil
import unittest

from color_runner import ColorTextResult, ColorTextRunner

from tools.colored_logger import CYAN, GREEN, RED, with_color


def clearline(stream):
  stream.write('\r\033[K')
  stream.flush()


def term_width():
  return shutil.get_terminal_size()[0]


class SingleLineTestResult(ColorTextResult):
  """Similar to the standard TextTestResult but uses ANSI escape codes
  for color output and reusing a single line on the terminal.
  """

  def writeStatus(self, test, msg, color, line_pos):
    # Because the message can include the skip reason (which can be very long sometimes), truncate
    # it to a reasonable length to avoid exceeding line length.
    if len(msg) > 30:
      msg = msg[:30]
    # Format the line so that it fix within the terminal width, unless its less then min_len
    # in which case there is not much we can do, and we just overflow the line.
    min_len = line_pos + len(msg) + 5
    test_name = str(test)
    if term_width() > min_len:
      max_name = term_width() - min_len
      test_name = test_name[:max_name]
    line = f'{test_name} ... {with_color(color, msg)}'
    self._original_stderr.write(line)

  def _write_status(self, test, status):
    clearline(self._original_stderr)
    pos = self.writeProgressPrefix()
    # Add some color to the status message
    if status == 'ok':
      color = GREEN
    elif status.isupper():
      color = RED
      # Use a newline when a test fails, so you can see a list of failures while
      # the other tests are still running
      status += '\n'
    else:
      color = CYAN
    self.writeStatus(test, status, color, pos)
    self._original_stderr.flush()

  def startTest(self, test):
    self.progress_counter += 1
    # We explictly don't call TextTestResult.startTest here since we delay all printing
    # of results until `_write_status`
    unittest.TestResult.startTest(self, test)

  def printErrors(self):
    # All tests have been run at this point so print a final newline
    # to end out status line
    self._original_stderr.write('\n')
    super().printErrors()


class SingleLineTestRunner(ColorTextRunner):
  """Subclass of TextTestResult that uses SingleLineTestResult"""
  resultclass = SingleLineTestResult # type: ignore

  def __init__(self, *args, **kwargs):
    super().__init__(*args, buffer=True, **kwargs)
