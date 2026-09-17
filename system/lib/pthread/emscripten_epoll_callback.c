/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Backs emscripten_epoll_add_listener under PTHREADS: readiness lives on the
// main thread, the callback runs on the registering thread. Like
// _emscripten_run_callback_on_thread (html5/callback.c), but reports completion
// back to the main thread so it can pace the next delivery.

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

static void do_epoll_callback(void* arg) {
  epoll_callback_args_t* args = (epoll_callback_args_t*)arg;
  args->callback(args->userdata);
}

// On the main thread, after the delivery ran or its target thread went away.
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
