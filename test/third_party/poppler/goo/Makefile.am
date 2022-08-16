noinst_LTLIBRARIES = libgoo.la

if ENABLE_XPDF_HEADERS

poppler_goo_includedir = $(includedir)/poppler/goo
poppler_goo_include_HEADERS =			\
	GooHash.h				\
	GooList.h				\
	GooTimer.h				\
	GooMutex.h				\
	GooString.h				\
	gtypes.h				\
	gmem.h					\
	gfile.h					\
	FixedPoint.h				\
	PNGWriter.h				\
	JpegWriter.h				\
	ImgWriter.h				\
	GooLikely.h				\
	gstrtod.h

endif

if BUILD_LIBPNG
libpng_includes = $(LIBPNG_CFLAGS)
endif

INCLUDES =					\
	-I$(top_srcdir)				\
	$(libpng_includes)

libgoo_la_SOURCES =				\
	gfile.cc				\
	gmempp.cc				\
	GooHash.cc				\
	GooList.cc				\
	GooTimer.cc				\
	GooString.cc				\
	gmem.cc					\
	FixedPoint.cc				\
	PNGWriter.cc				\
	JpegWriter.cc				\
	ImgWriter.cc				\
	gstrtod.cc
