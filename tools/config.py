# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import sys
import logging
from typing import List, Optional

from . import utils, diagnostics
from .utils import path_from_root, exit_with_error, __rootpath__

logger = logging.getLogger('config')

# The following class can be overridden by the config file and/or
# environment variables.  Specifically any variable whose name
# is in ALL_UPPER_CASE is condifered a valid config file key.
# See parse_config_file below.
EMSCRIPTEN_ROOT = __rootpath__
NODE_JS = None
NODE_JS_TEST = None
BINARYEN_ROOT = None
SPIDERMONKEY_ENGINE = None
V8_ENGINE: Optional[List[str]] = None
LLVM_ROOT = None
LLVM_ADD_VERSION = None
CLANG_ADD_VERSION = None
CLOSURE_COMPILER = None
JS_ENGINES: List[List[str]] = []
WASMER = None
WASMTIME = None
WASM_ENGINES: List[List[str]] = []
FROZEN_CACHE = None
CACHE = None
PORTS = None
COMPILER_WRAPPER = None

# Set by init()
EM_CONFIG = None


def listify(x):
  if x is None or type(x) is list:
    return x
  return [x]


def fix_js_engine(old, new):
  if old is None:
    return
  global JS_ENGINES
  JS_ENGINES = [new if x == old else x for x in JS_ENGINES]
  return new


def root_is_writable():
  return os.access(__rootpath__, os.W_OK)


def normalize_config_settings():
  global CACHE, PORTS, LLVM_ADD_VERSION, CLANG_ADD_VERSION, CLOSURE_COMPILER
  global NODE_JS, NODE_JS_TEST, V8_ENGINE, JS_ENGINES, SPIDERMONKEY_ENGINE, WASM_ENGINES

  # EM_CONFIG stuff
  if not JS_ENGINES:
    JS_ENGINES = [NODE_JS]

  # Engine tweaks
  if SPIDERMONKEY_ENGINE:
    new_spidermonkey = SPIDERMONKEY_ENGINE
    if '-w' not in str(new_spidermonkey):
      new_spidermonkey += ['-w']
    SPIDERMONKEY_ENGINE = fix_js_engine(SPIDERMONKEY_ENGINE, new_spidermonkey)
  NODE_JS = fix_js_engine(NODE_JS, listify(NODE_JS))
  NODE_JS_TEST = fix_js_engine(NODE_JS_TEST, listify(NODE_JS_TEST))
  V8_ENGINE = fix_js_engine(V8_ENGINE, listify(V8_ENGINE))
  JS_ENGINES = [listify(engine) for engine in JS_ENGINES]
  WASM_ENGINES = [listify(engine) for engine in WASM_ENGINES]
  CLOSURE_COMPILER = listify(CLOSURE_COMPILER)
  if not CACHE:
    if FROZEN_CACHE or root_is_writable():
      CACHE = path_from_root('cache')
    else:
      # Use the legacy method of putting the cache in the user's home directory
      # if the emscripten root is not writable.
      # This is useful mostly for read-only installation and perhaps could
      # be removed in the future since such installations should probably be
      # setting a specific cache location.
      logger.debug('Using home-directory for emscripten cache due to read-only root')
      CACHE = os.path.expanduser(os.path.join('~', '.emscripten_cache'))
  if not PORTS:
    PORTS = os.path.join(CACHE, 'ports')


def set_config_from_tool_location(config_key, tool_binary, f):
  val = globals()[config_key]
  if val is None:
    path = shutil.which(tool_binary)
    if not path:
      if not os.path.isfile(EM_CONFIG):
        diagnostics.warn('config file not found: %s.  You can create one by hand or run `emcc --generate-config`', EM_CONFIG)
      exit_with_error('%s not set in config (%s), and `%s` not found in PATH', config_key, EM_CONFIG, tool_binary)
    globals()[config_key] = f(path)
  elif not val:
    exit_with_error('%s is set to empty value in %s', config_key, EM_CONFIG)


def parse_config_file():
  """Parse the emscripten config file using python's exec.

  Also check EM_<KEY> environment variables to override specific config keys.
  """
  config = {}
  config_text = utils.read_file(EM_CONFIG)
  try:
    exec(config_text, config)
  except Exception as e:
    exit_with_error('error in evaluating config file (%s): %s, text: %s', EM_CONFIG, str(e), config_text)

  CONFIG_KEYS = (
    'NODE_JS',
    'NODE_JS_TEST',
    'BINARYEN_ROOT',
    'SPIDERMONKEY_ENGINE',
    'V8_ENGINE',
    'LLVM_ROOT',
    'LLVM_ADD_VERSION',
    'CLANG_ADD_VERSION',
    'CLOSURE_COMPILER',
    'JS_ENGINES',
    'WASMER',
    'WASMTIME',
    'WASM_ENGINES',
    'FROZEN_CACHE',
    'CACHE',
    'PORTS',
    'COMPILER_WRAPPER',
  )

  # Only propagate certain settings from the config file.
  for key in CONFIG_KEYS:
    env_var = 'EM_' + key
    env_value = os.environ.get(env_var)
    if env_value is not None:
      if env_value in ('', '0'):
        env_value = None
      # Unlike the other keys these two should always be lists.
      if key in ('JS_ENGINES', 'WASM_ENGINES'):
        env_value = env_value.split(',')
      globals()[key] = env_value
    elif key in config:
      globals()[key] = config[key]


def read_config():
  if os.path.isfile(EM_CONFIG):
    parse_config_file()

  # In the past the default-generated .emscripten config file would read
  # certain environment variables.
  LEGACY_ENV_VARS = {
    'LLVM': 'EM_LLVM_ROOT',
    'BINARYEN': 'EM_BINARYEN_ROOT',
    'NODE': 'EM_NODE_JS',
    'LLVM_ADD_VERSION': 'EM_LLVM_ADD_VERSION',
    'CLANG_ADD_VERSION': 'EM_CLANG_ADD_VERSION',
  }

  for key, new_key in LEGACY_ENV_VARS.items():
    env_value = os.environ.get(key)
    if env_value and new_key not in os.environ:
      msg = f'legacy environment variable found: `{key}`.  Please switch to using `{new_key}` instead`'
      # Use `debug` instead of `warning` for `NODE` specifically
      # since there can be false positives:
      # See https://github.com/emscripten-core/emsdk/issues/862
      if key == 'NODE':
        logger.debug(msg)
      else:
        logger.warning(msg)

  set_config_from_tool_location('LLVM_ROOT', 'clang', os.path.dirname)
  set_config_from_tool_location('NODE_JS', 'node', lambda x: x)
  set_config_from_tool_location('BINARYEN_ROOT', 'wasm-opt', lambda x: os.path.dirname(os.path.dirname(x)))

  normalize_config_settings()


def generate_config(path):
  if os.path.exists(path):
    exit_with_error(f'config file already exists: `{path}`')

  # Note: repr is used to ensure the paths are escaped correctly on Windows.
  # The full string is replaced so that the template stays valid Python.

  config_data = utils.read_file(path_from_root('tools/config_template.py'))
  config_data = config_data.splitlines()[3:] # remove the initial comment
  config_data = '\n'.join(config_data) + '\n'
  # autodetect some default paths
  llvm_root = os.path.dirname(shutil.which('wasm-ld') or '/usr/bin/wasm-ld')
  config_data = config_data.replace('\'{{{ LLVM_ROOT }}}\'', repr(llvm_root))

  binaryen_root = os.path.dirname(os.path.dirname(shutil.which('wasm-opt') or '/usr/local/bin/wasm-opt'))
  config_data = config_data.replace('\'{{{ BINARYEN_ROOT }}}\'', repr(binaryen_root))

  node = shutil.which('node') or shutil.which('nodejs') or 'node'
  config_data = config_data.replace('\'{{{ NODE }}}\'', repr(node))

  # write
  utils.write_file(path, config_data)

  print('''\
An Emscripten settings file has been generated at:

  %s

It contains our best guesses for the important paths, which are:

  LLVM_ROOT       = %s
  BINARYEN_ROOT   = %s
  NODE_JS         = %s

Please edit the file if any of those are incorrect.\
''' % (path, llvm_root, binaryen_root, node), file=sys.stderr)


def find_config_file():
  # Emscripten configuration is done through the --em-config command line option
  # or the EM_CONFIG environment variable. If the specified string value contains
  # newline or semicolon-separated definitions, then these definitions will be
  # used to configure Emscripten.  Otherwise, the string is understood to be a
  # path to a settings file that contains the required definitions.
  # The search order from the config file is as follows:
  # 1. Specified on the command line (--em-config)
  # 2. Specified via EM_CONFIG environment variable
  # 3. Local .emscripten file, if found
  # 4. Local .emscripten file, as used by `emsdk --embedded` (two levels above,
  #    see below)
  # 5. User home directory config (~/.emscripten), if found.

  embedded_config = path_from_root('.emscripten')
  # For compatibility with `emsdk --embedded` mode also look two levels up.  The
  # layout of the emsdk puts emcc two levels below emsdk.  For example:
  #  - emsdk/upstream/emscripten/emcc
  #  - emsdk/emscripten/1.38.31/emcc
  # However `emsdk --embedded` stores the config file in the emsdk root.
  # Without this check, when emcc is run from within the emsdk in embedded mode
  # and the user forgets to first run `emsdk_env.sh` (which sets EM_CONFIG) emcc
  # will not see any config file at all and fall back to creating a new/empty
  # one.
  # We could remove this special case if emsdk were to write its embedded config
  # file into the emscripten directory itself.
  # See: https://github.com/emscripten-core/emsdk/pull/367
  emsdk_root = os.path.dirname(os.path.dirname(path_from_root()))
  emsdk_embedded_config = os.path.join(emsdk_root, '.emscripten')
  user_home_config = os.path.expanduser('~/.emscripten')

  if '--em-config' in sys.argv:
    i = sys.argv.index('--em-config')
    if len(sys.argv) <= i + 1:
      exit_with_error('--em-config must be followed by a filename')
    del sys.argv[i]
    # Now the i'th argument is the emconfig filename
    return sys.argv.pop(i)

  if 'EM_CONFIG' in os.environ:
    return os.environ['EM_CONFIG']

  if os.path.isfile(embedded_config):
    return embedded_config

  if os.path.isfile(emsdk_embedded_config):
    return emsdk_embedded_config

  if os.path.isfile(user_home_config):
    return user_home_config

  # No config file found.  Return the default location.
  if not root_is_writable():
    return user_home_config

  return embedded_config


def init():
  global EM_CONFIG
  EM_CONFIG = find_config_file()

  # We used to support inline EM_CONFIG.
  if '\n' in EM_CONFIG:
    exit_with_error('inline EM_CONFIG data no longer supported.  Please use a config file.')

  EM_CONFIG = os.path.expanduser(EM_CONFIG)

  # This command line flag needs to work even in the absence of a config
  # file, so we must process it here at script import time (otherwise
  # the error below will trigger).
  if '--generate-config' in sys.argv:
    generate_config(EM_CONFIG)
    sys.exit(0)

  if os.path.isfile(EM_CONFIG):
    logger.debug(f'using config file: {EM_CONFIG}')
  else:
    logger.debug('config file not found; using default config')

  # Emscripten compiler spawns other processes, which can reimport shared.py, so
  # make sure that those child processes get the same configuration file by
  # setting it to the currently active environment.
  os.environ['EM_CONFIG'] = EM_CONFIG

  read_config()


init()
