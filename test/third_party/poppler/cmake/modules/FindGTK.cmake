# - try to find GTK libraries
# Once done this will define
#
#  GTK_FOUND - system has GTK
#  GTK2_CFLAGS - the GTK CFlags
#  GTK2_LIBRARIES - Link these to use GTK
#
# Copyright 2008-2010 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

if (NOT WIN32)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(GTK2 "gtk+-2.0>=2.14" "gdk-pixbuf-2.0" "gthread-2.0" "gio-2.0")

  find_package_handle_standard_args(GTK DEFAULT_MSG GTK2_LIBRARIES GTK2_CFLAGS)

endif(NOT WIN32)
