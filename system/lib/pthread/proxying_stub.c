/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Stub implementation of the proxying queue API that does nothing, for use in
// single-threaded builds.

#include <stdlib.h>
#include <emscripten/proxying.h>

em_proxying_queue* em_proxying_queue_create() {
  return NULL;
}

void em_proxying_queue_destroy(em_proxying_queue* q) {}

em_proxying_queue* emscripten_proxy_get_system_queue() {
  return NULL;
}

void emscripten_proxy_execute_queue(em_proxying_queue* q) { abort(); }

void emscripten_proxy_finish(em_proxying_ctx* ctx) { abort(); }

int emscripten_proxy_async(em_proxying_queue* q,
                           pthread_t target_thread,
                           void (*func)(void*),
                           void* arg) {
  abort();
}

int emscripten_proxy_sync(em_proxying_queue* q,
                          pthread_t target_thread,
                          void (*func)(void*),
                          void* arg) {
  abort();
}

int emscripten_proxy_sync_with_ctx(em_proxying_queue* q,
                                   pthread_t target_thread,
                                   void (*func)(em_proxying_ctx*, void*),
                                   void* arg) {
  abort();
}
