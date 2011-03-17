dnl -*- mode: autoconf -*-
dnl Copyright 2009 Johan Dahlin
dnl
dnl This file is free software; the author(s) gives unlimited
dnl permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl

# serial 1

m4_define([_GOBJECT_INTROSPECTION_CHECK_INTERNAL],
[
    AC_BEFORE([AC_PROG_LIBTOOL],[$0])dnl setup libtool first
    AC_BEFORE([AM_PROG_LIBTOOL],[$0])dnl setup libtool first
    AC_BEFORE([LT_INIT],[$0])dnl setup libtool first

    dnl enable/disable introspection
    m4_if([$2], [require],
    [dnl
        enable_introspection=yes
    ],[dnl
        AC_ARG_ENABLE(introspection,
                  AS_HELP_STRING([--enable-introspection[=@<:@no/auto/yes@:>@]],
                                 [Enable introspection for this build]),, 
                                 [enable_introspection=auto])
    ])dnl

    AC_MSG_CHECKING([for gobject-introspection])

    dnl presence/version checking
    AS_CASE([$enable_introspection],
    [no], [dnl
        found_introspection="no (disabled, use --enable-introspection to enable)"
    ],dnl
    [yes],[dnl
        PKG_CHECK_EXISTS([gobject-introspection-1.0],,
                         AC_MSG_ERROR([gobject-introspection-1.0 is not installed]))
        PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $1],
                         found_introspection=yes,
                         AC_MSG_ERROR([You need to have gobject-introspection >= $1 installed to build AC_PACKAGE_NAME]))
    ],dnl
    [auto],[dnl
        PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $1], found_introspection=yes, found_introspection=no)
    ],dnl
    [dnl	
        AC_MSG_ERROR([invalid argument passed to --enable-introspection, should be one of @<:@no/auto/yes@:>@])
    ])dnl

    AC_MSG_RESULT([$found_introspection])

    INTROSPECTION_SCANNER=
    INTROSPECTION_COMPILER=
    INTROSPECTION_GENERATE=
    INTROSPECTION_GIRDIR=
    INTROSPECTION_TYPELIBDIR=
    if test "x$found_introspection" = "xyes"; then
       INTROSPECTION_SCANNER=`$PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0`
       INTROSPECTION_COMPILER=`$PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0`
       INTROSPECTION_GENERATE=`$PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0`
       INTROSPECTION_GIRDIR=`$PKG_CONFIG --variable=girdir gobject-introspection-1.0`
       INTROSPECTION_TYPELIBDIR="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
       INTROSPECTION_CFLAGS=`$PKG_CONFIG --cflags gobject-introspection-1.0`
       INTROSPECTION_LIBS=`$PKG_CONFIG --libs gobject-introspection-1.0`
       INTROSPECTION_MAKEFILE=`$PKG_CONFIG --variable=datadir gobject-introspection-1.0`/gobject-introspection-1.0/Makefile.introspection
    fi
    AC_SUBST(INTROSPECTION_SCANNER)
    AC_SUBST(INTROSPECTION_COMPILER)
    AC_SUBST(INTROSPECTION_GENERATE)
    AC_SUBST(INTROSPECTION_GIRDIR)
    AC_SUBST(INTROSPECTION_TYPELIBDIR)
    AC_SUBST(INTROSPECTION_CFLAGS)
    AC_SUBST(INTROSPECTION_LIBS)
    AC_SUBST(INTROSPECTION_MAKEFILE)

    AM_CONDITIONAL(HAVE_INTROSPECTION, test "x$found_introspection" = "xyes")
])


dnl Usage:
dnl   GOBJECT_INTROSPECTION_CHECK([minimum-g-i-version])

AC_DEFUN([GOBJECT_INTROSPECTION_CHECK],
[
  _GOBJECT_INTROSPECTION_CHECK_INTERNAL([$1])
])

dnl Usage:
dnl   GOBJECT_INTROSPECTION_REQUIRE([minimum-g-i-version])


AC_DEFUN([GOBJECT_INTROSPECTION_REQUIRE],
[
  _GOBJECT_INTROSPECTION_CHECK_INTERNAL([$1], [require])
])
