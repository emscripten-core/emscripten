cmake_minimum_required(VERSION 2.8)

project(cpp_library)

SET(CPP_LIBRARY_TYPE "SHARED" CACHE STRING "Library type to build")
SET_PROPERTY(CACHE CPP_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)

add_library( cpp_lib ${CPP_LIBRARY_TYPE} lib.cpp)
em_link_js_library(cpp_lib "lib.js")
