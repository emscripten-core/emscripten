# Locate OpenAL
# This module defines
# OPENAL_LIBRARY
# OPENAL_FOUND, if false, do not try to link to OpenAL 
# OPENAL_INCLUDE_DIR, where to find the headers

# The implementation is based on the standard FindOpenAL.cmake provided with CMake,
# but customized for targeting Emscripten only.

if (NOT OpenAL_FOUND OR NOT OPENAL_FOUND)
	SET(OpenAL_FOUND TRUE)
	SET(OPENAL_FOUND TRUE)

	# For Emscripten-compiled apps in the test suite (test_alut), this is expected...
	SET(OPENAL_INCLUDE_DIR "${EMSCRIPTEN_SYSROOT}/include")
	# ... but the stock FindOpenAL.cmake would have returned this.
	#SET(OPENAL_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include/AL")

	# Returning "-lopenal" is now considered mandatory
	SET(OPENAL_LIBRARY "-lopenal")
	SET(OPENAL_LIB "-lopenal")

	set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${OPENAL_INCLUDE_DIR}" "${OPENAL_INCLUDE_DIR}/AL")

	MARK_AS_ADVANCED(OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
endif()
