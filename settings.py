# This file will be copied to ~/.emscripten if that file doesn't exist.
# IMPORTANT: Edit it with the right paths!

EMSCRIPTEN_ROOT=os.path.expanduser("~/Dev/emscripten") # TODO: Use this

TEMP_DIR='/dev/shm'

LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.9/cbuild/bin')

LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-2.9/cbuild/install/bin/llvm-g++')

COMPILER_OPTS = ['-m32'] # Need to build as 32bit arch, for now -
                         # various errors on 64bit compilation
                         # WARNING: '-g' here will generate llvm bitcode that lli will crash on!

SPIDERMONKEY_ENGINE = [os.path.expanduser('~/Dev/tracemonkey/js/src/js'), '-m', '-j', '-p']
V8_ENGINE = [os.path.expanduser('~/Dev/v8/d8')]

# XXX Warning: Compiling the 'cubescript' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659. Possibly also some other tests as well.
#COMPILER_ENGINE=SPIDERMONKEY_ENGINE
COMPILER_ENGINE=V8_ENGINE

JS_ENGINE=V8_ENGINE

TIMEOUT = None

# Tools

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar')

