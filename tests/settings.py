TEMP_DIR='/dev/shm'

#LLVM_ROOT=os.path.expanduser('~/Dev/llvm-2.8/cbuild/Release/bin')
LLVM_ROOT=os.path.expanduser('~/Dev/llvm-svn/build/Release+Asserts/bin')

#LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-4.2-2.8.source/cbuild/install/bin/llvm-g++')
LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-svn/cbuild/install/bin/llvm-g++')

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
JS_ENGINE_PARAMS = ['--'] # For V8
JS_ENGINE_PARAMS = [] # For SpiderMonkey

TIMEOUT = None

# Tools

CLOSURE_COMPILER = os.path.expanduser('~/Dev/closure-compiler/compiler.jar')

