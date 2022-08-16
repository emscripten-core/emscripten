# - try to find GDK libraries
# Once done this will define
#
#  GDK_FOUND - system has GDK
#  GDK2_CFLAGS - the GDK CFlags
#  GDK2_LIBRARIES - Link these to use GDK
#
# Copyright 2008-2010 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

if (NOT WIN32)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(GDK2 "gdk-2.0")

  find_package_handle_standard_args(GDK DEFAULT_MSG GDK2_LIBRARIES GDK2_CFLAGS)

endif(NOT WIN32)
