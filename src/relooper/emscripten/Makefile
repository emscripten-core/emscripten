EMSCRIPTEN = ~/Dev/emscripten
EMCC = $(EMSCRIPTEN)/emcc
BINDINGS_GENERATOR = $(EMSCRIPTEN)/tools/bindings_generator.py
NATIVIZER = $(EMSCRIPTEN)/tools/nativize_llvm.py

all: relooper.js

relooper.js:
	$(EMCC) -O2 ../Relooper.cpp -I.. --post-js glue.js -o relooper.raw.js -s TOTAL_MEMORY=52428800 -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE='["memcpy", "memset", "malloc", "free", "puts"]'
	echo "// Relooper, (C) 2012 Alon Zakai, MIT license, https://github.com/kripken/Relooper" > relooper.js
	echo "var Relooper = (function() {" >> relooper.js
	cat relooper.raw.js >> relooper.js
	echo "  return Module.Relooper;" >> relooper.js
	echo "})();" >> relooper.js

clean:
	rm relooper.js

