# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import shutil
import unittest

from color_runner import BufferingMixin, ColorTextRunner

from tools.colored_logger import CYAN, GREEN, RED, with_color


def clearline(stream):
  stream.write('\r\033[K')
  stream.flush()


def term_width():
  return shutil.get_terminal_size()[0]


class SingleLineTestResult(BufferingMixin, unittest.TextTestResult):
  """Similar to the standard TextTestResult but uses ANSI escape codes
  for color output and reusing a single line on the terminal.
  """

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.progress_counter = 0

  def writeStatusLine(self, line):
    clearline(self._original_stderr)
    self._original_stderr.write(line)
    self._original_stderr.flush()

  def updateStatus(self, test, msg, color):
    progress = f'[{self.progress_counter}/{self.test_count}] '
    # Format the line so that it fix within the terminal width, unless it's less then min_len
    # in which case there is not much we can do, and we just overflow the line.
    min_len = len(progress) + len(msg) + 5
    test_name = str(test)
    if term_width() > min_len:
      max_name = term_width() - min_len
      test_name = test_name[:max_name]
    line = f'{with_color(CYAN, progress)}{test_name} ... {with_color(color, msg)}'
    self.writeStatusLine(line)

  def startTest(self, test):
    self.progress_counter += 1
    assert self.test_count > 0
    # Note: We explicitly do not use `super()` here but instead call `unittest.TestResult`. i.e.
    # we skip the superclass (since we don't want its specific behaviour) and instead call its
    # superclass.
    unittest.TestResult.startTest(self, test)
    if self.progress_counter == 1:
      self.updateStatus(test, '', GREEN)

  def addSuccess(self, test):
    unittest.TestResult.addSuccess(self, test)
    self.updateStatus(test, 'ok', GREEN)

  def addFailure(self, test, err):
    unittest.TestResult.addFailure(self, test, err)
    self.updateStatus(test, 'FAIL', RED)

  def addError(self, test, err):
    unittest.TestResult.addError(self, test, err)
    self.updateStatus(test, 'ERROR', RED)

  def addExpectedFailure(self, test, err):
    unittest.TestResult.addExpectedFailure(self, test, err)
    self.updateStatus(test, 'expected failure', RED)

  def addUnexpectedSuccess(self, test, err):
    unittest.TestResult.addUnexpectedSuccess(self, test, err)
    self.updateStatus(test, 'UNEXPECTED SUCCESS', RED)

  def addSkip(self, test, reason):
    unittest.TestResult.addSkip(self, test, reason)
    self.updateStatus(test, f"skipped '{reason}'", CYAN)

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
