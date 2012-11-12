# On Unix platforms, we must specify the absolute path to emcc for cmake, having emcc in PATH will cause cmake to fail finding it.
# The user must set the EMSCRIPTEN variable to point to the Emscripten root folder.

file(TO_CMAKE_PATH "$ENV{EMSCRIPTEN/}" EMCC_PATH)
include(Emscripten.cmake)
