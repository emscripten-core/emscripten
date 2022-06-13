"""See test_posixtest.py

This class is only in its own file so that we can
run the non-browser versions in parallel.

Currently only entire modules can be marked as
parallel.
"""

from common import BrowserCore


class posixtest_browser(BrowserCore):
  """Run the suite in the browser (serially)

  This class get populated dynamically below.
  """
  pass
