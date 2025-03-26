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

// Pass #define TEST_OFFSCREENCANVAS when testing against OffscreenCanvas support

void fill(int color)
{
  switch(color)
  {
    case 1: glClearColor(1, 0, 0, 1); printf("On thread %p: you should see a red canvas.\n", pthread_self()); break;
    case 2: glClearColor(0, 1, 0, 1); printf("On thread %p: you should see a green canvas.\n", pthread_self()); break;
    case 3: glClearColor(0, 0, 1, 1); printf("On thread %p: you should see a blue canvas.\n", pthread_self()); break;
  }
  glClear(GL_COLOR_BUFFER_BIT);
  EMSCRIPTEN_RESULT r = emscripten_webgl_commit_frame();
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  emscripten_thread_sleep(2000);
}

void *thread_main(void *param)
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = true;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);

  EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(ctx);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  fill((int)(intptr_t)param);

  r = emscripten_webgl_make_context_current(0);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  pthread_exit(0);
}

void run_thread(int param)
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");
  pthread_t thread;
  int rc = pthread_create(&thread, &attr, thread_main, (void*)param);
  assert(rc == 0);
  rc = pthread_join(thread, NULL);
  assert(rc == 0);
}

int main()
{
#ifdef TEST_OFFSCREENCANVAS
  if (!emscripten_supports_offscreencanvas())
  {
    printf("Current browser does not support OffscreenCanvas. Skipping this test.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
  }
#endif

  // Create a context
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = true;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);

  // Activate it
  EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(ctx);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // Draw
  fill(3);

  // Release context for the threads to capture the canvas
  r = emscripten_webgl_make_context_current(0);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // TODO: Optionally, also delete the above context
  // TODO: Test that threads share the context (main thread creates, pthread uses)
  run_thread(1);
  run_thread(2);
  run_thread(3);

  // Reactivate the context we got
  r = emscripten_webgl_make_context_current(ctx);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // And render with it again
  fill(1);

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
