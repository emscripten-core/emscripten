// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <emscripten/console.h>
#include <emscripten/heap.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

em_proxying_queue* user_queue = NULL;
_Thread_local bool intercept_malloc = false;
_Thread_local bool wait_in_malloc = false;
_Thread_local bool wait_for_worker_in_malloc = false;
_Atomic int in_malloc = 0;
_Atomic int worker_proxied = 0;
_Atomic int task_done = 0;

// In the actual implementation of malloc, the system proxying queue may be
// executed non-deterministically if malloc contends on its mutex and calls
// emscripten_futex_wait -> _emscripten_yield. This wraps malloc and realloc to
// make that behavior deterministic.
void maybe_execute_queues(void) {
  if (wait_in_malloc) {
    wait_in_malloc = false;
    int dummy = 0;
    emscripten_futex_wait(&dummy, 0, INFINITY);
  } else if (wait_for_worker_in_malloc) {
    wait_for_worker_in_malloc = false;
    in_malloc = 1;
    while (!task_done) {
      emscripten_futex_wait(&task_done, 0, INFINITY);
    }
  } else if (intercept_malloc && emscripten_is_main_runtime_thread()) {
    emscripten_proxy_execute_queue(emscripten_proxy_get_system_queue());
  }
}

void* malloc(size_t size) {
  maybe_execute_queues();
  return emscripten_builtin_malloc(size);
}

void* realloc(void* ptr, size_t size) {
  maybe_execute_queues();
  return emscripten_builtin_realloc(ptr, size);
}

void task(void* arg) {
  emscripten_out("task\n");
  task_done = 1;
  emscripten_futex_wake(&task_done, 1);
}

void nop(void* arg) {}

void execute_user_queue(void* arg) {
  emscripten_proxy_execute_queue(user_queue);
}

void* user_queue_worker_main(void* arg) {
  while (!in_malloc) {
  }
  emscripten_proxy_execute_queue(user_queue);
  task_done = 1;
  emscripten_futex_wake(&task_done, 1);
  return NULL;
}

void* lost_wakeup_worker_main(void* arg) {
  pthread_t main_thread = *(pthread_t*)arg;
  // Enqueue a task for the main thread on the system queue. This also calls
  // _emscripten_thread_notify(main_thread), setting NOTIFY_BIT on the main
  // thread's wait_addr.
  emscripten_proxy_async(
    emscripten_proxy_get_system_queue(), main_thread, task, NULL);
  worker_proxied = 1;
  while (!task_done) {
    emscripten_futex_wait(&task_done, 0, INFINITY);
  }
  return NULL;
}

int main() {
#if defined(TEST_SYSTEM_QUEUE)
  // Tests for a deadlock scenario that can occur when sending a task on the
  // system queue:
  // 1. Sending a task locks the queue.
  // 2. Allocating a new task queue calls malloc/realloc.
  // 3. Malloc then attempts to execute and lock the already-locked queue,
  //    causing a deadlock.
  intercept_malloc = true;
  emscripten_proxy_async(
    emscripten_proxy_get_system_queue(), pthread_self(), task, NULL);
  intercept_malloc = false;
#elif defined(TEST_USER_QUEUE)
  // Tests for a same-thread re-entrant lock on a user queue via a task on the
  // system queue:
  // 1. A task that executes `user_queue` is pending on the system queue.
  // 2. Sending a task on `user_queue` locks `user_queue->mutex` and allocates.
  // 3. Malloc executes the system queue, which runs `execute_user_queue` and
  //    attempts to lock `user_queue->mutex` on the same thread.
  user_queue = em_proxying_queue_create();
  assert(user_queue != NULL);
  emscripten_proxy_async(emscripten_proxy_get_system_queue(),
                         pthread_self(),
                         execute_user_queue,
                         NULL);
  intercept_malloc = true;
  emscripten_proxy_async(user_queue, pthread_self(), task, NULL);
  intercept_malloc = false;
  em_proxying_queue_destroy(user_queue);
#elif defined(TEST_USER_QUEUE_CONTENDED)
  // Tests for a cross-thread deadlock when `user_queue->mutex` is held across a
  // blocking allocation:
  // 1. Main thread sends a task on `user_queue` and blocks inside malloc
  //    waiting for `worker`.
  // 2. Worker calls `emscripten_proxy_execute_queue(user_queue)` before
  //    unblocking the main thread, which deadlocks if the main thread is
  //    holding `user_queue->mutex` across malloc.
  user_queue = em_proxying_queue_create();
  assert(user_queue != NULL);
  pthread_t worker;
  int rc = pthread_create(&worker, NULL, user_queue_worker_main, NULL);
  assert(rc == 0);
  wait_for_worker_in_malloc = true;
  emscripten_proxy_async(user_queue, worker, nop, NULL);
  pthread_join(worker, NULL);
  em_proxying_queue_destroy(user_queue);
#elif defined(TEST_LOST_WAKEUP)
  // Tests that a contended allocation while adding a new task queue in do_proxy
  // does not consume NOTIFY_BIT while skipping execution of the system queue,
  // which would lose the wakeup and cause pthread_join to deadlock:
  // 1. Worker proxies `task` to the main thread (setting NOTIFY_BIT on the main
  //    thread's wait_addr) and waits for `task` to finish before exiting.
  // 2. Main thread proxies `nop` to the worker, allocating a new task queue.
  // 3. That allocation simulates mutex contention by calling
  //    emscripten_futex_wait, which consumes NOTIFY_BIT and calls
  //    _emscripten_yield -> emscripten_proxy_execute_queue.
  // 4. If emscripten_proxy_execute_queue is skipped because the system queue is
  //    being added to, `task` is not executed and NOTIFY_BIT is lost, so
  //    pthread_join blocks forever waiting for the worker.
  pthread_t main_thread = pthread_self();
  pthread_t worker;
  int rc = pthread_create(&worker, NULL, lost_wakeup_worker_main, &main_thread);
  assert(rc == 0);
  // Spin rather than using pthread_barrier_wait or emscripten_futex_wait so we
  // do not call _emscripten_yield and execute the system queue (consuming
  // NOTIFY_BIT) before calling emscripten_proxy_async below.
  while (!worker_proxied) {
  }
  wait_in_malloc = true;
  emscripten_proxy_async(emscripten_proxy_get_system_queue(), worker, nop, NULL);
  pthread_join(worker, NULL);
#else
#error "Must define a test case"
#endif
  emscripten_out("done\n");
}
