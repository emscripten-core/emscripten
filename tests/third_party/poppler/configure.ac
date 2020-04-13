m4_define([poppler_version_major],[0])
m4_define([poppler_version_minor],[16])
m4_define([poppler_version_micro],[3])
m4_define([poppler_version],[poppler_version_major.poppler_version_minor.poppler_version_micro])

AC_PREREQ(2.59)
AC_INIT([poppler],[poppler_version],[https://bugs.freedesktop.org/enter_bug.cgi?product=poppler])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AM_CONFIG_HEADER(config.h)
AM_CONFIG_HEADER(poppler/poppler-config.h)
AC_C_BIGENDIAN

dnl ##### Initialize libtool.
AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])
AC_PROG_LIBTOOL

dnl ##### Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_CC_STDC
#if test -z "$CXX" -a "$CC" = "gcc"; then
#  CXX="gcc"
#fi
AC_PROG_CXX
if test "x$GXX" = "xyes"; then
  case "$host_os" in
    cygwin* | mingw*)
    ;;
    *)
      CXXFLAGS="$CXXFLAGS -ansi"
    ;;
  esac
fi
AC_PROG_INSTALL

dnl ##### Check for pkgconfig
PKG_PROG_PKG_CONFIG

dnl Enable these unconditionally.
AC_DEFINE([OPI_SUPPORT], [1], [Generate OPI comments in PS output.])
AC_DEFINE([MULTITHREADED], [1], [Enable multithreading support.])
AC_DEFINE([TEXTOUT_WORD_LIST], [1], [Enable word list support.])

dnl Check for OS specific flags
win32_libs=""
create_shared_lib=""
auto_import_flags=""
case "$host_os" in
  cygwin*)
    create_shared_lib="-no-undefined"
  ;;
  mingw*)
    os_win32=yes
    win32_libs="-lgdi32"
    create_shared_lib="-no-undefined"
    auto_import_flags="-Wl,--enable-auto-import"
  ;;
esac

AC_SUBST(win32_libs)
AC_SUBST(create_shared_lib)
AC_SUBST(auto_import_flags)

ACX_PTHREAD()

dnl Install xpdf headers
AC_ARG_ENABLE(xpdf-headers,
	      AC_HELP_STRING([--enable-xpdf-headers],
	                     [Install unsupported xpdf headers.]),
              enable_xpdf_headers=$enableval,
              enable_xpdf_headers="no")
AM_CONDITIONAL(ENABLE_XPDF_HEADERS, test x$enable_xpdf_headers = xyes)

AC_ARG_ENABLE(single-precision,
[  --enable-single-precision     use single precision arithmetic (instead of double precision) in the Splash backend],
AC_DEFINE(USE_FLOAT,      [1], [Use single precision arithmetic in the Splash backend]))

AC_ARG_ENABLE(fixedpoint,
[  --enable-fixedpoint     use fixed point (instead of double precision) arithmetic in the Splash backend],
AC_DEFINE(USE_FIXEDPOINT, [1], [Use fixed point arithmetic in the Splash backend]))

dnl Relocation support
AC_ARG_ENABLE(relocatable,
    AC_HELP_STRING([--disable-relocatable],
                   [Hardcode the poppler library location (on Windows).]),
    enable_relocatable=$enableval,
    [if test x$os_win32 = xyes; then
         # default to yes on native Windows.
         enable_relocatable="yes"
     else
         # default to no everywhere else.
         enable_relocatable="no"
     fi
    ]
)

if test x$enable_relocatable = xyes; then
    if test x$os_win32 = xyes; then
     AC_DEFINE([ENABLE_RELOCATABLE],
               [1],[Do not hardcode the library location])
    else
        AC_MSG_ERROR(
               [Invalid setting for relocatable, only supported on windows])

    fi
fi

AC_DEFINE_DIR(POPPLER_DATADIR, "{datarootdir}/poppler", [Poppler data dir])

dnl ##### Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT

dnl ##### Switch over to C++.  This will make the checks below a little
dnl ##### bit stricter (requiring function prototypes in include files).
dnl ##### (99% of xpdf is written in C++.)
AC_LANG_CPLUSPLUS

AC_CHECK_DECL(gettimeofday, [AC_CHECK_FUNC(gettimeofday, AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defines if gettimeofday is available on your system]))],[],[#include <sys/time.h>])
AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R, 1, [Defines if localtime_r is available on your system]))
AC_CHECK_FUNC(gmtime_r, AC_DEFINE(HAVE_GMTIME_R, 1, [Defines if gmtime_r is available on your system]))

dnl ##### Check for extra libraries needed by X.  (LynxOS needs this.)
AC_CHECK_FUNC(gethostbyname)
if test $ac_cv_func_gethostbyname = no; then
  AC_CHECK_LIB(bsd, gethostbyname, X_EXTRA_LIBS="$X_EXTRA_LIBS -lbsd")
fi

AC_CHECK_FUNC(nanosleep,,)
dnl try in librt if not found in current LIBS
if test x$ac_cv_func_nanosleep = xno
then
   AC_CHECK_LIB(rt,nanosleep, X_EXTRA_LIBS="$X_EXTRA_LIBS -lrt")
fi

dnl try in libposix4, if not found so far
if test x$ac_cv_func_nanosleep = xno && test x$ac_cv_lib_rt_nanosleep = xno
then
   AC_CHECK_LIB(posix4,nanosleep, X_EXTRA_LIBS="$X_EXTRA_LIBS -lposix4")
fi


dnl Test for libopenjpeg
AC_ARG_ENABLE(libopenjpeg,
	      AC_HELP_STRING([--disable-libopenjpeg],
	                     [Don't build against libopenjpeg.]),
              enable_libopenjpeg=$enableval,
              enable_libopenjpeg="try")
if test x$enable_libopenjpeg = xyes; then
  AC_CHECK_LIB([openjpeg], [opj_cio_open],,
	       AC_MSG_ERROR("*** libopenjpeg library not found ***"))
  AC_CHECK_HEADERS([openjpeg.h],,
		   AC_MSG_ERROR("*** libopenjpeg headers not found ***"))
elif test x$enable_libopenjpeg = xtry; then
  AC_CHECK_LIB([openjpeg], [opj_cio_open],
               [enable_libopenjpeg="yes"],
	       [enable_libopenjpeg="no"])
  AC_CHECK_HEADERS([openjpeg.h],,
		   [enable_libopenjpeg="no"])
fi

if test x$enable_libopenjpeg = xyes; then
  LIBOPENJPEG_LIBS="-lopenjpeg"
  AC_SUBST(LIBOPENJPEG_LIBS)
  AC_DEFINE(ENABLE_LIBOPENJPEG)
fi

AM_CONDITIONAL(BUILD_LIBOPENJPEG, test x$enable_libopenjpeg = xyes)
AH_TEMPLATE([ENABLE_LIBOPENJPEG],
	    [Use libopenjpeg instead of builtin jpeg2000 decoder.])


dnl ##### Checks for library functions.
AC_CHECK_FUNCS(popen mkstemp mkstemps)

dnl ##### Back to C for the library tests.
AC_LANG_C

dnl ##### Check for fseeko/ftello or fseek64/ftell64
dnl The LARGEFILE and FSEEKO macros have to be called in C, not C++, mode.
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(fseek64, xpdf_cv_func_fseek64=yes, xpdf_cv_func_fseek64=no)
AC_CHECK_FUNCS(ftell64, xpdf_cv_func_ftell64=yes, xpdf_cv_func_ftell64=no)
if test "$xpdf_cv_func_fseek64" = yes -a "$xpdf_cv_func_ftell64" = yes; then
  AC_DEFINE(HAVE_FSEEK64)
fi

dnl Test for zlib
AC_ARG_ENABLE([zlib],
  [AS_HELP_STRING([--enable-zlib],[Build with zlib])],
  [],[enable_zlib="no"])
if test x$enable_zlib = xyes; then
  AC_CHECK_LIB([z], [inflate],,
	       AC_MSG_ERROR("*** zlib library not found ***"))
  AC_CHECK_HEADERS([zlib.h],,
		   AC_MSG_ERROR("*** zlib headers not found ***"))
elif test x$enable_zlib = xtry; then
  AC_CHECK_LIB([z], [inflate],
               [enable_zlib="yes"],
	       [enable_zlib="no"])
  AC_CHECK_HEADERS([zlib.h],,
		   [enable_zlib="no"])
fi

if test x$enable_zlib = xyes; then
  ZLIB_LIBS="-lz"
  AC_SUBST(ZLIB_LIBS)
  AC_DEFINE(ENABLE_ZLIB)
fi

AM_CONDITIONAL(BUILD_ZLIB, test x$enable_zlib = xyes)
AH_TEMPLATE([ENABLE_ZLIB],
	    [Use zlib instead of builtin zlib decoder.])

dnl Test for libcurl
AC_ARG_ENABLE(libcurl,
	      AC_HELP_STRING([--enable-libcurl],
	                     [Build with libcurl based HTTP support.]),
              enable_libcurl=$enableval,
              enable_libcurl="no")

if test x$enable_libcurl = xyes; then
  PKG_CHECK_MODULES(LIBCURL, libcurl)
  AC_DEFINE(ENABLE_LIBCURL, 1, [Build against libcurl.])
  AC_DEFINE(POPPLER_HAS_CURL_SUPPORT, 1,
     [Support for curl based doc builder is compiled in.])
fi

AM_CONDITIONAL(BUILD_LIBCURL, test x$enable_libcurl = xyes)

dnl Test for libjpeg
AC_ARG_ENABLE(libjpeg,
	      AC_HELP_STRING([--disable-libjpeg],
	                     [Don't build against libjpeg.]),
              enable_libjpeg=$enableval,
              enable_libjpeg="try")
if test x$enable_libjpeg != xno; then
  POPPLER_FIND_JPEG
fi

AM_CONDITIONAL(BUILD_LIBJPEG, test x$enable_libjpeg = xyes)
AH_TEMPLATE([ENABLE_LIBJPEG],
            [Use libjpeg instead of builtin jpeg decoder.])

dnl Test for libpng
AC_ARG_ENABLE(libpng,
	      AC_HELP_STRING([--disable-libpng],
	                     [Do not build against libpng.]),
              enable_libpng=$enableval,
              enable_libpng="try")

if test x$enable_libpng != xno; then
  PKG_CHECK_MODULES(LIBPNG, libpng, [enable_libpng="yes"],
      [enable_libpng="no"])
fi

if test x$enable_libpng = xyes; then
  AC_DEFINE(ENABLE_LIBPNG, 1, [Build against libpng.])
fi

AM_CONDITIONAL(BUILD_LIBPNG, test x$enable_libpng = xyes)

dnl Check for freetype headers
FREETYPE_LIBS=
FREETYPE_CFLAGS=

# XXX EMSCRIPTEN: Force freetype detection
AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])
#PKG_CHECK_MODULES(FREETYPE, freetype2,
#                  [freetype_pkgconfig=yes], [freetype_pkgconfig=no])
#
#if test "x$freetype_pkgconfig" = "xyes"; then
#
#  AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])
#
#else
#
#  AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
#  if test "x$FREETYPE_CONFIG" != "xno" ; then
#
#    FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
#    FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
#    AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])
#
#  fi
#
#fi

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

AC_MSG_CHECKING([which font configuration to use])
AC_ARG_WITH([font_configuration],
   [AS_HELP_STRING([--with-font-configuration=fontconfig|win32],
                   [Select font configuration backend])],
   [],
   [if test x$os_win32 = xyes; then
        # default to win32 on native Windows.
        with_font_configuration=win32
    else
        # default to fontconig everywhere else.
        with_font_configuration=fontconfig
    fi
   ]
)
AC_MSG_RESULT([$with_font_configuration])

case $with_font_configuration in
  win32)
     AC_DEFINE([WITH_FONTCONFIGURATION_WIN32],
               [1],[Use win32 font configuration backend])
     # Set the minimum required Internet Explorer version to 5.0
     CPPFLAGS="$CPPFLAGS -D_WIN32_IE=0x0500"
     ;;
  fontconfig)
     AC_DEFINE([WITH_FONTCONFIGURATION_FONTCONFIG],
               [1],[Use fontconfig font configuration backend])
     PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.0.0)
     ;;
  *)
     AC_MSG_ERROR(
               [Invalid font configuration setting: $with_font_configuration])
     ;;
esac

AM_CONDITIONAL(BUILD_WITH_WIN32_FONTCONFIGURATION,
               test x$with_font_configuration = xwin32)

AC_ARG_ENABLE(splash-output,
              AC_HELP_STRING([--disable-splash-output],
	                     [Don't build the Splash graphics backend.]),,
              enable_splash_output="yes")
AM_CONDITIONAL(BUILD_SPLASH_OUTPUT, test x$enable_splash_output = xyes)
AH_TEMPLATE([HAVE_SPLASH], [Use splash for rendering.])
if test x$enable_splash_output = xyes; then
  AC_DEFINE(HAVE_SPLASH)
fi

CAIRO_VERSION="1.10.0"
AC_SUBST(CAIRO_VERSION)
AC_ARG_ENABLE(cairo-output,
              AC_HELP_STRING([--disable-cairo-output],
                             [Don't build the cairo graphics backend.]),
              enable_cairo_output=$enableval,
              enable_cairo_output="try")
if test x$enable_cairo_output = xyes; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION)
elif test x$enable_cairo_output = xtry; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION,
                    [enable_cairo_output="yes"],
                    [enable_cairo_output="no"])
fi

AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

AM_CONDITIONAL(BUILD_CAIRO_OUTPUT, test x$enable_cairo_output = xyes)
AH_TEMPLATE([HAVE_CAIRO], [Use cairo for rendering.])
if test x$enable_cairo_output = xyes; then
  AC_DEFINE(HAVE_CAIRO)
  CAIRO_FEATURE="#define POPPLER_HAS_CAIRO 1"
  CAIRO_REQ="cairo"
  AC_CHECK_HEADERS(fcntl.h sys/mman.h sys/stat.h)
else
  CAIRO_FEATURE="#undef POPPLER_HAS_CAIRO"
  CAIRO_REQ=""
fi
AC_SUBST(CAIRO_FEATURE)
AC_SUBST(CAIRO_REQ)

found_introspection=no
if test x$enable_cairo_output = xyes; then
  POPPLER_GLIB_DISABLE_DEPRECATED=""
  POPPLER_GLIB_DISABLE_SINGLE_INCLUDES=""

  GLIB_REQUIRED=2.18
  AC_ARG_ENABLE(poppler-glib,
	        AC_HELP_STRING([--disable-poppler-glib],
		               [Don't compile poppler glib wrapper.]),
              		       enable_poppler_glib=$enableval,
			       enable_poppler_glib="try")
  if test x$enable_poppler_glib = xyes; then
    PKG_CHECK_MODULES(POPPLER_GLIB, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION)
  elif test x$enable_poppler_glib = xtry; then
    PKG_CHECK_MODULES(POPPLER_GLIB, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION,
                      [enable_poppler_glib="yes"],
                      [enable_poppler_glib="no"])
  fi
  if test x$enable_poppler_glib = xyes; then
    # Check for introspection
    GOBJECT_INTROSPECTION_CHECK([0.6.7])

    GLIB_REQ="glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION"
    AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])
    POPPLER_GLIB_DISABLE_DEPRECATED="$POPPLER_GLIB_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED"
    POPPLER_GLIB_DISABLE_SINGLE_INCLUDES="$POPPLER_GLIB_DISABLE_SINGLE_INCLUDES -DG_DISABLE_SINGLE_INCLUDES"
  fi
else
  if test x$enable_poppler_glib = xyes; then
    AC_MSG_ERROR("Cairo output is required to build glib frontend")
  fi
  enable_poppler_glib="no"
  GLIB_REQ=""
fi
AM_CONDITIONAL(BUILD_POPPLER_GLIB, test x$enable_poppler_glib = xyes)
AM_CONDITIONAL(HAVE_INTROSPECTION, test "x$found_introspection" = "xyes")
AC_SUBST(GLIB_REQ)

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

dnl
dnl GDK
dnl
AC_ARG_ENABLE(gdk,
              AC_HELP_STRING([--disable-gdk],
	                     [Compile without gdk.]),
              enable_gdk=$enableval,
	      enable_gdk="try")

if test x$enable_poppler_glib = xyes; then
  if test x$enable_gdk = xyes; then
    PKG_CHECK_MODULES(GDK, gdk-2.0)
  elif test x$enable_gdk = xtry; then
    PKG_CHECK_MODULES(GDK, gdk-2.0,
                      [enable_gdk="yes"],
                      [enable_gdk="no"])
  fi
  if test x$enable_gdk = xyes; then
    GDK_REQ="gdk-2.0 gdk-pixbuf-2.0"
    AC_DEFINE(POPPLER_WITH_GDK,[1], [Have GDK])
    GDK_FEATURE="#define POPPLER_WITH_GDK 1"
    POPPLER_GLIB_DISABLE_DEPRECATED="$POPPLER_GLIB_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED"
    POPPLER_GLIB_DISABLE_SINGLE_INCLUDES="$POPPLER_GLIB_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_SINGLE_INCLUDES"
  else
    enable_gdk="no"
    GDK_FEATURE=""
    GDK_REQ=""
  fi
else
  enable_gdk="no"
  GDK_REQ=""
fi
AM_CONDITIONAL(BUILD_WITH_GDK, test x$enable_gdk = xyes)
AC_SUBST(GDK_CFLAGS)
AC_SUBST(GDK_LIBS)
AC_SUBST(GDK_FEATURE)
AC_SUBST(GDK_REQ)

AC_SUBST(POPPLER_GLIB_DISABLE_DEPRECATED)
AC_SUBST(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES)

AC_ARG_ENABLE(poppler-qt,
	      AC_HELP_STRING([--disable-poppler-qt],
			     [Don't compile poppler qt wrapper.]),
	      enable_poppler_qt=$enableval,
	      enable_poppler_qt="try")

if test x$enable_poppler_qt = xyes; then
  POPPLER_FIND_QT(POPPLER_QT,,)
elif test x$enable_poppler_qt = xtry; then
  POPPLER_FIND_QT(POPPLER_QT,
		  [enable_poppler_qt="yes"],
                  [enable_poppler_qt="no"])
fi
AC_SUBST(POPPLER_QT_CXXFLAGS)
AC_SUBST(POPPLER_QT_LIBS)

AM_CONDITIONAL(BUILD_POPPLER_QT, test "x$enable_poppler_qt" = "xyes")

dnl 
dnl Try Qt4
dnl

AC_ARG_ENABLE(poppler-qt4,
              AC_HELP_STRING([--disable-poppler-qt4],
	                     [Don't compile poppler Qt4 wrapper.]),
              enable_poppler_qt4=$enableval,
              enable_poppler_qt4="try")
if test x$enable_poppler_qt4 = xyes; then
  PKG_CHECK_MODULES(POPPLER_QT4, 
	            QtCore >= 4.4.0 QtGui >= 4.4.0 QtXml >= 4.4.0)
  PKG_CHECK_MODULES(POPPLER_QT4_TEST, 
	            QtTest >= 4.4.0)
elif test x$enable_poppler_qt4 = xtry; then
  PKG_CHECK_MODULES(POPPLER_QT4,
                    QtCore >= 4.4.0 QtGui >= 4.4.0 QtXml >= 4.4.0,
                    [enable_poppler_qt4="yes"],
                    [enable_poppler_qt4="no"])
  if test x$enable_poppler_qt4 = xyes; then
    PKG_CHECK_MODULES(POPPLER_QT4_TEST,
                    QtTest >= 4.4.0,
                    [enable_poppler_qt4="yes"],
                    [enable_poppler_qt4="no"])
  fi
fi

AC_SUBST(POPPLER_QT4_CXXFLAGS)
AC_SUBST(POPPLER_QT4_LIBS)
AC_SUBST(POPPLER_QT4_TEST_LIBS)

if test x$enable_poppler_qt4 = xyes; then
  AC_MSG_CHECKING([for Qt4 moc])
  MOCQT4=`which moc`
  mocversion=`$MOCQT4 -v 2>&1`
  mocversiongrep=`echo $mocversion | grep "Qt 4"`
  if test x"$mocversiongrep" != x"$mocversion"; then
    # moc was not the qt4 one, try with moc-qt4
    MOCQT4=`which moc-qt4`
    mocversion=`$MOCQT4 -v 2>&1`
    mocversiongrep=`echo $mocversion | grep "Qt 4"`
    if test x"$mocversiongrep" != x"$mocversion"; then
      # no valid moc found
      enable_poppler_qt4=no;
      MOCQT4="not found"
    fi
  fi
  AC_SUBST(MOCQT4)
  AC_MSG_RESULT([$MOCQT4])
fi

AM_CONDITIONAL(BUILD_POPPLER_QT4, test "x$enable_poppler_qt4" = "xyes")

dnl
dnl CPP frontend
dnl

AC_ARG_ENABLE(poppler-cpp,
              AC_HELP_STRING([--disable-poppler-cpp],
                             [Don't compile poppler cpp wrapper.]),
              enable_poppler_cpp=$enableval,
              enable_poppler_cpp="yes")
if test x$enable_poppler_cpp = xyes; then
  AM_ICONV()
  if test x$am_func_iconv != xyes; then
    enable_poppler_cpp=no
  fi
fi

AM_CONDITIONAL(BUILD_POPPLER_CPP, test "x$enable_poppler_cpp" = "xyes")


AC_ARG_ENABLE(gtk-test,
              AC_HELP_STRING([--disable-gtk-test],
	                     [Don't compile GTK+ test program.]),
              enable_gtk_test=$enableval,
              enable_gtk_test="try")
if test x$enable_gtk_test = xyes; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.14 gdk-pixbuf-2.0 gthread-2.0 gio-2.0)
elif test x$enable_gtk_test = xtry; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.14 gdk-pixbuf-2.0 gthread-2.0 gio-2.0,
                    [enable_gtk_test="yes"],
                    [enable_gtk_test="no"])
fi
AM_CONDITIONAL(BUILD_GTK_TEST, test x$enable_gtk_test = xyes -a x$enable_poppler_glib = xyes)

AC_ARG_ENABLE(abiword-output,
              AC_HELP_STRING([--disable-abiword-output],
                             [Don't build the abiword backend.]),
              enable_abiword_output=$enableval,
              enable_abiword_output="try")
if test x$enable_abiword_output = xyes; then
  PKG_CHECK_MODULES(ABIWORD, libxml-2.0)
elif test x$enable_abiword_output = xtry; then
  PKG_CHECK_MODULES(ABIWORD, libxml-2.0,
                    [enable_abiword_output="yes"],
                    [enable_abiword_output="no"])
fi

AM_CONDITIONAL(BUILD_ABIWORD_OUTPUT, test x$enable_abiword_output = xyes)

AC_ARG_ENABLE(utils,
              AC_HELP_STRING([--disable-utils],
	                     [Don't compile poppler command line utils.]),
              enable_utils=$enableval,
              enable_utils="yes")
AM_CONDITIONAL(BUILD_UTILS, test x$enable_utils = xyes)

AC_ARG_ENABLE(compile-warnings,
              AC_HELP_STRING([--enable-compile-warnings=@<:@no/yes/kde@:>@]
                             [Turn on compiler warnings.]),,
              [enable_compile_warnings="yes"])

dnl
dnl   Color Management
dnl

AC_ARG_ENABLE(cms,
	      AC_HELP_STRING([--disable-cms],
	                     [Don't use color management system.]),
              enable_cms=$enableval,
              enable_cms="try")
if test x$enable_cms = xyes; then
  PKG_CHECK_MODULES(LCMS, lcms)
elif test x$enable_cms = xtry; then
  PKG_CHECK_MODULES(LCMS, lcms, [enable_cms=yes], [enable_cms=no])
fi

if test "x$enable_cms" = "xyes"; then
  AC_DEFINE(USE_CMS, 1, [Defines if use cms])
fi
AM_CONDITIONAL(USE_CMS, test x$enable_cms = xyes)

if test "x$GCC" != xyes; then
  enable_compile_warnings=no
fi
case "$enable_compile_warnings" in
  no)	;;
  yes)	CXXFLAGS="-Wall -Wno-write-strings -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-align -fno-exceptions -fno-common $CXXFLAGS" ;;
  kde)	CXXFLAGS="-Wnon-virtual-dtor -Wno-long-long -Wundef \
		-D_XOPEN_SOURCE=600 -D_BSD_SOURCE -Wcast-align \
		-Wconversion -Wall -W -Wpointer-arith \
		-Wwrite-strings -O2 -Wformat-security \
		-Wmissing-format-attribute -fno-exceptions \
		-fno-common $CXXFLAGS" ;;
esac

case $($PKG_CONFIG --version) in
  0.?|0.1[0-7])
    PC_REQUIRES_PRIVATE="";
    PC_REQUIRES="poppler = $VERSION";;
  *)
    PC_REQUIRES_PRIVATE="Requires.private: poppler = $VERSION";
    PC_REQUIRES="";;
esac

AC_SUBST(PC_REQUIRES)
AC_SUBST(PC_REQUIRES_PRIVATE)

AC_SUBST([POPPLER_MAJOR_VERSION],[poppler_version_major])
AC_SUBST([POPPLER_MINOR_VERSION],[poppler_version_minor])
AC_SUBST([POPPLER_MICRO_VERSION],[poppler_version_micro])
AC_SUBST([POPPLER_VERSION],[poppler_version])

AC_OUTPUT([
Makefile
goo/Makefile
fofi/Makefile
splash/Makefile
poppler/Makefile
utils/Makefile
glib/Makefile
glib/poppler-features.h
glib/reference/Makefile
glib/reference/version.xml
glib/demo/Makefile
test/Makefile
qt/Makefile
qt4/Makefile
qt4/src/Makefile
qt4/tests/Makefile
qt4/demos/Makefile
cpp/Makefile
cpp/poppler-version.h
cpp/tests/Makefile
poppler.pc
poppler-uninstalled.pc
poppler-cairo.pc
poppler-cairo-uninstalled.pc
poppler-splash.pc
poppler-splash-uninstalled.pc
poppler-glib.pc
poppler-glib-uninstalled.pc
poppler-qt.pc
poppler-qt-uninstalled.pc
poppler-qt4.pc
poppler-qt4-uninstalled.pc
poppler-cpp.pc
poppler-cpp-uninstalled.pc])


echo ""
echo "Building poppler with support for:"
echo "  font configuration: $with_font_configuration"
echo "  splash output:      $enable_splash_output"
echo "  cairo output:       $enable_cairo_output"
echo "  abiword output:     $enable_abiword_output"
echo "  qt wrapper:         $enable_poppler_qt"
echo "  qt4 wrapper:        $enable_poppler_qt4"
echo "  glib wrapper:       $enable_poppler_glib"
echo "    use GDK:          $enable_gdk"
echo "    introspection:    $found_introspection"
echo "  cpp wrapper:        $enable_poppler_cpp"
echo "  use gtk-doc:        $enable_gtk_doc"
echo "  use libjpeg:        $enable_libjpeg"
echo "  use libpng:         $enable_libpng"
echo "  use zlib:           $enable_zlib"
echo "  use libcurl:        $enable_libcurl"
echo "  use libopenjpeg:    $enable_libopenjpeg"
echo "  use cms:            $enable_cms"
echo "  command line utils: $enable_utils"
echo ""

if test x$enable_splash_output = xno -a x$enable_cairo_output = xno; then
	echo "  Warning: There is no rendering backend enabled"
fi

if test x$enable_single_precision = xyes -a x$enable_fixedpoint = xyes; then
	echo "  Warning: Single precision and fixed point options should not be enabled at the same time"
fi

if test x$enable_libjpeg != xyes; then
	echo "  Warning: Using libjpeg is recommended"
fi

if test x$enable_zlib != xno; then
	echo "  Warning: Using zlib is not totally safe"
fi

if test x$enable_libopenjpeg != xyes; then
	echo "  Warning: Using libopenjpeg is recommended"
fi
