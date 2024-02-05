cmake_minimum_required(VERSION 3.0)
project(find_pkg_config)

message(STATUS "PKG_CONFIG_LIBDIR: $ENV{PKG_CONFIG_LIBDIR}")

find_package(PkgConfig REQUIRED QUIET)
if (NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "Could not find pkg-config executable!")
endif()

message(STATUS "Check that all .pc files shipped with Emscripten can be located correctly")
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLESV2 REQUIRED glesv2)
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(SDL REQUIRED sdl)
