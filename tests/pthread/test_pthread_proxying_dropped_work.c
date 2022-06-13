#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

em_proxying_queue* queue;

_Atomic int exploded = 0;

void explode(void* arg) {
  exploded = 1;
  assert(false);
}

void* proxy_to_self(void* arg) {
  emscripten_proxy_async(queue, pthread_self(), explode, NULL);
  return NULL;
}

void* do_nothing(void* arg) {
  return NULL;
}

int main() {
  emscripten_console_log("start");
  queue = em_proxying_queue_create();
  assert(queue);

  // Check that proxying to a thread that exits without a live runtime causes
  // the work to be dropped without other errors.
  pthread_t worker;
  pthread_create(&worker, NULL, do_nothing, NULL);
  emscripten_proxy_async(queue, worker, explode, NULL);

  // Check that a thread proxying to itself but exiting without a live runtime
  // causes the work to be dropped without other errors.
  pthread_t self_proxier;
  pthread_create(&self_proxier, NULL, proxy_to_self, NULL);

  pthread_join(worker, NULL);
  pthread_join(self_proxier, NULL);

  // Wait a bit (50 ms) to see if the `assert(pthread_self())` in
  // emscripten_proxy_execute_queue fires or if we explode.
  struct timespec time = {
    .tv_sec = 0,
    .tv_nsec = 50 * 1000 * 1000,
  };
  nanosleep(&time, NULL);

  assert(!exploded);
  emscripten_console_log("done");
}
