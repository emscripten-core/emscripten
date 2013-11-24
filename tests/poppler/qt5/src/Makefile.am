INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(POPPLER_QT5_CFLAGS)			\
	$(LCMS_CFLAGS)

SUFFIXES: .moc

poppler_includedir = $(includedir)/poppler/qt5

poppler_include_HEADERS =			\
	poppler-qt5.h				\
	poppler-link.h				\
	poppler-annotation.h			\
	poppler-form.h				\
	poppler-optcontent.h			\
	poppler-export.h			\
	poppler-media.h				\
	poppler-page-transition.h

lib_LTLIBRARIES = libpoppler-qt5.la

libpoppler_qt5_la_SOURCES =			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-fontinfo.cc			\
	poppler-embeddedfile.cc			\
	poppler-textbox.cc			\
	poppler-link.cc				\
	poppler-annotation.cc			\
	poppler-link-extractor.cc		\
	poppler-optcontent.cc			\
	poppler-optcontent.h			\
	poppler-page-transition.cc		\
	poppler-sound.cc			\
	poppler-form.cc				\
	poppler-base-converter.cc		\
	poppler-ps-converter.cc			\
	poppler-pdf-converter.cc		\
	poppler-qiodeviceoutstream.cc		\
	poppler-private.cc			\
	poppler-movie.cc			\
	poppler-media.cc			\
	poppler-annotation-helper.h		\
	poppler-page-private.h			\
	poppler-link-extractor-private.h	\
	poppler-annotation-private.h		\
	poppler-converter-private.h		\
	poppler-qiodeviceoutstream-private.h	\
	poppler-optcontent-private.h		\
	poppler-export.h			\
	poppler-private.h			\
	poppler-page-transition-private.h	\
	poppler-embeddedfile-private.h		\
	ArthurOutputDev.h			\
	ArthurOutputDev.cc

libpoppler_qt5_la_CXXFLAGS = 			\
	-Dpoppler_qt5_EXPORTS -fPIC

libpoppler_qt5_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(POPPLER_QT5_LIBS)

libpoppler_qt5_la_LDFLAGS = -version-info 1:0:0 @create_shared_lib@ @auto_import_flags@

# This rule lets GNU make create any *.moc from the equivalent *.h
.h.moc:
	$(AM_V_GEN) $(MOCQT5) -i $< -o $@

BUILT_SOURCES = poppler-optcontent.moc

clean-generic:
	rm -f *.moc
