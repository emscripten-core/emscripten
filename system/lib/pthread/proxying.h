/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle to a set of thread-local work queues to which work can be
// asynchronously or synchronously proxied from other threads. When work is
// proxied to a queue on a particular thread, that thread is notified to start
// processing work from that queue if it is not already doing so.
typedef struct em_proxying_queue em_proxying_queue;

// Create and destroy proxying queues.
em_proxying_queue* em_proxying_queue_create();
void em_proxying_queue_destroy(em_proxying_queue* q);

// Get the queue used for proxying low-level runtime work. Work on this queue
// may be processed at any time inside system functions, so it must be
// nonblocking and safe to run at any time, similar to a native signal handler.
em_proxying_queue* emscripten_proxy_get_system_queue();

// Execute all the tasks enqueued for the current thread on the given queue.
void emscripten_proxy_execute_queue(em_proxying_queue* q);

// Opaque handle to a currently-executing proxied task, used to signal the end
// of the task.
typedef struct em_proxying_ctx em_proxying_ctx;

// Signal the end of a proxied task.
void emscripten_proxy_finish(em_proxying_ctx* ctx);

// Enqueue `func` on the given queue and thread and return immediately. Returns
// 1 if the work was successfully enqueued and the target thread notified or 0
// otherwise.
int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg);

// Enqueue `func` on the given queue and thread and wait for it to finish
// executing before returning. Returns 1 if the task was successfully completed
// and 0 otherwise.
int emscripten_proxy_sync(em_proxying_queue* q,
                          pthread_t target_thread,
                          void (*func)(void*),
                          void* arg);

// Enqueue `func` on the given queue and thread and wait for it to be executed
// and for the task to be marked finished with `emscripten_proxying_finish`
// before returning. Returns 1 if the task was successfully completed and 0
// otherwise.
int emscripten_proxy_sync_with_ctx(em_proxying_queue* q,
                                   pthread_t target_thread,
                                   void (*func)(em_proxying_ctx*, void*),
                                   void* arg);

#ifdef __cplusplus
}
#endif
