cmake_minimum_required(VERSION 2.8)

project(test_cmake)

file(GLOB sourceFiles main.cpp)

file(GLOB preJsFiles pre*.js)
file(GLOB postJsFiles post*.js)
file(GLOB libraryJsFiles jslibrary*.js)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "-g4")
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

add_executable(test_cmake ${sourceFiles})

# GOTCHA: If your project has custom link flags, these must be set *before* calling any of the em_link_xxx functions!
set_target_properties(test_cmake PROPERTIES LINK_FLAGS "${linkFlags}")

message(STATUS "js libs '${libraryJsFiles}'")
# To link .js files using the --js-library flag, use the following helper function.
em_link_js_library(test_cmake ${libraryJsFiles})

# To link .js files using the --pre-js flag, use the following helper function.
em_link_pre_js(test_cmake ${preJsFiles})

# To link .js files using the --post-js flag, use the following helper function.
em_link_post_js(test_cmake ${postJsFiles})
