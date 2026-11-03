/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <pthread.h>

pthread_t worker_thread;

bool context_lost(int eventType, const void *reserved, void *userData) {
  EM_ASM({
    console.log('DEBUG context_lost userData: ' + $0);
  }, userData);
  if (pthread_self() != worker_thread) {
    emscripten_force_exit(1);
  }
  if (userData != (void*)0x1234) {
    emscripten_force_exit(2);
  }
  printf("Test finished with result 0\n");
  emscripten_force_exit(0);
  return true;
}

void* thread_main(void* arg) {
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(uintptr_t)arg;

  EMSCRIPTEN_RESULT res = emscripten_set_webglcontextlost_callback("#canvas", (void*)0x1234, false, context_lost);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

  MAIN_THREAD_EM_ASM({
    var glExt = GL.contexts[$0].GLctx.getExtension('WEBGL_lose_context');
    glExt.loseContext();
  }, context);

  emscripten_exit_with_live_runtime();
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context > 0);

  int rc = pthread_create(&worker_thread, NULL, thread_main, (void*)(uintptr_t)context);
  assert(rc == 0);
  return 0;
}
