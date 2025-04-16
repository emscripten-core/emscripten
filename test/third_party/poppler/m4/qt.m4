# POPPLER_FIND_QT(VARIABLE-PREFIX, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# Check whether the Qt3 libraries are available.
#
# --------------------------------------------------------------
AC_DEFUN([POPPLER_FIND_QT],
[
PKG_CHECK_MODULES([libqt3], [qt-mt], [
	ifelse([$2], , :, [$2])
], [
	ifelse([$3], ,
		[AC_MSG_FAILURE([Qt development libraries not found])],
		[$3])
])

])
