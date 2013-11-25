m4_define([poppler_version_major],[0])
m4_define([poppler_version_minor],[24])
m4_define([poppler_version_micro],[3])
m4_define([poppler_version],[poppler_version_major.poppler_version_minor.poppler_version_micro])

AC_PREREQ(2.59)
AC_INIT([poppler],[poppler_version],[https://bugs.freedesktop.org/enter_bug.cgi?product=poppler])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_HEADERS([config.h poppler/poppler-config.h])
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
      case "${CXXFLAGS}" in
      *-std=*)
        ;;
      *)
        CXXFLAGS="$CXXFLAGS -ansi"
        ;;
      esac
    ;;
  esac
fi
AC_PROG_INSTALL

dnl ##### Check for pkgconfig
PKG_PROG_PKG_CONFIG

dnl ##### Export the version
AC_DEFINE_UNQUOTED([POPPLER_VERSION], ["poppler_version"], [Defines the poppler version])

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

    # Use mingw's ansi stdio extensions
    CXXFLAGS="$CXXFLAGS -D__USE_MINGW_ANSI_STDIO=1"
  ;;
esac

AC_SUBST(win32_libs)
AC_SUBST(create_shared_lib)
AC_SUBST(auto_import_flags)

AX_PTHREAD()

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
AC_CHECK_HEADERS([stdint.h])

dnl ##### Switch over to C++.  This will make the checks below a little
dnl ##### bit stricter (requiring function prototypes in include files).
dnl ##### (99% of xpdf is written in C++.)
AC_LANG_CPLUSPLUS

AC_CHECK_DECL(gettimeofday, [AC_CHECK_FUNC(gettimeofday, AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defines if gettimeofday is available on your system]))],[],[#include <sys/time.h>])
AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R, 1, [Defines if localtime_r is available on your system]))
AC_CHECK_FUNC(gmtime_r, AC_DEFINE(HAVE_GMTIME_R, 1, [Defines if gmtime_r is available on your system]))
AC_CHECK_FUNC(rand_r, AC_DEFINE(HAVE_RAND_R, 1, [Defines if rand_r is available on your system]))

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


dnl Test for libopenjpeg. Versions prior to 1.4 do not provide a pkgconfig file.
AC_ARG_ENABLE(libopenjpeg,
	      AC_HELP_STRING([--disable-libopenjpeg],
	                     [Don't build against libopenjpeg.]),
              enable_libopenjpeg=$enableval,
              enable_libopenjpeg="try")
if test x$enable_libopenjpeg = xyes; then
  PKG_CHECK_MODULES(LIBOPENJPEG, libopenjpeg,
  		    [],
    		    [AC_CHECK_LIB([openjpeg], [opj_cio_open],
  			LIBOPENJPEG_LIBS="-lopenjpeg",
		        AC_MSG_ERROR("*** libopenjpeg library not found ***"))
		     AC_CHECK_HEADERS([openjpeg.h],,
		     	AC_MSG_ERROR("*** libopenjpeg headers not found ***"))])
elif test x$enable_libopenjpeg = xtry; then
  PKG_CHECK_MODULES(LIBOPENJPEG, libopenjpeg,
                    [enable_libopenjpeg="yes"],
  		    [AC_CHECK_LIB([openjpeg], [opj_cio_open],
		        [enable_libopenjpeg="yes"
  			 LIBOPENJPEG_LIBS="-lopenjpeg"],
	                [enable_libopenjpeg="no"])
		     AC_CHECK_HEADERS([openjpeg.h],,
		         [enable_libopenjpeg="no"])])
fi

if test x$enable_libopenjpeg = xyes; then
  AC_SUBST(LIBOPENJPEG_CFLAGS)
  AC_SUBST(LIBOPENJPEG_LIBS)
  AC_DEFINE(ENABLE_LIBOPENJPEG)
  PKG_CHECK_EXISTS(libopenjpeg >= 1.5,
      [AC_DEFINE(WITH_OPENJPEG_IGNORE_PCLR_CMAP_CDEF_FLAG, 1, [OpenJPEG with the OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG flag.])],
      [])
fi

AM_CONDITIONAL(BUILD_LIBOPENJPEG, test x$enable_libopenjpeg = xyes)
AH_TEMPLATE([ENABLE_LIBOPENJPEG],
	    [Use libopenjpeg instead of builtin jpeg2000 decoder.])

dnl ##### Test for libtiff
AC_ARG_ENABLE(libtiff,
		AC_HELP_STRING([--disable-libtiff],
				[Don't build against libtiff.]),
		enable_libtiff=$enableval,
		enable_libtiff="try")

AC_ARG_VAR([LIBTIFF_CFLAGS], [C compiler flags for LIBTIFF])
AC_ARG_VAR([LIBTIFF_LIBS], [linker flags to link LIBTIFF (default is -ltiff)])
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_CXXFLAGS="$CXXFLAGS"
ac_save_LIBS="$LDFLAGS"
CPPFLAGS="$CPPFLAGS $LIBTIFF_CFLAGS"
CXXFLAGS="$CXXFLAGS $LIBTIFF_CFLAGS"
LIBS="$LIBS $LIBTIFF_LIBS"
if test x$enable_libtiff = xyes; then
  if test x"$LIBTIFF_LIBS" != ; then
    AC_CHECK_FUNC([TIFFOpen],,
		  AC_MSG_ERROR("*** libtiff library not found ***"))
  else
    AC_CHECK_LIB([tiff], [TIFFOpen],,
		  AC_MSG_ERROR("*** libtiff library not found ***"))
  fi
  AC_CHECK_HEADERS([tiffio.h],,
		AC_MSG_ERROR("*** libtiff headers not found ***"))
elif test x$enable_libtiff = xtry; then
  if test x"$LIBTIFF_LIBS" != x; then
    AC_CHECK_FUNC([TIFFOpen],
		  [enable_libtiff="yes"],
		  [enable_libtiff="no"])
  else
    AC_CHECK_LIB([tiff], [TIFFOpen],
		 [enable_libtiff="yes"],
		 [enable_libtiff="no"])
  fi
  AC_CHECK_HEADERS([tiffio.h],,
		[enable_libtiff="no"])
fi
CPPFLAGS="$ac_save_CPPFLAGS"
CXXFLAGS="$ac_save_CXXFLAGS"
LIBS="$ac_save_LIBS"

if test x$enable_libtiff = xyes; then
  if test x"$LIBTIFF_LIBS" = x; then
    LIBTIFF_LIBS="-ltiff"
  fi
  AC_SUBST(LIBTIFF_CFLAGSS)
  AC_SUBST(LIBTIFF_LIBS)
  AC_DEFINE(ENABLE_LIBTIFF)
fi

AM_CONDITIONAL(BUILD_LIBTIFF, test x$enable_libtiff = xyes)
AH_TEMPLATE([ENABLE_LIBTIFF], [Build against libtiff.])
if test x$enable_libtiff = xyes; then
  AC_DEFINE(ENABLE_LIBTIFF, 1, [Build against libtiff.])
fi

dnl ##### Checks for library functions.
AC_CHECK_FUNCS(popen mkstemp mkstemps)
AC_CHECK_FUNCS(strcpy_s strcat_s)

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
AC_CHECK_FUNCS(pread64 lseek64)

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
AC_ARG_VAR([LIBJPEG_CFLAGS], [C compiler flags for LIBJPEG])
if test x$enable_libjpeg != xno; then

  dnl
  dnl POPPLER_FIND_JPEG uses "USER_INCLUDES" and "USER_LIBS"
  dnl to receive the flags for header and library directories.
  dnl
  ac_save_USER_INCLUDES="$USER_INCLUDES"
  ac_save_USER_LDFLAGS="$USER_LDFLAGS"
  USER_INCLUDES="$USER_INCLUDES $LIBJPEG_CFLAGS"
  USER_LDFLAGS="$USER_LDFLAGS $LIBJPEG_CFLAGS"
  POPPLER_FIND_JPEG

  dnl check INT16, INT32 typedef conflict in jmorecfg.h
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_CFLAGS="$CFLAGS"
  CPPFLAGS="$CPPFLAGS $LIBJPEG_CFLAGS"
  CFLAGS="$CFLAGS $LIBJPEG_CFLAGS"
  AC_MSG_CHECKING([libjpeg.h works correctly])
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([
#ifdef _WIN32
#include <windows.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <jpeglib.h>
],[{return 0;}])],[
    AC_MSG_RESULT([ok])
  ],[
    AC_MSG_RESULT([no])
    AC_MSG_CHECKING([libjpeg.h problem can be fixed by XMD_H macro])
    CPPFLAGS="$CPPFLAGS -DXMD_H"
    CFLAGS="$CFLAGS -DXMD_H"
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#ifdef _WIN32
#include <windows.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <jpeglib.h>
],[{return 0;}])],[
      AC_MSG_RESULT([ok, -DXMD_H is added to LIBJPEG_CFLAGS])
      LIBJPEG_CFLAGS="$LIBJPEG_CFLAGS -DXMD_H"
    ],[
      AC_MSG_RESULT([no, disable libjpeg])
      enable_libjpeg="no"
    ])
  ])
  CPPFLAGS="$ac_save_CPPFLAGS"
  CFLAGS="$ac_save_CFLAGS"

  dnl POPPLER_FIND_JPEG sets LIBJPEG_LIBS
  AC_SUBST(LIBJPEG_CFLAGS)
  USER_INCLUDES="$ac_save_USER_INCLUDES"
  USER_LDFLAGS="$ac_save_USER_LDFLAGS"
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

PKG_CHECK_MODULES(FREETYPE, freetype2,
                  [freetype_pkgconfig=yes], [freetype_pkgconfig=no])

if test "x$freetype_pkgconfig" = "xyes"; then

  AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])

else

  FREETYPE_LIBS=
  FREETYPE_CFLAGS=

  AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
  if test "x$FREETYPE_CONFIG" != "xno" ; then

    FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
    FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
    AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])

  fi

fi

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
use_cairo=""
if test x$enable_cairo_output = xyes; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION cairo-ft >= $CAIRO_VERSION)
elif test x$enable_cairo_output = xtry; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION cairo-ft >= $CAIRO_VERSION,
                    [enable_cairo_output="yes"],
                    [enable_cairo_output="no"
		     use_cairo="no (requires cairo >= $CAIRO_VERSION)"])
fi
if test "x$use_cairo" = "x"; then
   use_cairo=$enable_cairo_output
fi

AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

AM_CONDITIONAL(BUILD_CAIRO_OUTPUT, test x$enable_cairo_output = xyes)
AH_TEMPLATE([HAVE_CAIRO], [Use cairo for rendering.])
if test x$enable_cairo_output = xyes; then
  PDFTOCAIRO_DEPS="cairo cairo-ft"
  dnl Check for optional cairo backends used by pdftocairo
  PKG_CHECK_EXISTS(cairo-pdf, [PDFTOCAIRO_DEPS="$PDFTOCAIRO_DEPS cairo-pdf"], [])
  PKG_CHECK_EXISTS(cairo-ps,  [PDFTOCAIRO_DEPS="$PDFTOCAIRO_DEPS cairo-ps"],  [])
  PKG_CHECK_EXISTS(cairo-svg, [PDFTOCAIRO_DEPS="$PDFTOCAIRO_DEPS cairo-svg"], [])

  PKG_CHECK_MODULES(PDFTOCAIRO, $PDFTOCAIRO_DEPS)
  AC_SUBST(PDFTOCAIRO_CFLAGS)
  AC_SUBST(PDFTOCAIRO_LIBS)

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

use_glib=""
found_introspection=no
if test x$enable_cairo_output = xyes; then
  POPPLER_GLIB_DISABLE_DEPRECATED=""
  POPPLER_GLIB_DISABLE_SINGLE_INCLUDES=""

  GLIB_REQUIRED=2.18
  AC_SUBST(GLIB_REQUIRED)
  AC_ARG_ENABLE(poppler-glib,
	        AC_HELP_STRING([--disable-poppler-glib],
		               [Don't compile poppler glib wrapper.]),
              		       enable_poppler_glib=$enableval,
			       enable_poppler_glib="try")
  if test x$enable_poppler_glib = xyes; then
    PKG_CHECK_MODULES(POPPLER_GLIB, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED gio-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION)
  elif test x$enable_poppler_glib = xtry; then
    PKG_CHECK_MODULES(POPPLER_GLIB, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED gio-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION,
                      [enable_poppler_glib="yes"],
                      [enable_poppler_glib="no"
                      use_glib="no (requires glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED gio-2.0 >= $GLIB_REQUIRED cairo >= $CAIRO_VERSION)"])
  fi
  if test x$enable_poppler_glib = xyes; then
    # Check for introspection
    GOBJECT_INTROSPECTION_CHECK([0.6.7])

    AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])
    POPPLER_GLIB_DISABLE_DEPRECATED="$POPPLER_GLIB_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED"
    POPPLER_GLIB_DISABLE_SINGLE_INCLUDES="$POPPLER_GLIB_DISABLE_SINGLE_INCLUDES -DG_DISABLE_SINGLE_INCLUDES"
  fi
else
  if test x$enable_poppler_glib = xyes; then
    AC_MSG_ERROR("Cairo output is required to build glib frontend")
  fi
  use_glib="no (requires cairo output)"
  enable_poppler_glib="no"
fi
if test x"$use_glib" = x; then
   use_glib="$enable_poppler_glib"
fi
AM_CONDITIONAL(BUILD_POPPLER_GLIB, test x$enable_poppler_glib = xyes)
AM_CONDITIONAL(HAVE_INTROSPECTION, test "x$found_introspection" = "xyes")
AC_SUBST(GLIB_REQ)
AC_SUBST(POPPLER_GLIB_DISABLE_DEPRECATED)
AC_SUBST(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES)

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

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
  AC_CHECK_TOOL(MOCQT4, moc)
  AC_MSG_CHECKING([for Qt4 moc])
  mocversion=`$MOCQT4 -v 2>&1`
  mocversiongrep=`echo $mocversion | grep "Qt 4"`
  if test x"$mocversiongrep" != x"$mocversion"; then
    AC_MSG_RESULT([no])
    # moc was not the qt4 one, try with moc-qt4
    AC_CHECK_TOOL(MOCQT42, moc-qt4)
    AC_MSG_CHECKING([for Qt4 moc-qt4])
    mocversion=`$MOCQT42 -v 2>&1`
    mocversiongrep=`echo $mocversion | grep "Qt 4"`
    if test x"$mocversiongrep" != x"$mocversion"; then
      # no valid moc found
      enable_poppler_qt4=no;
      MOCQT4="not found"
    else
      MOCQT4=$MOCQT42
    fi
  fi
  AC_SUBST(MOCQT4)
  AC_MSG_RESULT([$MOCQT4])
fi

AM_CONDITIONAL(BUILD_POPPLER_QT4, test "x$enable_poppler_qt4" = "xyes")

dnl 
dnl Try Qt5
dnl

AC_ARG_ENABLE(poppler-qt5,
              AC_HELP_STRING([--disable-poppler-qt5],
	                     [Don't compile poppler Qt5 wrapper.]),
              enable_poppler_qt5=$enableval,
              enable_poppler_qt5="try")
if test x$enable_poppler_qt5 = xyes; then
  PKG_CHECK_MODULES(POPPLER_QT5, 
	            Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Xml >= 5.0.0 Qt5Widgets >= 5.0.0)
  PKG_CHECK_MODULES(POPPLER_QT5_TEST, 
	            Qt5Test >= 5.0.0 ) 
elif test x$enable_poppler_qt5 = xtry; then
  PKG_CHECK_MODULES(POPPLER_QT5,
                    Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Xml >= 5.0.0 Qt5Widgets >= 5.0.0,
                    [enable_poppler_qt5="yes"],
                    [enable_poppler_qt5="no"])
  if test x$enable_poppler_qt5 = xyes; then
    PKG_CHECK_MODULES(POPPLER_QT5_TEST,
                    Qt5Test >= 5.0.0,
                    [enable_poppler_qt5="yes"],
                    [enable_poppler_qt5="no"])
  fi
fi

AC_SUBST(POPPLER_QT5_CXXFLAGS)
AC_SUBST(POPPLER_QT5_LIBS)
AC_SUBST(POPPLER_QT5_TEST_LIBS)

if test x$enable_poppler_qt5 = xyes; then
  AC_CHECK_TOOL(MOCQT5, moc)
  AC_MSG_CHECKING([for Qt5 moc])
  mocversion=`$MOCQT5 -v 2>&1`
  mocversiongrep=`echo $mocversion | grep "Qt 5"`
  if test x"$mocversiongrep" != x"$mocversion"; then
    AC_MSG_RESULT([no])
    # moc was not the qt5 one, try with moc-qt5
    AC_CHECK_TOOL(MOCQT52, moc-qt5)
    AC_MSG_CHECKING([for Qt5 moc-qt5])
    mocversion=`$MOCQT52 -v 2>&1`
    mocversiongrep=`echo $mocversion | grep "Qt 5"`
    if test x"$mocversiongrep" != x"$mocversion"; then
      AC_CHECK_TOOL(QTCHOOSER, qtchooser)
      AC_MSG_CHECKING([for qtchooser])
      qt5tooldir=`QT_SELECT=qt5 qtchooser -print-env | grep QTTOOLDIR | cut -d '=' -f 2 | cut -d \" -f 2`
      mocversion=`$qt5tooldir/moc -v 2>&1`
      mocversiongrep=`echo $mocversion | grep "Qt 5"`
      if test x"$mocversiongrep" != x"$mocversion"; then
        # no valid moc found
        enable_poppler_qt5=no;
        MOCQT5="not found"
      else
        MOCQT5=$qt5tooldir/moc
      fi
    else
      MOCQT5=$MOCQT52
    fi
  fi
  AC_SUBST(MOCQT5)
  AC_MSG_RESULT([$MOCQT5])
fi

AM_CONDITIONAL(BUILD_POPPLER_QT5, test "x$enable_poppler_qt5" = "xyes")

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
  PKG_CHECK_MODULES(GTK_TEST, gtk+-3.0 >= 3.0 gdk-pixbuf-2.0)
elif test x$enable_gtk_test = xtry; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-3.0 >= 3.0 gdk-pixbuf-2.0,
                    [enable_gtk_test="yes"],
                    [enable_gtk_test="no"])
fi
AM_CONDITIONAL(BUILD_GTK_TEST, test x$enable_gtk_test = xyes -a x$enable_poppler_glib = xyes)

AC_ARG_ENABLE(utils,
              AC_HELP_STRING([--disable-utils],
	                     [Don't compile poppler command line utils.]),
              enable_utils=$enableval,
              enable_utils="yes")
AM_CONDITIONAL(BUILD_UTILS, test x$enable_utils = xyes)

AC_ARG_ENABLE(compile-warnings,
              AC_HELP_STRING([--enable-compile-warnings=@<:@no/yes/kde@:>@],
                             [Turn on compiler warnings.]),,
              [enable_compile_warnings="yes"])

dnl
dnl   Color Management
dnl

AC_ARG_ENABLE(cms,
              AC_HELP_STRING([--enable-cms=@<:@auto/lcms1/lcms2/none@:>@],
                             [Use color management system. 'auto' prefers lcms2 over lcms1 if both are available [[default=auto]]]),
              [enable_cms=$enableval],
              [enable_cms="auto"])
if test x$enable_cms = xauto; then
  PKG_CHECK_MODULES(LCMS, lcms2, [lcms2=yes], [lcms2=no])
  if test x$lcms2 = xno; then
      PKG_CHECK_MODULES(LCMS, lcms, [lcms1=yes], [lcms1=no])
  fi
elif test x$enable_cms = xlcms1; then
  PKG_CHECK_MODULES(LCMS, lcms, [lcms1=yes], [lcms1=no])
elif test x$enable_cms = xlcms2; then
  PKG_CHECK_MODULES(LCMS, lcms2, [lcms2=yes], [lcms2=no])
fi

if test x$lcms1 = xyes || test x$lcms2 = xyes; then
  enable_cms=yes
  AC_DEFINE(USE_CMS, 1, [Defines if use cms])
  if test x$lcms1 = xyes; then
    lcms1=yes;
    AC_DEFINE(USE_LCMS1, 1, [Defines if use lcms1])
  fi
fi

AM_CONDITIONAL(USE_CMS, test x$enable_cms = xyes)
AM_CONDITIONAL(USE_LCMS1, test x$lcms1 = xyes)

AC_ARG_WITH([testdatadir],
   [AS_HELP_STRING([--with-testdatadir=/path/to/testdatadir],
                   [Specify test data dir])],
   [case $withval in
       [[\\/]]* | ?:[[\\/]]* )  # Absolute name.
          TESTDATADIR=$withval ;;
       *) # Relative name.
          TESTDATADIR=$ac_pwd/$withval ;;
    esac
   ],
   [case $srcdir in
       [[\\/]]* | ?:[[\\/]]* )  # Absolute name.
          TESTDATADIR=$srcdir/../test ;;
       *) # Relative name.
          TESTDATADIR=$ac_pwd/$srcdir/../test ;;
    esac
   ]
)
AC_MSG_CHECKING([for test data in $TESTDATADIR])
if test -d $TESTDATADIR && test -f $TESTDATADIR/test-poppler.c; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_MSG_WARN([
 No test data found in $TESTDATADIR.
 You will not be able to run 'make check' successfully.

 The test data is not included in the source packages
 and is also not part of the main git repository. Instead,
 you can checkout the test data from its own git
 repository with:

   git clone git://git.freedesktop.org/git/poppler/test

 You should checkout the test data as a sibling of your
 poppler source folder or specify the location of your
 checkout with --with-testdatadir=/path/to/checkoutdir/test.
])
fi
AC_SUBST(TESTDATADIR)

if test "x$GCC" != xyes; then
  enable_compile_warnings=no
fi
case "$enable_compile_warnings" in
  no)	;;
  yes)	CXXFLAGS="-Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-align -fno-exceptions -fno-check-new -fno-common $CXXFLAGS";
        CFLAGS="-Wall $CFLAGS" ;;
  kde)	CXXFLAGS="-Wnon-virtual-dtor -Wno-long-long -Wundef \
		-D_XOPEN_SOURCE=600 -D_BSD_SOURCE -Wcast-align \
		-Wconversion -Wall -W -Wpointer-arith \
		-Wwrite-strings -O2 -Wformat-security \
		-Wmissing-format-attribute -fno-exceptions -fno-check-new \
		-fno-common $CXXFLAGS" ;;
esac
CXXFLAGS="-fPIC $CXXFLAGS";

case `$PKG_CONFIG --version` in
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
qt4/Makefile
qt4/src/Makefile
qt4/tests/Makefile
qt4/demos/Makefile
qt5/Makefile
qt5/src/Makefile
qt5/tests/Makefile
qt5/demos/Makefile
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
poppler-qt4.pc
poppler-qt4-uninstalled.pc
poppler-qt5.pc
poppler-qt5-uninstalled.pc
poppler-cpp.pc
poppler-cpp-uninstalled.pc])


echo ""
echo "Building poppler with support for:"
echo "  font configuration: $with_font_configuration"
echo "  splash output:      $enable_splash_output"
echo "  cairo output:       $use_cairo"
echo "  qt4 wrapper:        $enable_poppler_qt4"
echo "  qt5 wrapper:        $enable_poppler_qt5"
echo "  glib wrapper:       $use_glib"
echo "    introspection:    $found_introspection"
echo "  cpp wrapper:        $enable_poppler_cpp"
echo "  use gtk-doc:        $enable_gtk_doc"
echo "  use libjpeg:        $enable_libjpeg"
echo "  use libpng:         $enable_libpng"
echo "  use libtiff:        $enable_libtiff"
echo "  use zlib:           $enable_zlib"
echo "  use libcurl:        $enable_libcurl"
echo "  use libopenjpeg:    $enable_libopenjpeg"
echo "  use cms:            $enable_cms"
if test x$enable_cms = xyes;then
    if test x$lcms1 = xyes;then
        echo "      with lcms1"
    else
        echo "      with lcms2"
    fi
fi
echo "  command line utils: $enable_utils"
echo "  test data dir:      $TESTDATADIR"
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
