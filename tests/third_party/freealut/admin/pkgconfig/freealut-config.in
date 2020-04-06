#!/bin/sh

prefix="@prefix@"
exec_prefix="@exec_prefix@"
bindir="@bindir@"
includedir="@includedir@"
libdir="@libdir@"
PACKAGE_VERSION="@PACKAGE_VERSION@"

freealut_dynamic_ldflags="-lalut"
# TODO: configure should provide the following...
freealut_static_ldflags="-lalut"

prefix_set=no
echo_prefix=no
exec_prefix_set=no
echo_exec_prefix=no
bindir_set=no
echo_bindir=no
includedir_set=no
echo_includedir=no
libdir_set=no
echo_libdir=no
echo_version=no
echo_cflags=no
static_libs=no
echo_libs=no

usage()
{
  cat <<EOF 1>&2
Usage: freealut-config [OPTIONS]
Options:
  --prefix[=DIR]       print/set default prefix
  --exec-prefix[=DIR]  print/set prefix for machine-specific files
  --bindir[=DIR]       print/set prefix for executable programs
  --includedir[=DIR]   print/set prefix for include files
  --libdir[=DIR]       print/set prefix for libraries
  --version            print freealut package version
  --cflags             print flags for C compiler
  --static             specify that static linker flags are wanted
  --libs               print flags for linker
EOF
  exit 1
}

if test $# -eq 0; then
  usage
fi

while test $# -gt 0; do
  case "$1" in
  -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) optarg="" ;;
  esac

  case $1 in
    --prefix=*)
      prefix="${optarg}"
      prefix_set="yes"
      ;;
    --prefix)
      echo_prefix="yes"
      ;;
    --exec-prefix=*)
      exec_prefix="${optarg}"
      exec_prefix_set="yes"
      ;;
    --exec-prefix)
      echo_exec_prefix="yes"
      ;;
    --bindir=*)
      bindir="${optarg}"
      bindir_set="yes"
      ;;
    --bindir)
      echo_bindir="yes"
      ;;
    --includedir=*)
      includedir="${optarg}"
      includedir_set="yes"
      ;;
    --includedir)
      echo_includedir="yes"
      ;;
    --libdir=*)
      libdir="${optarg}"
      libdir_set="yes"
      ;;
    --libdir)
      echo_libdir="yes"
      ;;
    --version)
      echo_version="yes"
      ;;
    --cflags)
      echo_cflags="yes"
      ;;
    --static)
      static_libs="yes"
      ;;
    --libs)
      echo_libs="yes"
      ;;
    *)
      usage
      ;;
  esac
  shift
done

if test "${echo_prefix}" = "yes"; then
  echo "${prefix}"
fi

if test "${exec_prefix_set}" = "no" && test "${prefix_set}" = "yes"; then
  exec_prefix="$prefix"
  exec_prefix_set="yes"
fi

if test "$echo_exec_prefix" = "yes"; then
  echo "${exec_prefix}"
fi

if test "${bindir_set}" = "no" && test "${exec_prefix_set}" = "yes"; then
  bindir="${exec_prefix}/bin"
fi

if test "$echo_bindir" = "yes"; then
  echo "${bindir}"
fi

if test "${includedir_set}" = "no" && test "${prefix_set}" = "yes"; then
  includedir="${prefix}/include"
fi

if test "$echo_includedir" = "yes"; then
  echo "${includedir}"
fi

if test "${libdir_set}" = "no" && test "${exec_prefix_set}" = "yes"; then
  libdir="${exec_prefix}/lib"
fi

if test "$echo_libdir" = "yes"; then
  echo "${libdir}"
fi

if test "$echo_version" = "yes"; then
  echo "${PACKAGE_VERSION}"
fi

if test "$echo_cflags" = "yes"; then
  if test "${includedir}" = "/usr/include" ; then
    incpath=""
  else
    incpath="-I${includedir}"
  fi
  echo "${incpath}"
fi

if test "$echo_libs" = "yes"; then
  if test "${libdir}" = "/usr/lib" ; then
    libpathflag=""
  else
    libpathflag="-L${libdir}"
  fi
  if test "${static_libs}" = "yes"; then
     ldflags="${freealut_static_ldflags}"
  else
     ldflags="${freealut_dynamic_ldflags}"
  fi
  echo "${libpathflag} ${ldflags}"
fi
