# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utilities for mapping browser versions to webassembly features."""

import logging
from enum import IntEnum, auto

from .settings import settings, user_settings
from . import diagnostics

logger = logging.getLogger('feature_matrix')


class Feature(IntEnum):
  NON_TRAPPING_FPTOINT = auto()
  SIGN_EXT = auto()
  BULK_MEMORY = auto()
  MUTABLE_GLOBALS = auto()
  JS_BIGINT_INTEGRATION = auto()
  THREADS = auto()
  GLOBALTHIS = auto()


default_features = {Feature.SIGN_EXT, Feature.MUTABLE_GLOBALS}

min_browser_versions = {
  Feature.NON_TRAPPING_FPTOINT: {
    'chrome': 75,
    'firefox': 65,
    'safari': 150000,
  },
  Feature.SIGN_EXT: {
    'chrome': 74,
    'firefox': 62,
    'safari': 140100,
  },
  Feature.BULK_MEMORY: {
    'chrome': 75,
    'firefox': 79,
    'safari': 150000,
  },
  Feature.MUTABLE_GLOBALS: {
    'chrome': 74,
    'firefox': 61,
    'safari': 120000,
  },
  Feature.JS_BIGINT_INTEGRATION: {
    'chrome': 67,
    'firefox': 68,
    'safari': 150000,
  },
  Feature.THREADS: {
    'chrome': 74,
    'firefox': 79,
    'safari': 140100,
  },
  Feature.GLOBALTHIS: {
    'chrome': 71,
    'edge': 79,
    'firefox': 65,
    'safari': 120100,
    # 'node': 120000
  },
}


def caniuse(feature):
  min_versions = min_browser_versions[feature]
  if settings.MIN_CHROME_VERSION < min_versions['chrome']:
    return False
  # For edge we just use the same version requirements as chrome since,
  # at least for modern versions of edge, they share version numbers.
  if settings.MIN_EDGE_VERSION < min_versions['chrome']:
    return False
  if settings.MIN_FIREFOX_VERSION < min_versions['firefox']:
    return False
  if settings.MIN_SAFARI_VERSION < min_versions['safari']:
    return False
  # IE don't support any non-MVP features
  if settings.MIN_IE_VERSION != 0x7FFFFFFF:
    return False
  return True


def enable_feature(feature, reason):
  """Updates default settings for browser versions such that the given
  feature is available everywhere.
  """
  for name, min_version in min_browser_versions[feature].items():
    name = f'MIN_{name.upper()}_VERSION'
    if settings[name] < min_version:
      if name in user_settings:
        # If the user explicitly chose an older version we issue a warning.
        diagnostics.warning(
            'compatibility',
            f'{name}={user_settings[name]} is not compatible with {reason} '
            f'({min_version} or above required)')
      else:
        # Otherwise we bump the minimum version to accommodate the feature.
        setattr(settings, name, min_version)


# apply minimum browser version defaults based on user settings. if
# a user requests a feature that we know is only supported in browsers
# from a specific version and above, we can assume that browser version.
def apply_min_browser_versions():
  if settings.WASM_BIGINT:
    enable_feature(Feature.JS_BIGINT_INTEGRATION, 'WASM_BIGINT')
  if settings.USE_PTHREADS:
    enable_feature(Feature.THREADS, 'pthreads')
    enable_feature(Feature.BULK_MEMORY, 'pthreads')
