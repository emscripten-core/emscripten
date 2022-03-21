#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "proxying.h"

// The first two queues will be zombies and the next two will be created just to
// cull the zombies.
em_proxying_queue* queues[4];

#ifndef SANITIZER

// If we are not using sanitizers (which need to use their own allocators),
// override free so we can track when queues are actually freed.

int queues_freed[4] = {};

extern void emscripten_builtin_free(void* mem);
void __attribute__((noinline)) free(void* ptr) {
  for (int i = 0; i < 4; i++) {
    if (ptr && queues[i] == ptr) {
      queues_freed[i] = 1;
      queues[i] = NULL;
      break;
    }
  }
  emscripten_builtin_free(ptr);
}

#endif // SANITIZER

_Atomic int should_execute = 0;
_Atomic int executed[2] = {};

void task(void* arg) { *(_Atomic int*)arg = 1; }

void* execute_and_free_queue(void* arg) {
  // Wait until we are signaled to execute the queue.
  while (!should_execute) {
  }

  // Execute the proxied work then free the empty queues.
  for (int i = 0; i < 2; i++) {
    emscripten_proxy_execute_queue(queues[i]);
    em_proxying_queue_destroy(queues[i]);
  }

  // Exit with a live runtime so the queued work notification is received and we
  // try to execute the queue again, even though it has been destroyed.
  emscripten_exit_with_live_runtime();
}

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

  // Wait for the tasks to be executed.
  while (!executed[0] || !executed[1]) {
  }

  // Break the queue abstraction to wait for the refcounts to be decreased.
  while (*(_Atomic int*)queues[0] || *(_Atomic int*)queues[1]) {
  }

#ifndef SANITIZER
  // Our zombies should not have been freed yet.
  assert(!queues_freed[0]);
  assert(!queues_freed[1]);
#endif // SANITIZER

  // Cull the zombies!
  queues[2] = em_proxying_queue_create();

#ifndef SANITIZER
  // Now they should be free.
  assert(queues_freed[0]);
  assert(queues_freed[1]);
  assert(!queues_freed[2]);
#endif // SANITIZER

  em_proxying_queue_destroy(queues[2]);

#ifndef SANITIZER
  // The new queue should have been immediately freed.
  assert(queues_freed[2]);
#endif // SANITIZER

  // Cull again, but this time there should be nothing to cull.
  queues[3] = em_proxying_queue_create();
  em_proxying_queue_destroy(queues[3]);

#ifndef SANITIZER
  // The new queue should have been immediately freed.
  assert(queues_freed[3]);
#endif // SANITIZER

  emscripten_console_log("done");
}
