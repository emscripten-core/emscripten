# Locate OpenGL
# This module defines:
# OPENGL_FOUND       - system has OpenGL
# OPENGL_XMESA_FOUND - system has XMESA
# OPENGL_GLU_FOUND   - system has GLU
# OPENGL_GLU_INCLUDE_DIR -the GLU include directory (<GL/glu.h>)
# OPENGL_INCLUDE_DIR - the GL include directory
# OPENGL_LIBRARIES   - Link these to use OpenGL and GLU
# OPENGL_gl_LIBRARY  - Path to OpenGL Library
# OPENGL_glu_LIBRARY - Path to GLU Library

# This module defines IMPORTED targets:
# OpenGL::GL         - system has OpenGL
# OpenGL::OpenGL     - alias for OpenGL::GL
# OpenGL::GLU        - system has GLU

# The implementation is based on the standard FindOpenGL.cmake provided with CMake,
# but customized for targeting Emscripten only.

# These libraries are provided with Emscripten
SET(OPENGL_FOUND TRUE)
SET(OPENGL_GLU_FOUND TRUE)

# Doesn't look like this one is part of Emscripten
SET(OPENGL_XMESA_FOUND FALSE)

# This is the path where <GL/gl.h> is found
SET(OPENGL_INCLUDE_DIR "${EMSCRIPTEN_SYSROOT}/include")

# This is the path where <GL/glu.h> is found
SET(OPENGL_GLU_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}")

SET(OPENGL_gl_LIBRARY "GL")
SET(OPENGL_glu_LIBRARY "GLU")
SET(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
)

if(NOT TARGET OpenGL::GL)
  add_library(OpenGL::GL INTERFACE IMPORTED)
  set_target_properties(OpenGL::GL PROPERTIES
    IMPORTED_LIBNAME "${OPENGL_gl_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
  )
endif()
if(NOT TARGET OpenGL::OpenGL)
  add_library(OpenGL::OpenGL INTERFACE IMPORTED)
  set_target_properties(OpenGL::OpenGL PROPERTIES INTERFACE_LINK_LIBRARIES OpenGL::GL)
endif()

if(NOT TARGET OpenGL::GLU)
  add_library(OpenGL::GLU INTERFACE IMPORTED)
  set_target_properties(OpenGL::GLU PROPERTIES
    IMPORTED_LIBNAME "${OPENGL_glu_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_GLU_INCLUDE_DIR}"
  )
endif()
