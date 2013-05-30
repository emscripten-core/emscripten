# Makefile for Lzip - Data compressor based on the LZMA algorithm
# Copyright (C) 2008, 2009, 2010, 2011 Antonio Diaz Diaz.
# This file was generated automatically by configure. Do not edit.
#
# This Makefile is free software: you have unlimited permission
# to copy, distribute and modify it.

pkgname = lzip
pkgversion = 1.12
progname = lzip
VPATH = .
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datadir = $(prefix)/share
infodir = $(datadir)/info
mandir = $(datadir)/man
sysconfdir = $(prefix)/etc
CXX = g++
CPPFLAGS = 
CXXFLAGS = -Wall -W -O2
LDFLAGS = 

DISTNAME = $(pkgname)-$(pkgversion)
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -p -m 755
INSTALL_DATA = $(INSTALL) -p -m 644
INSTALL_DIR = $(INSTALL) -d -m 755
SHELL = /bin/sh

objs = arg_parser.o decoder.o encoder.o fast_encoder.o main.o
recobjs = arg_parser.o decoder.o lziprecover.o
unzobjs = arg_parser.o unzcrash.o


.PHONY : all install install-info install-man install-strip \
         uninstall uninstall-info uninstall-man \
         doc info man check dist clean distclean

all : $(progname) lziprecover

$(progname) : $(objs)
	$(CXX) $(LDFLAGS) -o $@ $(objs)

$(progname)_profiled : $(objs)
	$(CXX) $(LDFLAGS) -pg -o $@ $(objs)

lziprecover : $(recobjs)
	$(CXX) $(LDFLAGS) -o $@ $(recobjs)

unzcrash : $(unzobjs)
	$(CXX) $(LDFLAGS) -o $@ $(unzobjs)

main.o : main.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DPROGVERSION=\"$(pkgversion)\" -c -o $@ $<

lziprecover.o : lziprecover.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DPROGVERSION=\"$(pkgversion)\" -c -o $@ $<

unzcrash.o : testsuite/unzcrash.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DPROGVERSION=\"$(pkgversion)\" -c -o $@ $<

%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(objs)        : Makefile
arg_parser.o   : arg_parser.h
decoder.o      : lzip.h decoder.h
encoder.o      : lzip.h encoder.h
fast_encoder.o : lzip.h encoder.h fast_encoder.h
main.o         : arg_parser.h lzip.h decoder.h encoder.h fast_encoder.h
lziprecover.o  : arg_parser.h lzip.h decoder.h Makefile
unzcrash.o     : arg_parser.h Makefile


doc : info man

info : $(VPATH)/doc/$(pkgname).info

$(VPATH)/doc/$(pkgname).info : $(VPATH)/doc/$(pkgname).texinfo
	cd $(VPATH)/doc && makeinfo $(pkgname).texinfo

man : $(VPATH)/doc/$(progname).1 $(VPATH)/doc/lziprecover.1

$(VPATH)/doc/$(progname).1 : $(progname)
	help2man -n 'reduces the size of files' \
	  -o $@ ./$(progname)

$(VPATH)/doc/lziprecover.1 : lziprecover
	help2man -n 'recovers data from damaged lzip files' \
	  -o $@ --no-info ./lziprecover

Makefile : $(VPATH)/configure $(VPATH)/Makefile.in
	./config.status

check : all
	@$(VPATH)/testsuite/check.sh $(VPATH)/testsuite $(pkgversion)

install : all install-info install-man
	if [ ! -d "$(DESTDIR)$(bindir)" ] ; then $(INSTALL_DIR) "$(DESTDIR)$(bindir)" ; fi
	$(INSTALL_PROGRAM) ./$(progname) "$(DESTDIR)$(bindir)/$(progname)"
	$(INSTALL_PROGRAM) ./lziprecover "$(DESTDIR)$(bindir)/lziprecover"

install-info :
	if [ ! -d "$(DESTDIR)$(infodir)" ] ; then $(INSTALL_DIR) "$(DESTDIR)$(infodir)" ; fi
	$(INSTALL_DATA) $(VPATH)/doc/$(pkgname).info "$(DESTDIR)$(infodir)/$(pkgname).info"
	-install-info --info-dir="$(DESTDIR)$(infodir)" $(DESTDIR)$(infodir)/$(pkgname).info

install-man :
	if [ ! -d "$(DESTDIR)$(mandir)/man1" ] ; then $(INSTALL_DIR) "$(DESTDIR)$(mandir)/man1" ; fi
	$(INSTALL_DATA) $(VPATH)/doc/$(progname).1 "$(DESTDIR)$(mandir)/man1/$(progname).1"
	$(INSTALL_DATA) $(VPATH)/doc/lziprecover.1 "$(DESTDIR)$(mandir)/man1/lziprecover.1"

install-strip : all
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall : uninstall-info uninstall-man
	-rm -f "$(DESTDIR)$(bindir)/$(progname)"
	-rm -f "$(DESTDIR)$(bindir)/lziprecover"

uninstall-info :
	-install-info --info-dir="$(DESTDIR)$(infodir)" --remove "$(DESTDIR)$(infodir)/$(pkgname).info"
	-rm -f "$(DESTDIR)$(infodir)/$(pkgname).info"

uninstall-man :
	-rm -f "$(DESTDIR)$(mandir)/man1/$(progname).1"
	-rm -f "$(DESTDIR)$(mandir)/man1/lziprecover.1"

dist : doc
	ln -sf $(VPATH) $(DISTNAME)
	tar -cvf $(DISTNAME).tar \
	  $(DISTNAME)/AUTHORS \
	  $(DISTNAME)/COPYING \
	  $(DISTNAME)/ChangeLog \
	  $(DISTNAME)/INSTALL \
	  $(DISTNAME)/Makefile.in \
	  $(DISTNAME)/NEWS \
	  $(DISTNAME)/README \
	  $(DISTNAME)/configure \
	  $(DISTNAME)/doc/$(progname).1 \
	  $(DISTNAME)/doc/lziprecover.1 \
	  $(DISTNAME)/doc/$(pkgname).info \
	  $(DISTNAME)/doc/$(pkgname).texinfo \
	  $(DISTNAME)/testsuite/check.sh \
	  $(DISTNAME)/testsuite/test.txt \
	  $(DISTNAME)/testsuite/test_bad[1-5].lz \
	  $(DISTNAME)/testsuite/test_sync.lz \
	  $(DISTNAME)/testsuite/test_v[01].lz \
	  $(DISTNAME)/testsuite/unzcrash.cc \
	  $(DISTNAME)/*.h \
	  $(DISTNAME)/*.cc
	rm -f $(DISTNAME)
	lzip -v -9 $(DISTNAME).tar

clean :
	-rm -f $(progname) $(progname)_profiled $(objs)
	-rm -f lziprecover lziprecover.o unzcrash unzcrash.o

distclean : clean
	-rm -f Makefile config.status *.tar *.tar.lz
