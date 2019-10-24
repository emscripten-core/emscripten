#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <unistd.h>

// Tests that the size of an OffscreenCanvas can be queried and set from the main thread.
// Supporting this is needed since it is very common to resize Canvas size on main thread while relayouting DOM in JavaScript code.

// Define the following to test the scenario where the pthread that owns the OffscreenCanvas is running its own synchronously blocking loop (never yields to event loop).
// If not defined, the pthread that owns the OffscreenCanvas is using emscripten_set_main_loop() so periodically yields back to the event loop.
// #define TEST_SYNC_BLOCKING_LOOP

void thread_local_main_loop()
{
  int w = 0, h = 0;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  if (w == 699 && h == 299)
  {
    printf("Observed OffscreenCanvas resize to 699x299 from main thread! Test passed!\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif

#ifndef TEST_SYNC_BLOCKING_LOOP
    emscripten_cancel_main_loop();
#endif

    EM_ASM(noExitRuntime=false);
    exit(0);
  }
  printf("%dx%d\n", w, h);
}

void *thread_main(void *arg)
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = EM_TRUE;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);

  // To start off, change the size of the OffscreenCanvas, main thread will test that it sees this change.
  printf("pthread resizing OffscreenCanvas to size 355x233.\n");
  emscripten_set_canvas_element_size("#canvas", 355, 233);

  // In pthread, keep polling the canvas size until we see it being changed from the main thread.
#ifdef TEST_SYNC_BLOCKING_LOOP
  for(;;)
  {
    thread_local_main_loop();
    emscripten_current_thread_process_queued_calls();
    usleep(16*1000);
  }
#else
  emscripten_set_main_loop(thread_local_main_loop, 1, 0);
#endif

  EM_ASM(noExitRuntime=true);
  return 0;
}

void resize_canvas(void *)
{
  // Test that on the main thread, we can observe size changes to the canvas size.
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(w == 355 && "We did not observe the effect of pthread having resized OffscreenCanvas");
  assert(h == 233);
  printf("Main thread saw canvas to get resized to %dx%d.\n", w, h);

  // Test that on the main thread, we can also change the size. (pthread listens to see this)
  printf("Main thread resizing OffscreenCanvas to size 699x299.\n");
  emscripten_set_canvas_element_size("#canvas", 699, 299);
}

//should be able to do this regardless of offscreen canvas support
void get_canvas_size()
{
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  assert(h == 150);
  assert(w == 300);
}

int main()
{
  get_canvas_size();
  if (!emscripten_supports_offscreencanvas())
  {
    printf("Current browser does not support OffscreenCanvas. Skipping the rest of the tests.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");

  pthread_t thread;
  printf("Creating thread.\n");
  pthread_create(&thread, &attr, thread_main, NULL);
  pthread_detach(thread);
  EM_ASM(noExitRuntime=true);

  // Wait for a while, then change the canvas size on the main thread.
  printf("Waiting for 5 seconds for good measure.\n");
  emscripten_async_call(resize_canvas, 0, 5000);
}
