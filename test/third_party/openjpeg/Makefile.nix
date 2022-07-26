#top Makefile.nix
include  config.nix

TARGET  = openjpeg
COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99 -fPIC

INCLUDE = -I. -Ilibopenjpeg
LIBRARIES = -lstdc++

SRCS = ./libopenjpeg/bio.c ./libopenjpeg/cio.c ./libopenjpeg/dwt.c \
 ./libopenjpeg/event.c ./libopenjpeg/image.c ./libopenjpeg/j2k.c \
 ./libopenjpeg/j2k_lib.c ./libopenjpeg/jp2.c ./libopenjpeg/jpt.c \
 ./libopenjpeg/mct.c ./libopenjpeg/mqc.c ./libopenjpeg/openjpeg.c \
 ./libopenjpeg/pi.c ./libopenjpeg/raw.c ./libopenjpeg/t1.c \
 ./libopenjpeg/t2.c ./libopenjpeg/tcd.c ./libopenjpeg/tgt.c \

INCLS = ./libopenjpeg/bio.h ./libopenjpeg/cio.h ./libopenjpeg/dwt.h \
 ./libopenjpeg/event.h ./libopenjpeg/fix.h ./libopenjpeg/image.h \
 ./libopenjpeg/int.h ./libopenjpeg/j2k.h ./libopenjpeg/j2k_lib.h \
 ./libopenjpeg/jp2.h ./libopenjpeg/jpt.h ./libopenjpeg/mct.h \
 ./libopenjpeg/mqc.h ./libopenjpeg/openjpeg.h ./libopenjpeg/pi.h \
 ./libopenjpeg/raw.h ./libopenjpeg/t1.h ./libopenjpeg/t2.h \
 ./libopenjpeg/tcd.h ./libopenjpeg/tgt.h ./libopenjpeg/opj_malloc.h \
 ./libopenjpeg/opj_includes.h

INSTALL_LIBDIR = $(prefix)/lib
headerdir = openjpeg-$(MAJOR).$(MINOR)
INSTALL_INCLUDE = $(prefix)/include/$(headerdir)

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

MODULES = $(SRCS:.c=.o)

CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

LIBNAME = lib$(TARGET)

ifeq ($(ENABLE_SHARED),yes)
SHAREDLIB = $(LIBNAME).so.$(MAJOR).$(MINOR).$(BUILD)
else
STATICLIB = $(LIBNAME).a
endif

default: all

all: OpenJPEG
	make -C codec -f Makefile.nix all
	make -C mj2 -f Makefile.nix all
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix all
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix all
endif

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

OpenJPEG: $(STATICLIB) $(SHAREDLIB)
	install -d bin
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 $(SHAREDLIB) bin
	(cd bin && ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR))
	(cd bin && ln -sf $(SHAREDLIB) $(LIBNAME).so)
else
	install -m 644 $(STATICLIB) bin
endif

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(ENABLE_SHARED),yes)
$(SHAREDLIB): $(MODULES)
	$(CC) -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)
else
$(STATICLIB): $(MODULES)
	rm -f $(STATICLIB)
	$(AR) r $@ $(MODULES)
endif

install: OpenJPEG
	install -d $(DESTDIR)$(INSTALL_LIBDIR) 
ifeq ($(ENABLE_SHARED),yes)
	install -m 755 -o root -g root $(SHAREDLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(SHAREDLIB) $(LIBNAME).so.$(MAJOR).$(MINOR) )
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	ln -sf $(SHAREDLIB) $(LIBNAME).so )
else
	install -m 644 -o root -g root $(STATICLIB) $(DESTDIR)$(INSTALL_LIBDIR)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && ranlib $(STATICLIB) )
endif
	install -d $(DESTDIR)$(INSTALL_INCLUDE)
	install -m 644 -o root -g root libopenjpeg/openjpeg.h \
	$(DESTDIR)$(INSTALL_INCLUDE)
	(cd $(DESTDIR)$(prefix)/include && \
	ln -sf $(headerdir)/openjpeg.h openjpeg.h)
	make -C codec -f Makefile.nix install
	make -C mj2 -f Makefile.nix install
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix install
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix install
endif
	$(LDCONFIG)
	make -C doc -f Makefile.nix install

ifeq ($(WITH_JPWL),yes)
jpwl-all:
	make -C jpwl -f Makefile.nix all

jpwl-install: jpwl-all
	make -C jpwl -f Makefile.nix install
	$(LDCONFIG)

jpwl-clean:
	make -C jpwl -f Makefile.nix clean

jpwl-uninstall:
	make -C jpwl -f Makefile.nix uninstall
endif

ifeq ($(WITH_JP3D),yes)
jp3d-all:
	make -C jp3d -f Makefile.nix all

jp3d-install: jp3d-all
	make -C jp3d -f Makefile.nix install
	$(LDCONFIG)

jp3d-clean:
	make -C jp3d -f Makefile.nix clean

jp3d-uninstall:
	make -C jp3d -f Makefile.nix uninstall
endif

doc-all:
	make -C doc -f Makefile.nix all

doc-install: doc-all
	make -C doc -f Makefile.nix install

clean:
	rm -rf bin
	rm -f core u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB)
	make -C codec -f Makefile.nix clean
	make -C mj2 -f Makefile.nix clean
	make -C doc -f Makefile.nix clean
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix clean
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix clean
endif

doc-clean:
	make -C doc -f Makefile.nix clean

uninstall:
ifeq ($(ENABLE_SHARED),yes)
	(cd $(DESTDIR)$(INSTALL_LIBDIR) && \
	rm -f $(LIBNAME).so $(LIBNAME).so.$(MAJOR).$(MINOR) $(SHAREDLIB) )
else
	rm -f $(DESTDIR)$(INSTALL_LIBDIR)/$(STATICLIB)
endif
	$(LDCONFIG)
	rm -f $(DESTDIR)$(prefix)/include/openjpeg.h
	rm -rf $(DESTDIR)$(INSTALL_INCLUDE)
	make -C codec -f Makefile.nix uninstall
	make -C mj2 -f Makefile.nix uninstall
	make -C doc -f Makefile.nix uninstall
ifeq ($(WITH_JPWL),yes)
	make -C jpwl -f Makefile.nix uninstall
endif
ifeq ($(WITH_JP3D),yes)
	make -C jp3d -f Makefile.nix uninstall
endif

distclean: clean
	rm -rf bin
