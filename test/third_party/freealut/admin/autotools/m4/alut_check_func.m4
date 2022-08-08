# ALUT_CHECK_FUNC(PROLOGUE, FUNCTION, ARGUMENTS)
# -----------------------------------------
AC_DEFUN([ALUT_CHECK_FUNC],
[AS_VAR_PUSHDEF([alut_var], [alut_cv_func_$2])dnl
AC_CACHE_CHECK([for $2],
  alut_var,
  [AC_LINK_IFELSE([AC_LANG_PROGRAM([$1], [$2 $3;])],
                  [AS_VAR_SET(alut_var, yes)],
                  [AS_VAR_SET(alut_var, no)])])
if test AS_VAR_GET(alut_var) = yes; then
  AC_DEFINE_UNQUOTED(AS_TR_CPP([HAVE_$2]), [1],
                    [Define to 1 if you have the `$2' function.])
fi
AS_VAR_POPDEF([alut_var])dnl
])# ALUT_CHECK_FUNC
