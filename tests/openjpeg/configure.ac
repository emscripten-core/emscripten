#AC_PREREQ([2.65])
define([OPJ_MAJOR], [1])
define([OPJ_MINOR], [4])
define([OPJ_BUILD], [0])
#
define([JP3D_MAJOR], [1])
define([JP3D_MINOR], [3])
define([JP3D_BUILD], [0])
#
#
#AC_INIT (PACKAGE, VERSION, [BUG-REPORT], [TARNAME], [URL])
AC_INIT(OpenJPEG,
OPJ_MAJOR.OPJ_MINOR.OPJ_BUILD,
http://code.google.com/p/openjpeg/,
openjpeg-OPJ_MAJOR.OPJ_MINOR.OPJ_BUILD,
http://www.openjpeg.org/)
AC_CONFIG_SRCDIR([libopenjpeg/openjpeg.c])
AC_CONFIG_HEADERS(opj_config.h)
#
AC_CANONICAL_SYSTEM
#foreign:package will not follow the GNU Standards
AM_INIT_AUTOMAKE(foreign)
#
MAJOR_NR=OPJ_MAJOR
MINOR_NR=OPJ_MINOR
BUILD_NR=OPJ_BUILD
AC_SUBST(MAJOR_NR)
AC_SUBST(MINOR_NR)
AC_SUBST(BUILD_NR)
#
JP3D_MAJOR_NR=JP3D_MAJOR
JP3D_MINOR_NR=JP3D_MINOR
JP3D_BUILD_NR=JP3D_BUILD
AC_SUBST(JP3D_MAJOR_NR)
AC_SUBST(JP3D_MINOR_NR)
AC_SUBST(JP3D_BUILD_NR)
#
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_PROG_MAKE_SET
#AC_HEADER_DIRENT
#
# Tests for Windows
#
AC_EXEEXT
AC_OBJEXT
#
# Configure libtool
AC_ENABLE_SHARED
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_SETUP
AC_PROG_LIBTOOL
#
AC_C_BIGENDIAN
#
LIB_MATH=""
AC_CHECK_LIB(m,sqrt,LIB_MATH="-lm",,)
LIB_Z=""
AC_CHECK_LIB(z,inflate,LIB_Z="-lz",,)
#
LIBS="$LIBS $LIB_MATH $LIB_Z"
#
AC_PATH_PROG([PKGCONFIG], [pkg-config])
#
# ------------------------------
# Test for PNG
# ------------------------------
AC_MSG_CHECKING(whether to build with PNG support)
pngincludes=""
pnglibs=""
png_header_found="no"
with_libpng="yes"
#
AC_ARG_ENABLE(png,
[  --enable-png[=[yes|no]]          Build with PNG support [ [default=yes] ]],
test "$enable_png" = "no"  && with_libpng="no")
#
if test "x$with_libpng" = xyes ; then
  with_libpng="no"
  AC_PATH_PROG([LIBPNG_CONFIG], [libpng-config])
#
  pngheader=""
  AC_ARG_WITH(png_includes,
 [  --with-png-includes=DIR         PNG includes in nonstandard DIR])
  if test -n "$with_png_includes" ; then
   pngheader="$with_png_includes"
   pngincludes="-I$with_png_includes"
  else
   if test -n "$PKGCONFIG" ; then
     pngincludes="`$PKGCONFIG --variable=includedir libpng`"
     if test -n "$pngincludes" ; then
      pngheader="$pngincludes"
      pngincludes="-I$pngincludes"
     fi
   else
     if test -n "$LIBPNG_CONFIG" ; then
      pngincludes="`$LIBPNG_CONFIG --I_opts`"
      pngheader="`$LIBPNG_CONFIG --I_opts | $SED 's/-I//'`"
     fi
   fi
  fi
  if test -n "$pngheader" ; then
   pngheader="$pngheader/png.h"
  else
   pngheader="png.h"
  fi
#
  AC_CHECK_HEADER($pngheader,png_header_found="yes",,)
#
  AC_ARG_WITH(png_libraries,
 [  --with-png-libraries=DIR        PNG library in nonstandard DIR])
  if [[ -n "$with_png_libraries" ]] ; then
   pnglibs="-L$with_png_libraries -lpng"
  else
    if test -n "$LIBPNG_CONFIG" ; then
     pnglibs="`$LIBPNG_CONFIG --ldflags`"
    else
      if test -n "$PKGCONFIG" ; then
        pnglibs="`$PKGCONFIG --libs libpng`"
      fi
    fi
  fi
  savedLIBS="$LIBS"
  LIBS="$pnglibs $LIB_Z $LIB_MATH"
#
  AC_CHECK_LIB(png,png_create_read_struct,,pnglibs="",)
#
  LIBS="$savedLIBS"
  if test -n "$pnglibs" ; then
    if test -n "$pngincludes" ; then
      with_libpng="yes"
      png_header_found="yes"
      LIBS="$LIBS $pnglibs"
      AC_DEFINE(HAVE_LIBPNG, [1], [define to 1 if you have libpng])
    fi
  fi
fi
#
AC_MSG_RESULT(png is usable:$with_libpng)
AM_CONDITIONAL([with_libpng], [test x$with_libpng = xyes])
AC_SUBST(pngincludes)
AC_SUBST(pnglibs)
# -------------------------------
# Test for TIFF
# -------------------------------
AC_MSG_CHECKING(whether to build with TIFF support)
tifflibs=""
tiffincludes=""
with_libtiff="yes"
tiff_header_found="no"
#
AC_ARG_ENABLE(tiff,
[  --enable-tiff[=[yes|no]]         Build with TIFF support [ [default=yes] ]],
test "$enable_tiff" = "no"  && with_libtiff="no")
#
if test "x$with_libtiff" = xyes ; then
#TIFF compiled with JPEG and JBIG support?
 libjpeg=""
 AC_CHECK_LIB(jpeg,jpeg_read_header,libjpeg=-ljpeg,,)
 libjbig=""
 AC_CHECK_LIB(jbig,jbg_enc_init,libjbig=-ljbig,,)
 libjbig85=""
 AC_CHECK_LIB(jbig85,jbg85_enc_init,libjbig85=-ljbig85,,)
#standard path
 with_libtiff="no"
 savedLIBS="$LIBS"
 LIBS="-ltiff $libjpeg $libjbig $libjbig85 $LIB_Z $LIB_MATH"
 AC_CHECK_LIB(tiff, TIFFOpen, tifflibs=-ltiff)
 LIBS="$savedLIBS"
 if test -n "$tifflibs" ; then
  AC_CHECK_HEADER(tiff.h,tiff_header_found="yes",,)
  if test "$tiff_header_found" = "yes" ; then 
   with_libtiff="yes"
   LIBS="$LIBS -ltiff $libjpeg $libjbig $libjbig85"
   AC_DEFINE(HAVE_LIBTIFF, [1], [define to 1 if you have libtiff])
  fi
 fi
#
 if test "x$with_libtiff" = xno ; then
#non-standard path
  AC_ARG_WITH(tiff_includes,
  [  --with-tiff-includes=DIR        TIFF includes in nonstandard DIR])
  if test -n "$with_tiff_includes" ; then
   tiffheader="$with_tiff_includes/tiff.h"
   AC_CHECK_HEADER($tiffheader,tiff_header_found="yes",,)
   if test "$tiff_header_found" = "yes" ; then
     tiffincludes="-I$with_tiff_includes"
   fi
  fi
  AC_ARG_WITH(tiff_libraries,
  [  --with-tiff-libraries=DIR       TIFF library in nonstandard DIR])
  if test -n "$with_tiff_libraries" ; then
   tifflibs="-L$with_tiff_libraries"
  fi
  tifflibs="$tifflibs -ltiff"
  savedLIBS="$LIBS"
  LIBS="$tifflibs $libjpeg $libjbig $libjbig85 $LIB_Z $LIB_MATH"
  with_libtiff="no"
#
  AC_CHECK_LIB(tiff, TIFFOpen, , tifflibs="",)
#
  if test -n "$tifflibs" ; then
   if test "$tiff_header_found" = "yes" ; then
    with_libtiff="yes"
    AC_DEFINE(HAVE_LIBTIFF, [1], [define to 1 if you have libtiff])
   fi
  fi
  LIBS="$savedLIBS $tifflibs $libjpeg $libjbig $libjbig85"
 fi
fi
#
AC_MSG_RESULT(tiff is usable:$with_libtiff)
AM_CONDITIONAL([with_libtiff], [test x$with_libtiff = xyes])
AC_SUBST(tiffincludes)
AC_SUBST(tifflibs)
# -------------------------------
# Test for LCMS version 2.x 
# -------------------------------
AC_MSG_CHECKING(whether to build with LCMS support)
lcms1includes=""
lcms1libs=""
lcms2includes=""
lcms2libs=""
with_liblcms1="no"
lcms1_header_found="no"
lcms2_header_found="no"
have_lcms1_version=""
have_lcms2_version=""
with_liblcms2="yes"
#
AC_ARG_ENABLE(lcms2,
[  --enable-lcms2[=[yes|no]]        Build with LCMS-2 support [ [default=yes] ]],
test "$enable_lcms2" = "no"  && with_liblcms2="no")
#
if test "x$with_liblcms2" = xyes ; then
  with_liblcms2="no"
  lcms2header=""
  AC_ARG_WITH(lcms2_includes,
 [  --with-lcms2-includes=DIR       LCMS-2 includes in nonstandard DIR])
  if test -n "$with_lcms2_includes" ; then
   lcms2header="$with_lcms2_includes"
   lcms2includes="-I$with_lcms2_includes"
  else
   if test -n "$PKGCONFIG" ; then
     lcms2includes="`$PKGCONFIG --variable=prefix lcms2`"
   fi
   if test -n "$lcms2includes" ; then
    lcms2header="$lcms2_includes"
    lcms2includes="-I$lcms2includes/include"
   fi
  fi
  if test -n "$lcms2header" ; then
   lcms2header="$lcms2header/lcms2.h"
  else
   lcms2header="lcms2.h"
  fi
  AC_CHECK_HEADER($lcms2header,lcms2_header_found="yes",,)
#
  AC_ARG_WITH(lcms2_libraries,
 [  --with-lcms2-libraries=DIR      LCMS-2 library in nonstandard DIR])
  if test -n "$with_lcms2_libraries" ; then
   lcms2libs="-L$with_lcms2_libraries -llcms2"
  else
   if test -n "$PKGCONFIG" ; then
     lcms2libs="`$PKGCONFIG --libs lcms2`"
   fi
   if test -z "$lcms2libs" ; then
    lcms2libs="-llcms2"
   fi
   lcms2libs="$lcms2libs"
  fi
  savedLIBS="$LIBS"
  LIBS="$lcms2libs $LIB_MATH"
  with_liblcms2="no"
#
  AC_CHECK_LIB(lcms2, cmsOpenProfileFromFile, ,lcms2libs="",)
#
  if test -n "$lcms2libs" ; then
   if test "$lcms2_header_found" = "yes" ; then
    with_liblcms2="yes"
    AC_DEFINE(HAVE_LIBLCMS2, [1], [define to 1 if you have lcms version 2.x])
    if test -n "$PKGCONFIG" ; then
      have_lcms2_version="`$PKGCONFIG --modversion lcms2`"
    fi
   fi
  fi
  if test "$with_liblcms2" = "no" ; then
    lcms2libs=""
    lcms2includes=""
    lcms2libs=""
  fi
  LIBS="$savedLIBS $lcms2libs"
fi
#
AC_MSG_RESULT(lcms2 is usable:$with_liblcms2)
AM_CONDITIONAL([with_liblcms2], [test x$with_liblcms2 = xyes])
AC_SUBST(lcms2includes)
AC_SUBST(lcms2libs)
AC_MSG_RESULT(lcms2libs:$lcms2libs)
# ----------------------------
# Test for LCMS version 1.x
# ----------------------------
if [[ -z "$lcms2libs" ]] ; then
 with_liblcms1="yes"
 AC_ARG_ENABLE(lcms1,
 [  --enable-lcms1[=[yes|no]]        Build with LCMS-1 support [ [default=yes] ]],
 test "$enable_lcms1" = "no"  && with_liblcms1="no")
#
 if test "x$with_liblcms1" = xyes ; then
   with_liblcms1="no"
   lcms1header=""
   AC_ARG_WITH(lcms1_includes,
  [  --with-lcms1-includes=DIR       LCMS-1 includes in nonstandard DIR])
   if test -n "$with_lcms1_includes" ; then
    lcms1header="$with_lcms1_includes"
    lcms1includes="-I$with_lcms1_includes"
   else
    if test -n "$PKGCONFIG" ; then
      lcms1includes="`$PKGCONFIG --variable=includedir lcms`"
    fi
    if test -n "$lcms1includes" ; then
     lcms1header="$lcms1includes"
     lcms1includes="-I$lcms1includes"
    fi
   fi
   if test -n "$lcms1header" ; then
    lcms1header="$lcms1header/lcms.h"
   else
    lcms1header="lcms.h"
   fi
   AC_CHECK_HEADER($lcms1header,lcms1_header_found="yes",,)
#
   AC_ARG_WITH(lcms1_libraries,
  [  --with-lcms1-libraries=DIR      LCMS-1 library in nonstandard DIR])
   if test -n "$with_lcms1_libraries" ; then
    lcms1libs="-L$with_lcms1_libraries -llcms"
   else
    if test -n "$PKGCONFIG" ; then
      lcms1libs="`$PKGCONFIG --libs lcms`"
    fi
    if test -z "$lcms1libs" ; then
     lcms1libs="-llcms"
    fi
    lcms1libs="$lcms1libs"
   fi
   savedLIBS="$LIBS"
   LIBS="$lcms1libs $LIB_MATH"
   with_liblcms1="no"
#
   AC_CHECK_LIB(lcms, cmsOpenProfileFromFile, ,lcms1libs="",)
#
   if test -n "$lcms1libs" ; then
    if test "$lcms1_header_found" = "yes" ; then
     with_liblcms1="yes"
     AC_DEFINE(HAVE_LIBLCMS1, [1], [define to 1 if you have lcms version 1.x])
     if test -n "$PKGCONFIG" ; then
       have_lcms1_version="`$PKGCONFIG --modversion lcms`"
     fi
    fi
   fi
   if test "$with_liblcms1" = "no" ; then
     lcms1libs=""
     lcms1includes=""
     lcms1libs=""
   fi
   LIBS="$savedLIBS $lcms1libs"
 fi
AC_MSG_RESULT(lcms is usable:$with_liblcms1)
fi
#
AM_CONDITIONAL([with_liblcms1], [test x$with_liblcms1 = xyes])
AC_SUBST(lcms1includes)
AC_SUBST(lcms1libs)
#
# -------------------------------
# Test for USE_JPWL
# -------------------------------
AC_MSG_CHECKING(whether to build with JPWL support)
with_libjpwl="no"
jpwl_dir=""
AC_ARG_ENABLE(jpwl,
 [  --enable-jpwl[=[yes|no]]         Build with JPWL support [ [default=no] ]],
 test "$enable_jpwl" = "yes" && with_libjpwl="yes")
#
AC_MSG_RESULT($with_libjpwl)
AM_CONDITIONAL([with_libjpwl], [test x$with_libjpwl = xyes])
if test "$with_libjpwl" = "yes" ; then
 jpwl_dir="jpwl"
fi
AC_SUBST(jpwl_dir)
#
# -------------------------------
# Test for creating SHARED LIBS
# wanted result: shared xor static
# -------------------------------
with_sharedlibs="yes"
AC_MSG_CHECKING(whether to build shared libraries)
AC_ARG_ENABLE(shared,
[  --enable-shared[=[yes|no]]       Build shared libs [ [default=yes] ]],
test "$enable_shared" = "no" && with_sharedlibs="no")
#
AC_MSG_RESULT($with_sharedlibs)
AM_CONDITIONAL([with_sharedlibs], [test x$with_sharedlibs = xyes])
#
# -------------------------------
# Test for creating JP3D
# -------------------------------
with_jp3d="no"
jp3d_dir=""
AC_MSG_CHECKING(whether to build the JP3D library)
AC_ARG_ENABLE(jp3d,
 [  --enable-jp3d[=[yes|no]]         Build jp3d libs [ [default=no] ]],
 test "$enable_jp3d" = "yes" && with_jp3d="yes")
#
AC_MSG_RESULT($with_jp3d)
if test "$with_jp3d" = "yes" ; then
 jp3d_dir="jp3d"
fi
AC_SUBST(jp3d_dir)
#
AC_CHECK_PROG(with_doxygen, doxygen, [yes],[no],,)
AM_CONDITIONAL([with_doxygen], [test x$with_doxygen = xyes])
#
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libopenjpeg.pc])
AC_CONFIG_FILES([libopenjpeg/Makefile])
AC_CONFIG_FILES([jpwl/Makefile])
AC_CONFIG_FILES([codec/Makefile])
AC_CONFIG_FILES([mj2/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([jp3d/Makefile])
AC_CONFIG_FILES([jp3d/libjp3dvm/Makefile])
AC_CONFIG_FILES([jp3d/codec/Makefile])
AC_OUTPUT
#
echo "----------------------------------------------"
echo "           your configuration"
echo ""
echo "--with-png-libraries: $with_libpng"
echo "libpng header       : $png_header_found"
echo "--enable-tiff       : $with_libtiff"
echo "libtiff header      : $tiff_header_found"
echo "--enable-lcms2      : $with_liblcms2"
echo "lcms2 header        : $lcms2_header_found"
echo "lcms2 version       : $have_lcms2_version"
echo "--enable-lcms1      : $with_liblcms1"
echo "lcms1 header        : $lcms1_header_found"
echo "lcms1 version       : $have_lcms1_version"
echo "--enable-jpwl       : $with_libjpwl"
echo "--enable-jp3d       : $with_jp3d"
echo "--enable-shared     : $with_sharedlibs"
echo "----------------------------------------------"
