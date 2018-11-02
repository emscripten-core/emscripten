# This file will be edited (the {{{ }}} things), and then ~/.emscripten created
# with the result, if ~/.emscripten doesn't exist.

# Note: If you put paths relative to the home directory, do not forget
# os.path.expanduser

# Note: On Windows, remember to escape backslashes! I.e. PYTHON='c:\Python27\'
# is not valid, but PYTHON='c:\\Python27\\' and PYTHON='c:/Python27/'
# are.

import os

# This is used by external projects in order to find emscripten.  It is not used
# by emscripten itself.
EMSCRIPTEN_ROOT = os.path.expanduser(os.getenv('EMSCRIPTEN', '{{{ EMSCRIPTEN_ROOT }}}')) # directory

LLVM_ROOT = os.path.expanduser(os.getenv('LLVM', '{{{ LLVM_ROOT }}}')) # directory
BINARYEN_ROOT = os.path.expanduser(os.getenv('BINARYEN', '')) # if not set, we will use it from ports

# If not specified, defaults to sys.executable.
#PYTHON = 'python'

# Add this if you have manually built the JS optimizer executable (in
# Emscripten/tools/optimizer) and want to run it from a custom location.
# Alternatively, you can set this as the environment variable
# EMSCRIPTEN_NATIVE_OPTIMIZER.
#EMSCRIPTEN_NATIVE_OPTIMIZER='/path/to/custom/optimizer(.exe)'

# See below for notes on which JS engine(s) you need
NODE_JS = os.path.expanduser(os.getenv('NODE', '{{{ NODE }}}')) # executable
SPIDERMONKEY_ENGINE = [os.path.expanduser(os.getenv('SPIDERMONKEY', 'js'))] # executable
V8_ENGINE = os.path.expanduser(os.getenv('V8', 'd8')) # executable

JAVA = 'java' # executable

TEMP_DIR = '{{{ TEMP }}}'

#CLOSURE_COMPILER = '..' # define this to not use the bundled version

################################################################################


# Pick the JS engine to use for running the compiler. This engine must exist, or
# nothing can be compiled.
#
# This should be left on node.js, as that is the VM we test running the
# compiler in. Other VMs may or may not work.

COMPILER_ENGINE = NODE_JS


# All JS engines to use when running the automatic tests. Not all the engines in
# this list must exist (if they don't, they will be skipped in the test runner).
#
# Recommendation: If you already have node installed, use that. If you can, also
#                 build spidermonkey from source as well to get more test
#                 coverage.

JS_ENGINES = [NODE_JS] # add this if you have spidermonkey installed too, SPIDERMONKEY_ENGINE]
