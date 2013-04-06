# ALUT_EVAL_STDERR(COMMAND)
# -------------------------
# Eval COMMAND, save its stderr (without lines resulting from shell tracing)
# into the file conftest.err and the exit status in the variable alut_status.
AC_DEFUN([ALUT_EVAL_STDERR],
[{ (eval $1) 2>conftest.er1
  alut_status=$?
  grep -v '^ *+' conftest.er1 >conftest.err
  rm -f conftest.er1
  (exit $alut_status); }[]dnl
])# ALUT_EVAL_STDERR
