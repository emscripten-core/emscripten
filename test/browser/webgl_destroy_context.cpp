// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

void report_result(int result) {
  printf("Test finished with result %d\n", result);
  emscripten_force_exit(result);
}

void finish(void*) {
  report_result(0);
}

bool context_lost(int eventType, const void *reserved, void *userData) {
  printf("C code received a signal for WebGL context lost! This should not happen!\n");
  report_result(1);
  return 0;
}

bool context_restored(int eventType, const void *reserved, void *userData) {
  printf("C code received a signal for WebGL context restored! This should not happen!\n");
  report_result(1);
  return 0;
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_set_webglcontextlost_callback("#canvas", 0, 0, context_lost);
  emscripten_set_webglcontextrestored_callback("#canvas", 0, 0, context_restored);
  // When we force a context loss, we should get an event, i.e. context_lost_desired() should get called.
  EM_ASM({
      // The GL object is accessed here in a closure unsafe manner, so this test should not be run with closure enabled.
      Module['firstGLContextExt'] = GL.contexts[$0].GLctx.getExtension('WEBGL_lose_context');
    }, context);

  emscripten_webgl_destroy_context(context);

  EM_ASM({
      Module['firstGLContextExt'].loseContext();
    }, context);

  emscripten_async_call(finish, 0, 3000);
  return 99;
}
