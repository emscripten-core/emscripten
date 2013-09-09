cmake_minimum_required(VERSION 2.8)

project(hello_world_gles)

file(GLOB sourceFiles ../../hello_world_gles.c)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

SET(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(hello_world_gles ${sourceFiles})
set_target_properties(hello_world_gles PROPERTIES LINK_FLAGS "${linkFlags}")
