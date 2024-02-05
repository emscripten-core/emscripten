################################################################################
# Process this file with autoconf to produce a configure script.
################################################################################

AC_INIT([freealut library], [1.1.0], [openal-devel@opensource.creative.com], [freealut])
AC_CONFIG_AUX_DIR([admin/autotools])
AM_INIT_AUTOMAKE
AC_PREREQ([2.56])
AC_CONFIG_SRCDIR([AUTHORS])
AC_CONFIG_HEADERS([config.h])

# Compatibility hack for older autoconf versions
m4_ifdef([AS_HELP_STRING], [], [AC_DEFUN([AS_HELP_STRING], [AC_HELP_STRING($][@)])])

################################################################################
## libtool shared library version.
################################################################################

# Some information from the libtool info pages and the autobook regarding how to
# handle the library version info:
#
# 1. Start with version information of `0:0:0' for each libtool library.
#
# 2. Update the version information only immediately before a public release of
#    your software. More frequent updates are unnecessary, and only guarantee
#    that the current interface number gets larger faster.
#
# 3. If the library source code has changed at all since the last update, then
#    increment REVISION (`C:R:A' becomes `C:r+1:A'). This is a new revision of
#    the current interface.
#
# 4. If any interfaces have been added, removed, or changed since the last
#    update, increment CURRENT, and set REVISION to 0. This is the first
#    revision of a new interface.
#
# 5. If any interfaces have been added since the last public release, then
#    increment AGE. This release is backwards compatible with the previous
#    release.
#
# 6. If any interfaces have been removed since the last public release, then set
#    AGE to 0. This release has a new, but backwards incompatible interface.
#
# NEVER try to set the interface numbers so that they correspond to the release
# number of your package. This is an abuse that only fosters misunderstanding of
# the purpose of library versions. Furthermore, do not confuse those versions
# with the version of the specification which is implemented.

CURRENT=1
REVISION=0
AGE=1

AC_SUBST([VERSIONINFO], ["$CURRENT:$REVISION:$AGE"])

################################################################################
# Checks for programs.
################################################################################

AC_PROG_CC
AC_C_CONST
ALUT_C__ATTRIBUTE__

# Note that -fvisibility=... support implies __attribute__((visibility(...))) 
# support.
ALUT_CHECK_FLAG([-fvisibility=hidden],
                [AM_CFLAGS="$AM_CFLAGS -fvisibility=hidden"
                 AC_DEFINE([HAVE_GCC_VISIBILITY], [1],
                           [Define to 1 if we are using a GCC with symbol visibility support.])])

# test_suite/test_retrostuff tests deprecated functions, but we don't want to
# get compiler warnings because of that.
ALUT_CHECK_FLAG([-Wno-deprecated-declarations],
                [alut_wno_deprecated_declarations=yes],
                [alut_wno_deprecated_declarations=no])
AM_CONDITIONAL([WNO_DEPRECATED_DECLARATIONS],
               [test x"$alut_wno_deprecated_declarations" = xyes])

AC_EXEEXT
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

AC_DEFINE([ALUT_BUILD_LIBRARY], [1], [Define to 1 if you want to build the ALUT DLL.])

# Checks for libraries. (not perfect yet)
AC_SEARCH_LIBS([pthread_self], [pthread])
AC_SEARCH_LIBS([alGetError], [openal32 openal])

################################################################################
# Checks for header files.
################################################################################

# We could possibly need struct timespec and random(), which are not ANSI.
AC_DEFINE([_XOPEN_SOURCE], [500], [Define to 500 if Single Unix conformance is wanted, 600 for sixth revision.])

# We might need nanosleep, which is a POSIX IEEE Std 1003.1b-1993 feature.
AC_DEFINE([_POSIX_C_SOURCE], [199309], [Define to the POSIX version that should be used.])

# Without __NO_CTYPE tolower and friends are macros which introduce a GLIBC 2.3
# dependency. By defining this identifier we are currently backwards compatible
# to GLIBC 2.1.3, which is a good thing. In addition, the macros lead to code
# which triggers warnings with -Wunreachable-code.
AC_DEFINE([__NO_CTYPE], [1], [Define to 1 if tolower and friends should not be macros.])

AC_HEADER_STDC
AC_CHECK_HEADERS([AL/alc.h AL/al.h basetsd.h ctype.h math.h stdio.h time.h windows.h])

# Checks for library functions.
ALUT_CHECK_FUNC([[@%:@include <time.h>]],
                [nanosleep], [[((struct timespec*)0, (struct timespec*)0)]])

ALUT_CHECK_FUNC([[@%:@include <unistd.h>]],
                [usleep], [[(0)]])

ALUT_CHECK_FUNC([[@%:@include <windows.h>]],
                [Sleep], [[(0)]])

ALUT_CHECK_FUNC([[@%:@include <sys/types.h>
                  @%:@include <sys/stat.h>
                  @%:@include <unistd.h>]],
                [stat], [[("", (struct stat*)0)]])

ALUT_CHECK_FUNC([[@%:@include <sys/types.h>
                  @%:@include <sys/stat.h>]],
                [_stat], [[("", (struct _stat*)0)]])

AC_CHECK_LIBM
AC_SUBST([LIBM])

################################################################################
# Build time configuration.
################################################################################

AC_ARG_ENABLE([warnings],
[AS_HELP_STRING([--enable-warnings],
                [enable pedantic compiler warnings @<:@default=yes@:>@])])

if test "x$enable_warnings" != xno; then
  # Doing it in two steps gives a nicer message...
  AX_CFLAGS_WARN_ALL_ANSI([flags])
  AM_CFLAGS="$AM_CFLAGS $flags"
fi

AC_ARG_ENABLE([more-warnings],
[AS_HELP_STRING([--enable-more-warnings],
                [enable even more compiler warnings @<:@default=no@:>@])])

if test "x$enable_more_warnings" = xyes; then
  if test "x$enable_warnings" = xno; then
    AC_MSG_WARN([--enable-more-warnings ignored because of --disable-warnings])
  elif test "x$GCC" != xyes; then
    AC_MSG_WARN([--enable-more-warnings ignored because no GCC was detected])
  else
    # The long list of warning options below contains every GCC warning option
    # which is not automatically enabled with -Wall. The only exceptions to this
    # rule are:
    #
    #    -Wpadded:
    #      Perhaps good for optimizing out data layout, but not in general.
    #
    #    -Wconversion:
    #      Passing e.g. float as an argument is fine, we always have prototypes.
    #
    # Note that some older GCC versions give false positives about unreachable
    # code.
    AM_CFLAGS="$AM_CFLAGS -W -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winline -Wlong-long -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-compare -Wstrict-prototypes -Wundef -Wunreachable-code -Wwrite-strings"

    # Check for GCC 4.x-only warning options.
    ALUT_CHECK_FLAG([-Wdeclaration-after-statement],
                    [AM_CFLAGS="$AM_CFLAGS -Wdeclaration-after-statement"])
    ALUT_CHECK_FLAG([-Winvalid-pch],
                    [AM_CFLAGS="$AM_CFLAGS -Winvalid-pch"])
    ALUT_CHECK_FLAG([-Wmissing-field-initializers],
                    [AM_CFLAGS="$AM_CFLAGS -Wmissing-field-initializers"])
    # We cheat here a bit: The code generated by AC_LANG_PROGRAM triggers a
    # warning with -Wold-style-definition, so we assume that this flag is
    # supported whenever -Wvariadic-macros is.
    ALUT_CHECK_FLAG([-Wvariadic-macros],
                    [AM_CFLAGS="$AM_CFLAGS -Wvariadic-macros -Wold-style-definition"])
  fi
fi

AC_ARG_ENABLE([werror],
[AS_HELP_STRING([--enable-werror],
                [enable failure on all warnings @<:@default=no@:>@])])

if test "x$enable_werror" = xyes; then
  if test "x$enable_warnings" = xno; then
    AC_MSG_WARN([--enable-werror ignored because of --disable-warnings])
  elif test "x$GCC" != xyes; then
    AC_MSG_WARN([--enable-werror ignored because no GCC was detected])
  else
    AM_CFLAGS="$AM_CFLAGS -Werror"
  fi
fi

AC_ARG_ENABLE([efence],
[AS_HELP_STRING([--enable-efence],
                [enable Electric Fence support @<:@default=no@:>@])])

if test "x$enable_efence" = xyes; then
  alut_saved_LIBS=$LIBS
  LIBS="-lefence $LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([extern int EF_ALIGNMENT;], [EF_ALIGNMENT = 8;])],
    [:],
    [AC_MSG_WARN([--enable-efence ignored because the Electric Fence library was not found.])
     LIBS=$alut_saved_LIBS])
fi

################################################################################
# Generate output.
################################################################################

AC_SUBST([AM_CFLAGS])

# NOTE: Do not break the following line, otherwise we are into CR/LF vs. LF
# trouble! This is a buglet in autoconf IMHO, but easy to work around.
AC_CONFIG_FILES([Makefile admin/Makefile admin/pkgconfig/Makefile admin/pkgconfig/freealut-config admin/pkgconfig/freealut.pc admin/RPM/freealut.spec examples/Makefile include/Makefile src/Makefile test_suite/Makefile])

AC_OUTPUT
