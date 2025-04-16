/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <webgl/webgl1.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2.h>

#include "webgl_internal.h"

void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes *attributes) {
  memset(attributes, 0, sizeof(*attributes));

  attributes->alpha = 1;
  attributes->depth = 1;
  attributes->antialias = 1;
  attributes->premultipliedAlpha = 1;
  attributes->majorVersion = 1;
  attributes->enableExtensionsByDefault = 1;

  // Default context initialization state (user can override):
  // - if main thread is creating the context, default to the context not being
  //   shared between threads - enabling sharing has performance overhead,
  //   because it forces the context to be OffscreenCanvas or
  //   OffscreenFramebuffer.
  // - if a web worker is creating the context, default to using OffscreenCanvas
  //   if available, or proxying via Offscreen Framebuffer if not
  if (!emscripten_is_main_runtime_thread()) {
    attributes->proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK;
  }
}

#if defined(__EMSCRIPTEN_PTHREADS__) && defined(__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

static pthread_key_t currentActiveWebGLContext;
pthread_key_t currentThreadOwnsItsWebGLContext;
static pthread_once_t tlsInit = PTHREAD_ONCE_INIT;

static void InitWebGLTls() {
  pthread_key_create(&currentActiveWebGLContext, NULL);
  pthread_key_create(&currentThreadOwnsItsWebGLContext, NULL);
}

// When OFFSCREEN_FRAMEBUFFER is enabled the EMSCRIPTEN_WEBGL_CONTEXT_HANDLE
// is a pointer to a struct with two fields.  See registerContext in
// library_webgl.js
typedef struct WebGLContextHandle {
  uint32_t explicit_swap_control;
  pthread_t owning_thread;
} WebGLContextHandle;

static inline pthread_t GetOwningThread(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE handle) {
  return ((WebGLContextHandle*)handle)->owning_thread;
}

static inline pthread_t GetCurrentTargetThread() {
  return GetOwningThread(emscripten_webgl_get_current_context());
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char *target, const EmscriptenWebGLContextAttributes *attributes) {
  GL_FUNCTION_TRACE();
  if (!attributes) {
    emscripten_err("emscripten_webgl_create_context: attributes pointer is null!");
    return 0;
  }
  pthread_once(&tlsInit, InitWebGLTls);

  if (attributes->proxyContextToMainThread == EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS ||
    (attributes->proxyContextToMainThread == EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK && !emscripten_supports_offscreencanvas())) {
    EmscriptenWebGLContextAttributes attrs = *attributes;
    attrs.renderViaOffscreenBackBuffer = true;
    return (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)emscripten_sync_run_in_main_runtime_thread_ptr(EM_FUNC_SIG_PPP, &emscripten_webgl_do_create_context, target, &attrs);
  } else {
    return emscripten_webgl_do_create_context(target, attributes);
  }
}

EMSCRIPTEN_RESULT emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context) {
  GL_FUNCTION_TRACE();
  if (emscripten_webgl_get_current_context() == context)
    return EMSCRIPTEN_RESULT_SUCCESS;

  if (context && GetOwningThread(context) == pthread_self()) {
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current_calling_thread(context);
    if (r == EMSCRIPTEN_RESULT_SUCCESS) {
      pthread_setspecific(currentActiveWebGLContext, (void*)context);
      pthread_setspecific(currentThreadOwnsItsWebGLContext, (void*)1);
    }
    return r;
  } else {
    EMSCRIPTEN_RESULT r = emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IP, &emscripten_webgl_make_context_current_calling_thread, context);
    if (r == EMSCRIPTEN_RESULT_SUCCESS) {
      pthread_setspecific(currentActiveWebGLContext, (void*)context);
      pthread_setspecific(currentThreadOwnsItsWebGLContext, (void*)0);
      _emscripten_proxied_gl_context_activated_from_main_browser_thread(context);
    }
    return r;
  }
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_get_current_context(void) {
  return (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)pthread_getspecific(currentActiveWebGLContext);
}

EMSCRIPTEN_RESULT emscripten_webgl_commit_frame(void) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext))
    return emscripten_webgl_do_commit_frame();
  else
    return (EMSCRIPTEN_RESULT)emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_I, &emscripten_webgl_do_commit_frame);
}

static void *memdup(const void *ptr, size_t sz) {
  if (!ptr) return 0;
  void *dup = malloc(sz);
  if (dup) memcpy(dup, ptr, sz);
  return dup;
}

ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glActiveTexture, GLenum);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glAttachShader, GLuint, GLuint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glBindAttribLocation, GLuint, GLuint, const GLchar*);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindBuffer, GLenum, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindFramebuffer, GLenum, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindRenderbuffer, GLenum, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindTexture, GLenum, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VFFFF, void, glBlendColor, GLfloat, GLfloat, GLfloat, GLfloat);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glBlendEquation, GLenum);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBlendEquationSeparate, GLenum, GLenum);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBlendFunc, GLenum, GLenum);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glBlendFuncSeparate, GLenum, GLenum, GLenum, GLenum);

void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glBufferData(target, size, data, usage);
    return;
  }

  if (size < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(data, size);
    if (ptr || !data) { // glBufferData(data=0) can always be handled asynchronously
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIPPI, &emscripten_glBufferData, ptr, target, size, ptr, usage);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIPPI, &emscripten_glBufferData, target, size, data, usage);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glBufferSubData(target, offset, size, data);
    return;
  }

  if (size < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(data, size);
    if (ptr || !data) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIII, &emscripten_glBufferSubData, ptr, target, offset, size, ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, &emscripten_glBufferSubData, target, offset, size, data);
}

RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLenum, glCheckFramebufferStatus, GLenum);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glClear, GLbitfield);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VFFFF, void, glClearColor, GLfloat, GLfloat, GLfloat, GLfloat);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VF, void, glClearDepthf, GLfloat);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glClearStencil, GLint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glColorMask, GLboolean, GLboolean, GLboolean, GLboolean);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glCompileShader, GLuint);
VOID_SYNC_GL_FUNCTION_8(EM_FUNC_SIG_VIIIIIIII, void, glCompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *);
VOID_SYNC_GL_FUNCTION_9(EM_FUNC_SIG_VIIIIIIIII, void, glCompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *);
ASYNC_GL_FUNCTION_8(EM_FUNC_SIG_VIIIIIIII, void, glCopyTexImage2D, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
ASYNC_GL_FUNCTION_8(EM_FUNC_SIG_VIIIIIIII, void, glCopyTexSubImage2D, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
RET_SYNC_GL_FUNCTION_0(EM_FUNC_SIG_I, GLuint, glCreateProgram);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLuint, glCreateShader, GLenum);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glCullFace, GLenum);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteBuffers, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteFramebuffers, GLsizei, const GLuint *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDeleteProgram, GLuint);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteRenderbuffers, GLsizei, const GLuint *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDeleteShader, GLuint);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteTextures, GLsizei, const GLuint *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDepthFunc, GLenum);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDepthMask, GLboolean);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VFF, void, glDepthRangef, GLfloat, GLfloat);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDetachShader, GLuint, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDisable, GLenum);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDisableVertexAttribArray, GLuint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glDrawArrays, GLenum, GLint, GLsizei);
// TODO: The following #define FULL_ES2 does not yet exist, we'll need to compile this file twice, for FULL_ES2 mode and without
#if FULL_ES2
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glDrawElements, GLenum, GLsizei, GLenum, const void *);
#else
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glDrawElements, GLenum, GLsizei, GLenum, const void *);
#endif
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glEnable, GLenum);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glEnableVertexAttribArray, GLuint);
VOID_SYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glFinish);
VOID_SYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glFlush); // TODO: THIS COULD POTENTIALLY BE ASYNC
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glFrontFace, GLenum);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGenBuffers, GLsizei, GLuint *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glGenerateMipmap, GLenum);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGenFramebuffers, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGenRenderbuffers, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGenTextures, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIII, void, glGetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
VOID_SYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIII, void, glGetActiveUniform, GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetAttachedShaders, GLuint, GLsizei, GLsizei *, GLuint *);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_IIP, GLint, glGetAttribLocation, GLuint, const GLchar *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGetBooleanv, GLenum, GLboolean *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetBufferParameteriv, GLenum, GLenum, GLint *);
RET_SYNC_GL_FUNCTION_0(EM_FUNC_SIG_I, GLenum, glGetError);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGetFloatv, GLenum, GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetFramebufferAttachmentParameteriv, GLenum, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIP, void, glGetIntegerv, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetProgramiv, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIPP, void, glGetProgramInfoLog, GLuint, GLsizei, GLsizei *, GLchar *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetRenderbufferParameteriv, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetShaderiv, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIPP, void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei *, GLchar *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIPP, void, glGetShaderPrecisionFormat, GLenum, GLenum, GLint *, GLint *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIPP, void, glGetShaderSource, GLuint, GLsizei, GLsizei *, GLchar *);
RET_PTR_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_PI, const GLubyte *, glGetString, GLenum);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetTexParameterfv, GLenum, GLenum, GLfloat *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetTexParameteriv, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetUniformfv, GLuint, GLint, GLfloat *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetUniformiv, GLuint, GLint, GLint *);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_IIP, GLint, glGetUniformLocation, GLuint, const GLchar *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetVertexAttribfv, GLuint, GLenum, GLfloat *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetVertexAttribiv, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIP, void, glGetVertexAttribPointerv, GLuint, GLenum, void **);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glHint, GLenum, GLenum);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsBuffer, GLuint);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsEnabled, GLenum);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsFramebuffer, GLuint);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsProgram, GLuint);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsRenderbuffer, GLuint);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsShader, GLuint);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsTexture, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VF, void, glLineWidth, GLfloat);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glLinkProgram, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glPixelStorei, GLenum, GLint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VFF, void, glPolygonOffset, GLfloat, GLfloat);
VOID_SYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIIP, void, glReadPixels, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
ASYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glReleaseShaderCompiler);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glSampleCoverage, GLfloat, GLboolean);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glScissor, GLint, GLint, GLsizei, GLsizei);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glShaderBinary, GLsizei, const GLuint *, GLenum, const void *, GLsizei);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIPP, void, glShaderSource, GLuint, GLsizei, const GLchar *const*, const GLint *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glStencilFunc, GLenum, GLint, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glStencilFuncSeparate, GLenum, GLenum, GLint, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glStencilMask, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glStencilMaskSeparate, GLenum, GLuint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glStencilOp, GLenum, GLenum, GLenum);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glStencilOpSeparate, GLenum, GLenum, GLenum, GLenum);

static ssize_t ImageSize(int width, int height, GLenum format, GLenum type) {
  int numChannels;
  switch (format) {
    case GL_ALPHA: case GL_LUMINANCE: case GL_DEPTH_COMPONENT: case GL_RED: case GL_RED_INTEGER: numChannels = 1; break;
    case GL_RG: case GL_RG_INTEGER: numChannels = 2; break;
    case GL_RGB: case 0x8C40/*GL_SRGB_EXT*/: case GL_RGB_INTEGER: numChannels = 3; break;
    case GL_RGBA: case 0x8C42/*GL_SRGB_ALPHA_EXT*/: case GL_RGBA_INTEGER: numChannels = 4; break;
    default: return -1;
  }
  int sizePerPixel;
  switch (type) {
    case GL_UNSIGNED_BYTE: case GL_BYTE: sizePerPixel = numChannels; break;
    case GL_UNSIGNED_SHORT: case 0x8D61/*GL_HALF_FLOAT_OES*/: case GL_HALF_FLOAT: case GL_SHORT: case GL_UNSIGNED_SHORT_5_6_5: case GL_UNSIGNED_SHORT_4_4_4_4: case GL_UNSIGNED_SHORT_5_5_5_1: sizePerPixel = numChannels*2; break;
    case GL_UNSIGNED_INT: case GL_FLOAT: case GL_INT: case GL_UNSIGNED_INT_5_9_9_9_REV: case GL_UNSIGNED_INT_2_10_10_10_REV: case GL_UNSIGNED_INT_10F_11F_11F_REV: case GL_UNSIGNED_INT_24_8: sizePerPixel = numChannels*4; break;
    default: return -1;
  }
  return width*height*sizePerPixel;
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    return;
  }

  ssize_t sz = ImageSize(width, height, format, type);
  if (!pixels || (sz >= 0 && sz < 256*1024)) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(pixels, sz);
    if (ptr || !pixels) {
     emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIIIIIIIP, &emscripten_glTexImage2D, ptr, target, level, internalformat, width, height, border, format, type, ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIIIIIIIP, &emscripten_glTexImage2D, target, level, internalformat, width, height, border, format, type, pixels);
}

ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIF, void, glTexParameterf, GLenum, GLenum, GLfloat);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glTexParameterfv, GLenum, GLenum, const GLfloat *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glTexParameteri, GLenum, GLenum, GLint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glTexParameteriv, GLenum, GLenum, const GLint *);

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    return;
  }

  ssize_t sz = ImageSize(width, height, format, type);
  if (!pixels || (sz >= 0 && sz < 256*1024)) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(pixels, sz);
    if (ptr || !pixels) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIIIIIIII, &emscripten_glTexSubImage2D, ptr, target, level, xoffset, yoffset, width, height, format, type, ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIIIIIIII, &emscripten_glTexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIF, void, glUniform1f, GLint, GLfloat);

void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform1fv(location, count, value);
    return;
  }

  size_t sz = sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform1fv, ptr, location, count, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform1fv, location, count, value);
}

ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glUniform1i, GLint, GLint);

void glUniform1iv(GLint location, GLsizei count, const GLint *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform1iv(location, count, value);
    return;
  }

  size_t sz = sizeof(GLint)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform1iv, ptr, location, count, (GLint*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform1iv, location, count, value);
}
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIFF, void, glUniform2f, GLint, GLfloat, GLfloat);

void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform2fv(location, count, value);
    return;
  }

  size_t sz = 2*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform2fv, ptr, location, count, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform2fv, location, count, value);
}

ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform2i, GLint, GLint, GLint);

void glUniform2iv(GLint location, GLsizei count, const GLint *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform2iv(location, count, value);
    return;
  }

  size_t sz = 2*sizeof(GLint)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform2iv, ptr, location, count, (GLint*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform2iv, location, count, value);
}
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIFFF, void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat);

void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform3fv(location, count, value);
    return;
  }

  size_t sz = 3*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform3fv, ptr, location, count, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform3fv, location, count, value);
}

ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniform3i, GLint, GLint, GLint, GLint);

void glUniform3iv(GLint location, GLsizei count, const GLint *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform3iv(location, count, value);
    return;
  }

  size_t sz = 3*sizeof(GLint)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform3iv, ptr, location, count, (GLint*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform3iv, location, count, value);
}
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIFFFF, void, glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat);

void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform4fv(location, count, value);
    return;
  }

  size_t sz = 4*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform4fv, ptr, location, count, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform4fv, location, count, value);
}

ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glUniform4i, GLint, GLint, GLint, GLint, GLint);

void glUniform4iv(GLint location, GLsizei count, const GLint *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniform4iv(location, count, value);
    return;
  }

  size_t sz = 4*sizeof(GLint)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIII, &emscripten_glUniform4iv, ptr, location, count, (GLint*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, &emscripten_glUniform4iv, location, count, value);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniformMatrix2fv(location, count, transpose, value);
    return;
  }
  size_t sz = 2*2*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIII, &emscripten_glUniformMatrix2fv, ptr, location, count, transpose, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, &emscripten_glUniformMatrix2fv, location, count, transpose, value);
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniformMatrix3fv(location, count, transpose, value);
    return;
  }
  size_t sz = 3*3*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIII, &emscripten_glUniformMatrix3fv, ptr, location, count, transpose, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, &emscripten_glUniformMatrix3fv, location, count, transpose, value);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) {
    emscripten_glUniformMatrix4fv(location, count, transpose, value);
    return;
  }
  size_t sz = 4*4*sizeof(GLfloat)*count;
  if (sz < 256*1024) { // run small buffer sizes asynchronously by copying - large buffers run synchronously
    void *ptr = memdup(value, sz);
    if (ptr) {
      emscripten_dispatch_to_thread(GetCurrentTargetThread(), EM_FUNC_SIG_VIIIP, &emscripten_glUniformMatrix4fv, ptr, location, count, transpose, (GLfloat*)ptr);
      return;
    }
    // Fall through on allocation failure and run synchronously.
  }

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIIP, &emscripten_glUniformMatrix4fv, location, count, transpose, value);
}

ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glUseProgram, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glValidateProgram, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VIF, void, glVertexAttrib1f, GLuint, GLfloat);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttrib1fv, GLuint, const GLfloat *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIFF, void, glVertexAttrib2f, GLuint, GLfloat, GLfloat);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttrib2fv, GLuint, const GLfloat *);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIFFF, void, glVertexAttrib3f, GLuint, GLfloat, GLfloat, GLfloat);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttrib3fv, GLuint, const GLfloat *);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIFFFF, void, glVertexAttrib4f, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttrib4fv, GLuint, const GLfloat *);

// TODO: The following #define FULL_ES2 does not yet exist, we'll need to compile this file twice, for FULL_ES2 mode and without
#if FULL_ES2
VOID_SYNC_GL_FUNCTION_6(EM_FUNC_SIG_PIIIIIP, void, glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
#else
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIIP, void, glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
#endif
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glViewport, GLint, GLint, GLsizei, GLsizei);

VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGenQueriesEXT, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteQueriesEXT, GLsizei, const GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsQueryEXT, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBeginQueryEXT, GLenum, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glEndQueryEXT, GLenum);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glQueryCounterEXT, GLuint, GLenum);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryivEXT, GLenum, GLenum, GLint *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryObjectivEXT, GLenum, GLenum, GLint *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryObjectuivEXT, GLenum, GLenum, GLuint *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryObjecti64vEXT, GLenum, GLenum, GLint64 *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryObjectui64vEXT, GLenum, GLenum, GLuint64 *);

#endif // ~(__EMSCRIPTEN_PTHREADS__ && __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

#if GL_ENABLE_GET_PROC_ADDRESS

// Returns a function pointer to the given WebGL 1 extension function, when queried without
// a GL extension suffix such as "EXT", "OES", or "ANGLE". This function is used by
// emscripten_GetProcAddress() to implement legacy GL emulation semantics for portability.
void *_webgl1_match_ext_proc_address_without_suffix(const char *name) {
  RETURN_FN_WITH_SUFFIX(glGenQueries, EXT);
  RETURN_FN_WITH_SUFFIX(glDeleteQueries, EXT);
  RETURN_FN_WITH_SUFFIX(glIsQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glBeginQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glEndQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glQueryCounter, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectuiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjecti64v, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectui64v, EXT);

  // WebGL 1 , Extensions
  RETURN_FN_WITH_SUFFIX(glBindVertexArray, OES);
  RETURN_FN_WITH_SUFFIX(glDeleteVertexArrays, OES);
  RETURN_FN_WITH_SUFFIX(glGenVertexArrays, OES);
  RETURN_FN_WITH_SUFFIX(glIsVertexArray, OES);
  RETURN_FN_WITH_SUFFIX(glDrawBuffers, WEBGL);
  RETURN_FN_WITH_SUFFIX(glDrawArraysInstanced, ANGLE);
  RETURN_FN_WITH_SUFFIX(glDrawElementsInstanced, ANGLE);
  RETURN_FN_WITH_SUFFIX(glVertexAttribDivisor, ANGLE);
  RETURN_FN_WITH_SUFFIX(glGenQueries, EXT);
  RETURN_FN_WITH_SUFFIX(glDeleteQueries, EXT);
  RETURN_FN_WITH_SUFFIX(glIsQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glBeginQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glEndQuery, EXT);
  RETURN_FN_WITH_SUFFIX(glQueryCounter, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectuiv, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjecti64v, EXT);
  RETURN_FN_WITH_SUFFIX(glGetQueryObjectui64v, EXT);

  return 0;
}

void *emscripten_webgl1_get_proc_address(const char *name) {
  RETURN_FN(glActiveTexture);
  RETURN_FN(glAttachShader);
  RETURN_FN(glBindAttribLocation);
  RETURN_FN(glBindBuffer);
  RETURN_FN(glBindFramebuffer);
  RETURN_FN(glBindRenderbuffer);
  RETURN_FN(glBindTexture);
  RETURN_FN(glBlendColor);
  RETURN_FN(glBlendEquation);
  RETURN_FN(glBlendEquationSeparate);
  RETURN_FN(glBlendFunc);
  RETURN_FN(glBlendFuncSeparate);
  RETURN_FN(glBufferData);
  RETURN_FN(glBufferSubData);
  RETURN_FN(glCheckFramebufferStatus);
  RETURN_FN(glClear);
  RETURN_FN(glClearColor);
  RETURN_FN(glClearDepthf);
  RETURN_FN(glClearStencil);
  RETURN_FN(glColorMask);
  RETURN_FN(glCompileShader);
  RETURN_FN(glCompressedTexImage2D);
  RETURN_FN(glCompressedTexSubImage2D);
  RETURN_FN(glCopyTexImage2D);
  RETURN_FN(glCopyTexSubImage2D);
  RETURN_FN(glCreateProgram);
  RETURN_FN(glCreateShader);
  RETURN_FN(glCullFace);
  RETURN_FN(glDeleteBuffers);
  RETURN_FN(glDeleteFramebuffers);
  RETURN_FN(glDeleteProgram);
  RETURN_FN(glDeleteRenderbuffers);
  RETURN_FN(glDeleteShader);
  RETURN_FN(glDeleteTextures);
  RETURN_FN(glDepthFunc);
  RETURN_FN(glDepthMask);
  RETURN_FN(glDepthRangef);
  RETURN_FN(glDetachShader);
  RETURN_FN(glDisable);
  RETURN_FN(glDisableVertexAttribArray);
  RETURN_FN(glDrawArrays);
  RETURN_FN(glDrawElements);
  RETURN_FN(glEnable);
  RETURN_FN(glEnableVertexAttribArray);
  RETURN_FN(glFinish);
  RETURN_FN(glFlush);
  RETURN_FN(glFramebufferRenderbuffer);
  RETURN_FN(glFramebufferTexture2D);
  RETURN_FN(glFrontFace);
  RETURN_FN(glGenBuffers);
  RETURN_FN(glGenerateMipmap);
  RETURN_FN(glGenFramebuffers);
  RETURN_FN(glGenRenderbuffers);
  RETURN_FN(glGenTextures);
  RETURN_FN(glGetActiveAttrib);
  RETURN_FN(glGetActiveUniform);
  RETURN_FN(glGetAttachedShaders);
  RETURN_FN(glGetAttribLocation);
  RETURN_FN(glGetBooleanv);
  RETURN_FN(glGetBufferParameteriv);
  RETURN_FN(glGetError);
  RETURN_FN(glGetFloatv);
  RETURN_FN(glGetFramebufferAttachmentParameteriv);
  RETURN_FN(glGetIntegerv);
  RETURN_FN(glGetProgramiv);
  RETURN_FN(glGetProgramInfoLog);
  RETURN_FN(glGetRenderbufferParameteriv);
  RETURN_FN(glGetShaderiv);
  RETURN_FN(glGetShaderInfoLog);
  RETURN_FN(glGetShaderPrecisionFormat);
  RETURN_FN(glGetShaderSource);
  RETURN_FN(glGetString);
  RETURN_FN(glGetTexParameterfv);
  RETURN_FN(glGetTexParameteriv);
  RETURN_FN(glGetUniformfv);
  RETURN_FN(glGetUniformiv);
  RETURN_FN(glGetUniformLocation);
  RETURN_FN(glGetVertexAttribfv);
  RETURN_FN(glGetVertexAttribiv);
  RETURN_FN(glGetVertexAttribPointerv);
  RETURN_FN(glHint);
  RETURN_FN(glIsBuffer);
  RETURN_FN(glIsEnabled);
  RETURN_FN(glIsFramebuffer);
  RETURN_FN(glIsProgram);
  RETURN_FN(glIsRenderbuffer);
  RETURN_FN(glIsShader);
  RETURN_FN(glIsTexture);
  RETURN_FN(glLineWidth);
  RETURN_FN(glLinkProgram);
  RETURN_FN(glPixelStorei);
  RETURN_FN(glPolygonOffset);
  RETURN_FN(glReadPixels);
  RETURN_FN(glReleaseShaderCompiler);
  RETURN_FN(glRenderbufferStorage);
  RETURN_FN(glSampleCoverage);
  RETURN_FN(glScissor);
  RETURN_FN(glShaderBinary);
  RETURN_FN(glShaderSource);
  RETURN_FN(glStencilFunc);
  RETURN_FN(glStencilFuncSeparate);
  RETURN_FN(glStencilMask);
  RETURN_FN(glStencilMaskSeparate);
  RETURN_FN(glStencilOp);
  RETURN_FN(glStencilOpSeparate);
  RETURN_FN(glTexImage2D);
  RETURN_FN(glTexParameterf);
  RETURN_FN(glTexParameterfv);
  RETURN_FN(glTexParameteri);
  RETURN_FN(glTexParameteriv);
  RETURN_FN(glTexSubImage2D);
  RETURN_FN(glUniform1f);
  RETURN_FN(glUniform1fv);
  RETURN_FN(glUniform1i);
  RETURN_FN(glUniform1iv);
  RETURN_FN(glUniform2f);
  RETURN_FN(glUniform2fv);
  RETURN_FN(glUniform2i);
  RETURN_FN(glUniform2iv);
  RETURN_FN(glUniform3f);
  RETURN_FN(glUniform3fv);
  RETURN_FN(glUniform3i);
  RETURN_FN(glUniform3iv);
  RETURN_FN(glUniform4f);
  RETURN_FN(glUniform4fv);
  RETURN_FN(glUniform4i);
  RETURN_FN(glUniform4iv);
  RETURN_FN(glUniformMatrix2fv);
  RETURN_FN(glUniformMatrix3fv);
  RETURN_FN(glUniformMatrix4fv);
  RETURN_FN(glUseProgram);
  RETURN_FN(glValidateProgram);
  RETURN_FN(glVertexAttrib1f);
  RETURN_FN(glVertexAttrib1fv);
  RETURN_FN(glVertexAttrib2f);
  RETURN_FN(glVertexAttrib2fv);
  RETURN_FN(glVertexAttrib3f);
  RETURN_FN(glVertexAttrib3fv);
  RETURN_FN(glVertexAttrib4f);
  RETURN_FN(glVertexAttrib4fv);
  RETURN_FN(glVertexAttribPointer);
  RETURN_FN(glViewport);
  RETURN_FN(glGenQueriesEXT);
  RETURN_FN(glDeleteQueriesEXT);
  RETURN_FN(glIsQueryEXT);
  RETURN_FN(glBeginQueryEXT);
  RETURN_FN(glEndQueryEXT);
  RETURN_FN(glQueryCounterEXT);
  RETURN_FN(glGetQueryivEXT);
  RETURN_FN(glGetQueryObjectivEXT);
  RETURN_FN(glGetQueryObjectuivEXT);
  RETURN_FN(glGetQueryObjecti64vEXT);
  RETURN_FN(glGetQueryObjectui64vEXT);

  // WebGL 1 Extensions
  RETURN_FN(glBindVertexArrayOES);
  RETURN_FN(glDeleteVertexArraysOES);
  RETURN_FN(glGenVertexArraysOES);
  RETURN_FN(glIsVertexArrayOES);
  RETURN_FN(glDrawBuffersWEBGL);
  RETURN_FN(glDrawArraysInstancedANGLE);
  RETURN_FN(glDrawElementsInstancedANGLE);
  RETURN_FN(glVertexAttribDivisorANGLE);
  RETURN_FN(glGenQueriesEXT);
  RETURN_FN(glDeleteQueriesEXT);
  RETURN_FN(glIsQueryEXT);
  RETURN_FN(glBeginQueryEXT);
  RETURN_FN(glEndQueryEXT);
  RETURN_FN(glQueryCounterEXT);
  RETURN_FN(glGetQueryivEXT);
  RETURN_FN(glGetQueryObjectivEXT);
  RETURN_FN(glGetQueryObjectuivEXT);
  RETURN_FN(glGetQueryObjecti64vEXT);
  RETURN_FN(glGetQueryObjectui64vEXT);
  RETURN_FN(glPolygonOffsetClampEXT);
  RETURN_FN(glClipControlEXT);
  RETURN_FN(glPolygonModeWEBGL);

  return 0;
}

#endif
