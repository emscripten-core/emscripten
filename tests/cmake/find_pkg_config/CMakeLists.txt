cmake_minimum_required(VERSION 3.0)
project(find_pkg_config)

message(STATUS "PKG_CONFIG_LIBDIR: $ENV{PKG_CONFIG_LIBDIR}")

find_package(PkgConfig REQUIRED)

pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLESV2 REQUIRED glesv2)
pkg_check_modules(SDL2 REQUIRED sdl)
