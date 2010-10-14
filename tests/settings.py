TEMP_DIR='/dev/shm'

LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.8/cbuild/Release/bin') # Might not need 'Release'
#LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin') # Might not need 'Release'

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

LLVM_DIS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-dis'))
LLVM_DIS_OPTS = []
if '2.8' in LLVM_ROOT:
  LLVM_DIS_OPTS += ['-show-annotations']

SPIDERMONKEY_ENGINE = [os.path.expanduser('~/Dev/tracemonkey/js/src/js'), '-m'] # No |-j| due to Mozilla bug XXX
V8_ENGINE = [os.path.expanduser('~/Dev/v8/d8')]

# XXX Warning: Compiling the 'sauer' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659. Possibly also some other tests as well.
#COMPILER_ENGINE=SPIDERMONKEY_ENGINE
COMPILER_ENGINE=V8_ENGINE

OUTPUT_TO_SCREEN = 0 # useful for debugging specific tests, or for subjectively seeing what parts are slow

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar')

