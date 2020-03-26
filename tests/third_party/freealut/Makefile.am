# Build, install and distribute the following subdirectories:
SUBDIRS = admin src include examples test_suite

# We keep our local autoconf macros in a separate 'm4'subdirectory.
ACLOCAL_AMFLAGS = -I admin/autotools/m4

# The following files should be in our source distribution in addition to the
# standard ones included by automake itself:
EXTRA_DIST =							\
	admin/autotools/.indent.pro				\
	admin/RPM/freealut.spec					\
	admin/VisualStudio6/alut/alut.dsp			\
	admin/VisualStudio6/alut.dsw				\
	admin/VisualStudio6/hello_world/hello_world.dsp		\
	admin/VisualStudio6/playfile/playfile.dsp		\
	admin/VisualStudioDotNET/alut/alut.vcproj		\
	admin/VisualStudioDotNET/alut.sln			\
	admin/VisualStudioDotNET/hello_world/hello_world.vcproj	\
	admin/VisualStudioDotNET/playfile/playfile.vcproj	\
	admin/CMakeModules/FindCompilerAttribute.cmake		\
	admin/CMakeModules/FindCompilerFlagsSet.cmake		\
	admin/CMakeModules/FindCompilerVisibility.cmake		\
	admin/CMakeModules/FindConfigHelper.cmake		\
	admin/CMakeModules/FindSleepFunction.cmake		\
	autogen.sh						\
	doc/alut.css						\
	doc/alut.html						\
	CMakeLists.txt

# Automatically rebuild libtool if it becomes out-of-date.
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# The Creative repository has some broken permissions, let's fix this.
permissions:
	find . -type f \! -perm +222 \! -name "*.sh" -exec chmod +w {} \;

# Automatically reformat all C headers and sources with indent according to
# admin/autotools/.indent.pro, but be make-friendly and take some precautions to
# leave those files untouched which wouldn't change. Do not reformat the public
# ALUT header, it won't look the way we want it due to the 'extern "C" { ... }' 
# wrapper in it.
indent:
	@for i in `find . \( -name "*.c" -o -name "*.h" \) \! -name alut.h -print` ; do \
	    cat $$i | ( cd admin/autotools ; indent ) > indent.tmp ; \
	    if diff $$i indent.tmp > /dev/null 2>&1 ; then \
		: ; \
	    else \
		echo Reformatted $$i ; \
		$(RM) $$i ; \
		mv indent.tmp $$i ; \
	    fi ; \
	done
	@$(RM) indent.tmp

# Targets which are no files
.PHONY: permissions indent
