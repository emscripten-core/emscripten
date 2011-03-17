if BUILD_GTK_TEST

if BUILD_SPLASH_OUTPUT

gtk_splash_test =				\
	gtk-splash-test

splash_includes =				\
	-I$(top_srcdir)/splash

endif

if BUILD_CAIRO_OUTPUT

gtk_cairo_test =				\
	gtk-cairo-test

pdf_inspector =				\
	pdf_inspector

cairo_includes =				\
	$(CAIRO_CFLAGS)				\
	$(FREETYPE_CFLAGS)

endif

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
	$(GTK_TEST_CFLAGS)			\
	$(FONTCONFIG_CFLAGS)

noinst_PROGRAMS = $(gtk_splash_test) $(gtk_cairo_test) $(pdf_inspector) $(perf_test) $(pdf_fullrewrite)

AM_LDFLAGS = @auto_import_flags@

gtk_splash_test_SOURCES =			\
       gtk-splash-test.cc

gtk_splash_test_LDADD =				\
	$(top_builddir)/poppler/libpoppler.la	\
	$(GTK_TEST_LIBS)

gtk_cairo_test_SOURCES =			\
       gtk-cairo-test.cc

gtk_cairo_test_LDADD =					\
	$(top_builddir)/poppler/libpoppler.la		\
	$(top_builddir)/glib/libpoppler-glib.la		\
	$(CAIRO_LIBS)					\
	$(GTK_TEST_LIBS)				\
	$(FONTCONFIG_LIBS)

pdf_inspector_SOURCES =			\
       pdf-inspector.cc

pdf_inspector_LDADD =					\
	$(top_builddir)/poppler/libpoppler-cairo.la	\
	$(top_builddir)/poppler/libpoppler.la		\
	$(CAIRO_LIBS)					\
	$(FREETYPE_LIBS)				\
	$(GTK_TEST_LIBS)				\
	$(X_EXTRA_LIBS)

perf_test_SOURCES =			\
       perf-test.cc                     \
       perf-test-preview-dummy.cc

perf_test_LDADD =				\
	$(top_builddir)/poppler/libpoppler.la	\
	$(FREETYPE_LIBS)					\
	$(X_EXTRA_LIBS)

pdf_fullrewrite_SOURCES = \
	pdf-fullrewrite.cc

pdf_fullrewrite_LDADD = \
	$(top_builddir)/poppler/libpoppler.la

EXTRA_DIST =					\
	pdf-operators.c				\
	pdf-inspector.ui
