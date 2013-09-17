# This file is a 'toolchain description file' for CMake.
# It teaches CMake about the Emscripten compiler, so that CMake can generate makefiles
# from CMakeLists.txt that invoke emcc.

# To use this toolchain file with CMake, invoke CMake with the following command line parameters
# cmake -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Platform/Emscripten.cmake
#       -DCMAKE_BUILD_TYPE=<Debug|RelWithDebInfo|Release|MinSizeRel>
#       -G "Unix Makefiles" (Linux and OSX)
#       -G "MinGW Makefiles" (Windows)
#       <path/to/CMakeLists.txt> # Note, pass in here ONLY the path to the file, not the filename 'CMakeLists.txt' itself.

# After that, build the generated Makefile with the command 'make'. On Windows, you may download and use 'mingw32-make' instead.

# The following variable describes the target OS we are building to.
# Ideally, this could be 'Emscripten', but as Emscripten mimics the Linux platform, setting this to Linux will allow more of existing software to build.
# Be sure to run Emscripten test_openjpeg if planning to change this.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CROSSCOMPILING TRUE)

# Do a no-op access on the CMAKE_TOOLCHAIN_FILE variable so that CMake will not issue a warning on it being unused.
if (CMAKE_TOOLCHAIN_FILE)
endif()

# Locate where the Emscripten compiler resides in relative to this toolchain file.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	get_filename_component(GUESS_EMSCRIPTEN_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
	if (EXISTS "${GUESS_EMSCRIPTEN_ROOT_PATH}/emranlib")
		set(EMSCRIPTEN_ROOT_PATH "${GUESS_EMSCRIPTEN_ROOT_PATH}")
	endif()
endif()

# If not found by above search, locate using the EMSCRIPTEN environment variable.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	set(EMSCRIPTEN_ROOT_PATH "$ENV{EMSCRIPTEN}")
endif()

# Abort if not found. 
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	message(FATAL_ERROR "Could not locate the Emscripten compiler toolchain directory! Either set the EMSCRIPTEN environment variable, or pass -DEMSCRIPTEN_ROOT_PATH=xxx to CMake to explicitly specify the location of the compiler!")
endif()

# Normalize, convert Windows backslashes to forward slashes or CMake will crash.
get_filename_component(EMSCRIPTEN_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}" ABSOLUTE)

if ("${CMAKE_MODULE_PATH}" STREQUAL "")
	set(CMAKE_MODULE_PATH "${EMSCRIPTEN_ROOT_PATH}/cmake")
endif()

set(CMAKE_FIND_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}/cmake")

if (CMAKE_HOST_WIN32)
	set(EMCC_SUFFIX ".bat")
else()
	set(EMCC_SUFFIX "")
endif()

# Specify the compilers to use for C and C++
if ("${CMAKE_C_COMPILER}" STREQUAL "")
	set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}")
endif()
if ("${CMAKE_CXX_COMPILER}" STREQUAL "")
	set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++${EMCC_SUFFIX}")
endif()

if ("${CMAKE_AR}" STREQUAL "")
	set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}")
endif()

if ("${CMAKE_RANLIB}" STREQUAL "")
	set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}")
endif()

# Don't do compiler autodetection, since we are cross-compiling.
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" Clang)
CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" Clang)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# We would prefer to specify a standard set of Clang+Emscripten-friendly common convention for suffix files, especially for CMake executable files,
# but if these are adjusted, ${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake will fail, since it depends on being able to compile output files with predefined names.
#SET(CMAKE_LINK_LIBRARY_SUFFIX "")
#SET(CMAKE_STATIC_LIBRARY_PREFIX "")
#SET(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
#SET(CMAKE_SHARED_LIBRARY_PREFIX "")
#SET(CMAKE_SHARED_LIBRARY_SUFFIX ".bc")
#IF (NOT CMAKE_EXECUTABLE_SUFFIX)
#	SET(CMAKE_EXECUTABLE_SUFFIX ".js")
#endif()
#SET(CMAKE_FIND_LIBRARY_PREFIXES "")
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".bc")

SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

# Specify the program to use when building static libraries. Force Emscripten-related command line options to clang.
set(CMAKE_CXX_ARCHIVE_CREATE "${CMAKE_CXX_COMPILER} ${CMAKE_START_TEMP_FILE} -o <TARGET> -emit-llvm <LINK_FLAGS> <OBJECTS>${CMAKE_END_TEMP_FILE}")
set(CMAKE_C_ARCHIVE_CREATE "${CMAKE_C_COMPILER} ${CMAKE_START_TEMP_FILE} -o <TARGET> -emit-llvm <LINK_FLAGS> <OBJECTS>${CMAKE_END_TEMP_FILE}")

# Set a global EMSCRIPTEN variable that can be used in client CMakeLists.txt to detect when building using Emscripten.
# There seems to be some kind of bug with CMake, so you might need to define this manually on the command line with "-DEMSCRIPTEN=1".
set(EMSCRIPTEN 1)

# We are cross-compiling, so unset the common CMake variables that represent the target platform. Leave UNIX define enabled, since Emscripten
# mimics a Linux environment.
SET(WIN32)
SET(APPLE)

set(CMAKE_C_SIZEOF_DATA_PTR 4)
set(CMAKE_CXX_SIZEOF_DATA_PTR 4)

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELEASE")
set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_MINSIZEREL")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELWITHDEBINFO")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELEASE")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_MINSIZEREL")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELWITHDEBINFO")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELEASE")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-O2" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELEASE")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "-O2" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELEASE")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "-O2" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO")

function(em_validate_asmjs_after_build target)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo Validating build output for asm.js... COMMAND "python" ARGS "${EMSCRIPTEN_ROOT_PATH}/tools/validate_asmjs.py" "$<TARGET_FILE:${target}>")
endfunction()
