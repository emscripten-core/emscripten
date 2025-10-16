import common
import os
import unittest

EMTEST_RETRY_COUNT = int(os.getenv('EMTEST_RETRY_COUNT', '0'))


class RetryableTestCase(unittest.TestCase):
  '''This class patches in to the Python unittest TestCase object to incorporate
  support for an environment variable EMTEST_RETRY_COUNT=x, which enables a
  failed test to be automatically re-run to test if the failure might have been
  due to an instability.'''

  def run(self, result=None):
    retries_left = EMTEST_RETRY_COUNT

    num_fails = len(result.failures)
    num_errors = len(result.errors)

    while retries_left >= 0:
      super().run(result)

      # The test passed if it didn't accumulate an error.
      if len(result.failures) == num_fails and len(result.errors) == num_errors:
        return

      retries_left -= 1
      if retries_left >= 0:
        if len(result.failures) != num_fails:
          err = result.failures.pop(-1)
        elif len(result.errors) != num_errors:
          err = result.errors.pop(-1)
        else:
          raise Exception('Internal error in RetryableTestCase: did not detect an error')

        common.record_flaky_test(self.id(), EMTEST_RETRY_COUNT - retries_left, EMTEST_RETRY_COUNT, str(err))
