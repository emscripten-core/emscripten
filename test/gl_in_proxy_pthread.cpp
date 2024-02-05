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

int main()
{
  if (!emscripten_supports_offscreencanvas())
  {
    printf("Current browser does not support OffscreenCanvas. Skipping this test.\n");
    return 0;
  }
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
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

#if ASYNCIFY
    emscripten_sleep(16);
#else
    double now = emscripten_get_now();
    while(emscripten_get_now() - now < 16) /*no-op*/;
#endif
  }

  emscripten_webgl_make_context_current(0);
  emscripten_webgl_destroy_context(ctx);
  printf("Thread quit\n");
  return 0;
}
