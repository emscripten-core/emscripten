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
#include <emscripten/threading.h>
#include <bits/errno.h>
#include <stdlib.h>

pthread_t thread;

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;

int numThreadsCreated = 0;

int result = 0;

void *ThreadMain(void *arg)
{
  printf("ThreadMain\n");
  switch(numThreadsCreated)
  {
    case 1: printf("Thread 1 started: you should see the WebGL canvas fade from black to red.\n"); break;
    case 2: printf("Thread 2 started: you should see the WebGL canvas fade from black to green.\n"); break;
    case 3: printf("Thread 3 started: you should see the WebGL canvas fade from black to blue.\n"); break;
  }
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = true;
  ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  double color = 0;
  for(int i = 0; i < 100; ++i)
  {
    color += 0.01;
    switch(numThreadsCreated)
    {
      case 1: glClearColor(color, 0, 0, 1); break;
      case 2: glClearColor(0, color, 0, 1); break;
      case 3: glClearColor(0, 0, color, 1); break;
    }
    glClear(GL_COLOR_BUFFER_BIT);
    EMSCRIPTEN_RESULT r = emscripten_webgl_commit_frame();
    assert(r == EMSCRIPTEN_RESULT_SUCCESS);

    double now = emscripten_get_now();
    while(emscripten_get_now() - now < 16) /*no-op*/;
  }

  emscripten_webgl_make_context_current(0);
  emscripten_webgl_destroy_context(ctx);
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
    REPORT_RESULT(1); // But report success, so that runs on non-supporting browsers don't raise noisy errors.
#endif
    exit(0);
  }  
  if (rc)
  {
    printf("Failed to create thread! error: %d\n", rc);
    exit(0);
  }
  pthread_attr_destroy(&attr);
  ++numThreadsCreated;
}

void *mymain(void* arg)
{
  for(int i = 0; i < 3; ++i)
  {
    printf("Creating thread %d\n", i+1);
    CreateThread();
    printf("Waiting for thread to finish.\n");
    pthread_join(thread, 0);
    thread = 0;
  }
  printf("All done!\n");
  return 0;
}

// Tests that the OffscreenCanvas context can travel from main thread -> thread 1 -> thread 2
// #define TEST_CHAINED_WEBGL_CONTEXT_PASSING

// If set, the OffscreenCanvas is transferred from the main thread directly to thread 2, without giving it to thread 1
// in between.
// #define TRANSFER_TO_CHAINED_THREAD_FROM_MAIN_THREAD

int main()
{
#ifdef TEST_CHAINED_WEBGL_CONTEXT_PASSING
  pthread_attr_t attr;
  pthread_attr_init(&attr);
#ifndef TRANSFER_TO_CHAINED_THREAD_FROM_MAIN_THREAD
  emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");
#endif
  int rc = pthread_create(&thread, &attr, mymain, 0);
  if (rc == ENOSYS)
  {
    printf("Test Skipped! OffscreenCanvas is not supported!\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1); // But report success, so that runs on non-supporting browsers don't raise noisy errors.
#endif
    exit(0);
  }
  emscripten_exit_with_live_runtime();
#else
  mymain(0);
#endif
  return 0;
}
