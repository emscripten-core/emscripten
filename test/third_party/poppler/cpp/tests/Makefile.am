AM_LDFLAGS = @auto_import_flags@

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/utils			\
	-I$(top_srcdir)/cpp

LDADDS =					\
	$(top_builddir)/poppler/libpoppler.la	\
	$(top_builddir)/cpp/libpoppler-cpp.la


noinst_PROGRAMS =				\
	poppler-dump				\
	poppler-render

poppler_dump_SOURCES = \
	$(top_srcdir)/utils/parseargs.cc	\
	poppler-dump.cpp
poppler_dump_LDADD = $(LDADDS)


poppler_render_SOURCES = \
	$(top_srcdir)/utils/parseargs.cc	\
	poppler-render.cpp
poppler_render_LDADD = $(LDADDS)

