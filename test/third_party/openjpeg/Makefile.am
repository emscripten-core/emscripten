#OpenJPEG top source directory
SUBDIRS = libopenjpeg codec mj2 @jpwl_dir@ @jp3d_dir@ doc

clean-local:
	@rm -rf bin
	@l='$(SUBDIRS)'; for f in $$l ; do \
	rm -f $$f/.report.txt ; \
	done

distclean-local: clean-local

pkgconf_dir = /usr/lib/pkgconfig

install-data-hook:
	@echo ""
	@l='$(SUBDIRS)'; for f in $$l ; do \
	cat $$f/.report.txt ; \
	done
	@echo ""
	@echo "Installing: libopenjpeg1.pc $(pkgconf_dir)"
	@echo ""
	@$(INSTALL) -d $(pkgconf_dir)
	@cp libopenjpeg.pc $(pkgconf_dir)/libopenjpeg1.pc
	@(cd $(pkgconf_dir) && $(RM) libopenjpeg.pc && \
	$(LN_S) libopenjpeg1.pc libopenjpeg.pc)

uninstall-hook:
	@(cd $(pkgconf_dir) && $(RM) libopenjpeg.pc libopenjpeg1.pc)
