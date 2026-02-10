// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#define GL_RGBA32F                        0x8814

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.enableExtensionsByDefault = true;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_webgl_make_context_current(context);

  // Test what the return values of GL_IMPLEMENTATION_COLOR_READ_TYPE and GL_IMPLEMENTATION_COLOR_READ_FORMAT are.
  GLint type = -1, format = -1;
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
  printf("GL_IMPLEMENTATION_COLOR_READ_TYPE for FBO 0: 0x%x\n", type);
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
  printf("GL_IMPLEMENTATION_COLOR_READ_FORMAT for FBO 0: 0x%x\n", format);

  // Try glReadPixels() in that format.
  unsigned char data[16];
  glReadPixels(0, 0, 1, 1, format, type, &data);
  assert(glGetError() == 0 && "glReadPixels of FBO 0 in implementation defined format failed");

  // Try glReadPixels() in the format that is mandated to work by the spec.
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
  assert(glGetError() == 0 && "glReadPixels of FBO 0 in GL_RGBA+GL_UNSIGNED_BYTE format failed");

  // Test the same with a floating point render target bound.
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  GLuint rb;
  glGenRenderbuffers(1, &rb);
  glBindRenderbuffer(GL_RENDERBUFFER, rb);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
  printf("GL_IMPLEMENTATION_COLOR_READ_TYPE for FBO with float renderbuffer: 0x%x\n", type);
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
  printf("GL_IMPLEMENTATION_COLOR_READ_FORMAT for FBO with float renderbuffer: 0x%x\n", format);

  // Try glReadPixels() in that format.
  if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
    printf("Oops, WebGL implementation returns an implementation defined color type(==GL_UNSIGNED_BYTE) & format(==GL_RGBA) that should "
           "not be possible to be used to sample WebGL floating point color renderbuffer according to WEBGL_color_buffer_float.\n");
  }

  while (glGetError() != 0);
  glReadPixels(0, 0, 1, 1, format, type, &data);
  if (glGetError() != 0) printf("glReadPixels of FBO with floating point color renderbuffer in implementation defined format failed");

  // Try glReadPixels() in the format that is mandated to work by the spec. WEBGL_color_buffer_float says
  // that GL_RGBA and GL_UNSIGNED_BYTE no longer work but instead GL_RGBA and GL_FLOAT must be used.
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, &data);
  assert(glGetError() == 0 && "glReadPixels of FBO with floating point color renderbuffer in GL_RGBA+GL_FLOAT format failed");

  return 0;
}
