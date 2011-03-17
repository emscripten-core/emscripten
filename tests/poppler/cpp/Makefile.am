INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	-I$(top_srcdir)/poppler

SUBDIRS = . tests

poppler_includedir = $(includedir)/poppler/cpp
poppler_include_HEADERS =			\
	poppler-document.h			\
	poppler-embedded-file.h			\
	poppler-font.h				\
	poppler-global.h			\
	poppler-image.h			\
	poppler-page.h				\
	poppler-page-renderer.h			\
	poppler-page-transition.h		\
	poppler-rectangle.h			\
	poppler-toc.h				\
	$(builddir)/poppler-version.h

lib_LTLIBRARIES = libpoppler-cpp.la
libpoppler_cpp_la_SOURCES =			\
	poppler-document.cpp			\
	poppler-document-private.h		\
	poppler-embedded-file.cpp		\
	poppler-embedded-file-private.h		\
	poppler-font.cpp			\
	poppler-global.cpp			\
	poppler-image.cpp			\
	poppler-image-private.h			\
	poppler-page.cpp			\
	poppler-page-private.h			\
	poppler-page-renderer.cpp		\
	poppler-page-transition.cpp		\
	poppler-private.cpp			\
	poppler-private.h			\
	poppler-rectangle.cpp			\
	poppler-toc.cpp				\
	poppler-toc-private.h			\
	poppler-version.cpp

libpoppler_cpp_la_CXXFLAGS =			\
	-Dpoppler_cpp_EXPORTS

libpoppler_cpp_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(LIBICONV)

libpoppler_cpp_la_LDFLAGS = -version-info 2:0:2 @create_shared_lib@ @auto_import_flags@
