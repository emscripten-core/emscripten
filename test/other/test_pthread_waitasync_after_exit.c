#define _GNU_SOURCE
#include <assert.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static _Atomic int* word;
static _Atomic bool woken = false;
static pthread_barrier_t started;

void* dummy_thread(void* arg) {
  return NULL;
}

void* waiter_thread(void* arg) {
  *word = 2;
  pthread_barrier_wait(&started);
  int res = __builtin_wasm_memory_atomic_wait32((int*)word, 2, -1);
  assert(res == ATOMICS_WAIT_OK);
  woken = true;
  return NULL;
}

// See https://github.com/emscripten-core/emscripten/issues/27492
// Test that exiting threads clear their Atomics.waitAsync waiters. Otherwise,
// recycling the freed pthread_t struct memory via malloc can cause a stale
// waiter to intercept Atomics.notify notifications meant for another thread.
int main(void) {
  pthread_barrier_init(&started, NULL, 2);

  pthread_t dead;
  pthread_create(&dead, NULL, dummy_thread, NULL);
  pthread_join(dead, NULL);

  // malloc should reuse the memory block freed when dummy_thread exited.
  word = malloc(1024);

  pthread_t w;
  pthread_create(&w, NULL, waiter_thread, NULL);

  pthread_barrier_wait(&started);
  emscripten_thread_sleep(100);

  *word = 0;
  __builtin_wasm_memory_atomic_notify((int*)word, 1);

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 5;
  if (pthread_timedjoin_np(w, NULL, &ts) != 0 || !woken) {
    printf("DEADLOCK: parked thread was not woken!\n");
    return 1;
  }

  pthread_barrier_destroy(&started);
  printf("done\n");
  return 0;
}
