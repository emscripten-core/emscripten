#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>

// Test that task_queues destroyed with pending notifications are added to the
// zombie list and are culled when a new task queue is allocated after their
// notifications have been cleared.

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

_Atomic bool worker_started = false;
_Atomic bool should_execute = false;
_Atomic int executed[2] = {};

#define EVENT_LOOP_TURNS 2

// Delay culling until the event loop has turned enough for the notifications on
// the two zombie task queues to have been received and cleared, allowing them
// to be freed.
void increment_flag(void* arg) {
  _Atomic int* flag = arg;
  if (atomic_fetch_add(flag, 1) + 1 < EVENT_LOOP_TURNS) {
    emscripten_async_call(increment_flag, arg, 0);
  }
}

void task(void* arg) { emscripten_async_call(increment_flag, arg, 0); }

void* execute_and_free_queue(void* arg) {
  // Signal the main thread to proxy work to us.
  worker_started = true;

  // Wait until we are signaled to execute the queue.
  while (!should_execute) {
    sched_yield();
  }

  // Execute the proxied work then free the empty queues.
  for (int i = 0; i < 2; i++) {
    emscripten_proxy_execute_queue(queues[i]);
    em_proxying_queue_destroy(queues[i]);
  }

  // Exit with a live runtime so the queued work notification can be received
  // and cleared, allowing the zombie task queues to be culled.
  emscripten_exit_with_live_runtime();
}

void nop(void* arg) {}

int main() {
  emscripten_console_log("start");
  for (int i = 0; i < 2; i++) {
    queues[i] = em_proxying_queue_create();
    assert(queues[i]);
  }

  // Create the worker and wait for it to enter Wasm.
  pthread_t worker;
  pthread_create(&worker, NULL, execute_and_free_queue, NULL);
  while (!worker_started) {
    sched_yield();
  }

  // Now that the worker has started, proxy work to it. This will allocate task
  // queues and send the worker notifications. The worker will execute the tasks
  // and destroy the proxy queues before returning to the event loop to receive
  // the notifications, so the task queues will end up on the zombie list.
  for (int i = 0; i < 2; i++) {
    emscripten_proxy_async(queues[i], worker, task, &executed[i]);
  }
  should_execute = true;

  // Wait for the queues to be destroyed and for the worker event loop to turn
  // enough to clear the notifications
  while (executed[0] < EVENT_LOOP_TURNS || executed[1] < EVENT_LOOP_TURNS) {
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
  // For each of the two culled task queues, the queue itself and its task
  // vector should have been freed.
  assert(frees_after_cull - frees_before_cull == 4);
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
