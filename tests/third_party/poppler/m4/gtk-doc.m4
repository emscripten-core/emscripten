dnl -*- mode: autoconf -*-

# serial 1

dnl Usage:
dnl   GTK_DOC_CHECK([minimum-gtk-doc-version])
AC_DEFUN([GTK_DOC_CHECK],
[
  AC_BEFORE([AC_PROG_LIBTOOL],[$0])dnl setup libtool first
  AC_BEFORE([AM_PROG_LIBTOOL],[$0])dnl setup libtool first

  dnl check for tools we added during development
  AC_PATH_PROG([GTKDOC_CHECK],[gtkdoc-check])
  AC_PATH_PROGS([GTKDOC_REBASE],[gtkdoc-rebase],[true])
  AC_PATH_PROG([GTKDOC_MKPDF],[gtkdoc-mkpdf])

  dnl for overriding the documentation installation directory
  AC_ARG_WITH([html-dir],
    AS_HELP_STRING([--with-html-dir=PATH], [path to installed docs]),,
    [with_html_dir='${datadir}/gtk-doc/html'])
  HTML_DIR="$with_html_dir"
  AC_SUBST([HTML_DIR])

  dnl enable/disable documentation building
  AC_ARG_ENABLE([gtk-doc],
    AS_HELP_STRING([--enable-gtk-doc],
                   [use gtk-doc to build documentation [[default=no]]]),,
    [enable_gtk_doc=no])

  if test x$enable_gtk_doc = xyes; then
    ifelse([$1],[],
      [PKG_CHECK_EXISTS([gtk-doc],,
                        AC_MSG_ERROR([gtk-doc not installed and --enable-gtk-doc requested]))],
      [PKG_CHECK_EXISTS([gtk-doc >= $1],,
                        AC_MSG_ERROR([You need to have gtk-doc >= $1 installed to build $PACKAGE_NAME]))])
  fi

  AC_MSG_CHECKING([whether to build gtk-doc documentation])
  AC_MSG_RESULT($enable_gtk_doc)

  dnl enable/disable output formats
  AC_ARG_ENABLE([gtk-doc-html],
    AS_HELP_STRING([--enable-gtk-doc-html],
                   [build documentation in html format [[default=yes]]]),,
    [enable_gtk_doc_html=yes])
    AC_ARG_ENABLE([gtk-doc-pdf],
      AS_HELP_STRING([--enable-gtk-doc-pdf],
                     [build documentation in pdf format [[default=no]]]),,
      [enable_gtk_doc_pdf=no])

  if test -z "$GTKDOC_MKPDF"; then
    enable_gtk_doc_pdf=no
  fi


  AM_CONDITIONAL([ENABLE_GTK_DOC], [test x$enable_gtk_doc = xyes])
  AM_CONDITIONAL([GTK_DOC_BUILD_HTML], [test x$enable_gtk_doc_html = xyes])
  AM_CONDITIONAL([GTK_DOC_BUILD_PDF], [test x$enable_gtk_doc_pdf = xyes])
  AM_CONDITIONAL([GTK_DOC_USE_LIBTOOL], [test -n "$LIBTOOL"])
  AM_CONDITIONAL([GTK_DOC_USE_REBASE], [test -n "$GTKDOC_REBASE"])
])
