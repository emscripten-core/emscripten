if BUILD_SPLASH_OUTPUT

pdftoppm_SOURCES =				\
	pdftoppm.cc				\
	$(common)

pdftoppm_binary = pdftoppm

pdftoppm_manpage = pdftoppm.1

endif

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	-I$(top_srcdir)/utils			\
	-I$(top_srcdir)/poppler			\
	$(UTILS_CFLAGS)				\
	$(PDFTOCAIRO_CFLAGS)

LDADD =						\
	$(top_builddir)/poppler/libpoppler.la	\
	$(UTILS_LIBS)

if BUILD_CAIRO_OUTPUT

pdftocairo_SOURCES =				\
	pdftocairo.cc				\
	$(common)

if USE_CMS
PDFTOCAIRO_CFLAGS += $(LCMS_CFLAGS)
PDFTOCAIRO_LIBS += $(LCMS_LIBS)
endif

pdftocairo_LDADD =				\
	$(top_builddir)/poppler/libpoppler-cairo.la \
	$(LDADD)				\
	$(PDFTOCAIRO_LIBS)			\
	$(PTHREAD_LIBS)


pdftocairo_binary = pdftocairo

pdftocairo_manpage = pdftocairo.1

endif

AM_LDFLAGS = @auto_import_flags@

bin_PROGRAMS =					\
	pdfdetach				\
	pdffonts				\
	pdfimages				\
	pdfinfo					\
	pdftops					\
	pdftotext				\
	pdftohtml				\
	pdfseparate				\
	pdfunite				\
	$(pdftoppm_binary)			\
	$(pdftocairo_binary)

dist_man1_MANS =				\
	pdfdetach.1				\
	pdffonts.1				\
	pdfimages.1				\
	pdfinfo.1				\
	pdftops.1				\
	pdftotext.1				\
	pdftohtml.1				\
	pdfseparate.1				\
	pdfunite.1				\
	$(pdftoppm_manpage)			\
	$(pdftocairo_manpage)

common = parseargs.cc parseargs.h

pdfdetach_SOURCES = 				\
	pdfdetach.cc				\
	$(common)

pdffonts_SOURCES =				\
	pdffonts.cc				\
	$(common)

pdfimages_SOURCES =				\
	pdfimages.cc				\
	ImageOutputDev.cc			\
	ImageOutputDev.h			\
	JSInfo.cc				\
	JSInfo.h				\
	$(common)

pdfinfo_SOURCES =				\
	pdfinfo.cc				\
	printencodings.cc			\
	printencodings.h			\
	JSInfo.cc				\
	JSInfo.h				\
	$(common)

pdftops_SOURCES =				\
	pdftops.cc				\
	$(common)

pdftotext_SOURCES =				\
	pdftotext.cc				\
	printencodings.cc			\
	printencodings.h			\
	$(common)

pdftohtml_SOURCES =				\
	pdftohtml.cc				\
	HtmlFonts.cc				\
	HtmlFonts.h				\
	HtmlLinks.cc				\
	HtmlLinks.h				\
	HtmlOutputDev.cc			\
	HtmlOutputDev.h				\
	HtmlUtils.h				\
	$(common)

# HtmlOutputDev uses goo/PNGWriter.h that may depend on libpng header
pdftohtml_CXXFLAGS = $(AM_CXXFLAGS)
if BUILD_LIBPNG
pdftohtml_CXXFLAGS += $(LIBPNG_CFLAGS)
endif

pdfseparate_SOURCES =				\
	pdfseparate.cc				\
	$(common)

pdfunite_SOURCES =				\
	pdfunite.cc				\
	$(common)

# Yay, automake!  It should be able to figure out that it has to dist
# pdftoppm.1, but nooo.  So we just add it here.

EXTRA_DIST = pdf2xml.dtd pdftoppm.1
