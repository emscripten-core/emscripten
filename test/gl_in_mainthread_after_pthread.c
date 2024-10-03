// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <bits/errno.h>
#include <stdlib.h>

pthread_t thread;

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;

_Atomic bool threadRunning = false;

void *ThreadMain(void *arg)
{
  printf("Thread started. You should see the WebGL canvas fade from black to red.\n");
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = true;
  ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

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
  threadRunning = false;
  printf("Thread quit\n");
  pthread_exit(0);
}

void CreateThread()
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");
  int rc = pthread_create(&thread, &attr, ThreadMain, 0);
  if (rc == ENOSYS)
  {
    printf("Test Skipped! OffscreenCanvas is not supported!\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(0); // But report success, so that runs on non-supporting browsers don't raise noisy errors.
#endif
    exit(0);
  }
  pthread_attr_destroy(&attr);
  threadRunning = true;
}

//#define TEST_MAIN_THREAD_EXPLICIT_COMMIT

void MainThreadRender()
{
  static double color = 0;
  color += 0.01;
  glClearColor(0, color, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
#ifdef TEST_MAIN_THREAD_EXPLICIT_COMMIT
  EMSCRIPTEN_RESULT r = emscripten_webgl_commit_frame();
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  // In explicit swap control mode, whatever we draw here shouldn't show up.
  glClearColor(1, 0, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
#endif
#
  if (color >= 1.0)
  {
    printf("Test finished.\n");
    emscripten_cancel_main_loop();
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
  }
}

void PollThreadExit(void * arg)
{
  if (!threadRunning)
  {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
#ifdef TEST_MAIN_THREAD_EXPLICIT_COMMIT
    attr.explicitSwapControl = true;
#endif
    ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
    printf("Main thread rendering. You should see the WebGL canvas fade from black to green.\n");
    emscripten_set_main_loop(MainThreadRender, 0, 0);
  }
  else
  {
    emscripten_async_call(PollThreadExit, 0, 1000);
  }
}

int main()
{
  CreateThread();
  emscripten_async_call(PollThreadExit, 0, 1000);
  return 0;
}
