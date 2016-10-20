cmake_minimum_required(VERSION 2.8)

project(cpp_with_emscripten_val)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NO_GNU_EXTENSIONS)
	set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(cpp_with_emscripten_val main.cpp)
