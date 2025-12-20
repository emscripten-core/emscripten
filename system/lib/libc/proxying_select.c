/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <sys/select.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include "emscripten_internal.h"

#if _REENTRANT
typedef struct proxied_select_t {
  int n;
  fd_set *rfds;
  fd_set *wfds;
  fd_set *efds;
  int64_t timeout;
  int result;
} proxied_select_t;

static void call_newselect(em_proxying_ctx* ctx, void* arg) {
  proxied_select_t* t = arg;
  _newselect_js(ctx, arg, t->n, t->rfds, t->wfds, t->efds, t->timeout);
}

void _emscripten_proxy_newselect_finish(em_proxying_ctx* ctx, void* arg, int ret) {
  proxied_select_t* t = arg;
  t->result = ret;
  emscripten_proxy_finish(ctx);
}

static int proxy_newselect(int n, fd_set *rfds, fd_set *wfds, fd_set *efds, int64_t timeout) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  pthread_t target = emscripten_main_runtime_thread_id();
  proxied_select_t t = {.n = n, .rfds = rfds, .wfds = wfds, .efds = efds, .timeout = timeout};
  if (!emscripten_proxy_sync_with_ctx(q, target, call_newselect, &t)) {
    assert(false && "emscripten_proxy_sync failed");
    return -1;
  }
  return t.result;
}
#endif

// Marked as weak since libwasmfs needs to be able to override this.
weak int __syscall__newselect(int nfds, intptr_t _readfds, intptr_t _writefds, intptr_t _exceptfds, int64_t timeout) {
  fd_set* readfds = (fd_set*)_readfds;
  fd_set* writefds = (fd_set*)_writefds;
  fd_set* exceptfds = (fd_set*)_exceptfds;
#if _REENTRANT
  if (!emscripten_is_main_runtime_thread()) {
    return proxy_newselect(nfds, readfds, writefds, exceptfds, timeout);
  }
#endif
  return _newselect_js(0, 0, nfds, readfds, writefds, exceptfds, timeout);
}
