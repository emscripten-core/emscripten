INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	$(FREETYPE_CFLAGS)

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
