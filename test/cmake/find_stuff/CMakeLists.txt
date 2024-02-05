cmake_minimum_required(VERSION 3.0)

project(find_stuff)

# Test that we can find system headers
find_file(HAS_STRING_H_1 NAMES string.h)
if (NOT HAS_STRING_H_1)
  message(FATAL_ERROR "No string.h via find_file")
endif()
message(STATUS "find_file string.h: ${HAS_STRING_H_1}")

find_path(HAS_STRING_H_2 NAMES string.h)
if (NOT HAS_STRING_H_2)
  message(FATAL_ERROR "No string.h via find_path")
endif()
message(STATUS "find_path string.h: ${HAS_STRING_H_2}")

include(CheckIncludeFile)
check_include_file(string.h HAS_STRING_H_3)
if (NOT HAS_STRING_H_3)
  message(FATAL_ERROR "No string.h via check_include_file")
endif()

# Test that we can find system headers
include(CheckIncludeFileCXX)
check_include_file_cxx(string HAS_STRING)
if (NOT HAS_STRING)
  message(FATAL_ERROR "No string header found via find_file")
endif()
message(STATUS "find_file string: ${HAS_STRING}")

# Test that we can libraries that exist in the sysroot
find_library(HAS_ZLIB NAMES libz.a)
if (NOT HAS_ZLIB)
  message(FATAL_ERROR "libz.a not found via find_library")
endif()
message(STATUS "find_library libz.a: ${HAS_ZLIB}")

# Test that we can find host executables via find_program
find_program(HAS_CMAKE NAMES cmake)
if (NOT HAS_CMAKE)
  message(FATAL_ERROR "cmake not found via find_program")
endif()
message(STATUS "find_program cmake: ${HAS_CMAKE}")

# Check that find_path honors PATHS
# See https://github.com/emscripten-core/emscripten/issues/10078
find_path(FOO_TXT foo.txt PATHS "${CMAKE_CURRENT_LIST_DIR}" NO_CMAKE_FIND_ROOT_PATH)
if (NOT FOO_TXT)
  message(FATAL_ERROR "Can't find foo.txt in the current directory.")
endif()
message(STATUS "find_path foo: ${FOO_TXT}")
