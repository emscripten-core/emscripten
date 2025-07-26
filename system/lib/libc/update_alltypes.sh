#!/bin/sh
# This script updates alltypes.h based on the contents of alltypes.h.in.
# In upstream this must be done by the top level Makefile.

musl_srcdir="$PWD/musl"
musl_includedir="$musl_srcdir/include"
emscripten_dir="$musl_srcdir/arch/emscripten/bits"
sed -f $musl_srcdir/tools/mkalltypes.sed \
  $emscripten_dir/alltypes.h.in \
  $musl_includedir/alltypes.h.in > $emscripten_dir/alltypes.h
