cmake_minimum_required(VERSION 2.8)

project(hello_world_gles)

file(GLOB sourceFiles ../../hello_world_gles.c)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

if (NOT CMAKE_AR OR NOT EXISTS "${CMAKE_AR}")
	message(FATAL_ERROR "CMAKE_AR='${CMAKE_AR}' does not exist for Emscripten toolchain!")
endif()

if (NOT CMAKE_RANLIB OR NOT EXISTS "${CMAKE_RANLIB}")
	message(FATAL_ERROR "CMAKE_RANLIB='${CMAKE_RANLIB}' does not exist for Emscripten toolchain!")
endif()

if (NOT CMAKE_C_COMPILER OR NOT EXISTS "${CMAKE_C_COMPILER}")
	message(FATAL_ERROR "CMAKE_C_COMPILER='${CMAKE_C_COMPILER}' does not exist for Emscripten toolchain!")
endif()

if (NOT CMAKE_CXX_COMPILER OR NOT EXISTS "${CMAKE_CXX_COMPILER}")
	message(FATAL_ERROR "CMAKE_CXX_COMPILER='${CMAKE_CXX_COMPILER}' does not exist for Emscripten toolchain!")
endif()

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

SET(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(hello_world_gles ${sourceFiles})
set_target_properties(hello_world_gles PROPERTIES LINK_FLAGS "${linkFlags}")

# Validating asm.js requires SpiderMonkey JS VM - detect its presence via the SPIDERMONKEY environment variable.
if (DEFINED ENV{SPIDERMONKEY} AND CMAKE_BUILD_TYPE STREQUAL Release)
	em_validate_asmjs_after_build(hello_world_gles)
endif()
