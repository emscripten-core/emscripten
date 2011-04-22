#! /bin/sh

if [ "$USER" = "root" ]; then
	echo "*** You cannot do this as "$USER" please use a normal user account."
	exit 1
fi
if test ! -f configure.ac ; then
	echo "*** Please invoke this script from directory containing configure.ac."
	exit 1
fi

echo "running aclocal"
aclocal
rc=$?

if test $rc -eq 0; then
	echo "running libtool"
	libtoolize --force --automake --copy
	rc=$?
else
	echo "An error occured, autogen.sh stopping."
	exit $rc
fi

if test $rc -eq 0; then
	echo "libtool worked."
else
	echo "libtool not found. trying glibtool."
	glibtoolize --force --automake --copy
	rc=$?
fi

if test $rc -eq 0; then
	echo "running automake"
	automake --add-missing --copy
	rc=$?
else
	echo "An error occured, autogen.sh stopping."
	exit $rc
fi

if test $rc -eq 0; then
	echo "running autoheader"
	autoheader
	rc=$?
else
	echo "An error occured, autogen.sh stopping."
	exit $rc
fi

if test $rc -eq 0; then
	echo "running autoconf"
	autoconf
	rc=$?
else
	echo "An error occured, autogen.sh stopping."
	exit $rc
fi

echo "autogen.sh complete"
exit $rc
