import unittest
import os
import sys

# Ensure repo root is on sys.path so `tools` can be imported when running tests directly
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
if REPO_ROOT not in sys.path:
  sys.path.insert(0, REPO_ROOT)

# Import from repo tools
from tools import settings, building


class TestFastMathFlag(unittest.TestCase):
  def setUp(self):
    # Start from a clean settings snapshot for each test
    settings.settings.attrs.clear()
    settings.settings['OPT_LEVEL'] = 2
    settings.settings['SHRINK_LEVEL'] = 1

  def test_fast_math_disabled(self):
    settings.settings['FAST_MATH'] = 0
    opts = building.get_last_binaryen_opts()
    self.assertNotIn('--fast-math', opts)

  def test_fast_math_enabled(self):
    settings.settings['FAST_MATH'] = 1
    opts = building.get_last_binaryen_opts()
    self.assertIn('--fast-math', opts)


if __name__ == '__main__':
  unittest.main()


