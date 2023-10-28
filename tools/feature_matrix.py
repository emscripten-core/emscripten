# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utilities for mapping browser versions to webassembly features."""

import logging

from .settings import settings, user_settings
from . import diagnostics

from json import load
from functools import reduce
from operator import getitem
from pathlib import Path

logger = logging.getLogger('feature_matrix')

with open(Path(__file__).parent / 'browser_compat_data.json', 'r') as f:
  browser_compat_data = load(f)


# This value is used to indicate that a feature is not supported
# in a given browser (by setting minimum versions to INT32_MAX).
TARGET_NOT_SUPPORTED = 0x7FFFFFFF


def get_min_versions(path):
  min_versions = reduce(getitem, path.split('.'), browser_compat_data)['#']
  return {
    'MIN_CHROME_VERSION': min_versions.get('chrome', TARGET_NOT_SUPPORTED),
    'MIN_FIREFOX_VERSION': min_versions.get('firefox', TARGET_NOT_SUPPORTED),
    'MIN_SAFARI_VERSION': min_versions.get('safari', TARGET_NOT_SUPPORTED),
    'MIN_NODE_VERSION': min_versions.get('nodejs', TARGET_NOT_SUPPORTED),
  }


def caniuse(path):
  min_versions = get_min_versions(path)

  def check_version(setting_name):
    setting_value = settings[setting_name]
    if setting_value < min_versions[setting_name]:
      logger.debug(f'cannot use {path} because {setting_name} is too old: {setting_value}')
      return False
    return True

  return all(check_version(setting_name) for setting_name in min_versions)


def enable_feature(path):
  """Updates default settings for browser versions such that the given
  feature is available everywhere.
  """
  min_versions = get_min_versions(path)
  for name, min_version in min_versions.items():
    if settings[name] < min_version:
      if name in user_settings:
        # If the user explicitly chose an older version we issue a warning.
        diagnostics.warning(
            'compatibility',
            f'{name}={user_settings[name]} is not compatible with {path} '
            f'({min_version} or above required)')
      else:
        # Otherwise we bump the minimum version to accommodate the feature.
        settings[name] = min_version


# apply minimum browser version defaults based on user settings. if
# a user requests a feature that we know is only supported in browsers
# from a specific version and above, we can assume that browser version.
def apply_min_browser_versions():
  if settings.WASM_BIGINT:
    enable_feature('wasm.bigInt')
  if settings.PTHREADS:
    enable_feature('wasm.threads')
  # if settings.AUDIO_WORKLET:
  #   enable_feature(Feature.GLOBALTHIS, 'AUDIO_WORKLET')
