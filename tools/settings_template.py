# This file will be edited (the {{{ }}} things), and written to `.emscripten`
# when emscripten is first used and no config file is found.

# Note: If you put paths relative to the home directory, do not forget
# os.path.expanduser
#
# Any config setting <KEY> in this file can be overridden by setting the
# EM_<KEY> environment variable. For example, settings EM_LLVM_ROOT override
# the setting in this file.
#
# Note: On Windows, remember to escape backslashes! I.e. LLVM='c:\llvm\'
# is not valid, but LLVM='c:\\llvm\\' and LLVM='c:/llvm/'
# are.

import os

# This is used by external projects in order to find emscripten.  It is not used
# by emscripten itself.
EMSCRIPTEN_ROOT = os.path.expanduser(os.getenv('EMSCRIPTEN', '{{{ EMSCRIPTEN_ROOT }}}')) # directory

LLVM_ROOT = os.path.expanduser(os.getenv('LLVM', '{{{ LLVM_ROOT }}}')) # directory
BINARYEN_ROOT = os.path.expanduser(os.getenv('BINARYEN', '')) # directory

# Location of the node binary to use for running the JS parts of the compiler.
# This engine must exist, or nothing can be compiled.
NODE_JS = os.path.expanduser(os.getenv('NODE', '{{{ NODE }}}')) # executable

JAVA = 'java' # executable

################################################################################
#
# Test suite options:
#
# Alternative JS engines to use during testing:
#
# SPIDERMONKEY_ENGINE = [os.path.expanduser(os.getenv('SPIDERMONKEY', 'js'))] # executable
# V8_ENGINE = os.path.expanduser(os.getenv('V8', 'd8')) # executable
#
# All JS engines to use when running the automatic tests. Not all the engines in
# this list must exist (if they don't, they will be skipped in the test runner).
#
# JS_ENGINES = [NODE_JS] # add V8_ENGINE or SPIDERMONKEY_ENGINE if you have them installed too.
#
# WASMER = os.path.expanduser(os.path.join('~', '.wasmer', 'bin', 'wasmer'))
# WASMTIME = os.path.expanduser(os.path.join('~', 'wasmtime'))
#
# Wasm engines to use in STANDALONE_WASM tests.
#
# WASM_ENGINES = [] # add WASMER or WASMTIME if you have them installed
#
# Other options
#
# FROZEN_CACHE = True # never clears the cache, and disallows building to the cache
