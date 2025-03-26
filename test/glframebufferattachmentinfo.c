/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int check_equal(int expected, int actual) {
  if (expected == actual) {
    printf("ok\n");
    return 1;
  } else {
    printf("error: expected = %d, actual = %d\n", expected, actual);
    return 0;
  }
}

int main() {
  EGLDisplay dpy;
  EGLint dpy_attrib_list[] = {
    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
    EGL_DEPTH_SIZE, 16,
    EGL_NONE
  };
  EGLConfig *configs;
  EGLint num_config;
  EGLNativeWindowType win;
  EGLSurface surface;
  EGLContext ctx;
  EGLint ctx_attrib_list[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(dpy, NULL, NULL);
  eglChooseConfig(dpy, dpy_attrib_list, NULL, 0, &num_config);
  configs = malloc(sizeof(EGLConfig) * num_config);
  eglChooseConfig(dpy, dpy_attrib_list, configs, num_config, &num_config);
  memset(&win, 0, sizeof(EGLNativeWindowType));
#if defined(__EMSCRIPTEN__)
  // Simply skip window creation because the handle is ignored by Emscripten.
#else
#error not implemented
#endif
  surface = eglCreateWindowSurface(dpy, configs[0], win, NULL);
  ctx = eglCreateContext(dpy, configs[0], EGL_NO_CONTEXT, ctx_attrib_list);
  eglMakeCurrent(dpy, surface, surface, ctx);

  GLuint fbo;
  GLuint tex;
  GLuint rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex, 0);
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 256, 256);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

  GLint params;
  int result = 1;
  printf("check type of GL_COLOR_ATTACHMENT0\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);
  result &= check_equal(GL_TEXTURE, params);
  printf("check type of GL_DEPTH_ATTACHMENT\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);
  result &= check_equal(GL_RENDERBUFFER, params);
  printf("check name of GL_COLOR_ATTACHMENT0\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &params);
  result &= check_equal(tex, params);
  printf("check name of GL_DEPTH_ATTACHMENT\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &params);
  result &= check_equal(rbo, params);
  printf("check mipmap level of GL_COLOR_ATTACHMENT0\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &params);
  result &= check_equal(0, params);
  printf("check cube map face of GL_COLOR_ATTACHMENT0\n");
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &params);
  result &= check_equal(GL_TEXTURE_CUBE_MAP_POSITIVE_X, params);

  eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(dpy, ctx);
  eglDestroySurface(dpy, surface);
#if defined(__EMSCRIPTEN__)
  // No window needs to be closed/destroyed.
#else
#error not implemented
#endif
  free(configs);
  eglTerminate(dpy);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
  return 0;
}
