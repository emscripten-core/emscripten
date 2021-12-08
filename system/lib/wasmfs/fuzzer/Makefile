EMCC = ../../../../emcc
SOURCES = fuzzer.cpp random.cpp workload.cpp support/command-line.cpp

# Build with WasmFS enabled and pthreads.
EMFLAGS = -s WASMFS -s PROXY_TO_PTHREAD -s USE_PTHREADS -s ALLOW_MEMORY_GROWTH=1 -Werror -Wno-pthreads-mem-growth -I. --profiling-funcs 
CFLAGS = -std=c++17

.PHONY: build
build: $(SOURCES)
	$(EMCC) $(SOURCES) -o fuzzer.js $(EMFLAGS) $(CFLAGS)

# Build with debugging info.
.PHONY: debug
debug: $(SOURCES)
	$(EMCC) $(SOURCES) -o fuzzer.js $(EMFLAGS) -D FUZZER_DEBUG $(CFLAGS)

.PHONY: clean
clean:
	rm *.js *wasm
