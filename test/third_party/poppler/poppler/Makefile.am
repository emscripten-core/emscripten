if BUILD_SPLASH_OUTPUT

splash_sources =				\
	SplashOutputDev.cc

splash_headers =				\
	SplashOutputDev.h

splash_includes =				\
	$(SPLASH_CFLAGS)

splash_libs =					\
	$(SPLASH_LIBS)				\
	$(top_builddir)/splash/libsplash.la

endif

if BUILD_POPPLER_QT4

poppler_arthur = libpoppler-arthur.la

libpoppler_arthur_la_SOURCES =			\
	ArthurOutputDev.h			\
	ArthurOutputDev.cc

arthur_includes =				\
	$(POPPLER_QT4_CFLAGS)

arthur_libs =					\
	$(POPPLER_QT4_LIBS)

endif



if BUILD_CAIRO_OUTPUT

poppler_cairo = libpoppler-cairo.la

cairo_includes =				\
	$(CAIRO_CFLAGS)

cairo_libs =					\
	$(CAIRO_LIBS)

libpoppler_cairo_la_SOURCES =			\
	CairoFontEngine.cc			\
	CairoFontEngine.h			\
	CairoOutputDev.cc			\
	CairoOutputDev.h			\
	CairoRescaleBox.cc			\
	CairoRescaleBox.h

endif

if BUILD_LIBJPEG

libjpeg_sources =				\
	DCTStream.h				\
	DCTStream.cc

libjpeg_libs =					\
	$(LIBJPEG_LIBS)

endif

if BUILD_LIBPNG

libpng_libs =					\
	$(LIBPNG_LIBS)
libpng_includes =				\
	$(LIBPNG_CFLAGS)

endif

if BUILD_LIBOPENJPEG

libjpeg2000_sources =				\
	JPEG2000Stream.h			\
	JPEG2000Stream.cc

libjpeg2000_libs =				\
	$(LIBOPENJPEG_LIBS)

else

libjpeg2000_sources =				\
	JPXStream.h			\
	JPXStream.cc

endif


if BUILD_ZLIB

zlib_sources =					\
	FlateStream.h				\
	FlateStream.cc

zlib_libs = 					\
	$(ZLIB_LIBS)

endif

if BUILD_LIBCURL

libcurl_libs =					\
	$(LIBCURL_LIBS)

libcurl_includes =				\
	$(LIBCURL_CFLAGS)

curl_headers =					\
	CurlCachedFile.h			\
	CurlPDFDocBuilder.h

curl_sources =					\
	CurlCachedFile.cc			\
	CurlPDFDocBuilder.cc

endif

if BUILD_ABIWORD_OUTPUT

abiword_sources =				\
	ABWOutputDev.h				\
	ABWOutputDev.cc

abiword_includes =				\
	$(ABIWORD_CFLAGS)

abiword_libs =					\
	$(ABIWORD_LIBS)

endif

if USE_CMS
cms_includes = $(LCMS_CFLAGS)
cms_libs = $(LCMS_LIBS)
endif

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	$(cms_includes)				\
	$(splash_includes)			\
	$(cairo_includes)			\
	$(arthur_includes)			\
	$(abiword_includes)			\
	$(libpng_includes)			\
	$(libcurl_includes)			\
	$(FREETYPE_CFLAGS)			\
	$(FONTCONFIG_CFLAGS)

lib_LTLIBRARIES = libpoppler.la

noinst_LTLIBRARIES = $(poppler_cairo)		\
	$(poppler_arthur)

CFLAGS+=$(PTHREAD_CFLAGS)
CXXFLAGS+=$(PTHREAD_CFLAGS)

libpoppler_la_LIBADD =				\
	$(top_builddir)/goo/libgoo.la		\
	$(top_builddir)/fofi/libfofi.la		\
	$(cms_libs)				\
	$(splash_libs)				\
	$(libjpeg_libs)				\
	$(libpng_libs)				\
	$(zlib_libs)				\
	$(libcurl_libs)				\
	$(libjpeg2000_libs)			\
	$(abiword_libs)				\
	$(FREETYPE_LIBS)			\
	$(FONTCONFIG_LIBS)			\
	$(PTHREAD_LIBS)				\
	$(win32_libs)

libpoppler_la_LDFLAGS = -version-info 13:0:0 @create_shared_lib@ @auto_import_flags@

if ENABLE_XPDF_HEADERS

poppler_includedir = $(includedir)/poppler
poppler_include_HEADERS =	\
	$(splash_headers)	\
	$(curl_headers)		\
	Annot.h			\
	Array.h			\
	BuiltinFont.h		\
	BuiltinFontTables.h	\
	CachedFile.h		\
	Catalog.h		\
	CharCodeToUnicode.h	\
	CMap.h			\
	DateInfo.h		\
	Decrypt.h		\
	Dict.h			\
	Error.h			\
	FileSpec.h		\
	FontEncodingTables.h	\
	FontInfo.h		\
	Form.h 			\
	Function.h		\
	Gfx.h			\
	GfxFont.h		\
	GfxState.h		\
	GfxState_helpers.h	\
	GlobalParams.h		\
	Hints.h			\
	JArithmeticDecoder.h	\
	JBIG2Stream.h		\
	Lexer.h			\
	Linearization.h 	\
	Link.h			\
	LocalPDFDocBuilder.h	\
	Movie.h                 \
	NameToCharCode.h	\
	Object.h		\
	OptionalContent.h	\
	Outline.h		\
	OutputDev.h		\
	Page.h			\
	PageTransition.h	\
	Parser.h		\
	PDFDoc.h		\
	PDFDocBuilder.h		\
	PDFDocEncoding.h	\
	PDFDocFactory.h		\
	PopplerCache.h		\
	ProfileData.h		\
	PreScanOutputDev.h	\
	PSTokenizer.h		\
	Rendition.h		\
	StdinCachedFile.h	\
	StdinPDFDocBuilder.h	\
	Stream-CCITT.h		\
	Stream.h		\
	UnicodeMap.h		\
	UnicodeMapTables.h	\
	UnicodeTypeTable.h	\
	UnicodeCClassTables.h	\
	UnicodeCompTables.h	\
	UnicodeDecompTables.h	\
	XRef.h			\
	CharTypes.h		\
	CompactFontTables.h	\
	ErrorCodes.h		\
	NameToUnicodeTable.h	\
	PSOutputDev.h		\
	TextOutputDev.h		\
	SecurityHandler.h	\
	UTF8.h			\
	XpdfPluginAPI.h		\
	Sound.h
nodist_poppler_include_HEADERS = poppler-config.h

endif

libpoppler_la_SOURCES =		\
	$(splash_sources)	\
	$(cairo_sources)	\
	$(arthur_sources)	\
	$(libjpeg_sources)	\
	$(zlib_sources)		\
	$(libjpeg2000_sources)	\
	$(abiword_sources)	\
	$(curl_sources)		\
	Annot.cc		\
	Array.cc 		\
	BuiltinFont.cc		\
	BuiltinFontTables.cc	\
	CachedFile.cc		\
	Catalog.cc 		\
	CharCodeToUnicode.cc	\
	CMap.cc			\
	DateInfo.cc		\
	Decrypt.cc		\
	Dict.cc 		\
	Error.cc 		\
	FileSpec.cc		\
	FontEncodingTables.cc	\
	Form.cc 		\
	FontInfo.cc		\
	Function.cc		\
	Gfx.cc 			\
	GfxFont.cc 		\
	GfxState.cc		\
	GlobalParams.cc		\
	Hints.cc		\
	JArithmeticDecoder.cc	\
	JBIG2Stream.cc		\
	Lexer.cc 		\
	Linearization.cc 	\
	Link.cc 		\
	LocalPDFDocBuilder.cc	\
	Movie.cc                \
	NameToCharCode.cc	\
	Object.cc 		\
	OptionalContent.cc	\
	Outline.cc		\
	OutputDev.cc 		\
	Page.cc 		\
	PageTransition.cc	\
	Parser.cc 		\
	PDFDoc.cc 		\
	PDFDocEncoding.cc	\
	PDFDocFactory.cc	\
	PopplerCache.cc		\
	ProfileData.cc		\
	PreScanOutputDev.cc \
	PSTokenizer.cc		\
	Rendition.cc		\
	StdinCachedFile.cc	\
	StdinPDFDocBuilder.cc	\
	Stream.cc 		\
	strtok_r.cpp		\
	UnicodeMap.cc		\
	UnicodeTypeTable.cc	\
	XRef.cc			\
	PSOutputDev.cc		\
	TextOutputDev.cc	\
	PageLabelInfo.h		\
	PageLabelInfo.cc	\
	SecurityHandler.cc	\
	Sound.cc		\
	XpdfPluginAPI.cc

EXTRA_DIST = gen-unicode-tables.py	\
             GlobalParamsWin.cc
