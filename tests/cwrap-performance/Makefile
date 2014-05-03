EMSCRIPTEN_ROOT?=../../

all:add.js benchmark-native

add.js:add.c Makefile ../../src/preamble.js
	$(EMSCRIPTEN_ROOT)emcc -O3 --closure 1 -s EXPORTED_FUNCTIONS="['_add', '_concat']" add.c -o add.js

benchmark-native:add.c benchmark.o
	cc -O3 add.c benchmark.o -o benchmark-native

benchmark.o: benchmark.c
	cc -O0 -c benchmark.c -o benchmark.o

clean:
	rm -rf add.js benchmark-native
