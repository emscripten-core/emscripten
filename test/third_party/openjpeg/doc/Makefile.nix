#doc Makefile.nix
include ../config.nix

doc_dir = $(prefix)/share/doc/openjpeg-$(MAJOR).$(MINOR)
man_dir = $(prefix)/share/man

ifeq ($(HAS_DOXYGEN),yes)
docs:
	doxygen Doxyfile.dox

endif

clean:
	rm -rf html

install:
	cp man/man1/* .
	cp man/man3/* .
	install -d $(doc_dir)
	install -d $(man_dir)/man1 $(man_dir)/man3
ifeq ($(WITH_JPWL),yes)
	(cd $(man_dir)/man1 && rm -f JPWL_image_to_j2k* JPWL_j2k_to_image* )
	(cd $(man_dir)/man3 && rm -f JPWL_libopenjpeg* )
endif
	(cd $(man_dir)/man1 && rm -f image_to_j2k* j2k_to_image* j2k_dump* )
	(cd $(man_dir)/man3 && rm -f libopenjpeg* )
	gzip -f image_to_j2k.1 j2k_to_image.1 j2k_dump.1 libopenjpeg.3
	cp -f image_to_j2k.1.gz j2k_to_image.1.gz j2k_dump.1.gz $(man_dir)/man1
	cp -f libopenjpeg.3.gz $(man_dir)/man3
	rm -f *\.gz
ifeq ($(WITH_JPWL),yes)
	(cd $(man_dir)/man1 && \
	ln -sf image_to_j2k.1.gz JPWL_image_to_j2k.1.gz && \
	ln -sf j2k_to_image.1.gz JPWL_j2k_to_image.1.gz)
	(cd $(man_dir)/man3 && ln -sf libopenjpeg.3.gz JPWL_libopenjpeg.3.gz)
endif
	cp ../LICENSE ../CHANGES $(doc_dir)

uninstall:
	rm -rf $(doc_dir)
ifeq ($(WITH_JPWL),yes)	
	(cd $(man_dir)/man1 && rm -f JPWL_image_to_j2k* JPWL_j2k_to_image* )
	(cd $(man_dir)/man3 && rm -f JPWL_libopenjpeg* )
endif
	(cd $(man_dir)/man1 && rm -f image_to_j2k* j2k_to_image* j2k_dump* )
	(cd $(man_dir)/man3 && rm -f libopenjpeg* )

