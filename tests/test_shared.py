"""Unittests for internal shared code in test/shared.py"""

from __future__ import print_function
import os
from runner import RunnerCore

from tools import shared


class TestBuilding(RunnerCore):
  def test_is_bitcode(self):
    fname = os.path.join(self.get_dir(), 'tmp.o')

    with open(fname, 'wb') as f:
      f.write(b'foo')
    self.assertFalse(shared.Building.is_bitcode(fname))

    with open(fname, 'wb') as f:
      f.write(b'\xDE\xC0\x17\x0B')
      f.write(16 * b'\x00')
      f.write(b'BC')
    self.assertTrue(shared.Building.is_bitcode(fname))

    with open(fname, 'wb') as f:
      f.write(b'BC')
    self.assertTrue(shared.Building.is_bitcode(fname))

  def test_is_ar(self):
    fname = os.path.join(self.get_dir(), 'tmp.a')

    with open(fname, 'wb') as f:
      f.write(b'foo')
    self.assertFalse(shared.Building.is_ar(fname))

    with open(fname, 'wb') as f:
      f.write(b'!<arch>\n')
    self.assertTrue(shared.Building.is_ar(fname))
