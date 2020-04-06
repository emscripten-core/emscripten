# ALUT_C__ATTRIBUTE__
# -------------------
AC_DEFUN([ALUT_C__ATTRIBUTE__],
[AC_CACHE_CHECK([whether the C compiler supports __attribute__],
  [alut_cv___attribute__],
  [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    void  foo (int bar __attribute__((unused)) ) { }
    static void baz (void) __attribute__((unused));
    static void baz (void) { }]])],
    [alut_cv___attribute__=yes],
    [alut_cv___attribute__=no])])
if test "$alut_cv___attribute__" = yes; then
  AC_DEFINE([HAVE___ATTRIBUTE__], [1],
            [Define to 1 if your C compiler supports __attribute__.])
fi
])# ALUT_C__ATTRIBUTE__
