// A joinable thread's "finished" message only reaches the main thread once it
// returns to the event loop.  By then the main thread has joined that thread
// and created a new one that reuses its pthread_t.  The late message must not
// stop the new, still running thread from taking part in dlopen()'s sync.

#include <assert.h>
#include <dlfcn.h>
#include <emscripten/eventloop.h>
#include <emscripten/threading.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*func_t)();

static pthread_t waiter;
static _Atomic int go;
static func_t (*side_func_address)();
static int result;

static void* noop(void* arg) { return NULL; }

static void* wait_then_call(void* arg) {
  while (!go) {
    emscripten_futex_wait(&go, 0, INFINITY);
  }
  if (side_func_address) {
    result = side_func_address()();
  }
  return NULL;
}

static void release_waiter() {
  go = 1;
  emscripten_futex_wake(&go, INT_MAX);
  pthread_join(waiter, NULL);
  go = 0;
}

static void load_and_call(void* arg) {
  void* handle = dlopen("libside.so", RTLD_NOW);
  assert(handle);
  side_func_address = dlsym(handle, "side_func_address");
  assert(side_func_address);
  // The waiter traps here if dlopen() skipped it.
  release_waiter();
  assert(result == 43);
  printf("done\n");
  exit(0);
}

int main() {
  // The new thread almost always gets the joined thread's block back, but
  // retry rather than depend on it.
  for (int i = 0; i < 10; i++) {
    pthread_t joined;
    pthread_create(&joined, NULL, noop, NULL);
    pthread_join(joined, NULL);
    pthread_create(&waiter, NULL, wait_then_call, NULL);
    if (pthread_equal(waiter, joined)) {
      emscripten_set_timeout(load_and_call, 100, NULL);
      return 0;
    }
    release_waiter();
  }
  printf("no thread reused a joined thread's pthread_t\n");
  return 1;
}
