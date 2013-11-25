# -*- mode: makefile -*-

####################################
# Everything below here is generic #
####################################

if GTK_DOC_USE_LIBTOOL
GTKDOC_CC = $(LIBTOOL) --tag=CC --mode=compile $(CC) $(INCLUDES) $(GTKDOC_DEPS_CFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
GTKDOC_LD = $(LIBTOOL) --tag=CC --mode=link $(CC) $(GTKDOC_DEPS_LIBS) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS)
GTKDOC_RUN = $(LIBTOOL) --mode=execute
else
GTKDOC_CC = $(CC) $(INCLUDES) $(GTKDOC_DEPS_CFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
GTKDOC_LD = $(CC) $(GTKDOC_DEPS_LIBS) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS)
GTKDOC_RUN =
endif

# We set GPATH here; this gives us semantics for GNU make
# which are more like other make's VPATH, when it comes to
# whether a source that is a target of one rule is then
# searched for in VPATH/GPATH.
#
GPATH = $(srcdir)

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

SETUP_FILES = \
	$(content_files)		\
	$(DOC_MAIN_SGML_FILE)		\
	$(DOC_MODULE)-sections.txt	\
	$(DOC_MODULE)-overrides.txt

EXTRA_DIST = 				\
	$(HTML_IMAGES)			\
	$(SETUP_FILES)

DOC_STAMPS=setup-build.stamp scan-build.stamp sgml-build.stamp \
	html-build.stamp pdf-build.stamp \
	sgml.stamp html.stamp pdf.stamp

SCANOBJ_FILES = 		 \
	$(DOC_MODULE).args 	 \
	$(DOC_MODULE).hierarchy  \
	$(DOC_MODULE).interfaces \
	$(DOC_MODULE).prerequisites \
	$(DOC_MODULE).signals

REPORT_FILES = \
	$(DOC_MODULE)-undocumented.txt \
	$(DOC_MODULE)-undeclared.txt \
	$(DOC_MODULE)-unused.txt

CLEANFILES = $(SCANOBJ_FILES) $(REPORT_FILES) $(DOC_STAMPS)

if ENABLE_GTK_DOC
if GTK_DOC_BUILD_HTML
HTML_BUILD_STAMP=html-build.stamp
else
HTML_BUILD_STAMP=
endif
if GTK_DOC_BUILD_PDF
PDF_BUILD_STAMP=pdf-build.stamp
else
PDF_BUILD_STAMP=
endif

all-local: $(HTML_BUILD_STAMP) $(PDF_BUILD_STAMP)
else
all-local:
endif

docs: $(HTML_BUILD_STAMP) $(PDF_BUILD_STAMP)

$(REPORT_FILES): sgml-build.stamp

#### setup ####

GTK_DOC_V_SETUP=$(GTK_DOC_V_SETUP_$(V))
GTK_DOC_V_SETUP_=$(GTK_DOC_V_SETUP_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_SETUP_0=@echo "  DOC   Preparing build";

setup-build.stamp:
	-$(GTK_DOC_V_SETUP)if test "$(abs_srcdir)" != "$(abs_builddir)" ; then \
	    files=`echo $(SETUP_FILES) $(expand_content_files) $(DOC_MODULE).types`; \
	    if test "x$$files" != "x" ; then \
	        for file in $$files ; do \
	            test -f $(abs_srcdir)/$$file && \
	                cp -pu $(abs_srcdir)/$$file $(abs_builddir)/$$file || true; \
	        done; \
	    fi; \
	fi
	$(AM_V_at)touch setup-build.stamp


#### scan ####

GTK_DOC_V_SCAN=$(GTK_DOC_V_SCAN_$(V))
GTK_DOC_V_SCAN_=$(GTK_DOC_V_SCAN_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_SCAN_0=@echo "  DOC   Scanning header files";

GTK_DOC_V_INTROSPECT=$(GTK_DOC_V_INTROSPECT_$(V))
GTK_DOC_V_INTROSPECT_=$(GTK_DOC_V_INTROSPECT_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_INTROSPECT_0=@echo "  DOC   Introspecting gobjects";

scan-build.stamp: $(HFILE_GLOB) $(CFILE_GLOB)
	$(GTK_DOC_V_SCAN)_source_dir='' ; \
	for i in $(DOC_SOURCE_DIR) ; do \
	    _source_dir="$${_source_dir} --source-dir=$$i" ; \
	done ; \
	gtkdoc-scan --module=$(DOC_MODULE) --ignore-headers="$(IGNORE_HFILES)" $${_source_dir} $(SCAN_OPTIONS) $(EXTRA_HFILES)
	$(GTK_DOC_V_INTROSPECT)if grep -l '^..*$$' $(DOC_MODULE).types > /dev/null 2>&1 ; then \
	    scanobj_options=""; \
	    gtkdoc-scangobj 2>&1 --help | grep  >/dev/null "\-\-verbose"; \
	    if test "$(?)" = "0"; then \
	        if test "x$(V)" = "x1"; then \
	            scanobj_options="--verbose"; \
	        fi; \
	    fi; \
	    CC="$(GTKDOC_CC)" LD="$(GTKDOC_LD)" RUN="$(GTKDOC_RUN)" CFLAGS="$(GTKDOC_CFLAGS) $(CFLAGS)" LDFLAGS="$(GTKDOC_LIBS) $(LDFLAGS)" \
	    gtkdoc-scangobj $(SCANGOBJ_OPTIONS) $$scanobj_options --module=$(DOC_MODULE); \
	else \
	    for i in $(SCANOBJ_FILES) ; do \
	        test -f $$i || touch $$i ; \
	    done \
	fi
	$(AM_V_at)touch scan-build.stamp

$(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_MODULE)-overrides.txt: scan-build.stamp
	@true

#### xml ####

GTK_DOC_V_XML=$(GTK_DOC_V_XML_$(V))
GTK_DOC_V_XML_=$(GTK_DOC_V_XML_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_XML_0=@echo "  DOC   Building XML";

sgml-build.stamp: setup-build.stamp $(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_MODULE)-overrides.txt $(expand_content_files)
	$(GTK_DOC_V_XML)_source_dir='' ; \
	for i in $(DOC_SOURCE_DIR) ; do \
	    _source_dir="$${_source_dir} --source-dir=$$i" ; \
	done ; \
	gtkdoc-mkdb --module=$(DOC_MODULE) --output-format=xml --expand-content-files="$(expand_content_files)" --main-sgml-file=$(DOC_MAIN_SGML_FILE) $${_source_dir} $(MKDB_OPTIONS)
	$(AM_V_at)touch sgml-build.stamp

sgml.stamp: sgml-build.stamp
	@true

#### html ####

GTK_DOC_V_HTML=$(GTK_DOC_V_HTML_$(V))
GTK_DOC_V_HTML_=$(GTK_DOC_V_HTML_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_HTML_0=@echo "  DOC   Building HTML";

GTK_DOC_V_XREF=$(GTK_DOC_V_XREF_$(V))
GTK_DOC_V_XREF_=$(GTK_DOC_V_XREF_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_XREF_0=@echo "  DOC   Fixing cross-references";

html-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	$(GTK_DOC_V_HTML)rm -rf html && mkdir html && \
	mkhtml_options=""; \
	gtkdoc-mkhtml 2>&1 --help | grep  >/dev/null "\-\-verbose"; \
	if test "$(?)" = "0"; then \
	  if test "x$(V)" = "x1"; then \
	    mkhtml_options="$$mkhtml_options --verbose"; \
	  fi; \
	fi; \
	gtkdoc-mkhtml 2>&1 --help | grep  >/dev/null "\-\-path"; \
	if test "$(?)" = "0"; then \
	  mkhtml_options="$$mkhtml_options --path=\"$(abs_srcdir)\""; \
	fi; \
	cd html && gtkdoc-mkhtml $$mkhtml_options $(MKHTML_OPTIONS) $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-@test "x$(HTML_IMAGES)" = "x" || \
	for file in $(HTML_IMAGES) ; do \
	  if test -f $(abs_srcdir)/$$file ; then \
	    cp $(abs_srcdir)/$$file $(abs_builddir)/html; \
	  fi; \
	  if test -f $(abs_builddir)/$$file ; then \
	    cp $(abs_builddir)/$$file $(abs_builddir)/html; \
	  fi; \
	done;
	$(GTK_DOC_V_XREF)gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=html --html-dir=$(HTML_DIR) $(FIXXREF_OPTIONS)
	$(AM_V_at)touch html-build.stamp

#### pdf ####

GTK_DOC_V_PDF=$(GTK_DOC_V_PDF_$(V))
GTK_DOC_V_PDF_=$(GTK_DOC_V_PDF_$(AM_DEFAULT_VERBOSITY))
GTK_DOC_V_PDF_0=@echo "  DOC   Building PDF";

pdf-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	$(GTK_DOC_V_PDF)rm -f $(DOC_MODULE).pdf && \
	mkpdf_options=""; \
	gtkdoc-mkpdf 2>&1 --help | grep  >/dev/null "\-\-verbose"; \
	if test "$(?)" = "0"; then \
	  if test "x$(V)" = "x1"; then \
	    mkpdf_options="$$mkpdf_options --verbose"; \
	  fi; \
	fi; \
	if test "x$(HTML_IMAGES)" != "x"; then \
	  for img in $(HTML_IMAGES); do \
	    part=`dirname $$img`; \
	    echo $$mkpdf_options | grep >/dev/null "\-\-imgdir=$$part "; \
	    if test $$? != 0; then \
	      mkpdf_options="$$mkpdf_options --imgdir=$$part"; \
	    fi; \
	  done; \
	fi; \
	gtkdoc-mkpdf --path="$(abs_srcdir)" $$mkpdf_options $(DOC_MODULE) $(DOC_MAIN_SGML_FILE) $(MKPDF_OPTIONS)
	$(AM_V_at)touch pdf-build.stamp

##############

clean-local:
	@rm -f *~ *.bak
	@rm -rf .libs

distclean-local:
	@rm -rf xml html $(REPORT_FILES) $(DOC_MODULE).pdf \
	    $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt
	@if test "$(abs_srcdir)" != "$(abs_builddir)" ; then \
	    rm -f $(SETUP_FILES) $(expand_content_files) $(DOC_MODULE).types; \
	fi

maintainer-clean-local:
	@rm -rf xml html

install-data-local:
	@installfiles=`echo $(builddir)/html/*`; \
	if test "$$installfiles" = '$(builddir)/html/*'; \
	then echo 1>&2 'Nothing to install' ; \
	else \
	  if test -n "$(DOC_MODULE_VERSION)"; then \
	    installdir="$(DESTDIR)$(TARGET_DIR)-$(DOC_MODULE_VERSION)"; \
	  else \
	    installdir="$(DESTDIR)$(TARGET_DIR)"; \
	  fi; \
	  $(mkinstalldirs) $${installdir} ; \
	  for i in $$installfiles; do \
	    echo ' $(INSTALL_DATA) '$$i ; \
	    $(INSTALL_DATA) $$i $${installdir}; \
	  done; \
	  if test -n "$(DOC_MODULE_VERSION)"; then \
	    mv -f $${installdir}/$(DOC_MODULE).devhelp2 \
	      $${installdir}/$(DOC_MODULE)-$(DOC_MODULE_VERSION).devhelp2; \
	  fi; \
	  $(GTKDOC_REBASE) --relative --dest-dir=$(DESTDIR) --html-dir=$${installdir}; \
	fi

uninstall-local:
	@if test -n "$(DOC_MODULE_VERSION)"; then \
	  installdir="$(DESTDIR)$(TARGET_DIR)-$(DOC_MODULE_VERSION)"; \
	else \
	  installdir="$(DESTDIR)$(TARGET_DIR)"; \
	fi; \
	rm -rf $${installdir}

#
# Require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtkdoc: docs
else
dist-check-gtkdoc:
	@echo "*** gtk-doc must be installed and enabled in order to make dist"
	@false
endif

dist-hook: dist-check-gtkdoc dist-hook-local
	@mkdir $(distdir)/html
	@cp ./html/* $(distdir)/html
	@-cp ./$(DOC_MODULE).pdf $(distdir)/
	@-cp ./$(DOC_MODULE).types $(distdir)/
	@-cp ./$(DOC_MODULE)-sections.txt $(distdir)/
	@cd $(distdir) && rm -f $(DISTCLEANFILES)
	@$(GTKDOC_REBASE) --online --relative --html-dir=$(distdir)/html

.PHONY : dist-hook-local docs
