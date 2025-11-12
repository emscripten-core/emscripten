# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import shutil

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
  max_status_msg = 20

  def _write_status(self, _test, status):
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
    status = status[:self.max_status_msg]
    line = f'{with_color(color, status)}'
    self.stream.write(line)
    self.stream.flush()

  def startTest(self, test):
    self.showAll = False
    super().startTest(test)
    self.showAll = True
    clearline(self.stream)
    prefix_len = self.writeProgressPrefix()
    max_desc = term_width() - prefix_len - len(' ... ') - self.max_status_msg
    desc = str(test)[:max_desc]
    self.stream.write(desc)
    self.stream.write(' ... ')
    self.stream.flush()

  def printErrors(self):
    # All tests have been run at this point so print a final newline
    # to end out status line
    self.stream.write('\n')
    super().printErrors()


class SingleLineTestRunner(ColorTextRunner):
  """Subclass of TextTestResult that uses SingleLineTestResult"""
  resultclass = SingleLineTestResult # type: ignore

  def __init__(self, *args, **kwargs):
    super().__init__(*args, buffer=True, **kwargs)
