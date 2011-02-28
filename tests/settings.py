TEMP_DIR='/dev/shm'

LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.8/cbuild/Release/bin') # Might not need 'Release'
#LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin')

CLANG=os.path.expanduser(os.path.join(LLVM_ROOT, 'clang++'))
LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-4.2-2.8.source/cbuild/install/bin/llvm-g++')

COMPILERS = {
  'clang': {
    'path': CLANG,
    'quantum_size': 4, # See settings.js
  },
  'llvm_gcc': {
    'path': LLVM_GCC,
    'quantum_size': 4,
  }
}

COMPILER_OPTS = ['-m32'] # Need to build as 32bit arch, for now -
                         # various errors on 64bit compilation
                         # WARNING: '-g' here will generate llvm bitcode that lli will crash on!

LLVM_OPT=os.path.expanduser(os.path.join(LLVM_ROOT, 'opt'))

LLVM_AS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-as'))
LLVM_DIS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-dis'))
LLVM_DIS_OPTS = ['-show-annotations'] # For LLVM 2.8+. For 2.7, you may need to do just    []

SPIDERMONKEY_ENGINE = [os.path.expanduser('~/Dev/tracemonkey/js/src/js'), '-m', '-j', '-p']
V8_ENGINE = [os.path.expanduser('~/Dev/v8/d8')]

# XXX Warning: Compiling the 'cubescript' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659. Possibly also some other tests as well.
#COMPILER_ENGINE=SPIDERMONKEY_ENGINE
COMPILER_ENGINE=V8_ENGINE

JS_ENGINE=V8_ENGINE
JS_ENGINE_PARAMS = ['--'] # For V8
JS_ENGINE_PARAMS = [] # For SpiderMonkey

OUTPUT_TO_SCREEN = 0 # useful for debugging specific tests, or for subjectively seeing what parts are slow

TIMEOUT = None

# Tools

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar')

