cmake_minimum_required(VERSION 3.24)

project(whole)

add_library(whole_lib STATIC lib.c)
add_executable(whole main.c)
target_link_libraries(whole $<LINK_LIBRARY:WHOLE_ARCHIVE,whole_lib>)
