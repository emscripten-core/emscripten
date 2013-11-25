# - Try to find the libopenjpeg library
# Once done this will define
#
#  LIBOPENJPEG_FOUND - system has libopenjpeg
#  LIBOPENJPEG_INCLUDE_DIRS - the libopenjpeg include directories
#  LIBOPENJPEG_LIBRARIES - Link these to use libopenjpeg
#  LIBOPENJPEG_INCLUDE_DIR is internal and deprecated for use

# Copyright (c) 2008, Albert Astals Cid, <aacid@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

  # in cache already
  set(LIBOPENJPEG_FOUND TRUE)

else (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

  #reset vars
  set(LIBOPENJPEG_LIBRARIES)
  set(LIBOPENJPEG_INCLUDE_DIR)

  find_path (LIBOPENJPEG_INCLUDE_DIR openjpeg.h PATH_SUFFIXES openjpeg)
  find_library(LIBOPENJPEG_LIBRARIES openjpeg)
  if(LIBOPENJPEG_INCLUDE_DIR AND LIBOPENJPEG_LIBRARIES)

  set(CMAKE_REQUIRED_INCLUDES ${LIBOPENJPEG_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${LIBOPENJPEG_LIBRARIES})

check_cxx_source_compiles("
#include <openjpeg.h>
int main()
{
  int foo = OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
  return 0;
}
" WITH_OPENJPEG_IGNORE_PCLR_CMAP_CDEF_FLAG)

  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)

    set(LIBOPENJPEG_FOUND TRUE)
  endif(LIBOPENJPEG_INCLUDE_DIR AND LIBOPENJPEG_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LibOpenJPEG DEFAULT_MSG LIBOPENJPEG_LIBRARIES LIBOPENJPEG_INCLUDE_DIR)

endif (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

set(LIBOPENJPEG_INCLUDE_DIRS ${LIBOPENJPEG_INCLUDE_DIR})
