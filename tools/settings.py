# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import difflib
import json
import os
import re

from .utils import path_from_root, exit_with_error
from . import diagnostics

attrs = {}
legacy_settings = {}
alt_names = {}
internal_settings = set()


def load_settings():
  attrs.clear()
  legacy_settings.clear()
  alt_names.clear()
  internal_settings.clear()

  # Load the JS defaults into python.
  settings = open(path_from_root('src', 'settings.js')).read().replace('//', '#')
  settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
  # Variable TARGET_NOT_SUPPORTED is referenced by value settings.js (also beyond declaring it),
  # so must pass it there explicitly.
  exec(settings, {'attrs': attrs})

  settings = open(path_from_root('src', 'settings_internal.js')).read().replace('//', '#')
  settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
  internal_attrs = {}
  exec(settings, {'attrs': internal_attrs})
  attrs.update(internal_attrs)

  if 'EMCC_STRICT' in os.environ:
    attrs['STRICT'] = int(os.environ.get('EMCC_STRICT'))

  # Special handling for LEGACY_SETTINGS.  See src/setting.js for more
  # details
  for legacy in attrs['LEGACY_SETTINGS']:
    if len(legacy) == 2:
      name, new_name = legacy
      legacy_settings[name] = (None, 'setting renamed to ' + new_name)
      alt_names[name] = new_name
      alt_names[new_name] = name
      default_value = attrs[new_name]
    else:
      name, fixed_values, err = legacy
      legacy_settings[name] = (fixed_values, err)
      default_value = fixed_values[0]
    assert name not in attrs, 'legacy setting (%s) cannot also be a regular setting' % name
    if not attrs['STRICT']:
      attrs[name] = default_value

  internal_settings.update(internal_attrs.keys())


def set_setting(name, value):
  if name == 'STRICT' and value:
    for a in legacy_settings:
      attrs.pop(a, None)

  if name in legacy_settings:
    # TODO(sbc): Rather then special case this we should have STRICT turn on the
    # legacy-settings warning below
    if attrs['STRICT']:
      exit_with_error('legacy setting used in strict mode: %s', name)
    fixed_values, error_message = legacy_settings[name]
    if fixed_values and value not in fixed_values:
      exit_with_error('Invalid command line option -s ' + name + '=' + str(value) + ': ' + error_message)
    diagnostics.warning('legacy-settings', 'use of legacy setting: %s (%s)', name, error_message)

  if name in alt_names:
    alt_name = alt_names[name]
    attrs[alt_name] = value

  if name not in attrs:
    msg = "Attempt to set a non-existent setting: '%s'\n" % name
    suggestions = difflib.get_close_matches(name, list(attrs.keys()))
    suggestions = [s for s in suggestions if s not in legacy_settings]
    suggestions = ', '.join(suggestions)
    if suggestions:
      msg += ' - did you mean one of %s?\n' % suggestions
    msg += " - perhaps a typo in emcc's  -s X=Y  notation?\n"
    msg += ' - (see src/settings.js for valid values)'
    exit_with_error(msg)

  attrs[name] = value


class SettingsManager:
  def __init__(self):
    load_settings()

  # Transforms the Settings information into emcc-compatible args (-s X=Y, etc.). Basically
  # the reverse of load_settings, except for -Ox which is relevant there but not here
  def serialize(self):
    ret = []
    for key, value in attrs.items():
      if key == key.upper():  # this is a hack. all of our settings are ALL_CAPS, python internals are not
        jsoned = json.dumps(value, sort_keys=True)
        ret += ['-s', key + '=' + jsoned]
    return ret

  def to_dict(self):
    return attrs.copy()

  def keys(self):
    return attrs.keys()

  def __getattr__(self, attr):
    if attr in attrs:
      return attrs[attr]
    else:
      raise AttributeError("Settings object has no attribute '%s'" % attr)

  def __setattr__(self, attr, value):
    set_setting(attr, value)

  def get(self, key):
    return attrs.get(key)

  def __getitem__(self, key):
    return attrs[key]

  def __setitem__(self, key, value):
    attrs[key] = value
