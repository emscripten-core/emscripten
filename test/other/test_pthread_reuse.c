#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/console.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

_Atomic int ready;

void markReady(void* arg) {
  ready = 1;
}

void* thread_main(void* arg) {
  int keepalive = (intptr_t)arg;
  emscripten_outf("in thread_main: %d", keepalive);
  if (keepalive) {
    // Exit with live runtime.  Once we are in the event loop call markReady.
    emscripten_async_call(markReady, NULL, 0);
    emscripten_exit_with_live_runtime();
  }
  return NULL;
}

void checkThreadPool() {
  int running = EM_ASM_INT(return PThread.runningWorkers.length);
  int unused = EM_ASM_INT(return PThread.unusedWorkers.length);
  printf("running=%d unused=%d\n", running, unused);
  assert(running == 0);
  assert(unused == 1);
}

int main() {
  printf("in main\n");
  checkThreadPool();
  pthread_t t, t2;

  for (int i = 0; i < 3; i++) {
    ready = 0;

    // Create a thread that exit's with a non-zero keepalive counter
    pthread_create(&t, NULL, thread_main, (void*)1);
    while (!ready) {}
    pthread_cancel(t);
    pthread_join(t, NULL);
    checkThreadPool();

    // Create a second thread that should re-use the same worker.
    // This thread should reset its keepalive counter on startup and
    // be able to exit normally.
    pthread_create(&t2, NULL, thread_main, NULL);
    pthread_join(t2, NULL);
    checkThreadPool();
  }
  printf("done\n");
}
