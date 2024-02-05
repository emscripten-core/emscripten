# - Find LCMS
# Find the LCMS includes and library
# This module defines
#  LCMS_INCLUDE_DIR, where to find lcms.h
#  LCMS_LIBRARIES, the libraries needed to use LCMS.
#  LCMS_VERSION, The value of LCMS_VERSION defined in lcms.h
#  LCMS_FOUND, If false, do not try to use LCMS.


# Copyright (c) 2008, Adrian Page, <adrian@pagenet.plus.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if(NOT WIN32)
   find_package(PkgConfig)
   pkg_check_modules(PC_LCMS lcms)
   set(LCMS_DEFINITIONS ${PC_LCMS_CFLAGS_OTHER})
endif(NOT WIN32)

find_path(LCMS_INCLUDE_DIR lcms.h
   PATHS
   ${PC_LCMS_INCLUDEDIR}
   ${PC_LCMS_INCLUDE_DIRS}
   PATH_SUFFIXES lcms liblcms1
)

find_library(LCMS_LIBRARIES NAMES lcms liblcms lcms-1 liblcms-1
   PATHS     
   ${PC_LCMS_LIBDIR}
   ${PC_LCMS_LIBRARY_DIRS}
   PATH_SUFFIXES lcms
)

if(LCMS_INCLUDE_DIR AND LCMS_LIBRARIES)
   set(LCMS_FOUND TRUE)
else(LCMS_INCLUDE_DIR AND LCMS_LIBRARIES)
   set(LCMS_FOUND FALSE)
endif(LCMS_INCLUDE_DIR AND LCMS_LIBRARIES)

if(LCMS_FOUND)
   set(FIND_LCMS_VERSION_SOURCE
      "#include <lcms.h>\n int main()\n {\n return LCMS_VERSION;\n }\n")
   set(FIND_LCMS_VERSION_SOURCE_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/FindLCMS.cxx)
   file(WRITE "${FIND_LCMS_VERSION_SOURCE_FILE}" "${FIND_LCMS_VERSION_SOURCE}")

   set(FIND_LCMS_VERSION_ADD_INCLUDES 
      "-DINCLUDE_DIRECTORIES:STRING=${LCMS_INCLUDE_DIR}")

   try_run(LCMS_VERSION LCMS_COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${FIND_LCMS_VERSION_SOURCE_FILE}
      CMAKE_FLAGS "${FIND_LCMS_VERSION_ADD_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)

   if(LCMS_COMPILE_RESULT AND NOT LCMS_VERSION STREQUAL FAILED_TO_RUN)
      if(NOT LCMS_FIND_QUIETLY)
         string(SUBSTRING ${LCMS_VERSION} 0 1 LCMS_MAJOR_VERSION)
         string(SUBSTRING ${LCMS_VERSION} 1 2 LCMS_MINOR_VERSION)
         message(STATUS "Found lcms version ${LCMS_MAJOR_VERSION}.${LCMS_MINOR_VERSION}, ${LCMS_LIBRARIES}")
      endif(NOT LCMS_FIND_QUIETLY)
   else(LCMS_COMPILE_RESULT AND NOT LCMS_VERSION STREQUAL FAILED_TO_RUN)
      if(NOT LCMS_FIND_QUIETLY)
         message(STATUS "Found lcms but failed to find version ${LCMS_LIBRARIES}")
         file(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
            "Find lcms version failed with the following output:\n${OUTPUT}\nFindLCMS.cxx:\n${FIND_LCMS_VERSION_SOURCE}\n\n")
      endif(NOT LCMS_FIND_QUIETLY)
      set(LCMS_VERSION NOTFOUND)
   endif(LCMS_COMPILE_RESULT AND NOT LCMS_VERSION STREQUAL FAILED_TO_RUN)
else(LCMS_FOUND)
   if(NOT LCMS_FIND_QUIETLY)
      if(LCMS_FIND_REQUIRED)
         message(FATAL_ERROR "Required package lcms NOT found")
      else(LCMS_FIND_REQUIRED)
         message(STATUS "lcms NOT found")
      endif(LCMS_FIND_REQUIRED)
   endif(NOT LCMS_FIND_QUIETLY)
endif(LCMS_FOUND)

mark_as_advanced(LCMS_INCLUDE_DIR LCMS_LIBRARIES LCMS_VERSION)

