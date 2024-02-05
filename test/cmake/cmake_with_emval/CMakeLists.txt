cmake_minimum_required(VERSION 3.0)

project(cmake_with_emval)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NO_GNU_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(cmake_with_emval main.cpp)
