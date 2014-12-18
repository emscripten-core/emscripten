cmake_minimum_required(VERSION 2.8)

project(asmjs_optimizer)

file(GLOB sourceFiles *.cpp)
file(GLOB headerFiles *.h)

set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} ${cFlags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cFlags}")

add_executable(optimizer ${sourceFiles} ${headerFiles})
