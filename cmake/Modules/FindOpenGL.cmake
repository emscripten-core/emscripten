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
SET(OPENGL_INCLUDE_DIR "${EMSCRIPTEN_SYSROOT}/include")

SET(OPENGL_gl_LIBRARY "GL")
SET(OPENGL_glu_LIBRARY "GLU")
SET(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
)
