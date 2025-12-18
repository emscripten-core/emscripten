/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include "emscripten_internal.h"

typedef struct proxied_select_t {
  int n;
  void *rfds;
  void *wfds;
  void *efds;
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

int _emscripten_proxy_newselect(int n, void *rfds, void *wfds, void *efds, int64_t timeout) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  pthread_t target = emscripten_main_runtime_thread_id();
  proxied_select_t t = {.n = n, .rfds = rfds, .wfds = wfds, .efds = efds, .timeout = timeout};
  if (!emscripten_proxy_sync_with_ctx(q, target, call_newselect, &t)) {
    assert(false && "emscripten_proxy_sync failed");
    return -1;
  }
  return t.result;
}
