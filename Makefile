VERSION = $(shell cat emscripten-version.txt | sed s/\"//g)
DESTDIR ?= out/emscripten-$(VERSION)
DISTFILE = emscripten-$(VERSION).tar.bz2

no_default:
	@echo 'Error: You must specify an explicit make target (e.g. `dist` or `install`)'
	@exit 1

dist: $(DISTFILE)

install:
	@rm -rf $(DESTDIR)
	./tools/install.py $(DESTDIR)
	npm install --omit=dev --prefix $(DESTDIR)

emcc_native:
	cmake -B out/build_emcc_native -S tools/emcc_native -G Ninja
	cmake --build out/build_emcc_native
	cmake --install out/build_emcc_native

# Create an distributable archive of emscripten suitable for use
# by end users. This archive excludes node_modules as it can include native
# modules which can't be safely pre-packaged.
$(DISTFILE): install
	tar cf $@ --exclude=node_modules -C `dirname $(DESTDIR)` `basename $(DESTDIR)`

.PHONY: dist install emcc_native
