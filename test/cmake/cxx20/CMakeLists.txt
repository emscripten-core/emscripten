cmake_minimum_required(VERSION 3.16...3.28)

project(cxx20test)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL")

add_executable(cxx20test main.cpp)
