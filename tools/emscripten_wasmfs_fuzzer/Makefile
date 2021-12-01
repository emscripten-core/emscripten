EMCC = ../../emcc
SOURCES = fuzzer.cpp random.cpp support/command-line.cpp

# Build with WasmFS enabled and pthreads.
EMFLAGS = -s WASMFS -s NO_DEFAULT_TO_CXX -s PROXY_TO_PTHREAD -s USE_PTHREADS -sINITIAL_MEMORY=3GB -Werror -Wno-pthreads-mem-growth -I. --profiling-funcs 
CFLAGS = -std=c++17

build: $(SOURCES)
	$(EMCC) $(SOURCES) -o fuzzer.js $(EMFLAGS) $(CFLAGS)

# Build with debugging info.
build_debug: $(SOURCES)
	$(EMCC) $(SOURCES) -o fuzzer.js $(EMFLAGS) -D FUZZER_DEBUG $(CFLAGS)
