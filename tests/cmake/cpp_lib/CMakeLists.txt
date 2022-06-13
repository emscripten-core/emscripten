cmake_minimum_required(VERSION 3.0)

project(cpp_library)

set(CPP_LIBRARY_TYPE "SHARED" CACHE STRING "Library type to build")
set_property(CACHE CPP_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)

add_library(cpp_lib ${CPP_LIBRARY_TYPE} lib.cpp)
em_link_js_library(cpp_lib "lib.js")
