cmake_minimum_required(VERSION 2.8)

project(hello_world)

file(GLOB sourceFiles ../../hello_world.cpp)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

SET(CMAKE_EXECUTABLE_SUFFIX ".js")

if (WIN32)
	message(FATAL_ERROR "WIN32 should not be defined when cross-compiling!")
endif()

if (APPLE)
	message(FATAL_ERROR "APPLE should not be defined when cross-compiling!")
endif()

if (NOT EMSCRIPTEN)
	message(FATAL_ERROR "EMSCRIPTEN should be defined when cross-compiling!")
endif()

if (NOT CMAKE_C_SIZEOF_DATA_PTR)
	message(FATAL_ERROR "CMAKE_C_SIZEOF_DATA_PTR was not defined!")
endif()

add_executable(hello_world ${sourceFiles})
set_target_properties(hello_world PROPERTIES LINK_FLAGS "${linkFlags}")
