# This file will be copied to ~/.emscripten if that file doesn't exist.
# IMPORTANT: Edit it with the right paths!

EMSCRIPTEN_ROOT=os.path.expanduser("~/Dev/emscripten") # TODO: Use this

TEMP_DIR='/tmp'

LLVM_ROOT=os.path.expanduser('~/Dev/llvm-3.0/cbuild/bin')

COMPILER_OPTS = []

SPIDERMONKEY_ENGINE = [os.path.expanduser('~/Dev/mozilla-central/js/src/js'), '-m', '-n']
V8_ENGINE = [os.path.expanduser('~/Dev/v8/d8')]

#COMPILER_ENGINE=SPIDERMONKEY_ENGINE
COMPILER_ENGINE=V8_ENGINE

JS_ENGINE=V8_ENGINE

TIMEOUT = None

# Tools

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar')
NODE_JS = 'node'

