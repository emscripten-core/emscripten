# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
import unittest

from tools.colored_logger import CYAN, GREEN, RED, with_color


class BufferingMixin:
  """This class takes care of redirecting `logging` output in `buffer=True` mode.

  To use this class inherit from it along with a one of the standard unittest result
  classes.
  """
  def _setupStdout(self):
    super()._setupStdout()
    # In addition to redirecting sys.stderr and sys.stdout, also update the python
    # loggers which hold cached versions of these handles.
    if self.buffer:
      for handler in logging.root.handlers:
        if handler.stream == self._original_stderr:
          handler.stream = self._stderr_buffer

  def _restoreStdout(self):
    super()._restoreStdout()
    if self.buffer:
      for handler in logging.root.handlers:
        if handler.stream == self._stderr_buffer:
          handler.stream = self._original_stderr


class ProgressMixin:
  test_count = 0
  progress_counter = 0
  fail_count = 0

  def addError(self, test, err):
    self.fail_count += 1
    super().addError(test, err)

  def addFailure(self, test, err):
    self.fail_count += 1
    super().addFailure(test, err)

  def addUnexpectedSuccess(self, test):
    self.fail_count += 1
    super().addUnexpectedSuccess(test)

  def getProgressPrefix(self):
    base_progress = f'[{self.progress_counter}/{self.test_count}'
    rtn = len(base_progress)
    progress = with_color(CYAN, base_progress)
    if self.fail_count:
      fail_str = f'-{self.fail_count}'
      progress += with_color(RED, fail_str)
      rtn += len(fail_str)
    progress += with_color(CYAN, '] ')
    return progress, rtn + 2

  def writeProgressPrefix(self):
    progress, len = self.getProgressPrefix()
    self.stream.write(progress)
    self.stream.flush()
    return len

  def startTest(self, test):
    self.progress_counter += 1
    if self.showAll:
      self.writeProgressPrefix()
    super().startTest(test)


class ColorTextResult(BufferingMixin, ProgressMixin, unittest.TextTestResult):
  """Adds color the printed test result."""
  def _write_status(self, test, status):
    # Add some color to the status message
    if status == 'ok':
      color = GREEN
    elif status.isupper():
      color = RED
    else:
      color = CYAN
    super()._write_status(test, with_color(color, status))


class ColorTextRunner(unittest.TextTestRunner):
  """Subclass of TextTestRunner that uses ColorTextResult"""
  resultclass = ColorTextResult # type: ignore

  def __init__(self, *args, **kwargs):
    super().__init__(*args, verbosity=2, **kwargs)

  def _makeResult(self):
    result = super()._makeResult()
    result.test_count = self.test_count
    return result

  def run(self, test):
    self.test_count = test.countTestCases()
    return super().run(test)
