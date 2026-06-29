# This file will be edited (the {{{ }}} things), and written to `.emscripten`
# when emscripten is first used and no config file is found.

# Note: You can use environment variables in your settings. e.g. You can use
# `$HOME` on UNIX-like systems.  You can also use the `~/` prefix for
# referencing paths relative to the home directory.  In addition you can use
# `$CFGDIR` for the directory where the config itself file lives.
#
# Any config setting <KEY> in this file can be overridden by setting the
# EM_<KEY> environment variable. For example, settings EM_LLVM_ROOT override
# the setting in this file.
#
# Note: On Windows, remember to escape backslashes! I.e. LLVM='c:\llvm\'
# is not valid, but LLVM='c:\\llvm\\' and LLVM='c:/llvm/'
# are.

LLVM_ROOT = '{{{ LLVM_ROOT }}}' # directory
BINARYEN_ROOT = '{{{ BINARYEN_ROOT }}}' # directory

# Location of the node binary to use for running the JS parts of the compiler.
# This engine must exist, or nothing can be compiled.
NODE_JS = '{{{ NODE }}}' # executable

################################################################################
#
# Test suite options:
#
# Alternative JS engines to use during testing:
#
# NODE_JS_TEST = 'node' # executable
# SPIDERMONKEY_ENGINE = ['js'] # executable
# V8_ENGINE = 'd8' # executable
#
# All JS engines to use when running the automatic tests. Not all the engines in
# this list must exist (if they don't, they will be skipped in the test runner).
#
# JS_ENGINES = [NODE_JS_TEST] # add V8_ENGINE or SPIDERMONKEY_ENGINE if you have them installed too.
#
# WASMER = '~/.wasmer/bin/wasmer'
# WASMTIME = '~/wasmtime'
#
# Wasm engines to use in STANDALONE_WASM tests.
#
# WASM_ENGINES = [] # add WASMER or WASMTIME if you have them installed
#
################################################################################
#
# Other options
#
# FROZEN_CACHE = True # never clears the cache, and disallows building to the cache
