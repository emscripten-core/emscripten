# Locate OpenGL
# This module defines:
# OPENGL_FOUND       - system has OpenGL
# OPENGL_XMESA_FOUND - system has XMESA
# OPENGL_GLU_FOUND   - system has GLU
# OPENGL_INCLUDE_DIR - the GL include directory
# OPENGL_LIBRARIES   - Link these to use OpenGL and GLU
# OPENGL_gl_LIBRARY  - Path to OpenGL Library
# OPENGL_glu_LIBRARY - Path to GLU Library

# The implementation is based on the standard FindOpenGL.cmake provided with CMake,
# but customized for targeting Emscripten only.

# These libraries are provided with Emscripten
SET(OPENGL_FOUND TRUE)
SET(OPENGL_GLU_FOUND TRUE)

# Doesn't look like this one is part of Emscripten
SET(OPENGL_XMESA_FOUND FALSE)

# This is the path where <GL/gl.h> is found
SET(OPENGL_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include")

# No library to link against for OpenGL, since Emscripten picks it up automatically from library_gl.js,
# but need to report something, or CMake thinks we failed in the search.
SET(OPENGL_LIBRARIES "nul")
SET(OPENGL_gl_LIBRARY "nul")
SET(OPENGL_glu_LIBRARY "nul")

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
)
