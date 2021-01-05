# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import logging
from .utils import path_from_root, exit_with_error, __rootpath__, which

logger = logging.getLogger('shared')

# The following class can be overridden by the config file and/or
# environment variables.  Specifically any variable whose name
# is in ALL_UPPER_CASE is condifered a valid config file key.
# See parse_config_file below.
EMSCRIPTEN_ROOT = __rootpath__
NODE_JS = None
BINARYEN_ROOT = None
SPIDERMONKEY_ENGINE = None
V8_ENGINE = None
LLVM_ROOT = None
LLVM_ADD_VERSION = None
CLANG_ADD_VERSION = None
CLOSURE_COMPILER = None
JAVA = None
JS_ENGINE = None
JS_ENGINES = None
WASMER = None
WASMTIME = None
WASM_ENGINES = []
FROZEN_CACHE = None
CACHE = None
PORTS = None
COMPILER_WRAPPER = None


def listify(x):
  if type(x) is not list:
    return [x]
  return x


def fix_js_engine(old, new):
  if old is None:
    return
  global JS_ENGINES
  JS_ENGINES = [new if x == old else x for x in JS_ENGINES]
  return new


def root_is_writable():
  return os.access(__rootpath__, os.W_OK)


def normalize_config_settings():
  global CACHE, PORTS, JAVA, LLVM_ADD_VERSION, CLANG_ADD_VERSION
  global NODE_JS, V8_ENGINE, JS_ENGINE, JS_ENGINES, SPIDERMONKEY_ENGINE, WASM_ENGINES

  # EM_CONFIG stuff
  if not JS_ENGINES:
    JS_ENGINES = [NODE_JS]
  if not JS_ENGINE:
    JS_ENGINE = JS_ENGINES[0]

  # Engine tweaks
  if SPIDERMONKEY_ENGINE:
    new_spidermonkey = SPIDERMONKEY_ENGINE
    if '-w' not in str(new_spidermonkey):
      new_spidermonkey += ['-w']
    SPIDERMONKEY_ENGINE = fix_js_engine(SPIDERMONKEY_ENGINE, new_spidermonkey)
  NODE_JS = fix_js_engine(NODE_JS, listify(NODE_JS))
  V8_ENGINE = fix_js_engine(V8_ENGINE, listify(V8_ENGINE))
  JS_ENGINE = fix_js_engine(JS_ENGINE, listify(JS_ENGINE))
  JS_ENGINES = [listify(engine) for engine in JS_ENGINES]
  WASM_ENGINES = [listify(engine) for engine in WASM_ENGINES]
  if not CACHE:
    if root_is_writable():
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

  if JAVA is None:
    logger.debug('JAVA not defined in ' + config_file_location() + ', using "java"')
    JAVA = 'java'

  # Tools/paths
  if LLVM_ADD_VERSION is None:
    LLVM_ADD_VERSION = os.getenv('LLVM_ADD_VERSION')

  if CLANG_ADD_VERSION is None:
    CLANG_ADD_VERSION = os.getenv('CLANG_ADD_VERSION')


def parse_config_file():
  """Parse the emscripten config file using python's exec.

  Also check EM_<KEY> environment variables to override specific config keys.
  """
  config = {}
  config_text = open(config_file, 'r').read() if config_file else EM_CONFIG
  try:
    exec(config_text, config)
  except Exception as e:
    exit_with_error('Error in evaluating %s (at %s): %s, text: %s', EM_CONFIG, config_file, str(e), config_text)

  CONFIG_KEYS = (
    'NODE_JS',
    'BINARYEN_ROOT',
    'SPIDERMONKEY_ENGINE',
    'V8_ENGINE',
    'LLVM_ROOT',
    'LLVM_ADD_VERSION',
    'CLANG_ADD_VERSION',
    'CLOSURE_COMPILER',
    'JAVA',
    'JS_ENGINE',
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
      globals()[key] = env_value
    elif key in config:
      globals()[key] = config[key]

  # Certain keys are mandatory
  for key in ('LLVM_ROOT', 'NODE_JS', 'BINARYEN_ROOT'):
    if key not in config:
      exit_with_error('%s is not defined in %s', key, config_file_location())
    if not globals()[key]:
      exit_with_error('%s is set to empty value in %s', key, config_file_location())

  if not NODE_JS:
    exit_with_error('NODE_JS is not defined in %s', config_file_location())

  normalize_config_settings()


# Returns the location of the emscripten config file.
def config_file_location():
  # Handle the case where there is no config file at all (i.e. If EM_CONFIG is passed as python code
  # direclty on the command line).
  if not config_file:
    return '<inline config>'

  return config_file


def generate_config(path, first_time=False):
  # Note: repr is used to ensure the paths are escaped correctly on Windows.
  # The full string is replaced so that the template stays valid Python.
  config_file = open(path_from_root('tools', 'settings_template.py')).read().splitlines()
  config_file = config_file[3:] # remove the initial comment
  config_file = '\n'.join(config_file)
  # autodetect some default paths
  config_file = config_file.replace('\'{{{ EMSCRIPTEN_ROOT }}}\'', repr(__rootpath__))
  llvm_root = os.path.dirname(which('llvm-dis') or '/usr/bin/llvm-dis')
  config_file = config_file.replace('\'{{{ LLVM_ROOT }}}\'', repr(llvm_root))

  node = which('node') or which('nodejs') or 'node'
  config_file = config_file.replace('\'{{{ NODE }}}\'', repr(node))

  abspath = os.path.abspath(os.path.expanduser(path))
  # write
  with open(abspath, 'w') as f:
    f.write(config_file)

  if first_time:
    print('''
==============================================================================
Welcome to Emscripten!

This is the first time any of the Emscripten tools has been run.

A settings file has been copied to %s, at absolute path: %s

It contains our best guesses for the important paths, which are:

  LLVM_ROOT       = %s
  NODE_JS         = %s
  EMSCRIPTEN_ROOT = %s

Please edit the file if any of those are incorrect.

This command will now exit. When you are done editing those paths, re-run it.
==============================================================================
''' % (path, abspath, llvm_root, node, __rootpath__), file=sys.stderr)


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
# layout of the emsdk puts emcc two levels below emsdk.  For exmaple:
#  - emsdk/upstream/emscripten/emcc
#  - emsdk/emscipten/1.38.31/emcc
# However `emsdk --embedded` stores the config file in the emsdk root.
# Without this check, when emcc is run from within the emsdk in embedded mode
# and the user forgets to first run `emsdk_env.sh` (which sets EM_CONFIG) emcc
# will not see any config file at all and fall back to creating a new/emtpy
# one.
# We could remove this special case if emsdk were to write its embedded config
# file into the emscripten directory itself.
# See: https://github.com/emscripten-core/emsdk/pull/367
emsdk_root = os.path.dirname(os.path.dirname(path_from_root()))
emsdk_embedded_config = os.path.join(emsdk_root, '.emscripten')
user_home_config = os.path.expanduser('~/.emscripten')

if '--em-config' in sys.argv:
  EM_CONFIG = sys.argv[sys.argv.index('--em-config') + 1]
  # And now remove it from sys.argv
  skip = False
  newargs = []
  for arg in sys.argv:
    if not skip and arg != '--em-config':
      newargs += [arg]
    elif arg == '--em-config':
      skip = True
    elif skip:
      skip = False
  sys.argv = newargs
  if not os.path.isfile(EM_CONFIG):
    if EM_CONFIG.startswith('-'):
      exit_with_error('Passed --em-config without an argument. Usage: --em-config /path/to/.emscripten or --em-config LLVM_ROOT=/path;...')
    if '=' not in EM_CONFIG:
      exit_with_error('File ' + EM_CONFIG + ' passed to --em-config does not exist!')
    else:
      EM_CONFIG = EM_CONFIG.replace(';', '\n') + '\n'
elif 'EM_CONFIG' in os.environ:
  EM_CONFIG = os.environ['EM_CONFIG']
elif os.path.exists(embedded_config):
  EM_CONFIG = embedded_config
elif os.path.exists(emsdk_embedded_config):
  EM_CONFIG = emsdk_embedded_config
elif os.path.exists(user_home_config):
  EM_CONFIG = user_home_config
else:
  if root_is_writable():
    generate_config(embedded_config, first_time=True)
  else:
    generate_config(user_home_config, first_time=True)
  sys.exit(0)

if '\n' in EM_CONFIG:
  config_file = None
  logger.debug('config is specified inline without a file')
else:
  config_file = os.path.expanduser(EM_CONFIG)
  logger.debug('emscripten config is located in ' + config_file)
  if not os.path.exists(config_file):
    exit_with_error('emscripten config file not found: ' + config_file)

# Emscripten compiler spawns other processes, which can reimport shared.py, so
# make sure that those child processes get the same configuration file by
# setting it to the currently active environment.
os.environ['EM_CONFIG'] = EM_CONFIG

parse_config_file()
