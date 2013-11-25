if BUILD_GTK_TEST

if BUILD_SPLASH_OUTPUT

splash_includes =				\
	-I$(top_srcdir)/splash

endif

if BUILD_CAIRO_OUTPUT

pdf_inspector =				\
	pdf_inspector

cairo_includes =				\
	$(CAIRO_CFLAGS)				\
	$(FREETYPE_CFLAGS)

endif

gtk_test = \
	gtk-test

endif

if BUILD_SPLASH_OUTPUT

perf_test =				\
	perf-test

endif

pdf_fullrewrite = \
	pdf-fullrewrite

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/glib			\
	-I$(top_builddir)/glib			\
	$(cairo_includes)			\
	$(GTK_TEST_CFLAGS)

noinst_PROGRAMS = $(pdf_inspector) $(perf_test) $(pdf_fullrewrite) $(gtk_test)

AM_LDFLAGS = @auto_import_flags@

gtk_test_SOURCES =					\
	gtk-test.cc

gtk_test_LDADD =					\
	$(top_builddir)/poppler/libpoppler.la           \
	$(top_builddir)/glib/libpoppler-glib.la         \
	$(CAIRO_LIBS)					\
	$(GTK_TEST_LIBS)				\
	$(PTHREAD_LIBS)


pdf_inspector_SOURCES =			\
       pdf-inspector.cc

pdf_inspector_LDADD =					\
	$(top_builddir)/poppler/libpoppler-cairo.la	\
	$(top_builddir)/poppler/libpoppler.la		\
	$(CAIRO_LIBS)					\
	$(FREETYPE_LIBS)				\
	$(GTK_TEST_LIBS)				\
	$(X_EXTRA_LIBS)					\
	$(PTHREAD_LIBS)

perf_test_SOURCES =			\
       perf-test.cc                     \
       perf-test-preview-dummy.cc

perf_test_LDADD =				\
	$(top_builddir)/poppler/libpoppler.la	\
	$(FREETYPE_LIBS)					\
	$(X_EXTRA_LIBS)

pdf_fullrewrite_SOURCES = \
	pdf-fullrewrite.cc		\
	../utils/parseargs.cc

pdf_fullrewrite_LDADD = \
	$(top_builddir)/poppler/libpoppler.la

EXTRA_DIST =					\
	pdf-operators.c				\
	pdf-inspector.ui
