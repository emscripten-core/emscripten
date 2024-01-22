#!/bin/sh
# This script updates alltypes.h based on the contents of alltypes.h.in.
# In upstream this must be done by the top level Makefile.

musl_srcdir="$PWD/../../.."
musl_includedir="$musl_srcdir/include"
sed -f $musl_srcdir/tools/mkalltypes.sed ./alltypes.h.in $musl_includedir/alltypes.h.in > alltypes.h
