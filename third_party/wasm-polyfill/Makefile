# Patches welcome to actually make this portable

default: tools/pack-asmjs.js jslib/load-wasm-worker.js

tools/pack-asmjs.js: src/pack-asmjs.cpp src/unpack.cpp src/unpack.h src/shared.h ../../tools/optimizer/parser.h ../../tools/optimizer/parser.cpp ../../tools/optimizer/istring.h
	mkdir -p tools
	$(EMSCRIPTEN)/emcc -O3 -std=c++11 -DCHECKED_OUTPUT_SIZE -Wall -pedantic \
	    --memory-init-file 0 --llvm-lto 1 -s TOTAL_MEMORY=67108864 -s WASM=0 \
      src/pack-asmjs.cpp src/unpack.cpp ../../tools/optimizer/parser.cpp \
	    -o tools/pack-asmjs.js -I../../tools/optimizer/ -s INVOKE_RUN=0
	cat src/pack-asmjs.js >> tools/pack-asmjs.js

jslib/load-wasm-worker.js: src/unpack.cpp src/unpack.h src/shared.h src/load-wasm-worker.js
	$(EMSCRIPTEN)/emcc -O3 -std=c++11 -Wall -pedantic \
	     --memory-init-file 0 --llvm-lto 1 -s TOTAL_MEMORY=67108864 -s WASM=0 \
	     src/unpack.cpp \
	     -o jslib/load-wasm-worker.js
	cat src/load-wasm-worker.js >> jslib/load-wasm-worker.js

.PHONY: test
test: tools/pack-asmjs tools/unpack-asmjs
	mkdir -p /tmp/test
	for tjs in test/*.js; do \
		t=$${tjs%.js}; \
		( tools/pack-asmjs $$t.js /tmp/$$t.wasm || \
		  ( echo "Failure running:  tools/pack-asmjs $$t.js /tmp/$$t.wasm" && false) ) && \
		\
		( tools/unpack-asmjs /tmp/$$t.wasm /tmp/$$t.js || \
		  ( echo "Failure running:  tools/unpack-asmjs /tmp/$$t.wasm /tmp/$$t.js" && false) ) && \
		\
		( diff $$t.js /tmp/$$t.js || \
		  ( echo "Failure running:  diff $$t.js /tmp/$$t.js" && false) ); \
		\
		true; \
	done

.PHONY: clean
clean:
	rm -f tools/pack-asmjs.js jslib/load-wasm-worker.js
	rmdir tools

