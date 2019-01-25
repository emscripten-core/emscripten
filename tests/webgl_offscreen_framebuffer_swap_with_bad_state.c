/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EXPLICIT_SWAP
  #error EXPLICIT_SWAP is required.
#endif

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = 1;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  glClearColor(0, 1, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnableVertexAttribArray(1);
  emscripten_webgl_commit_frame();

#ifdef REPORT_RESULT
  // This program doesn't have access to the "frontbuffer" (canvas contents), so
  // it can't check them. The result on the screen should be fullscreen green.
  REPORT_RESULT(0);
#endif
}
