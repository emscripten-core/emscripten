/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Backs emscripten_epoll_add_listener under PTHREADS: the epoll readiness lives
// on the main thread (the epoll syscalls are proxied there), but the user
// callback must run on the thread that registered it. This mirrors
// _emscripten_run_callback_on_thread in html5/callback.c, but reports back to
// the main thread when a delivery completes so it can pace the next one - the
// callback collects the ready events (via a proxied epoll_wait) itself, so the
// main thread must wait for that before firing again, or it would spin
// re-signalling the same still-ready level fd.

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <emscripten/epoll.h>
#include <emscripten/proxying.h>

#include "emscripten_internal.h"

typedef struct epoll_callback_args_t {
  em_epoll_callback callback;
  void* userdata;
  int token;
} epoll_callback_args_t;

// Runs on the registering thread: signal the user callback that events are
// pending (it collects them itself via epoll_wait).
static void do_epoll_callback(void* arg) {
  epoll_callback_args_t* args = (epoll_callback_args_t*)arg;
  args->callback(args->userdata);
}

// Runs back on the main thread once the delivery above has finished (or was
// cancelled because the target thread went away): let the JS layer re-derive.
static void do_epoll_done(void* arg) {
  epoll_callback_args_t* args = (epoll_callback_args_t*)arg;
  _emscripten_epoll_delivery_done(args->token);
  free(arg);
}

// Returns false if the target thread no longer exists.
bool _emscripten_epoll_run_callback_on_thread(pthread_t t,
                                              em_epoll_callback callback,
                                              void* userdata,
                                              int token) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  epoll_callback_args_t* args = malloc(sizeof(epoll_callback_args_t));
  args->callback = callback;
  args->userdata = userdata;
  args->token = token;

  if (!emscripten_proxy_callback(
        q, t, do_epoll_callback, do_epoll_done, do_epoll_done, args)) {
    free(args);
    return false;
  }
  return true;
}
