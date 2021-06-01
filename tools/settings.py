# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import difflib
import os
import re

from .utils import path_from_root, exit_with_error
from . import diagnostics

# Subset of settings that take a memory size (i.e. 1Gb, 64kb etc)
MEM_SIZE_SETTINGS = (
    'TOTAL_STACK',
    'INITIAL_MEMORY',
    'MEMORY_GROWTH_LINEAR_STEP',
    'MEMORY_GROWTH_GEOMETRIC_CAP',
    'GL_MAX_TEMP_BUFFER_SIZE',
    'MAXIMUM_MEMORY',
    'DEFAULT_PTHREAD_STACK_SIZE'
)

PORTS_SETTINGS = (
    # All port-related settings are valid at compile time
    'USE_SDL',
    'USE_LIBPNG',
    'USE_BULLET',
    'USE_ZLIB',
    'USE_BZIP2',
    'USE_VORBIS',
    'USE_COCOS2D',
    'USE_ICU',
    'USE_MODPLUG',
    'USE_SDL_MIXER',
    'USE_SDL_IMAGE',
    'USE_SDL_TTF',
    'USE_SDL_NET',
    'USE_SDL_GFX',
    'USE_LIBJPEG',
    'USE_OGG',
    'USE_REGAL',
    'USE_BOOST_HEADERS',
    'USE_HARFBUZZ',
    'USE_MPG123',
    'USE_GIFLIB',
    'USE_FREETYPE',
    'SDL2_MIXER_FORMATS',
    'SDL2_IMAGE_FORMATS',
)

# Subset of settings that apply at compile time.
# (Keep in sync with [compile] comments in settings.js)
COMPILE_TIME_SETTINGS = (
    'MEMORY64',
    'INLINING_LIMIT',
    'DISABLE_EXCEPTION_CATCHING',
    'DISABLE_EXCEPTION_THROWING',
    'MAIN_MODULE',
    'SIDE_MODULE',
    'RELOCATABLE',
    'STRICT',
    'EMSCRIPTEN_TRACING',
    'USE_PTHREADS',
    'SUPPORT_LONGJMP',
    'DEFAULT_TO_CXX',
    'WASM_OBJECT_FILES',

    # Internal settings used during compilation
    'EXCEPTION_CATCHING_ALLOWED',
    'EXCEPTION_HANDLING',
    'LTO',
    'OPT_LEVEL',
    'DEBUG_LEVEL',

    # This is legacy setting that we happen to handle very early on
    'RUNTIME_LINKED_LIBS',
    # TODO: should not be here
    'AUTO_ARCHIVE_INDEXES',
    'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE',
) + PORTS_SETTINGS


class SettingsManager:
  attrs = {}
  allowed_settings = []
  legacy_settings = {}
  alt_names = {}
  internal_settings = set()

  def __init__(self):
    self.attrs.clear()
    self.legacy_settings.clear()
    self.alt_names.clear()
    self.internal_settings.clear()
    self.allowed_settings.clear()

    # Load the JS defaults into python.
    settings = open(path_from_root('src', 'settings.js')).read().replace('//', '#')
    settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
    # Variable TARGET_NOT_SUPPORTED is referenced by value settings.js (also beyond declaring it),
    # so must pass it there explicitly.
    exec(settings, {'attrs': self.attrs})

    settings = open(path_from_root('src', 'settings_internal.js')).read().replace('//', '#')
    settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
    internal_attrs = {}
    exec(settings, {'attrs': internal_attrs})
    self.attrs.update(internal_attrs)

    if 'EMCC_STRICT' in os.environ:
      self.attrs['STRICT'] = int(os.environ.get('EMCC_STRICT'))

    # Special handling for LEGACY_SETTINGS.  See src/setting.js for more
    # details
    for legacy in self.attrs['LEGACY_SETTINGS']:
      if len(legacy) == 2:
        name, new_name = legacy
        self.legacy_settings[name] = (None, 'setting renamed to ' + new_name)
        self.alt_names[name] = new_name
        self.alt_names[new_name] = name
        default_value = self.attrs[new_name]
      else:
        name, fixed_values, err = legacy
        self.legacy_settings[name] = (fixed_values, err)
        default_value = fixed_values[0]
      assert name not in self.attrs, 'legacy setting (%s) cannot also be a regular setting' % name
      if not self.attrs['STRICT']:
        self.attrs[name] = default_value

    self.internal_settings.update(internal_attrs.keys())

  def dict(self):
    return self.attrs

  def keys(self):
    return self.attrs.keys()

  def limit_settings(self, allowed):
    self.allowed_settings.clear()
    if allowed:
      self.allowed_settings.extend(allowed)

  def __getattr__(self, attr):
    if self.allowed_settings:
      assert attr in self.allowed_settings, f"internal error: attempt to read setting '{attr}' while in limited settings mode"

    if attr in self.attrs:
      return self.attrs[attr]
    else:
      raise AttributeError(f"no such setting: '{attr}'")

  def __setattr__(self, name, value):
    if self.allowed_settings:
      assert name in self.allowed_settings, f"internal error: attempt to write setting '{name}' while in limited settings mode"

    if name == 'STRICT' and value:
      for a in self.legacy_settings:
        self.attrs.pop(a, None)

    if name in self.legacy_settings:
      # TODO(sbc): Rather then special case this we should have STRICT turn on the
      # legacy-settings warning below
      if self.attrs['STRICT']:
        exit_with_error('legacy setting used in strict mode: %s', name)
      fixed_values, error_message = self.legacy_settings[name]
      if fixed_values and value not in fixed_values:
        exit_with_error('Invalid command line option -s ' + name + '=' + str(value) + ': ' + error_message)
      diagnostics.warning('legacy-settings', 'use of legacy setting: %s (%s)', name, error_message)

    if name in self.alt_names:
      alt_name = self.alt_names[name]
      self.attrs[alt_name] = value

    if name not in self.attrs:
      msg = "Attempt to set a non-existent setting: '%s'\n" % name
      suggestions = difflib.get_close_matches(name, list(self.attrs.keys()))
      suggestions = [s for s in suggestions if s not in self.legacy_settings]
      suggestions = ', '.join(suggestions)
      if suggestions:
        msg += ' - did you mean one of %s?\n' % suggestions
      msg += " - perhaps a typo in emcc's  -s X=Y  notation?\n"
      msg += ' - (see src/settings.js for valid values)'
      exit_with_error(msg)

    self.attrs[name] = value

  def __getitem__(self, key):
    return self.attrs[key]

  def __setitem__(self, key, value):
    self.attrs[key] = value


settings = SettingsManager()
