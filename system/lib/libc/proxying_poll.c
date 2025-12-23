/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <poll.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include "emscripten_internal.h"

#if _REENTRANT
typedef struct proxied_poll_t {
  struct pollfd* fds;
  int nfds;
  int timeout;
  int result;
} proxied_poll_t;

static void call_poll_js(em_proxying_ctx* ctx, void* arg) {
  proxied_poll_t* t = arg;
  _poll_js(t->fds, t->nfds, t->timeout, ctx, arg);
}

void _emscripten_proxy_poll_finish(em_proxying_ctx* ctx, void* arg, int ret) {
  proxied_poll_t* t = arg;
  t->result = ret;
  emscripten_proxy_finish(ctx);
}

static int proxy_poll(struct pollfd *fds, int nfds, int timeout) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  pthread_t target = emscripten_main_runtime_thread_id();
  proxied_poll_t t = {.fds = fds, .nfds = nfds, .timeout = timeout};
  if (!emscripten_proxy_sync_with_ctx(q, target, call_poll_js, &t)) {
    assert(false && "emscripten_proxy_sync failed");
    return -1;
  }
  return t.result;
}
#endif

// Marked as weak since libwasmfs needs to be able to override this.
weak int __syscall_poll(intptr_t _fds, int nfds, int timeout) {
  struct pollfd* fds = (struct pollfd*)_fds;
#if _REENTRANT
  if (!emscripten_is_main_runtime_thread()) {
    return proxy_poll(fds, nfds, timeout);
  }
#endif
  return _poll_js(fds, nfds, timeout, 0, 0);
}
