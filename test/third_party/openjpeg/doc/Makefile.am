MAJOR = @MAJOR_NR@
MINOR = @MINOR_NR@
BUILD = @BUILD_NR@

if with_doxygen
docs:
	doxygen Doxyfile.dox
endif

doc_dir = $(datarootdir)/doc/openjpeg-$(MAJOR).$(MINOR)

all-local:
	@echo "" > .report.txt
	@echo "Installing: image_to_j2k.1 ${mandir}/man1" >> .report.txt
	@echo "Installing: j2k_to_image.1  ${mandir}/man1" >> .report.txt
	@echo "Installing: j2k_dump.1  ${mandir}/man1" >> .report.txt
	@echo "Installing: libopenjpeg.3  ${mandir}/man3" >> .report.txt
	@echo "Installing: LICENSE CHANGES $(doc_dir)" >> .report.txt
	@cp man/man1/* .
	@cp man/man3/* .

install-data-hook:
	$(INSTALL) -d $(doc_dir)
	$(INSTALL) -d ${mandir}/man1 ${mandir}/man3
if with_libjpwl
	@(cd ${mandir}/man1 && $(RM) JPWL_image_to_j2k* JPWL_j2k_to_image*)
	@(cd ${mandir}/man3 && $(RM) JPWL_libopenjpeg* )
endif
	@(cd ${mandir}/man1 && $(RM) image_to_j2k* j2k_to_image* j2k_dump* )
	@(cd ${mandir}/man3 && $(RM) libopenjpeg* )
	@gzip -f image_to_j2k.1 j2k_to_image.1 j2k_dump.1 libopenjpeg.3
	@cp image_to_j2k.1.gz j2k_to_image.1.gz j2k_dump.1.gz ${mandir}/man1
	@cp libopenjpeg.3.gz ${mandir}/man3
	@$(RM) *\.gz
if with_libjpwl
	@(cd ${mandir}/man1 && \
	$(LN_S) image_to_j2k.1.gz JPWL_image_to_j2k.1.gz && \
	$(LN_S) j2k_to_image.1.gz JPWL_j2k_to_image.1.gz)
	@(cd ${mandir}/man3 && $(LN_S) libopenjpeg.3.gz JPWL_libopenjpeg.3.gz)
endif
	@$(INSTALL) -m 644 ../LICENSE ../CHANGES $(doc_dir)

uninstall-hook:
	@(cd ${mandir}/man1 && $(RM) JPWL_image_to_j2k* JPWL_j2k_to_image* && \
	$(RM) image_to_j2k* j2k_to_image* j2k_dump* )
	@(cd ${mandir}/man3 && $(RM) JPWL_libopenjpeg* && \
	$(RM) libopenjpeg* )
	@(cd $(doc_dir) && $(RM) * )
	@rmdir $(doc_dir)
