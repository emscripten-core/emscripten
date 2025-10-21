# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utilities for mapping browser versions to webassembly features."""

import logging
from enum import IntEnum, auto

from . import diagnostics
from .settings import settings, user_settings

logger = logging.getLogger('feature_matrix')

UNSUPPORTED = 0x7FFFFFFF

# Oldest support browser versions.
# Emscripten unconditionally requires support for:
# - DedicatedWorkerGlobalScope.name parameter for multithreading support, which
#   landed first in Chrome 70, Firefox 55 and Safari 12.2.

# N.b. when modifying these values, update comments in src/settings.js on
# MIN_x_VERSION fields to match accordingly.
OLDEST_SUPPORTED_CHROME = 74  # Released on 2019-04-23
OLDEST_SUPPORTED_FIREFOX = 68  # Released on 2019-07-09
OLDEST_SUPPORTED_SAFARI = 120200  # Released on 2019-03-25
# 12.22.09 is the oldest version of node that we do any testing with.
# Keep this in sync with the test-node-compat in .circleci/config.yml.
OLDEST_SUPPORTED_NODE = 122209


class Feature(IntEnum):
  MUTABLE_GLOBALS = auto()
  NON_TRAPPING_FPTOINT = auto()
  SIGN_EXT = auto()
  BULK_MEMORY = auto()
  JS_BIGINT_INTEGRATION = auto()
  THREADS = auto()
  PROMISE_ANY = auto()
  MEMORY64 = auto()
  WORKER_ES6_MODULES = auto()
  OFFSCREENCANVAS_SUPPORT = auto()
  WASM_LEGACY_EXCEPTIONS = auto()
  WASM_EXCEPTIONS = auto()


disable_override_features = set()
enable_override_features = set()

min_browser_versions = {
  Feature.MUTABLE_GLOBALS: {
    'chrome': 74,
    'firefox': 61,
    'safari': 130100,
    'node': 120000,
  },
  Feature.NON_TRAPPING_FPTOINT: {
    'chrome': 75,
    'firefox': 65,
    'safari': 150000,
    'node': 130000,
  },
  Feature.SIGN_EXT: {
    'chrome': 74,
    'firefox': 62,
    'safari': 140100,
    'node': 120000,
  },
  Feature.BULK_MEMORY: {
    'chrome': 75,
    'firefox': 79,
    'safari': 150000,
    'node': 130000,
  },
  Feature.JS_BIGINT_INTEGRATION: {
    'chrome': 67,
    'firefox': 78,
    'safari': 150000,
    'node': 130000,
  },
  Feature.THREADS: {
    'chrome': 74,
    'firefox': 79,
    'safari': 140100,
    'node': 160400,
  },
  Feature.PROMISE_ANY: {
    'chrome': 85,
    'firefox': 79,
    'safari': 140000,
    'node': 150000,
  },
  Feature.MEMORY64: {
    'chrome': 128,
    'firefox': 129,
    'safari': UNSUPPORTED,
    'node': 230000,
  },
  # https://caniuse.com/mdn-api_worker_worker_ecmascript_modules: The ability to
  # call new Worker(url, { type: 'module' });
  Feature.WORKER_ES6_MODULES: {
    'chrome': 80,
    'firefox': 114,
    'safari': 150000,
    'node': 0, # This is a browser only feature, no requirements on Node.js
  },
  # OffscreenCanvas feature allows creating canvases that are not connected to
  # a visible DOM element, e.g. in a Worker.
  # https://caniuse.com/offscreencanvas
  Feature.OFFSCREENCANVAS_SUPPORT: {
    'chrome': 69,
    'firefox': 105,
    'safari': 170000,
    'node': 0, # This is a browser only feature, no requirements on Node.js
  },
  # Legacy Wasm exceptions was the first (now legacy) format for native
  # exception handling in WebAssembly.
  Feature.WASM_LEGACY_EXCEPTIONS: {
    'chrome': 95,
    'firefox': 100,
    'safari': 150200,
    'node': 170000,
  },
  # Wasm exceptions is a newer format for native exception handling in
  # WebAssembly.
  Feature.WASM_EXCEPTIONS: {
    'chrome': 137,
    'firefox': 131,
    'safari': 180400,
    # Supported with flag --experimental-wasm-exnref (TODO: Change this to
    # unflagged version of Node.js 260000 that ships Wasm EH enabled, after
    # Emscripten unit testing has migrated to Node.js 26, and Emsdk ships
    # Node.js 26)
    'node': 240000,
  },
}

# Static assertion to check that we actually need each of the above feature flags
# Once the OLDEST_SUPPORTED_XX versions are high enough they can/should be removed.
for feature, reqs in min_browser_versions.items():
  always_present = (reqs['chrome'] <= OLDEST_SUPPORTED_CHROME and
                    reqs['firefox'] <= OLDEST_SUPPORTED_FIREFOX and
                    reqs['safari'] <= OLDEST_SUPPORTED_SAFARI and
                    reqs['node'] <= OLDEST_SUPPORTED_NODE)
  assert not always_present, f'{feature.name} is no longer needed'


def caniuse(feature):
  if feature in disable_override_features:
    return False
  if feature in enable_override_features:
    return True

  min_versions = min_browser_versions[feature]

  def report_missing(setting_name):
    setting_value = getattr(settings, setting_name)
    logger.debug(f'cannot use {feature.name} because {setting_name} is too old: {setting_value}')

  if settings.MIN_CHROME_VERSION < min_versions['chrome']:
    report_missing('MIN_CHROME_VERSION')
    return False
  if settings.MIN_FIREFOX_VERSION < min_versions['firefox']:
    report_missing('MIN_FIREFOX_VERSION')
    return False
  if settings.MIN_SAFARI_VERSION < min_versions['safari']:
    report_missing('MIN_SAFARI_VERSION')
    return False
  if 'node' in min_versions and settings.MIN_NODE_VERSION < min_versions['node']:
    report_missing('MIN_NODE_VERSION')
    return False
  return True


def enable_feature(feature, reason, override=False):
  """Updates default settings for browser versions such that the given
  feature is available everywhere.
  """
  if override:
    enable_override_features.add(feature)
  for name, min_version in min_browser_versions[feature].items():
    name = f'MIN_{name.upper()}_VERSION'
    if settings[name] < min_version:
      if name in user_settings:
        # If the user explicitly chose an older version we issue a warning.
        if name == 'MIN_SAFARI_VERSION' and reason == 'pthreads':
          # But as a special case, don't warn when forcing on bulk memory on Safari.
          # This is because Safari implemented part of bulk memory along with threads in 14.1,
          # but not all of it. So bulk-mem is listed as supported in 15.0. So we want to
          # continue enabling bulk memory via pthreads without a warning in 14.1, but without
          # enabling other features requiring 15.0.
          continue
        diagnostics.warning(
            'compatibility',
            f'{name}={user_settings[name]} is not compatible with {reason} '
            f'({name}={min_version} or above required)')
      else:
        # If no conflict, bump the minimum version to accommodate the feature.
        logger.debug(f'Enabling {name}={min_version} to accommodate {reason}')
        setattr(settings, name, min_version)


def disable_feature(feature):
  """Allow the user to disable a feature that would otherwise be on by default.
  """
  disable_override_features.add(feature)


# apply minimum browser version defaults based on user settings. if
# a user requests a feature that we know is only supported in browsers
# from a specific version and above, we can assume that browser version.
def apply_min_browser_versions():
  if settings.WASM_BIGINT and 'WASM_BIGINT' in user_settings:
    # WASM_BIGINT is enabled by default, don't use it to enable other features
    # unless the user explicitly enabled it.
    enable_feature(Feature.JS_BIGINT_INTEGRATION, 'WASM_BIGINT')
  if settings.PTHREADS:
    enable_feature(Feature.THREADS, 'pthreads')
    enable_feature(Feature.BULK_MEMORY, 'pthreads')
  elif settings.WASM_WORKERS or settings.SHARED_MEMORY:
    enable_feature(Feature.BULK_MEMORY, 'shared-mem')
  if settings.RELOCATABLE:
    enable_feature(Feature.MUTABLE_GLOBALS, 'dynamic linking')
  if settings.MEMORY64 == 1:
    enable_feature(Feature.MEMORY64, 'MEMORY64')
  if settings.EXPORT_ES6 and settings.PTHREADS:
    enable_feature(Feature.WORKER_ES6_MODULES, 'EXPORT_ES6 with -pthread')
  if settings.EXPORT_ES6 and settings.WASM_WORKERS:
    enable_feature(Feature.WORKER_ES6_MODULES, 'EXPORT_ES6 with -sWASM_WORKERS')
  if settings.OFFSCREENCANVAS_SUPPORT:
    enable_feature(Feature.OFFSCREENCANVAS_SUPPORT, 'OFFSCREENCANVAS_SUPPORT')
  if settings.WASM_EXCEPTIONS or settings.SUPPORT_LONGJMP == 'wasm': # Wasm longjmp support will lean on Wasm (Legacy) EH
    if settings.WASM_LEGACY_EXCEPTIONS:
      enable_feature(Feature.WASM_LEGACY_EXCEPTIONS, 'Wasm Legacy exceptions (-fwasm-exceptions with -sWASM_LEGACY_EXCEPTIONS=1)')
    else:
      enable_feature(Feature.WASM_EXCEPTIONS, 'Wasm exceptions (-fwasm-exceptions with -sWASM_LEGACY_EXCEPTIONS=0)')
