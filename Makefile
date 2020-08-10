VERSION=$(shell cat emscripten-version.txt | sed s/\"//g)
DESTDIR=../emscripten-$(VERSION)
DISTFILE=emscripten-$(VERSION).tar.bz2

dist: $(DISTFILE)

install:
	@rm -rf $(DESTDIR)
	./tools/install.py $(DESTDIR)

# Create an distributable archive of emscripten suitable for use
# by end users.  This archive excludes parts of the codebase that
# are you only used by emscripten developers.
$(DISTFILE): install
	tar cf $@ $(EXCLUDE_PATTERN) -C `dirname $(DESTDIR)` `basename $(DESTDIR)`

.PHONY: dist install
