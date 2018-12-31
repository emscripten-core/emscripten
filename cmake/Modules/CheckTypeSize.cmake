#.rst:
# CheckTypeSize
# -------------
#
# Check sizeof a type
#
# ::
#
#   CHECK_TYPE_SIZE(TYPE VARIABLE [BUILTIN_TYPES_ONLY]
#                                 [LANGUAGE <language>])
#
# Check if the type exists and determine its size.  On return,
# "HAVE_${VARIABLE}" holds the existence of the type, and "${VARIABLE}"
# holds one of the following:
#
# ::
#
#    <size> = type has non-zero size <size>
#    "0"    = type has arch-dependent size (see below)
#    ""     = type does not exist
#
# Both ``HAVE_${VARIABLE}`` and ``${VARIABLE}`` will be created as internal
# cache variables.
#
# Furthermore, the variable "${VARIABLE}_CODE" holds C preprocessor code
# to define the macro "${VARIABLE}" to the size of the type, or leave
# the macro undefined if the type does not exist.
#
# The variable "${VARIABLE}" may be "0" when CMAKE_OSX_ARCHITECTURES has
# multiple architectures for building OS X universal binaries.  This
# indicates that the type size varies across architectures.  In this
# case "${VARIABLE}_CODE" contains C preprocessor tests mapping from
# each architecture macro to the corresponding type size.  The list of
# architecture macros is stored in "${VARIABLE}_KEYS", and the value for
# each key is stored in "${VARIABLE}-${KEY}".
#
# If the BUILTIN_TYPES_ONLY option is not given, the macro checks for
# headers <sys/types.h>, <stdint.h>, and <stddef.h>, and saves results
# in HAVE_SYS_TYPES_H, HAVE_STDINT_H, and HAVE_STDDEF_H.  The type size
# check automatically includes the available headers, thus supporting
# checks of types defined in the headers.
#
# If LANGUAGE is set, the specified compiler will be used to perform the
# check. Acceptable values are C and CXX
#
# Despite the name of the macro you may use it to check the size of more
# complex expressions, too.  To check e.g.  for the size of a struct
# member you can do something like this:
#
# ::
#
#   check_type_size("((struct something*)0)->member" SIZEOF_MEMBER)
#
#
#
# The following variables may be set before calling this macro to modify
# the way the check is run:
#
# ::
#
#   CMAKE_REQUIRED_FLAGS = string of compile command line flags
#   CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#   CMAKE_REQUIRED_INCLUDES = list of include directories
#   CMAKE_REQUIRED_LIBRARIES = list of libraries to link
#   CMAKE_REQUIRED_QUIET = execute quietly without messages
#   CMAKE_EXTRA_INCLUDE_FILES = list of extra headers to include

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

cmake_policy(PUSH)
cmake_policy(VERSION 3.0)

get_filename_component(__check_type_size_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)

#-----------------------------------------------------------------------------
# Helper function.  DO NOT CALL DIRECTLY.
function(__check_type_size_impl type var map builtin language)
  if(NOT CMAKE_REQUIRED_QUIET)
    message(STATUS "Check size of ${type}")
  endif()

  # Include header files.
  set(headers)
  if(builtin)
    if(HAVE_SYS_TYPES_H)
      set(headers "${headers}#include <sys/types.h>\n")
    endif()
    if(HAVE_STDINT_H)
      set(headers "${headers}#include <stdint.h>\n")
    endif()
    if(HAVE_STDDEF_H)
      set(headers "${headers}#include <stddef.h>\n")
    endif()
  endif()
  foreach(h ${CMAKE_EXTRA_INCLUDE_FILES})
    set(headers "${headers}#include \"${h}\"\n")
  endforeach()

  # Perform the check.

  if("${language}" STREQUAL "C")
    set(src ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${var}.c)
  elseif("${language}" STREQUAL "CXX")
    set(src ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${var}.cpp)
  else()
    message(FATAL_ERROR "Unknown language:\n  ${language}\nSupported languages: C, CXX.\n")
  endif()
  set(bin ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${var}.bin)
  configure_file(${__check_type_size_dir}/CheckTypeSize.c.in ${src} @ONLY)
  try_run(${var}_run_result HAVE_${var} ${CMAKE_BINARY_DIR} ${src}
    COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
    LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES}
    CMAKE_FLAGS
      "-DCOMPILE_DEFINITIONS:STRING=${CMAKE_REQUIRED_FLAGS}"
      "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}"
    RUN_OUTPUT_VARIABLE ${var}_run_output
    COMPILE_OUTPUT_VARIABLE output
    )

  if(${HAVE_${var}} AND NOT "${${var}_run_result}" STREQUAL "FAILED_TO_RUN")
    set(${var} ${${var}_run_result})
    if(NOT CMAKE_REQUIRED_QUIET)
      message(STATUS "Check size of ${type} - done")
    endif()
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining size of ${type} passed with the following output:\n${output}\n\n")
    set(${var} "${${var}}" CACHE INTERNAL "CHECK_TYPE_SIZE: sizeof(${type})")
  else()
    # The check failed to compile.
    if(NOT CMAKE_REQUIRED_QUIET)
      message(STATUS "Check size of ${type} - failed")
    endif()
    file(READ ${src} content)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining size of ${type} failed with the following output:\n${output}\n${src}:\n${content}\n\n")
    set(${var} "" CACHE INTERNAL "CHECK_TYPE_SIZE: ${type} unknown")
    file(REMOVE ${map})
  endif()
endfunction()

#-----------------------------------------------------------------------------
macro(CHECK_TYPE_SIZE TYPE VARIABLE)
  # parse arguments
  unset(doing)
  foreach(arg ${ARGN})
    if("x${arg}" STREQUAL "xBUILTIN_TYPES_ONLY")
      set(_CHECK_TYPE_SIZE_${arg} 1)
      unset(doing)
    elseif("x${arg}" STREQUAL "xLANGUAGE") # change to MATCHES for more keys
      set(doing "${arg}")
      set(_CHECK_TYPE_SIZE_${doing} "")
    elseif("x${doing}" STREQUAL "xLANGUAGE")
      set(_CHECK_TYPE_SIZE_${doing} "${arg}")
      unset(doing)
    else()
      message(FATAL_ERROR "Unknown argument:\n  ${arg}\n")
    endif()
  endforeach()
  if("x${doing}" MATCHES "^x(LANGUAGE)$")
    message(FATAL_ERROR "Missing argument:\n  ${doing} arguments requires a value\n")
  endif()
  if(DEFINED _CHECK_TYPE_SIZE_LANGUAGE)
    if(NOT "x${_CHECK_TYPE_SIZE_LANGUAGE}" MATCHES "^x(C|CXX)$")
      message(FATAL_ERROR "Unknown language:\n  ${_CHECK_TYPE_SIZE_LANGUAGE}.\nSupported languages: C, CXX.\n")
    endif()
    set(_language ${_CHECK_TYPE_SIZE_LANGUAGE})
  else()
    set(_language C)
  endif()

  # Optionally check for standard headers.
  if(_CHECK_TYPE_SIZE_BUILTIN_TYPES_ONLY)
    set(_builtin 0)
  else()
    set(_builtin 1)
    if("${_language}" STREQUAL "C")
      check_include_file(sys/types.h HAVE_SYS_TYPES_H)
      check_include_file(stdint.h HAVE_STDINT_H)
      check_include_file(stddef.h HAVE_STDDEF_H)
    elseif("${_language}" STREQUAL "CXX")
      check_include_file_cxx(sys/types.h HAVE_SYS_TYPES_H)
      check_include_file_cxx(stdint.h HAVE_STDINT_H)
      check_include_file_cxx(stddef.h HAVE_STDDEF_H)
    endif()
  endif()
  unset(_CHECK_TYPE_SIZE_BUILTIN_TYPES_ONLY)
  unset(_CHECK_TYPE_SIZE_LANGUAGE)

  # Compute or load the size or size map.
  set(${VARIABLE}_KEYS)
  set(_map_file ${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${VARIABLE}.cmake)
  if(NOT DEFINED HAVE_${VARIABLE})
    __check_type_size_impl(${TYPE} ${VARIABLE} ${_map_file} ${_builtin} ${_language})
  endif()
  include(${_map_file} OPTIONAL)
  set(_map_file)
  set(_builtin)

  # Create preprocessor code.
  if(${VARIABLE}_KEYS)
    set(${VARIABLE}_CODE)
    set(_if if)
    foreach(key ${${VARIABLE}_KEYS})
      set(${VARIABLE}_CODE "${${VARIABLE}_CODE}#${_if} defined(${key})\n# define ${VARIABLE} ${${VARIABLE}-${key}}\n")
      set(_if elif)
    endforeach()
    set(${VARIABLE}_CODE "${${VARIABLE}_CODE}#else\n# error ${VARIABLE} unknown\n#endif")
    set(_if)
  elseif(${VARIABLE})
    set(${VARIABLE}_CODE "#define ${VARIABLE} ${${VARIABLE}}")
  else()
    set(${VARIABLE}_CODE "/* #undef ${VARIABLE} */")
  endif()
endmacro()

#-----------------------------------------------------------------------------
cmake_policy(POP)
