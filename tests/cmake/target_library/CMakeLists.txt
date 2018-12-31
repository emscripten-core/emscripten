cmake_minimum_required(VERSION 2.8)

project(test_cmake)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cpp_lib ${CMAKE_CURRENT_BINARY_DIR}/cpp_lib)

option(BUILD_SHARED_LIBS "Build with shared libraries." OFF) 

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(linkFlags "-g4")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
	SET(linkFlags "-O2")
endif()

set(MAX_SRC_FILE_INDEX 30)
SET(TEST_SRC_FILE_BASE_NAME "this_is_a_test_src_file_with_a_quite_lengthy_name_to_simulate_very_long_command_line_length_problems_on_windows_")


foreach(i RANGE ${MAX_SRC_FILE_INDEX})
	set (TEST_FUNCTION_NAME "FooBar_${i}")
	configure_file("srcfile.cmake" "${TEST_SRC_FILE_BASE_NAME}${i}.c")
	configure_file("srcfile.cmake" "${TEST_SRC_FILE_BASE_NAME}${i}.cpp")
	list(APPEND TEST_SOURCES "${TEST_SRC_FILE_BASE_NAME}${i}.c" "${TEST_SRC_FILE_BASE_NAME}${i}.cpp")
endforeach()

add_library(test_cmake ${TEST_SOURCES})
target_link_libraries(test_cmake cpp_lib)

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

# GOTCHA: If your project has custom link flags, these must be set *before* calling any of the em_link_xxx functions!
set_target_properties(test_cmake PROPERTIES LINK_FLAGS "${linkFlags}")
