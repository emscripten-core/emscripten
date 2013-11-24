prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib@LIB_SUFFIX@
includedir=${prefix}/include

Name: poppler-glib
Description: GLib wrapper for poppler
Version: @POPPLER_VERSION@
Requires: glib-2.0 >= @GLIB_REQUIRED@ gobject-2.0 >= @GLIB_REQUIRED@ cairo >= @CAIRO_VERSION@
@PC_REQUIRES_PRIVATE@

Libs: -L${libdir} -lpoppler-glib
Cflags: -I${includedir}/poppler/glib
