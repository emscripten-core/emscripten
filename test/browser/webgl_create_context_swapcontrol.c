// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <assert.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.explicitSwapControl = true;
#ifdef USE_OFFSCREEN_FRAMEBUFFER
  attrs.renderViaOffscreenBackBuffer = true;
#endif
  // Test that creating a context succeeds
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context > 0); // Must have received a valid context.
  EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(emscripten_webgl_get_current_context() == context);
  return 0;
}
