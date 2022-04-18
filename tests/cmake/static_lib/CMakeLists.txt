cmake_minimum_required(VERSION 3.0)

project(static_library)

# Test mode that checks that it's possible to override the file suffix
# (independent of whether ar archive or LLVM bitcode file is created)
# Note that "-DCMAKE_STATIC_LIBRARY_SUFFIX=foo" cannot be passed to CMake from
# command line to override the suffix, but changing it needs
# to happen in the CMakeLists.txt file itself. (this is likely due to CMake
# internal script evaluation order(?))
if (SET_CUSTOM_SUFFIX_IN_PROJECT)
  set(CMAKE_STATIC_LIBRARY_SUFFIX ".somecustomsuffix")

  # Also check that we can control the prefix. Setting this to an empty string
  # can be useful to get rid of the "lib" prefix that gets added by default.
  set(CMAKE_STATIC_LIBRARY_PREFIX "myprefix_")
endif()

add_library(static_lib STATIC lib.cpp)
