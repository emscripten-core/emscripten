/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

#include "em_task_queue.h"
#include "pthread_impl.h"
#include "thread_mailbox.h"
#include "threading_internal.h"

int emscripten_thread_mailbox_ref(pthread_t thread) {
  // Attempt to increment the refcount, being careful not to increment it if we
  // ever observe a 0.
  int prev_count = thread->mailbox_refcount;
  while (1) {
    if (prev_count == 0) {
      // The mailbox is already closed!
      return 0;
    }
    int desired_count = prev_count + 1;
    if (atomic_compare_exchange_weak(
          &thread->mailbox_refcount, &prev_count, desired_count)) {
      return 1;
    }
  }
}

// Decrement and return the refcount.
void emscripten_thread_mailbox_unref(pthread_t thread) {
  int new_count = atomic_fetch_sub(&thread->mailbox_refcount, 1) - 1;
  assert(new_count >= 0);
  if (new_count == 0) {
    // The count is now zero. The thread that owns this queue may be waiting to
    // shut down. Notify the thread that it is safe to proceed now that the
    // mailbox is closed.
    __builtin_wasm_memory_atomic_notify((int*)&thread->mailbox_refcount, -1);
  }
}

// Defined in emscripten_thread_state.S.
int _emscripten_thread_supports_atomics_wait(void);

void _emscripten_thread_mailbox_shutdown(pthread_t thread) {
  assert(thread == pthread_self());

  // Decrement the refcount and wait for it to reach zero.
  assert(thread->mailbox_refcount > 0);
  int count = atomic_fetch_sub(&thread->mailbox_refcount, 1) - 1;

  while (count != 0) {
    // Wait if possible and otherwise spin.
    if (_emscripten_thread_supports_atomics_wait() &&
        __builtin_wasm_memory_atomic_wait32(
          (int*)&thread->mailbox_refcount, count, -1) == 0) {
      break;
    }
    count = thread->mailbox_refcount;
  }
  // TODO: Cancel tasks.

  // The mailbox will not be accessed again after this point.
  em_task_queue_destroy(thread->mailbox);
}

void _emscripten_thread_mailbox_init(pthread_t thread) {
  thread->mailbox = em_task_queue_create(thread);
  thread->mailbox_refcount = 1;
}

// Exported for use in worker.js, but otherwise an internal function.
EMSCRIPTEN_KEEPALIVE
void _emscripten_check_mailbox() {
  // Before we attempt to execute a request from another thread make sure we
  // are in sync with all the loaded code.
  // For example, in PROXY_TO_PTHREAD the atexit functions are called via
  // a proxied call, and without this call to syncronize we would crash if
  // any atexit functions were registered from a side module.
  assert(pthread_self());
  em_task_queue* mailbox = pthread_self()->mailbox;
  mailbox->notification = NOTIFICATION_RECEIVED;
  em_task_queue_execute(pthread_self()->mailbox);
  notification_state expected = NOTIFICATION_RECEIVED;
  atomic_compare_exchange_strong(
    &mailbox->notification, &expected, NOTIFICATION_NONE);
}

// Send a postMessage notification telling the target thread to check its
// mailbox when it returns to its event loop. Pass in the current thread and
// main thread ids to minimize calls back into Wasm.
void _emscripten_notify_mailbox(pthread_t target_thread,
                                pthread_t curr_thread,
                                pthread_t main_thread);

void emscripten_thread_mailbox_send(pthread_t thread, task t) {
  assert(thread->mailbox_refcount > 0);

  pthread_mutex_lock(&thread->mailbox->mutex);
  if (!em_task_queue_enqueue(thread->mailbox, t)) {
    assert(0 && "No way to correctly recover from allocation failure");
  }
  pthread_mutex_unlock(&thread->mailbox->mutex);

  // If there is no pending notification for this mailbox, create one. If an old
  // notification is currently being processed, it may or may not execute the
  // new work. In case it does not, the new notification will ensure the work is
  // still executed.
  notification_state previous =
    atomic_exchange(&thread->mailbox->notification, NOTIFICATION_PENDING);
  if (previous != NOTIFICATION_PENDING) {
    _emscripten_notify_mailbox(thread,
                               pthread_self(),
                               emscripten_main_runtime_thread_id());
  }
}
