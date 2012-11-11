cmake_minimum_required(VERSION 2.8)

project(hello_world.js)

file(GLOB sourceFiles ../../hello_world.cpp)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

add_executable(hello_world.js ${sourceFiles})
set_target_properties(hello_world.js PROPERTIES LINK_FLAGS "${linkFlags}")
