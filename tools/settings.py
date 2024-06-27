# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import copy
import difflib
import os
import re
from typing import Set, Dict, Any

from .utils import path_from_root, exit_with_error
from . import diagnostics

# Subset of settings that take a memory size (i.e. 1Gb, 64kb etc)
MEM_SIZE_SETTINGS = {
    'GLOBAL_BASE',
    'STACK_SIZE',
    'TOTAL_STACK',
    'INITIAL_HEAP',
    'INITIAL_MEMORY',
    'MEMORY_GROWTH_LINEAR_STEP',
    'MEMORY_GROWTH_GEOMETRIC_CAP',
    'GL_MAX_TEMP_BUFFER_SIZE',
    'MAXIMUM_MEMORY',
    'DEFAULT_PTHREAD_STACK_SIZE'
}

PORTS_SETTINGS = {
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
    'USE_SQLITE3',
}

# Subset of settings that apply only when generating JS
JS_ONLY_SETTINGS = {
    'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE',
    'INCLUDE_FULL_LIBRARY',
    'PROXY_TO_WORKER',
    'PROXY_TO_WORKER_FILENAME',
    'BUILD_AS_WORKER',
    'STRICT_JS',
    'SMALL_XHR_CHUNKS',
    'HEADLESS',
    'MODULARIZE',
    'EXPORT_ES6',
    'USE_ES6_IMPORT_META',
    'EXPORT_NAME',
    'DYNAMIC_EXECUTION',
    'PTHREAD_POOL_SIZE',
    'PTHREAD_POOL_SIZE_STRICT',
    'PTHREAD_POOL_DELAY_LOAD',
    'DEFAULT_PTHREAD_STACK_SIZE',
}

# Subset of settings that apply at compile time.
# (Keep in sync with [compile] comments in settings.js)
COMPILE_TIME_SETTINGS = {
    'MEMORY64',
    'INLINING_LIMIT',
    'DISABLE_EXCEPTION_CATCHING',
    'DISABLE_EXCEPTION_THROWING',
    'MAIN_MODULE',
    'SIDE_MODULE',
    'RELOCATABLE',
    'LINKABLE',
    'STRICT',
    'EMSCRIPTEN_TRACING',
    'PTHREADS',
    'USE_PTHREADS', # legacy name of PTHREADS setting
    'SHARED_MEMORY',
    'SUPPORT_LONGJMP',
    'WASM_OBJECT_FILES',
    'WASM_WORKERS',
    'BULK_MEMORY',

    # Internal settings used during compilation
    'EXCEPTION_CATCHING_ALLOWED',
    'WASM_EXCEPTIONS',
    'LTO',
    'OPT_LEVEL',
    'DEBUG_LEVEL',

    # Affects ports
    'GL_ENABLE_GET_PROC_ADDRESS', # NOTE: if SDL2 is updated to not rely on eglGetProcAddress(), this can be removed

    # This is legacy setting that we happen to handle very early on
    'RUNTIME_LINKED_LIBS',
}.union(PORTS_SETTINGS)

# Unlike `LEGACY_SETTINGS`, deprecated settings can still be used
# both on the command line and in the emscripten codebase.
#
# At some point in the future, once folks have stopped using these
# settings we can move them to `LEGACY_SETTINGS`.
DEPRECATED_SETTINGS = {
    'SUPPORT_ERRNO': 'emscripten no longer uses the setErrNo library function',
    'EXTRA_EXPORTED_RUNTIME_METHODS': 'please use EXPORTED_RUNTIME_METHODS instead',
    'DEMANGLE_SUPPORT': 'mangled names no longer appear in stack traces',
    'RUNTIME_LINKED_LIBS': 'you can simply list the libraries directly on the commandline now',
    'CLOSURE_WARNINGS': 'use -Wclosure instead',
    'LEGALIZE_JS_FFI': 'to disable JS type legalization use `-sWASM_BIGINT` or `-sSTANDALONE_WASM`',
    'ASYNCIFY_EXPORTS': 'please use JSPI_EXPORTS instead'
}

# Settings that don't need to be externalized when serializing to json because they
# are not used by the JS compiler.
INTERNAL_SETTINGS = {
    'SIDE_MODULE_IMPORTS',
}

user_settings: Dict[str, str] = {}


def default_setting(name, new_default):
  if name not in user_settings:
    setattr(settings, name, new_default)


class SettingsManager:
  attrs: Dict[str, Any] = {}
  types: Dict[str, Any] = {}
  allowed_settings: Set[str] = set()
  legacy_settings: Dict[str, tuple] = {}
  alt_names: Dict[str, str] = {}
  internal_settings: Set[str] = set()

  def __init__(self):
    self.attrs.clear()
    self.legacy_settings.clear()
    self.alt_names.clear()
    self.internal_settings.clear()
    self.allowed_settings.clear()

    # Load the JS defaults into python.
    def read_js_settings(filename, attrs):
      with open(filename) as fh:
        settings = fh.read()
      # Use a bunch of regexs to convert the file from JS to python
      # TODO(sbc): This is kind hacky and we should probably covert
      # this file in format that python can read directly (since we
      # no longer read this file from JS at all).
      settings = settings.replace('//', '#')
      settings = re.sub(r'var ([\w\d]+)', r'attrs["\1"]', settings)
      settings = re.sub(r'=\s+false\s*;', '= False', settings)
      settings = re.sub(r'=\s+true\s*;', '= True', settings)
      exec(settings, {'attrs': attrs})

    internal_attrs = {}
    read_js_settings(path_from_root('src/settings.js'), self.attrs)
    read_js_settings(path_from_root('src/settings_internal.js'), internal_attrs)
    self.attrs.update(internal_attrs)
    self.infer_types()

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

  def infer_types(self):
    for key, value in self.attrs.items():
      self.types[key] = type(value)

  def dict(self):
    return self.attrs

  def external_dict(self, skip_keys={}): # noqa
    external_settings = {k: v for k, v in self.dict().items() if k not in INTERNAL_SETTINGS and k not in skip_keys}
    # Only the names of the legacy settings are used by the JS compiler
    # so we can reduce the size of serialized json by simplifying this
    # otherwise complex value.
    external_settings['LEGACY_SETTINGS'] = [l[0] for l in external_settings['LEGACY_SETTINGS']]
    return external_settings

  def keys(self):
    return self.attrs.keys()

  def limit_settings(self, allowed):
    self.allowed_settings.clear()
    if allowed:
      self.allowed_settings.update(allowed)

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
        exit_with_error(f'invalid command line setting `-s{name}={value}`: {error_message}')
      diagnostics.warning('legacy-settings', 'use of legacy setting: %s (%s)', name, error_message)

    if name in self.alt_names:
      alt_name = self.alt_names[name]
      self.attrs[alt_name] = value

    if name not in self.attrs:
      msg = "Attempt to set a non-existent setting: '%s'\n" % name
      valid_keys = set(self.attrs.keys()).difference(self.internal_settings)
      suggestions = difflib.get_close_matches(name, valid_keys)
      suggestions = [s for s in suggestions if s not in self.legacy_settings]
      suggestions = ', '.join(suggestions)
      if suggestions:
        msg += ' - did you mean one of %s?\n' % suggestions
      msg += " - perhaps a typo in emcc's  -sX=Y  notation?\n"
      msg += ' - (see src/settings.js for valid values)'
      exit_with_error(msg)

    self.check_type(name, value)
    self.attrs[name] = value

  def check_type(self, name, value):
    if name in ('SUPPORT_LONGJMP', 'PTHREAD_POOL_SIZE', 'SEPARATE_DWARF', 'LTO'):
      return
    expected_type = self.types.get(name)
    if not expected_type:
      return
    # Allow itegers 1 and 0 for type `bool`
    if expected_type == bool:
      if value in (1, 0):
        value = bool(value)
      if value in ('True', 'False', 'true', 'false'):
        exit_with_error('attempt to set `%s` to `%s`; use 1/0 to set boolean settings' % (name, value))
    if type(value) is not expected_type:
      exit_with_error('setting `%s` expects `%s` but got `%s`' % (name, expected_type.__name__, type(value).__name__))

  def __getitem__(self, key):
    return self.attrs[key]

  def __setitem__(self, key, value):
    self.attrs[key] = value

  def backup(self):
    return copy.deepcopy(self.attrs)

  def restore(self, previous):
    self.attrs.update(previous)


settings = SettingsManager()
