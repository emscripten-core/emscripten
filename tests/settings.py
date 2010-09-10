DEBUG=False
TEMP_DIR='/dev/shm'
LLVM_GCC=os.path.expanduser('~/Dev/llvm/llvm-gcc-27/cbuild/bin/bin/llvm-g++')
LLVM_DIS=os.path.expanduser('~/Dev/llvm/llvm-27/cbuild/bin/llvm-dis')
SPIDERMONKEY_ENGINE=os.path.expanduser('~/Dev/tracemonkey/js/src/js')
V8_ENGINE=os.path.expanduser('~/Dev/v8/d8')

# XXX Warning: Running the 'sauer' test in SpiderMonkey can lead to an extreme amount of memory being
#              used, see Mozilla bug 593659.
#PARSER_ENGINE=SPIDERMONKEY_ENGINE
PARSER_ENGINE=V8_ENGINE

JS_ENGINE=SPIDERMONKEY_ENGINE
#JS_ENGINE=V8_ENGINE
JS_ENGINE_OPTS=[]#['-j']

