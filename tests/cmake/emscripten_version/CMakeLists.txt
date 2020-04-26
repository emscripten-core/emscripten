project(emscripten_version)

# EMSCRIPTEN_VERSION CMake variable is introduced in Emscripten version 1.38.6,
# so test that the value is at least that new.

if ("${EMSCRIPTEN_VERSION}" VERSION_GREATER 1.38.5)
  message(STATUS "Emscripten version is at least 1.38.6")
else()
  message(FATAL_ERROR "EMSCRIPTEN_VERSION is not present, or is older than 1.38.6: '${EMSCRIPTEN_VERSION}'")
endif()

# A particular gotcha about Emscripten version testing: do not use the following
# code:

#    if (${EMSCRIPTEN_VERSION} VERSION_GREATER "1.38.5")

# because this would fail when running on an Emscripten version older than
# 1.38.6. That is, note the needed double quotes around ${EMSCRIPTEN_VERSION}.
# This is because in older Emscripten versions, the EMSCRIPTEN_VERSION CMake
# variable did not yet exist, so the above if statement would expand to an empty
# left hand side: if ( VERSION_GREATER_EQUAL 1.38.6) which would be a syntax
# error. if ("" VERSION_GREATER_EQUAL 1.38.6) is fine however.
