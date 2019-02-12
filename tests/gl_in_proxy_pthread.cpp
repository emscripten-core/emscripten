#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <bits/errno.h>
#include <stdlib.h>

#if !defined(TEST_OFFSCREEN_CANVAS)
// Defining EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES to empty string will cause no canvases to be transferred.
#define EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES ""
#elif TEST_OFFSCREEN_CANVAS == 1
// Specifying #canvas should take Module.canvas to be transferred
#define EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES "#canvas"
#elif TEST_OFFSCREEN_CANVAS == 2
// Leaving EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES undefined will also transfer the default Module.canvas object
#endif

int main()
{
  if (!emscripten_supports_offscreencanvas())
  {
    printf("Current browser does not support OffscreenCanvas. Skipping this test.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
  }
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = EM_TRUE;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  printf("Created context with handle %u\n", (unsigned int)ctx);
  emscripten_webgl_make_context_current(ctx);

  printf("You should see the canvas fade from black to red.\n");
  double color = 0;
  for(int i = 0; i < 100; ++i)
  {
    color += 0.01;
    glClearColor(color, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EMSCRIPTEN_RESULT r = emscripten_webgl_commit_frame();
    assert(r == EMSCRIPTEN_RESULT_SUCCESS);

    double now = emscripten_get_now();
    while(emscripten_get_now() - now < 16) /*no-op*/;
  }

  emscripten_webgl_make_context_current(0);
  emscripten_webgl_destroy_context(ctx);
  printf("Thread quit\n");
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
