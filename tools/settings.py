# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import copy
import difflib
import os
import re
from typing import Any, Dict, Set

from . import diagnostics
from .utils import exit_with_error, path_from_root

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
    'DEFAULT_PTHREAD_STACK_SIZE',
    'ASYNCIFY_STACK_SIZE',
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
    'MODULARIZE',
    'EXPORT_ES6',
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
    'WASM_LEGACY_EXCEPTIONS',
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
#
# All settings here should be tagged as `[deprecated]` in settings.js
DEPRECATED_SETTINGS = {
    'RUNTIME_LINKED_LIBS': 'you can simply list the libraries directly on the commandline now',
    'CLOSURE_WARNINGS': 'use -Wclosure/-Wno-closure instead',
    'LEGALIZE_JS_FFI': 'to disable JS type legalization use `-sWASM_BIGINT` or `-sSTANDALONE_WASM`',
    'ASYNCIFY_EXPORTS': 'please use JSPI_EXPORTS instead',
    'LINKABLE': 'under consideration for removal (https://github.com/emscripten-core/emscripten/issues/25262)',
    'RELOCATABLE': ' under consideration for removal (https://github.com/emscripten-core/emscripten/issues/25262)',
    'PROXY_TO_WORKER': 'under consideration for removal (See https://github.com/emscripten-core/emscripten/issues/25440)',
    'PROXY_TO_WORKER_FILENAME': 'under consideration for removal (See https://github.com/emscripten-core/emscripten/issues/25440)',
}

# Settings that don't need to be externalized when serializing to json because they
# are not used by the JS compiler.
INTERNAL_SETTINGS = {
    'SIDE_MODULE_IMPORTS',
}

# List of incompatible settings, of the form (SETTINGS_A, SETTING_B, OPTIONAL_REASON_FOR_INCOMPAT)
INCOMPATIBLE_SETTINGS = [
    ('MINIMAL_RUNTIME', 'RELOCATABLE', None),
    ('WASM2JS', 'RELOCATABLE', None),
    ('MODULARIZE', 'PROXY_TO_WORKER', 'if you want to run in a worker with -sMODULARIZE, you likely want to do the worker side setup manually'),
    ('MODULARIZE', 'NO_DECLARE_ASM_MODULE_EXPORTS', None),
    ('EVAL_CTORS', 'WASM2JS', None),
    ('EVAL_CTORS', 'RELOCATABLE', 'movable segments'),
    # In Asyncify exports can be called more than once, and this seems to not
    # work properly yet (see test_emscripten_scan_registers).
    ('EVAL_CTORS', 'ASYNCIFY', None),
    ('PTHREADS_PROFILING', 'NO_ASSERTIONS', 'only works with ASSERTIONS enabled'),
    ('SOURCE_PHASE_IMPORTS', 'NO_EXPORT_ES6', None),
    ('STANDALONE_WASM', 'MINIMAL_RUNTIME', None),
    ('STRICT_JS', 'MODULARIZE', None),
    ('STRICT_JS', 'EXPORT_ES6', None),
    ('MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION', 'MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION', 'they are mutually exclusive'),
    ('MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION', 'SINGLE_FILE', None),
    ('MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION', 'SINGLE_FILE', None),
    ('SEPARATE_DWARF', 'WASM2JS', 'as there is no wasm file'),
    ('GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS', 'NO_GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS', None),
    ('MODULARIZE', 'NODEJS_CATCH_REJECTION', None),
    ('MODULARIZE', 'NODEJS_CATCH_EXIT', None),
    ('LEGACY_VM_SUPPORT', 'MEMORY64', None),
    ('CROSS_ORIGIN', 'NO_DYNAMIC_EXECUTION', None),
    ('CROSS_ORIGIN', 'NO_PTHREADS', None),
]

# For renamed settings the format is:
# [OLD_NAME, NEW_NAME]
# For removed settings (which now effectively have a fixed value and can no
# longer be changed) the format is:
# [OPTION_NAME, POSSIBLE_VALUES, ERROR_EXPLANATION], where POSSIBLE_VALUES is
# an array of values that will still be silently accepted by the compiler.
# First element in the list is the canonical/fixed value going forward.
# This allows existing build systems to keep specifying one of the supported
# settings, for backwards compatibility.
# When a setting has been removed, and we want to error on all values of it,
# we can set POSSIBLE_VALUES to an impossible value (like "disallowed" for a
# numeric setting, or -1 for a string setting).
LEGACY_SETTINGS = [
    ['BINARYEN', 'WASM'],
    ['TOTAL_STACK', 'STACK_SIZE'],
    ['BINARYEN_ASYNC_COMPILATION', 'WASM_ASYNC_COMPILATION'],
    ['UNALIGNED_MEMORY', [0], 'forced unaligned memory not supported in fastcomp'],
    ['FORCE_ALIGNED_MEMORY', [0], 'forced aligned memory is not supported in fastcomp'],
    ['PGO', [0], 'pgo no longer supported'],
    ['QUANTUM_SIZE', [4], 'altering the QUANTUM_SIZE is not supported'],
    ['FUNCTION_POINTER_ALIGNMENT', [2], 'Starting from Emscripten 1.37.29, no longer available (https://github.com/emscripten-core/emscripten/pull/6091)'],
    # Reserving function pointers is not needed - allowing table growth allows any number of new functions to be added.
    ['RESERVED_FUNCTION_POINTERS', 'ALLOW_TABLE_GROWTH'],
    ['BUILD_AS_SHARED_LIB', [0], 'Starting from Emscripten 1.38.16, no longer available (https://github.com/emscripten-core/emscripten/pull/7433)'],
    ['SAFE_SPLIT_MEMORY', [0], 'Starting from Emscripten 1.38.19, SAFE_SPLIT_MEMORY codegen is no longer available (https://github.com/emscripten-core/emscripten/pull/7465)'],
    ['SPLIT_MEMORY', [0], 'Starting from Emscripten 1.38.19, SPLIT_MEMORY codegen is no longer available (https://github.com/emscripten-core/emscripten/pull/7465)'],
    ['BINARYEN_METHOD', ['native-wasm'], 'Starting from Emscripten 1.38.23, Emscripten now always builds either to Wasm (-sWASM - default), or to JavaScript (-sWASM=0), other methods are not supported (https://github.com/emscripten-core/emscripten/pull/7836)'],
    ['BINARYEN_TRAP_MODE', [-1], 'The wasm backend does not support a trap mode (it always clamps, in effect)'],
    ['PRECISE_I64_MATH', [1, 2], 'Starting from Emscripten 1.38.26, PRECISE_I64_MATH is always enabled (https://github.com/emscripten-core/emscripten/pull/7935)'],
    ['MEMFS_APPEND_TO_TYPED_ARRAYS', [1], 'Starting from Emscripten 1.38.26, MEMFS_APPEND_TO_TYPED_ARRAYS=0 is no longer supported. MEMFS no longer supports using JS arrays for file data (https://github.com/emscripten-core/emscripten/pull/7918)'],
    ['ERROR_ON_MISSING_LIBRARIES', [1], 'missing libraries are always an error now'],
    ['EMITTING_JS', [1], 'The new STANDALONE_WASM flag replaces this (replace EMITTING_JS=0 with STANDALONE_WASM=1)'],
    ['SKIP_STACK_IN_SMALL', [0, 1], 'SKIP_STACK_IN_SMALL is no longer needed as the backend can optimize it directly'],
    ['SAFE_STACK', [0], 'Replace SAFE_STACK=1 with STACK_OVERFLOW_CHECK=2'],
    ['MEMORY_GROWTH_STEP', 'MEMORY_GROWTH_LINEAR_STEP'],
    ['ELIMINATE_DUPLICATE_FUNCTIONS', [0, 1], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
    ['ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS', [0], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
    ['ELIMINATE_DUPLICATE_FUNCTIONS_PASSES', [5], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
    # WASM_OBJECT_FILES is handled in emcc.py, supporting both 0 and 1 for now.
    ['WASM_OBJECT_FILES', [0, 1], 'For LTO, use -flto or -fto=thin instead; to disable LTO, just do not pass WASM_OBJECT_FILES=1 as 1 is the default anyhow'],
    ['TOTAL_MEMORY', 'INITIAL_MEMORY'],
    ['WASM_MEM_MAX', 'MAXIMUM_MEMORY'],
    ['BINARYEN_MEM_MAX', 'MAXIMUM_MEMORY'],
    ['BINARYEN_PASSES', [''], 'Use BINARYEN_EXTRA_PASSES to add additional passes'],
    ['SWAPPABLE_ASM_MODULE', [0], 'Fully swappable asm modules are no longer supported'],
    ['ASM_JS', [1], 'asm.js output is not supported anymore'],
    ['FINALIZE_ASM_JS', [0, 1], 'asm.js output is not supported anymore'],
    ['ASYNCIFY_WHITELIST', 'ASYNCIFY_ONLY'],
    ['ASYNCIFY_BLACKLIST', 'ASYNCIFY_REMOVE'],
    ['EXCEPTION_CATCHING_WHITELIST', 'EXCEPTION_CATCHING_ALLOWED'],
    ['SEPARATE_ASM', [0], 'Separate asm.js only made sense for fastcomp with asm.js output'],
    ['SEPARATE_ASM_MODULE_NAME', [''], 'Separate asm.js only made sense for fastcomp with asm.js output'],
    ['FAST_UNROLLED_MEMCPY_AND_MEMSET', [0, 1], 'The wasm backend implements memcpy/memset in C'],
    ['DOUBLE_MODE', [0, 1], 'The wasm backend always implements doubles normally'],
    ['PRECISE_F32', [0, 1, 2], 'The wasm backend always implements floats normally'],
    ['ALIASING_FUNCTION_POINTERS', [0, 1], 'The wasm backend always uses a single index space for function pointers, in a single Table'],
    ['AGGRESSIVE_VARIABLE_ELIMINATION', [0, 1], 'Wasm ignores asm.js-specific optimization flags'],
    ['SIMPLIFY_IFS', [1], 'Wasm ignores asm.js-specific optimization flags'],
    ['DEAD_FUNCTIONS', [[]], 'The wasm backend does not support dead function removal'],
    ['WASM_BACKEND', [-1], 'Only the wasm backend is now supported (note that setting it as -s has never been allowed anyhow)'],
    ['EXPORT_BINDINGS', [0, 1], 'No longer needed'],
    ['RUNNING_JS_OPTS', [0], 'Fastcomp cared about running JS which could alter asm.js validation, but not upstream'],
    ['EXPORT_FUNCTION_TABLES', [0], 'No longer needed'],
    ['BINARYEN_SCRIPTS', [''], 'No longer needed'],
    ['WARN_UNALIGNED', [0, 1], 'No longer needed'],
    ['ASM_PRIMITIVE_VARS', [[]], 'No longer needed'],
    ['WORKAROUND_IOS_9_RIGHT_SHIFT_BUG', [0], 'Wasm2JS does not support iPhone 4s, iPad 2, iPad 3, iPad Mini 1, Pod Touch 5 (devices with end-of-life at iOS 9.3.5) and older'],
    ['RUNTIME_FUNCS_TO_IMPORT', [[]], 'No longer needed'],
    ['LIBRARY_DEPS_TO_AUTOEXPORT', [[]], 'No longer needed'],
    ['EMIT_EMSCRIPTEN_METADATA', [0], 'No longer supported'],
    ['SHELL_FILE', [''], 'No longer supported'],
    ['LLD_REPORT_UNDEFINED', [1], 'Disabling is no longer supported'],
    ['MEM_INIT_METHOD', [0], 'No longer supported'],
    ['USE_PTHREADS', [0, 1], 'No longer needed. Use -pthread instead'],
    ['USES_DYNAMIC_ALLOC', [1], 'No longer supported. Use -sMALLOC=none'],
    ['REVERSE_DEPS', ['auto', 'all', 'none'], 'No longer needed'],
    ['RUNTIME_LOGGING', 'RUNTIME_DEBUG'],
    ['MIN_EDGE_VERSION', [0x7FFFFFFF], 'No longer supported'],
    ['MIN_IE_VERSION', [0x7FFFFFFF], 'No longer supported'],
    ['WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG', [0], 'No longer supported'],
    ['AUTO_ARCHIVE_INDEXES', [0, 1], 'No longer needed'],
    ['USE_ES6_IMPORT_META', [1], 'Disabling is no longer supported'],
    ['EXTRA_EXPORTED_RUNTIME_METHODS', [[]], 'No longer supported, use EXPORTED_RUNTIME_METHODS'],
    ['SUPPORT_ERRNO', [0], 'No longer supported'],
    ['DEMANGLE_SUPPORT', [0], 'No longer supported'],
    ['MAYBE_WASM2JS', [0], 'No longer supported (use -sWASM=2)'],
    ['HEADLESS', [0], 'No longer supported, use headless browsers or Node.js with JSDOM'],
    ['USE_OFFSET_COVERTER', [0], 'No longer supported, not needed with modern v8 versions'],
    ['ASYNCIFY_LAZY_LOAD_CODE', [0], 'No longer supported'],
    ['USE_WEBGPU', [0], 'No longer supported; replaced by --use-port=emdawnwebgpu, which implements a newer (but incompatible) version of webgpu.h - see tools/ports/emdawnwebgpu.py'],
]

user_settings: Dict[str, str] = {}


def default_setting(name, new_default):
  if name not in user_settings:
    setattr(settings, name, new_default)


class SettingsManager:
  attrs: Dict[str, Any] = {}
  defaults: Dict[str, tuple] = {}
  types: Dict[str, Any] = {}
  allowed_settings: Set[str] = set()
  legacy_settings: Dict[str, tuple] = {}
  alt_names: Dict[str, str] = {}
  internal_settings: Set[str] = set()

  def __init__(self):
    self.attrs.clear()
    self.legacy_settings.clear()
    self.defaults.clear()
    self.alt_names.clear()
    self.internal_settings.clear()
    self.allowed_settings.clear()

    # Load the JS defaults into python.
    def read_js_settings(filename, attrs):
      with open(filename) as fh:
        settings = fh.read()
      # Use a bunch of regexs to convert the file from JS to python
      # TODO(sbc): This is kind hacky and we should probably convert
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

    strict_override = False
    if 'EMCC_STRICT' in os.environ:
      strict_override = int(os.environ.get('EMCC_STRICT'))

    # Special handling for LEGACY_SETTINGS.  See src/setting.js for more
    # details
    for legacy in LEGACY_SETTINGS:
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
      if not strict_override:
        self.attrs[name] = default_value

    self.internal_settings.update(internal_attrs.keys())
    # Stash a deep copy of all settings in self.defaults.  This allows us to detect which settings
    # have local mods.
    self.defaults.update(copy.deepcopy(self.attrs))

    if strict_override:
      self.attrs['STRICT'] = strict_override

  def infer_types(self):
    for key, value in self.attrs.items():
      self.types[key] = type(value)

  def dict(self):
    return self.attrs

  def external_dict(self, skip_keys={}): # noqa
    external_settings = {}
    for key, value in self.dict().items():
      if value != self.defaults.get(key) and key not in INTERNAL_SETTINGS and key not in skip_keys:
        external_settings[key] = value # noqa: PERF403
    if not self.attrs['STRICT']:
      # When not running in strict mode we also externalize all legacy settings
      # (Since the external tools do process LEGACY_SETTINGS themselves)
      for key in self.legacy_settings:
        external_settings[key] = self.attrs[key]
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
    # These settings have a variable type so cannot be easily type checked.
    if name in ('SUPPORT_LONGJMP', 'PTHREAD_POOL_SIZE', 'SEPARATE_DWARF', 'LTO', 'MODULARIZE'):
      return
    expected_type = self.types.get(name)
    if not expected_type:
      return
    # Allow integers 1 and 0 for type `bool`
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
