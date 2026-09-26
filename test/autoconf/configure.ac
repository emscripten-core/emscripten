AC_INIT([test], [1.0])
AC_PROG_CC
AC_CHECK_FUNCS([getcwd getwd nonexistent_func])
AC_CHECK_HEADERS([stdio.h nonexistent_header.h])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>]], [[return 0;]])],
              [run_ok=yes], [run_ok=no])
AS_IF([test "x$run_ok" = xyes],
      [AC_MSG_NOTICE([run test passed])],
      [AC_MSG_ERROR([run test failed])])
AC_OUTPUT
