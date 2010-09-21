DEBUG=False
TEMP_DIR='/dev/shm'

CLANG=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin/clang++')
LLVM_GCC=os.path.expanduser('~/Dev/llvm-gcc-2.7/cbuild/install/bin/llvm-g++')

#COMPILER=LLVM_GCC
COMPILER=CLANG

COMPILER_OPTS = ['-m32'] # Need to build as 32bit arch, for now -
                         # various errors on 64bit compilation

LLVM_DIS=os.path.expanduser('~/Dev/llvm-2.7/cbuild/bin/llvm-dis')

SPIDERMONKEY_ENGINE=os.path.expanduser('~/Dev/m-c/js/src/js')
V8_ENGINE=os.path.expanduser('~/Dev/v8/d8')

# XXX Warning: Running the 'sauer' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659.
#PARSER_ENGINE=SPIDERMONKEY_ENGINE
PARSER_ENGINE=V8_ENGINE

JS_ENGINE=SPIDERMONKEY_ENGINE
#JS_ENGINE=V8_ENGINE

JS_ENGINE_OPTS=[]

