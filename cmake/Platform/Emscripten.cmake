# This file is a 'toolchain description file' for CMake.
# It teaches CMake about the Emscripten compiler, so that CMake can generate Unix Makefiles
# from CMakeLists.txt that invoke emcc.

# To use this toolchain file with CMake, invoke CMake with the following command line parameters
# cmake -DEMSCRIPTEN=1 
#       -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Platform/Emscripten.cmake
#       -DCMAKE_MODULE_PATH=<EmscriptenRoot>/cmake
#       -DCMAKE_BUILD_TYPE=<Debug|RelWithDebInfo|Release|MinSizeRel>
#       -G "Unix Makefiles"
#       <path/to/CMakeLists.txt> # Note, pass in here ONLY the path to the file, not the filename 'CMakeLists.txt' itself.

# After that, build the generated Makefile with the command 'make'. On Windows, you may download and use 'mingw32-make' instead.

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Emscripten)
SET(CMAKE_SYSTEM_VERSION 1)

if ("$ENV{EMCC_BIN}" STREQUAL "")
	message(ERROR "Environment variable EMCC_BIN has not been set! Please point it to Emscripten root directory!")
endif()

#message(STATUS "CMake is using Emscripten toolchain file, Emscripten root path '$ENV{EMCC_BIN}'.")

SET(CMAKE_FIND_ROOT_PATH $ENV{EMCC_BIN})

FILE(TO_CMAKE_PATH "$ENV{EMCC_BIN}" EMCC_PATH)

# Specify the compilers to use for C and C++
SET(CMAKE_C_COMPILER ${EMCC_PATH}/emcc)
SET(CMAKE_CXX_COMPILER ${EMCC_PATH}/em++)
SET(CMAKE_AR ${EMCC_PATH}/emar)
SET(CMAKE_RANLIB ${EMCC_PATH}/emranlib)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Specify the program to use when building static libraries. Force Emscripten-related command line options to clang.
SET(CMAKE_CXX_ARCHIVE_CREATE "${CMAKE_CXX_COMPILER} -o <TARGET> -emit-llvm <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_CREATE "${CMAKE_C_COMPILER} -o <TARGET> -emit-llvm <LINK_FLAGS> <OBJECTS>")

# Set a global EMSCRIPTEN variable that can be used in client CMakeLists.txt to detect when building using Emscripten.
# There seems to be some kind of bug with CMake, so you might need to define this manually on the command line with "-DEMSCRIPTEN=1".
SET(EMSCRIPTEN 1)
