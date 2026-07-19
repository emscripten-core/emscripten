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
# Other options
#
# FROZEN_CACHE = True # never clears the cache, and disallows building to the cache
