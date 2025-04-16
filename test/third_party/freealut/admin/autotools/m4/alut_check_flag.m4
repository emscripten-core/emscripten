# ALUT_CHECK_FLAG(FLAG, [ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
# -----------------------------------------------------------------------
# Check to see whether the compiler for the current language supports a
# particular option.
#
# Implementation note: When given an unkown option, some GCC versions issue an
# warning on stderr only, but return an exit value of 0 nevertheless.
# Consequently we have to check stderr *and* the exit value.
AC_DEFUN([ALUT_CHECK_FLAG],
[AC_LANG_COMPILER_REQUIRE()dnl
AC_LANG_CASE([C],          [alut_compiler="$CC"  m4_pushdef([alut_Flags], [CFLAGS])],
             [C++],        [alut_compiler="$CXX" m4_pushdef([alut_Flags], [CXXFLAGS])],
             [Fortran 77], [alut_compiler="$F77" m4_pushdef([alut_Flags], [FFLAGS])])
m4_pushdef([alut_Cache], [alut_cv_[]alut_Flags[]AS_TR_SH([$1])])[]dnl
AC_CACHE_CHECK([whether $alut_compiler accepts $1], [alut_Cache],
[AC_LANG_CONFTEST([AC_LANG_PROGRAM()])
alut_save_flags="$alut_Flags"
alut_Flags="$alut_Flags $1"
alut_Cache=no
if ALUT_EVAL_STDERR([$ac_compile conftest.$ac_ext]) >/dev/null; then
  test -s conftest.err || alut_Cache=yes
fi
alut_Flags="$alut_save_flags"
rm -f conftest.err conftest.$ac_ext])
AS_IF([test $alut_Cache = yes], [$2], [$3])[]dnl
m4_popdef([alut_Cache])[]dnl
m4_popdef([alut_Flags])[]dnl
])# ALUT_CHECK_FLAG
