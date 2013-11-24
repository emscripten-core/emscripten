AM_LDFLAGS = @auto_import_flags@

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt5/src			\
	$(POPPLER_QT5_CFLAGS)

LDADDS =					\
	$(top_builddir)/poppler/libpoppler.la	\
	$(top_builddir)/qt5/src/libpoppler-qt5.la	\
	$(POPPLER_QT5_LIBS)

SUFFIXES: .moc

# This rule lets GNU make create any *.moc from the equivalent *.h
.h.moc:
	$(AM_V_GEN) $(MOCQT5) -i $< -o $@

noinst_PROGRAMS = poppler_qt5viewer

poppler_qt5viewer_SOURCES =			\
	abstractinfodock.cpp			\
	abstractinfodock.h			\
	documentobserver.cpp			\
	documentobserver.h			\
	embeddedfiles.cpp			\
	embeddedfiles.h				\
	fonts.cpp				\
	fonts.h					\
	info.cpp				\
	info.h					\
	main_viewer.cpp				\
	metadata.cpp				\
	metadata.h				\
	navigationtoolbar.cpp			\
	navigationtoolbar.h			\
	optcontent.cpp				\
	optcontent.h				\
	pageview.cpp				\
	pageview.h				\
	permissions.cpp				\
	permissions.h				\
	thumbnails.cpp				\
	thumbnails.h				\
	toc.cpp					\
	toc.h					\
	viewer.cpp				\
	viewer.h

abstractinfodock.$(OBJEXT): abstractinfodock.moc
embeddedfiles.$(OBJEXT): embeddedfiles.moc
fonts.$(OBJEXT): fonts.moc
info.$(OBJEXT): info.moc
metadata.$(OBJEXT): metadata.moc
navigationtoolbar.$(OBJEXT): navigationtoolbar.moc
optcontent.$(OBJEXT): optcontent.moc
pageview.$(OBJEXT): pageview.moc
permissions.$(OBJEXT): permissions.moc
thumbnails.$(OBJEXT): thumbnails.moc
toc.$(OBJEXT): toc.moc
viewer.$(OBJEXT): viewer.moc

poppler_qt5viewer_LDADD = $(LDADDS)

clean-generic:
	rm -f *.moc
