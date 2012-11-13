# On Unix platforms, we must specify the absolute path to emcc for cmake, having emcc in PATH will cause cmake to fail finding it.
# The user must set the EMSCRIPTEN variable to point to the Emscripten root folder.

if ("$ENV{EMSCRIPTEN}" STREQUAL "")
	message(ERROR "Environment variable EMSCRIPTEN has not been set! Please point it to Emscripten root directory!")
endif()

set(CMAKE_C_COMPILER "$ENV{EMSCRIPTEN}/emcc")
set(CMAKE_CXX_COMPILER "$ENV{EMSCRIPTEN}/em++")
set(CMAKE_AR "$ENV{EMSCRIPTEN}/emar")
set(CMAKE_RANLIB "$ENV{EMSCRIPTEN}/emranlib")

include($ENV{EMSCRIPTEN}/cmake/Platform/Emscripten.cmake)
