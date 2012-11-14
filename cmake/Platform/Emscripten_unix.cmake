# On Unix platforms, we must specify the absolute path to emcc for cmake, having emcc in PATH will cause cmake to fail finding it.
# The user must set the EMSCRIPTEN variable to point to the Emscripten root folder.

# Try locating Emscripten root directory based on the location of this toolchain file.
get_filename_component(GUESS_EMSCRIPTEN_ROOT_PATH "${CMAKE_CURRENT_LIST_FILE}/../../.." ABSOLUTE)
if (EXISTS "${GUESS_EMSCRIPTEN_ROOT_PATH}/emcc")
	set(EMSCRIPTEN_ROOT_PATH "${GUESS_EMSCRIPTEN_ROOT_PATH}")
endif()

# If not found, try if the environment variable Emscripten was set.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	if ("$ENV{EMSCRIPTEN}" STREQUAL "")
		message(ERROR "Could not locate emcc and the environment variable EMSCRIPTEN has not been set! Please point it to Emscripten root directory!")
	else()
		set(EMSCRIPTEN_ROOT_PATH "$ENV{EMSCRIPTEN}")
	endif()
endif()

set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++")
set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar")
set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib")

include(${EMSCRIPTEN_ROOT_PATH}/cmake/Platform/Emscripten.cmake)
