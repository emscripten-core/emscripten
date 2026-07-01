// Copyright 2018 The Emscripten Authors.  All rights reserved.
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
#include <emscripten/threading.h>
#include <bits/errno.h>
#include <stdlib.h>

pthread_t thread;

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;

_Atomic int threadRunning = 0;

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

#if 0 // TODO: this doesn't work yet for some reason
  unsigned char data[64];
  glReadPixels(0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, data);
  assert(glGetError() == 0);
  printf("%d %d %d %d\n", data[0], data[1], data[2], data[3]);
#endif

  emscripten_webgl_make_context_current(0);
  emscripten_webgl_destroy_context(ctx);
  threadRunning = 0;
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
    REPORT_RESULT(0); // for now, report a skip as success
#endif
    exit(1);
  }
  pthread_attr_destroy(&attr);
  threadRunning = 1;
}

void PollThreadExit(void *arg)
{
  if (!threadRunning)
  {
    printf("Test finished.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
  } else {
    emscripten_async_call(PollThreadExit, 0, 100);
  }
}

int main()
{
  if (!emscripten_supports_offscreencanvas())
  {
    printf("Current browser does not support OffscreenCanvas. Skipping this test.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    return 0;
  }
  CreateThread();
  emscripten_async_call(PollThreadExit, 0, 100);
  return 0;
}
