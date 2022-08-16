#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "emscripten/threading.h"
#include "emscripten.h"

static _Atomic int doneShutdown;
static _Atomic int doneEntry;
static pthread_t mythread;

void *ThreadMain(void *threadid) {
  puts("hello world");
  doneEntry = 1;
  emscripten_exit_with_live_runtime();
  return NULL;
}

void Shutdown() {
  puts("got Shutdown");
  doneShutdown = 1;
}

void looper() {
  static int framecount = 0;
  framecount++;
  if (doneEntry) {
    puts("looper() : running Shutdown on thread");
    int rc = emscripten_dispatch_to_thread(mythread, EM_FUNC_SIG_V, Shutdown, NULL);
    assert(rc == 0);
    doneEntry = 0;
  }
  if (doneShutdown) {
    puts("looper() : done Shutdown exiting");
    emscripten_force_exit(0);
  }
}

int main(int argc, char* argv[]) {
  int rc = pthread_create(&mythread, NULL, ThreadMain, (void *)0);
  assert(rc == 0);
  emscripten_set_main_loop(looper, 10, 0);
  return 0;
}
