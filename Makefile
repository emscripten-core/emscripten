VERSION=$(shell cat emscripten-version.txt | sed s/\"//g)
DISTDIR=../emscripten-$(VERSION)
DISTFILE=emscripten-$(VERSION).tar.bz2

dist: $(DISTFILE)

install:
	@rm -rf $(DISTDIR)
	./tools/install.py $(DISTDIR)

# Create an distributable archive of emscripten suitable for use
# by end users.  This archive excludes parts of the codebase that
# are you only used by emscripten developers.
$(DISTFILE): install
	tar cf $@ $(EXCLUDE_PATTERN) -C `dirname $(DISTDIR)` `basename $(DISTDIR)`

.PHONY: dist install
