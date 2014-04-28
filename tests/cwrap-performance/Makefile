EMSCRIPTEN_ROOT=../../

all:benchmark.js benchmark-native

benchmark.js:add.c post.js Makefile
	$(EMSCRIPTEN_ROOT)emcc -s EXPORTED_FUNCTIONS="['_add', '_concat']" add.c --post-js post.js -o benchmark.js

benchmark-native:add.c benchmark.c
	cc -O0 add.c benchmark.c -o benchmark-native

clean:
	rm -rf benchmark.js
