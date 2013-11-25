INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	$(libjpeg_includes)			\
	$(libtiff_includes)			\
	$(libpng_includes)			\
	$(FREETYPE_CFLAGS)

# SplashBitmap includes JpegWriter.h, TiffWriter.h, PNGWriter.h
if BUILD_LIBJPEG
libjpeg_includes = $(LIBJPEG_CFLAGS)
endif

if BUILD_LIBTIFF
libtiff_includes = $(LIBTIFF_CFLAGS)
endif

if BUILD_LIBPNG
libpng_includes = $(LIBPNG_CFLAGS)
endif

noinst_LTLIBRARIES = libsplash.la

if ENABLE_XPDF_HEADERS

poppler_splash_includedir = $(includedir)/poppler/splash
poppler_splash_include_HEADERS =		\
	Splash.h				\
	SplashBitmap.h				\
	SplashClip.h				\
	SplashErrorCodes.h			\
	SplashFTFont.h				\
	SplashFTFontEngine.h			\
	SplashFTFontFile.h			\
	SplashFont.h				\
	SplashFontEngine.h			\
	SplashFontFile.h			\
	SplashFontFileID.h			\
	SplashGlyphBitmap.h			\
	SplashMath.h				\
	SplashPath.h				\
	SplashPattern.h				\
	SplashScreen.h				\
	SplashState.h				\
	SplashT1Font.h				\
	SplashT1FontEngine.h			\
	SplashT1FontFile.h			\
	SplashTypes.h				\
	SplashXPath.h				\
	SplashXPathScanner.h

endif

libsplash_la_SOURCES =				\
	Splash.cc				\
	SplashBitmap.cc				\
	SplashClip.cc				\
	SplashFTFont.cc				\
	SplashFTFontEngine.cc			\
	SplashFTFontFile.cc			\
	SplashFont.cc				\
	SplashFontEngine.cc			\
	SplashFontFile.cc			\
	SplashFontFileID.cc			\
	SplashPath.cc				\
	SplashPattern.cc			\
	SplashScreen.cc				\
	SplashState.cc				\
	SplashT1Font.cc				\
	SplashT1FontEngine.cc			\
	SplashT1FontFile.cc			\
	SplashXPath.cc				\
	SplashXPathScanner.cc
