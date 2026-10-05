/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdlib.h>

#include <emscripten/promise.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include "emscripten_internal.h"

// The descriptor table lives on the main thread, so the wait registers there;
// the promise lives on the calling thread, so when the two differ the result
// comes back through a proxying ctx, landing when that thread runs its queue.
typedef struct fd_wait {
  em_promise_t promise;
  int fd;
  int events;
  int revents;
  em_proxying_ctx* ctx;
} fd_wait;

// _emscripten_fd_wait_js (emscripten_internal.h) registers on the fd's
// wait-queue (main thread), calling _emscripten_fd_wait_done once any
// requested event is ready.

static void settle(void* arg) {
  fd_wait* w = arg;
  emscripten_promise_resolve(
    w->promise, EM_PROMISE_FULFILL, (void*)(intptr_t)w->revents);
  emscripten_promise_destroy(w->promise);
  free(w);
}

void _emscripten_fd_wait_done(fd_wait* w, int revents) {
  w->revents = revents;
#ifdef _REENTRANT
  if (w->ctx) {
    emscripten_proxy_finish(w->ctx);
    return;
  }
#endif
  settle(w);
}

#ifdef _REENTRANT
static void start(em_proxying_ctx* ctx, void* arg) {
  fd_wait* w = arg;
  w->ctx = ctx;
  _emscripten_fd_wait_js(w, w->fd, w->events);
}

static void cancel(void* arg) {
  fd_wait* w = arg;
  emscripten_promise_resolve(w->promise, EM_PROMISE_REJECT, NULL);
  emscripten_promise_destroy(w->promise);
  free(w);
}
#endif

em_promise_t emscripten_fd_promise(int fd, int events) {
  em_promise_t promise = emscripten_promise_create();
  fd_wait* w = malloc(sizeof(*w));
  if (!w) {
    emscripten_promise_resolve(promise, EM_PROMISE_REJECT, NULL);
    return promise;
  }
  *w = (fd_wait){.promise = promise, .fd = fd, .events = events};
  // Hand out a promise resolved to the internal one, which stays alive until
  // settled regardless of when the caller destroys theirs.
  em_promise_t ret = emscripten_promise_create();
  emscripten_promise_resolve(ret, EM_PROMISE_MATCH, promise);
#ifdef _REENTRANT
  if (!emscripten_is_main_runtime_thread()) {
    if (!emscripten_proxy_callback_with_ctx(emscripten_proxy_get_system_queue(),
                                            emscripten_main_runtime_thread_id(),
                                            start, settle, cancel, w)) {
      cancel(w);
    }
    return ret;
  }
#endif
  _emscripten_fd_wait_js(w, fd, events);
  return ret;
}
