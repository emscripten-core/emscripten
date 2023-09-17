// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

// WebGL 2 core does not support GL_UNSIGNED_SHORT_5_6_5_REV extension.
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.enableExtensionsByDefault = 0; // Make sure we don't accidentally enable an extension that would bring GL_UNSIGNED_SHORT_5_6_5_REV support.

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context);
  EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  void* data = new char[512];
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, data);
  assert(glGetError() != 0); // We should be getting an error
  return 0;
}
