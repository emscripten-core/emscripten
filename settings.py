# This file will be copied to ~/.emscripten if that file doesn't exist.
# IMPORTANT: Edit it with the right paths!

LLVM_ROOT=os.path.expanduser('~/Dev/llvm-3.0/cbuild/bin')

NODE_JS = 'node'
SPIDERMONKEY_ENGINE = [os.path.expanduser('~/Dev/mozilla-central/js/src/js'), '-m', '-n'] # optional, but recommended
V8_ENGINE = os.path.expanduser('~/Dev/v8/d8') # optional (mostly unneeded if you have node)

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar') # optional (needed for the benchmarks)

TEMP_DIR='/tmp'

########################################################################################################

# Pick the JS engine to use for running the compiler. Any of the three will work. This engine
# must exist, or nothing can be compiled.

COMPILER_ENGINE=NODE_JS
#COMPILER_ENGINE=SPIDERMONKEY_ENGINE
#COMPILER_ENGINE=V8_ENGINE

# JS engines to use when running the automatic tests. Modify this to include all
# the JS engines you have installed. Not all these engines must exist, if they do not,
# they will be skipped in the test runner.

JS_ENGINES=[NODE_JS, SPIDERMONKEY_ENGINE]

