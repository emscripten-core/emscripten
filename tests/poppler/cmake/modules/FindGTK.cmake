# - try to find GTK libraries
# Once done this will define
#
#  GTK_FOUND - system has GTK
#  GTK3_CFLAGS - the GTK CFlags
#  GTK3_LIBRARIES - Link these to use GTK
#
# Copyright 2008-2010 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

if (NOT WIN32)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(GTK3 "gtk+-3.0>=3.0" "gdk-pixbuf-2.0")

  find_package_handle_standard_args(GTK DEFAULT_MSG GTK3_LIBRARIES GTK3_CFLAGS)

endif(NOT WIN32)
