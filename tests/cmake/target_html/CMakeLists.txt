cmake_minimum_required(VERSION 2.8)

project(hello_world_gles.html)

file(GLOB sourceFiles ../../hello_world_gles.c)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

add_executable(hello_world_gles.html ${sourceFiles})
set_target_properties(hello_world_gles.html PROPERTIES LINK_FLAGS "${linkFlags}")
