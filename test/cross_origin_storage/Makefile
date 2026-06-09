# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

EMCC ?= emcc

.PHONY: all clean

all: index.js

index.js: main.cpp
	$(EMCC) main.cpp -o index.js \
	    -sCROSS_ORIGIN_STORAGE \
	    -sENVIRONMENT=web \
	    -sEXPORTED_RUNTIME_METHODS=ccall \
	    -sEXPORTED_FUNCTIONS=_greet \
	    -sALLOW_MEMORY_GROWTH

clean:
	rm -f index.js index.wasm
