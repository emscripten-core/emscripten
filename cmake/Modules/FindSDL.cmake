# Locate SDL
# This module defines:
# SDL_FOUND       - system has SDL
# SDL_INCLUDE_DIR - the SDL include directory
# SDL_LIBRARIES   - Link these to use SDL

# The implementation is based on the standard FindSDL.cmake provided with CMake,
# but customized for targeting Emscripten only.

# These libraries are provided with Emscripten
SET(SDL_FOUND TRUE)

# This is the path where <SDL.h> is found
SET(SDL_INCLUDE_DIR "${EMSCRIPTEN_SYSROOT}/include/SDL")
SET(SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR})

SET(SDL_LIBRARY SDL)
SET(SDL_LIBRARIES ${SDL_LIBRARY})

set(SDL_VERSION_MAJOR 1)
set(SDL_VERSION_MINOR 3)
set(SDL_VERSION_PATCH 0)
set(SDL_VERSION ${SDL_VERSION_MAJOR}.${SDL_VERSION_MINOR}.${SDL_VERSION_PATCH})

mark_as_advanced(
  SDL_INCLUDE_DIR
  SDL_LIBRARY
)

if (NOT TARGET SDL::SDL)
  add_library(SDL::SDL INTERFACE IMPORTED)
  set_target_properties(SDL::SDL PROPERTIES
    IMPORTED_LIBNAME "${SDL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL_INCLUDE_DIR}"
  )
endif()
