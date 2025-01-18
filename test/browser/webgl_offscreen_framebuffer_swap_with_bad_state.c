/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EXPLICIT_SWAP
  #error EXPLICIT_SWAP is required.
#endif

#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = 1;
#if TEST_WEBGL2
  attr.majorVersion = 2;
#endif
#if !TEST_ANTIALIAS
  attr.antialias = 0;
#endif

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
#ifdef TEST_DISABLE_VAO
  EM_ASM({
    console.log(GL.currentContext.defaultVao);
    assert(GL.currentContext.defaultVao);
    GL.currentContext.defaultVao = undefined;
  });
#endif

#if !TEST_WEBGL2 && TEST_VERIFY_WEBGL1_VAO_SUPPORT
  // This test cannot run without browser support for OES_vertex_array_object.
  // This check is just to verify that the browser has support; otherwise, we
  // will end up testing the non-VAO path. Enabling it here does not actually do
  // anything, because offscreen framebuffer has already been initialized. Note
  // that if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0, then offscreen
  // framebuffer will _never_ use VAOs on WebGL 1 (unless something enables
  // OES_vertex_array_object before createOffscreenFramebuffer runs).
  if (!emscripten_webgl_enable_extension(ctx, "OES_vertex_array_object")) {
    return 1;
  }
#endif

  glClearColor(0, 1, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnableVertexAttribArray(1);
  emscripten_webgl_commit_frame();

  if (glGetError() != GL_NO_ERROR) {
    return 1;
  }

  // C doesn't have access to the "frontbuffer" (canvas contents).
  // Escape via JavaScript to ensure the test passed.
  int canvas_is_green = EM_ASM_INT({
    GLctx.bindFramebuffer(GLctx.FRAMEBUFFER, null);
    var pixels = new Uint8Array(4);
    GLctx.readPixels(5, 5, 1, 1, GLctx.RGBA, GLctx.UNSIGNED_BYTE, pixels);
    return pixels[0] == 0 && pixels[1] == 255 && pixels[2] == 0 && pixels[3] == 255;
  });
  if (!canvas_is_green) {
    return 1;
  }

  return 0;
}
