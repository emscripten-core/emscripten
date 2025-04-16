prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib@LIB_SUFFIX@
includedir=${prefix}/include

Name: poppler-glib
Description: GLib wrapper for poppler
Version: @POPPLER_VERSION@
Requires: poppler-cairo = @POPPLER_VERSION@ @GLIB_REQ@ @GDK_REQ@
@PC_REQUIRES_PRIVATE@

Libs: -L${libdir} -lpoppler-glib
Cflags: -I${includedir}/poppler/glib
