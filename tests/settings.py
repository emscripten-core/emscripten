DEBUG=False
TEMP_DIR='/dev/shm'

CLANG=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin/clang++')
LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-2.7/cbuild/install/bin/llvm-g++')

COMPILERS = {
  'clang': {
    'path': CLANG,
    'quantum_size': 4, # See settings.js
  },
  'llvm_gcc': {
    'path': LLVM_GCC,
    'quantum_size': 1,
  }
}

COMPILER_OPTS = ['-m32'] # Need to build as 32bit arch, for now -
                         # various errors on 64bit compilation

LLVM_DIS=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin/llvm-dis')

SPIDERMONKEY_ENGINE=os.path.expanduser('~/Dev/mozilla-central/js/src/js')
V8_ENGINE=os.path.expanduser('~/Dev/v8/d8')

# XXX Warning: Compiling the 'sauer' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659. Possibly also some other tests as well.
#PARSER_ENGINE=SPIDERMONKEY_ENGINE
PARSER_ENGINE=V8_ENGINE

JS_ENGINE=SPIDERMONKEY_ENGINE
#JS_ENGINE=V8_ENGINE

JS_ENGINE_OPTS=[]

OUTPUT_TO_SCREEN = 0 # useful for debugging specific tests, or for subjectively seeing what parts are slow

