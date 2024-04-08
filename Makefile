VERSION = $(shell cat emscripten-version.txt | sed s/\"//g)
DESTDIR ?= ../emscripten-$(VERSION)
DISTFILE = emscripten-$(VERSION).tar.bz2

dist: $(DISTFILE)

install:
	@rm -rf $(DESTDIR)
	./tools/install.py $(DESTDIR)
	npm install --omit=dev --prefix $(DESTDIR)

# Create an distributable archive of emscripten suitable for use
# by end users. This archive excludes node_modules as it can include native
# modules which can't be safely pre-packaged.
$(DISTFILE): install
	tar cf $@ --exclude=node_modules -C `dirname $(DESTDIR)` `basename $(DESTDIR)`

.PHONY: dist install
