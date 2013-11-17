# Locate OpenAL
# This module defines
# OPENAL_LIBRARY
# OPENAL_FOUND, if false, do not try to link to OpenAL 
# OPENAL_INCLUDE_DIR, where to find the headers

# The implementation is based on the standard FindOpenAL.cmake provided with CMake,
# but customized for targeting Emscripten only.

if (NOT OPENAL_FOUND)
	SET(OPENAL_FOUND TRUE)

	# For Emscripten-compiled apps in the test suite (test_alut), this is expected...
	SET(OPENAL_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include")
	# ... but the stock FindOpenAL.cmake would have returned this.
	#SET(OPENAL_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include/AL")

	# No library to link against for OpenAL, this is picked up automatically by library_openal.js,
	# but need to report something, or CMake thinks we failed in the search.
	SET(OPENAL_LIBRARY "nul")
	SET(OPENAL_LIB "")

	set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${EMSCRIPTEN_ROOT_PATH}/system/include" "${EMSCRIPTEN_ROOT_PATH}/system/include/AL")

	MARK_AS_ADVANCED(OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
endif()
