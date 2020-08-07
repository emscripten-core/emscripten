pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = libenet.pc

enetincludedir=$(includedir)/enet
enetinclude_HEADERS = \
	include/enet/callbacks.h \
	include/enet/enet.h \
	include/enet/list.h \
	include/enet/protocol.h \
	include/enet/time.h \
	include/enet/types.h \
	include/enet/unix.h \
	include/enet/utility.h \
	include/enet/win32.h

lib_LTLIBRARIES = libenet.la
libenet_la_SOURCES = callbacks.c compress.c host.c list.c packet.c peer.c protocol.c unix.c win32.c
# see info '(libtool) Updating version info' before making a release
libenet_la_LDFLAGS = $(AM_LDFLAGS) -version-info 1:3:0
INCLUDES = -I$(top_srcdir)/include

ACLOCAL_AMFLAGS = -Im4
