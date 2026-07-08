/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Backs emscripten_epoll_set_callback under PTHREADS: the epoll readiness lives
// on the thread that owns the filesystem (the epoll syscalls are proxied
// there), but the user callback must run on the thread that registered it. This
// mirrors _emscripten_run_callback_on_thread in html5/callback.c, but carries
// the epoll callback's argument shape (epfd, events, nready, userdata) and,
// crucially, reports back to the registering thread when a delivery completes
// so it can pace the next one - a level-triggered fd stays ready until the
// callback drains it, so the FS thread must wait for that drain before
// re-deriving, or it would spin re-delivering the same readiness.

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include <emscripten/eventloop.h>
#include <emscripten/proxying.h>

#include "emscripten_internal.h"

typedef void (*em_epoll_callback)(int epfd,
                                  struct epoll_event* events,
                                  int nready,
                                  void* userdata);

typedef struct epoll_callback_args_t {
  em_epoll_callback callback;
  int epfd;
  int nready;
  void* userdata;
  int token;
  struct epoll_event events[];
} epoll_callback_args_t;

// Runs on the registering thread: deliver the ready set to the user callback.
static void do_epoll_callback(void* arg) {
  epoll_callback_args_t* args = (epoll_callback_args_t*)arg;
  args->callback(args->epfd, args->events, args->nready, args->userdata);
}

// Runs back on the FS-owning thread once the delivery above has finished (or
// was cancelled because the target thread went away): let the JS layer
// re-derive.
static void do_epoll_done(void* arg) {
  epoll_callback_args_t* args = (epoll_callback_args_t*)arg;
  _emscripten_epoll_delivery_done(args->token);
  free(arg);
}

void _emscripten_epoll_run_callback_on_thread(pthread_t t,
                                              em_epoll_callback callback,
                                              int epfd,
                                              struct epoll_event* events,
                                              int nready,
                                              void* userdata,
                                              int token) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  // Copy the events out synchronously so the caller's (reused) buffer is free
  // again the moment this returns; freed once the delivery completes.
  size_t bytes = nready * sizeof(struct epoll_event);
  epoll_callback_args_t* args = malloc(sizeof(epoll_callback_args_t) + bytes);
  args->callback = callback;
  args->epfd = epfd;
  args->nready = nready;
  args->userdata = userdata;
  args->token = token;
  memcpy(args->events, events, bytes);

  if (!emscripten_proxy_callback(
        q, t, do_epoll_callback, do_epoll_done, do_epoll_done, args)) {
    assert(false && "emscripten_proxy_callback failed");
  }
}

// Runs on the owning thread: adjust its (thread-local) runtime keepalive so the
// epoll callback holds the thread it was registered on, not the FS thread.
static void do_epoll_keepalive(void* arg) {
  if ((intptr_t)arg > 0) {
    emscripten_runtime_keepalive_push();
  } else {
    emscripten_runtime_keepalive_pop();
  }
}

void _emscripten_epoll_keepalive_on_thread(pthread_t t, int delta) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  if (!emscripten_proxy_async(
        q, t, do_epoll_keepalive, (void*)(intptr_t)delta)) {
    assert(false && "emscripten_proxy_async failed");
  }
}
