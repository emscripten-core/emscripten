#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <unistd.h>

#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
#define SANITIZER
#endif

// Proxying queues accessed from the worker thread.
em_proxying_queue* queues[2];

#ifndef SANITIZER

// If we are not using sanitizers (which need to use their own allocators),
// override free so we can track when queues are actually freed.

_Atomic int frees = 0;

void __attribute__((noinline)) free(void* ptr) {
  frees++;
  emscripten_builtin_free(ptr);
}

#endif // SANITIZER

_Atomic int should_execute = 0;
_Atomic int executed[2] = {};
_Atomic int processed = 0;

EMSCRIPTEN_KEEPALIVE
void register_processed(void) {
  processed++;
}

void set_flag(void* arg) { *(_Atomic int*)arg = 1; }

// Delay setting the flag until the next turn of the event loop so it can be set
// after the proxying queue is destroyed.
void task(void* arg) { emscripten_async_call(set_flag, arg, 0); }

void* execute_and_free_queue(void* arg) {
  // Wait until we are signaled to execute the queue.
  while (!should_execute) {
    sched_yield();
  }

  // Execute the proxied work then free the empty queues.
  for (int i = 0; i < 2; i++) {
    emscripten_proxy_execute_queue(queues[i]);
    em_proxying_queue_destroy(queues[i]);
  }

  // Exit with a live runtime so the queued work notification is received and we
  // try to execute the queue again, even though we already executed all its
  // work and we are now just waiting for the notifications to be received so we
  // can free it.
  emscripten_exit_with_live_runtime();
}

void nop(void* arg) {}

int main() {
  emscripten_console_log("start");
  for (int i = 0; i < 2; i++) {
    queues[i] = em_proxying_queue_create();
    assert(queues[i]);
  }

  // Create the worker and send it tasks.
  pthread_t worker;
  pthread_create(&worker, NULL, execute_and_free_queue, NULL);
  for (int i = 0; i < 2; i++) {
    emscripten_proxy_async(queues[i], worker, task, &executed[i]);
  }
  should_execute = 1;

  // Wait for the tasks to be executed. The queues will have been destroyed
  // after this.
  while (!executed[0] || !executed[1]) {
    sched_yield();
  }

#ifndef SANITIZER
  // Our zombies should not have been freed yet.
  int frees_before_cull = frees;
#endif // SANITIZER

  // Cull the zombies! (by forcing a new task queue to be allocated)
  em_proxying_queue* culler = em_proxying_queue_create();
  emscripten_proxy_async(culler, pthread_self(), nop, NULL);

#ifndef SANITIZER
  // Now they should be free.
  int frees_after_cull = frees;
  assert(frees_after_cull > frees_before_cull);
#endif // SANITIZER

  // If we try again, there should be nothing left to cull.
  em_proxying_queue* non_culler = em_proxying_queue_create();
  emscripten_proxy_async(non_culler, pthread_self(), nop, NULL);

#ifndef SANITIZER
  assert(frees == frees_after_cull);
#endif

  em_proxying_queue_destroy(culler);
  em_proxying_queue_destroy(non_culler);

  emscripten_console_log("done");
}
