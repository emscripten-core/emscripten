#
# Makefile for musl (requires GNU make)
#
# This is how simple every makefile should be...
# No, I take that back - actually most should be less than half this size.
#
# Use config.mak to override any of the following variables.
# Do not make changes here.
#

exec_prefix = /usr/local
bindir = $(exec_prefix)/bin

prefix = /usr/local/musl
includedir = $(prefix)/include
libdir = $(prefix)/lib
syslibdir = /lib

SRCS = $(sort $(wildcard src/*/*.c arch/$(ARCH)/src/*.c))
OBJS = $(SRCS:.c=.o)
LOBJS = $(OBJS:.o=.lo)
GENH = include/bits/alltypes.h
GENH_INT = src/internal/version.h
IMPH = src/internal/stdio_impl.h src/internal/pthread_impl.h src/internal/libc.h

LDFLAGS = 
LIBCC = -lgcc
CPPFLAGS =
CFLAGS = -Os -pipe
CFLAGS_C99FSE = -std=c99 -ffreestanding -nostdinc 

CFLAGS_ALL = $(CFLAGS_C99FSE)
CFLAGS_ALL += -D_XOPEN_SOURCE=700 -I./arch/$(ARCH) -I./src/internal -I./include
CFLAGS_ALL += $(CPPFLAGS) $(CFLAGS)
CFLAGS_ALL_STATIC = $(CFLAGS_ALL)
CFLAGS_ALL_SHARED = $(CFLAGS_ALL) -fPIC -DSHARED

AR      = $(CROSS_COMPILE)ar
RANLIB  = $(CROSS_COMPILE)ranlib
INSTALL = ./tools/install.sh

ARCH_INCLUDES = $(wildcard arch/$(ARCH)/bits/*.h)
ALL_INCLUDES = $(sort $(wildcard include/*.h include/*/*.h) $(GENH) $(ARCH_INCLUDES:arch/$(ARCH)/%=include/%))

EMPTY_LIB_NAMES = m rt pthread crypt util xnet resolv dl
EMPTY_LIBS = $(EMPTY_LIB_NAMES:%=lib/lib%.a)
CRT_LIBS = lib/crt1.o lib/Scrt1.o lib/crti.o lib/crtn.o
STATIC_LIBS = lib/libc.a
SHARED_LIBS = lib/libc.so
TOOL_LIBS = lib/musl-gcc.specs
ALL_LIBS = $(CRT_LIBS) $(STATIC_LIBS) $(SHARED_LIBS) $(EMPTY_LIBS) $(TOOL_LIBS)
ALL_TOOLS = tools/musl-gcc

LDSO_PATHNAME = $(syslibdir)/ld-musl-$(ARCH)$(SUBARCH).so.1

-include config.mak

all: $(ALL_LIBS) $(ALL_TOOLS)

install: install-libs install-headers install-tools

clean:
	rm -f crt/*.o
	rm -f $(OBJS)
	rm -f $(LOBJS)
	rm -f $(ALL_LIBS) lib/*.[ao] lib/*.so
	rm -f $(ALL_TOOLS)
	rm -f $(GENH) $(GENH_INT)
	rm -f include/bits

distclean: clean
	rm -f config.mak

include/bits:
	@test "$(ARCH)" || { echo "Please set ARCH in config.mak before running make." ; exit 1 ; }
	ln -sf ../arch/$(ARCH)/bits $@

include/bits/alltypes.h.in: include/bits

include/bits/alltypes.h: include/bits/alltypes.h.in include/alltypes.h.in tools/mkalltypes.sed
	sed -f tools/mkalltypes.sed include/bits/alltypes.h.in include/alltypes.h.in > $@

src/internal/version.h: $(wildcard VERSION .git)
	printf '#define VERSION "%s"\n' "$$(sh tools/version.sh)" > $@

src/internal/version.lo: src/internal/version.h

src/ldso/dynlink.lo: arch/$(ARCH)/reloc.h

crt/crt1.o crt/Scrt1.o: $(wildcard arch/$(ARCH)/crt_arch.h)

crt/Scrt1.o: CFLAGS += -fPIC

OPTIMIZE_SRCS = $(wildcard $(OPTIMIZE_GLOBS:%=src/%))
$(OPTIMIZE_SRCS:%.c=%.o) $(OPTIMIZE_SRCS:%.c=%.lo): CFLAGS += -O3

MEMOPS_SRCS = src/string/memcpy.c src/string/memmove.c src/string/memcmp.c src/string/memset.c
$(MEMOPS_SRCS:%.c=%.o) $(MEMOPS_SRCS:%.c=%.lo): CFLAGS += $(CFLAGS_MEMOPS)

# This incantation ensures that changes to any subarch asm files will
# force the corresponding object file to be rebuilt, even if the implicit
# rule below goes indirectly through a .sub file.
define mkasmdep
$(dir $(patsubst %/,%,$(dir $(1))))$(notdir $(1:.s=.o)): $(1)
endef
$(foreach s,$(wildcard src/*/$(ARCH)*/*.s),$(eval $(call mkasmdep,$(s))))

%.o: $(ARCH)$(ASMSUBARCH)/%.sub
	$(CC) $(CFLAGS_ALL_STATIC) -c -o $@ $(dir $<)$(shell cat $<)

%.o: $(ARCH)/%.s
	$(CC) $(CFLAGS_ALL_STATIC) -c -o $@ $<

%.o: %.c $(GENH) $(IMPH)
	$(CC) $(CFLAGS_ALL_STATIC) -c -o $@ $<

%.lo: $(ARCH)$(ASMSUBARCH)/%.sub
	$(CC) $(CFLAGS_ALL_SHARED) -c -o $@ $(dir $<)$(shell cat $<)

%.lo: $(ARCH)/%.s
	$(CC) $(CFLAGS_ALL_SHARED) -c -o $@ $<

%.lo: %.c $(GENH) $(IMPH)
	$(CC) $(CFLAGS_ALL_SHARED) -c -o $@ $<

lib/libc.so: $(LOBJS)
	$(CC) $(CFLAGS_ALL_SHARED) $(LDFLAGS) -nostdlib -shared \
	-Wl,-e,_start -Wl,-Bsymbolic-functions \
	-o $@ $(LOBJS) $(LIBCC)

lib/libc.a: $(OBJS)
	rm -f $@
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

$(EMPTY_LIBS):
	rm -f $@
	$(AR) rc $@

lib/%.o: crt/%.o
	cp $< $@

lib/musl-gcc.specs: tools/musl-gcc.specs.sh config.mak
	sh $< "$(includedir)" "$(libdir)" "$(LDSO_PATHNAME)" > $@

tools/musl-gcc: config.mak
	printf '#!/bin/sh\nexec "$${REALGCC:-gcc}" "$$@" -specs "%s/musl-gcc.specs"\n' "$(libdir)" > $@
	chmod +x $@

$(DESTDIR)$(bindir)/%: tools/%
	$(INSTALL) -D $< $@

$(DESTDIR)$(libdir)/%.so: lib/%.so
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(libdir)/%: lib/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(includedir)/bits/%: arch/$(ARCH)/bits/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(includedir)/%: include/%
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(LDSO_PATHNAME): $(DESTDIR)$(libdir)/libc.so
	$(INSTALL) -D -l $(libdir)/libc.so $@ || true

install-libs: $(ALL_LIBS:lib/%=$(DESTDIR)$(libdir)/%) $(if $(SHARED_LIBS),$(DESTDIR)$(LDSO_PATHNAME),)

install-headers: $(ALL_INCLUDES:include/%=$(DESTDIR)$(includedir)/%)

install-tools: $(ALL_TOOLS:tools/%=$(DESTDIR)$(bindir)/%)



.PRECIOUS: $(CRT_LIBS:lib/%=crt/%)

.PHONY: all clean install install-libs install-headers install-tools
