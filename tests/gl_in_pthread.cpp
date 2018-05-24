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

int threadRunning = 0;
int numThreadsCreated = 0;

int result = 0;

void *ThreadMain(void *arg)
{
  switch(numThreadsCreated)
  {
    case 1: printf("Thread 1 started: you should see the WebGL canvas fade from black to red.\n"); break;
    case 2: printf("Thread 2 started: you should see the WebGL canvas fade from black to green.\n"); break;
    case 3: printf("Thread 3 started: you should see the WebGL canvas fade from black to blue.\n"); break;
  }
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = EM_TRUE;
  ctx = emscripten_webgl_create_context(0, &attr);
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
  emscripten_atomic_store_u32(&threadRunning, 0);
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
  pthread_attr_destroy(&attr);
  emscripten_atomic_store_u32(&threadRunning, 1);
  ++numThreadsCreated;
}

void PollThreadExit(void *)
{
  if (!emscripten_atomic_load_u32(&threadRunning))
  {
    if (numThreadsCreated >= 3)
    {
      emscripten_atomic_store_u32(&result, 1);
#ifdef REPORT_RESULT
      REPORT_RESULT(result);
#endif
      return;
    }
    else
    {
      CreateThread();
    }
  }
  emscripten_async_call(PollThreadExit, 0, 1000);
}

void *mymain(void*)
{
  EM_ASM(Module['noExitRuntime'] = true;);
  CreateThread();
  emscripten_async_call(PollThreadExit, 0, 1000);
  return 0;
}

// #define TEST_CHAINED_WEBGL_CONTEXT_PASSING

int main()
{
#ifdef TEST_CHAINED_WEBGL_CONTEXT_PASSING
  EM_ASM(Module['noExitRuntime'] = true;);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");
  int rc = pthread_create(&thread, &attr, mymain, 0);
  if (rc == ENOSYS)
  {
    printf("Test Skipped! OffscreenCanvas is not supported!\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1); // But report success, so that runs on non-supporting browsers don't raise noisy errors.
#endif
    exit(0);
  }
#else
  mymain(0);
#endif
  return 0;
}
