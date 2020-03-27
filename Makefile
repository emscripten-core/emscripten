VERSION=$(shell cat emscripten-version.txt | sed s/\"//g)
DISTDIR=../emscripten-$(VERSION)
EXCLUDES=tests site __pycache__ node_modules docs Makefile
DISTFILE=emscripten-$(VERSION).tar.bz2
EXCLUDE_PATTERN=--exclude='*.pyc' --exclude='*/__pycache__'

dist: $(DISTFILE)

# Create an distributable archive of emscripten suitable for use
# by end users.  This archive excludes parts of the codebase that
# are you only used by emscripten developers.
$(DISTFILE):
	@rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	cp -ar * $(DISTDIR)
	for exclude in $(EXCLUDES); do rm -rf $(DISTDIR)/$$exclude; done
	tar cf $@ $(EXCLUDE_PATTERN) -C `dirname $(DISTDIR)` `basename $(DISTDIR)`

.PHONY: dist
