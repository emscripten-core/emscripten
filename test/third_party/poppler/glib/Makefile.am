AM_LDFLAGS = @auto_import_flags@

SUBDIRS = . reference demo
INCLUDES =					\
	-DG_LOG_DOMAIN=\"Poppler\"		\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(POPPLER_GLIB_CFLAGS)			\
	$(GDK_CFLAGS)				\
	$(FREETYPE_CFLAGS)			\
	$(FONTCONFIG_CFLAGS)                    \
	$(POPPLER_GLIB_DISABLE_DEPRECATED)	\
	$(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES)

stamp_files = stamp-poppler-enums.h

poppler-enums.h: stamp-poppler-enums.h
	@true

stamp-poppler-enums.h: poppler-enums.h.template $(poppler_glib_public_headers)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbh \
	&& (cmp -s xgen-etbh poppler-enums.h || cp xgen-etbh poppler-enums.h ) \
	&& rm -f xgen-etbh \
	&& echo timestamp > $(@F)

poppler-enums.c: poppler-enums.c.template $(poppler_glib_public_headers)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbc \
	&& (cmp -s xgen-etbc poppler-enums.c || cp xgen-etbc poppler-enums.c ) \
	&& rm -f xgen-etbc

EXTRA_DIST = \
	poppler-enums.h.template	\
	poppler-enums.c.template

poppler_glib_public_headers =			\
	poppler-action.h			\
	poppler-date.h				\
	poppler-document.h			\
	poppler-page.h				\
	poppler-attachment.h			\
	poppler-form-field.h			\
	poppler-annot.h				\
	poppler-layer.h				\
	poppler-media.h				\
	poppler-movie.h				\
	poppler.h

poppler_glib_includedir = $(includedir)/poppler/glib

poppler_glib_include_HEADERS =			\
	$(poppler_glib_public_headers)		\
	poppler-enums.h
nodist_poppler_glib_include_HEADERS = poppler-features.h

lib_LTLIBRARIES = libpoppler-glib.la
libpoppler_glib_la_SOURCES =			\
	$(BUILT_SOURCES)			\
	poppler-action.cc			\
	poppler-date.cc				\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-attachment.cc			\
	poppler-form-field.cc			\
	poppler-annot.cc			\
	poppler-layer.cc			\
	poppler-media.cc			\
	poppler-movie.cc			\
	poppler.cc				\
	poppler-private.h

libpoppler_glib_la_LIBADD =				\
	$(top_builddir)/poppler/libpoppler.la		\
	$(top_builddir)/poppler/libpoppler-cairo.la	\
	$(POPPLER_GLIB_LIBS)				\
	$(GDK_LIBS)					\
	$(FREETYPE_LIBS)				\
	$(FONTCONFIG_LIBS)

libpoppler_glib_la_LDFLAGS = -version-info 6:0:0 @create_shared_lib@ @auto_import_flags@

if BUILD_WITH_GDK
noinst_PROGRAMS = test-poppler-glib

test_poppler_glib_SOURCES =			\
       test-poppler-glib.cc

test_poppler_glib_LDADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-glib.la			\
	$(POPPLER_GLIB_LIBS)			\
	$(GDK_LIBS)				\
	$(FREETYPE_LIBS)
endif

BUILT_SOURCES =					\
	poppler-enums.c				\
	poppler-enums.h

CLEANFILES = $(BUILT_SOURCES) $(stamp_files)
DISTCLEANFILES = $(BUILT_SOURCES) $(stamp_files)
MAINTAINERCLEANFILES = $(BUILT_SOURCES) $(stamp_files)

# GObject Introspection
if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_files = $(libpoppler_glib_la_SOURCES) $(poppler_glib_include_HEADERS)
Poppler-0.16.gir: libpoppler-glib.la
Poppler_0_16_gir_INCLUDES = GObject-2.0 cairo-1.0
Poppler_0_16_gir_CFLAGS = $(INCLUDES) -I$(top_builddir)
Poppler_0_16_gir_LIBS = libpoppler-glib.la
Poppler_0_16_gir_FILES = $(addprefix $(srcdir)/, $(introspection_files))

if BUILD_WITH_GDK
Poppler_0_16_gir_INCLUDES += Gdk-2.0
endif

INTROSPECTION_GIRS += Poppler-0.16.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelibs_DATA)
endif #HAVE_INTROSPECTION
