/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A thread blocked in emscripten_proxy_sync[_with_ctx] is canceled while its
 * work is (a) in progress on the target and (b) handed a ctx the target has
 * not finished yet. The caller must exit with PTHREAD_CANCELED, but only once
 * the target is done with the caller-owned argument (its stack), i.e. when the
 * work completes or the ctx is finished.
 */

#include <assert.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static em_proxying_queue* q;
static pthread_t target;
static _Atomic bool stop_target;

static void* target_main(void* arg) {
  while (!stop_target) {
    emscripten_proxy_execute_queue(q);
    usleep(1000);
  }
  return NULL;
}

// (a) Work in progress on the target when the caller is canceled. The target
// keeps reading the caller's stack argument until released, so the canceled
// caller must not have unwound yet.
static _Atomic bool slow_started, slow_release, slow_done;

static void slow(void* arg) {
  slow_started = true;
  while (!slow_release) {
    usleep(1000);
  }
  assert(*(int*)arg == 42 && "caller stack freed while target still reads it");
  slow_done = true;
}

static void* sync_caller(void* arg) {
  int local = 42;
  emscripten_proxy_sync(q, target, slow, &local);
  assert(false && "should have been canceled");
  return NULL;
}

// (b) The target returns from the proxied function without finishing the ctx;
// the canceled caller is held until the ctx is finished.
static em_proxying_ctx* _Atomic stashed;
static _Atomic bool ctx_caller_exiting;

static void stash(em_proxying_ctx* ctx, void* arg) { stashed = ctx; }

static void note_exit(void* arg) { ctx_caller_exiting = true; }

static void* ctx_caller(void* arg) {
  int local = 42;
  pthread_cleanup_push(note_exit, NULL);
  emscripten_proxy_sync_with_ctx(q, target, stash, &local);
  pthread_cleanup_pop(0);
  assert(false && "should have been canceled");
  return NULL;
}

static void finish_stashed(void* arg) { emscripten_proxy_finish(stashed); }

static void noop(void* arg) {}

int main(void) {
  q = em_proxying_queue_create();
  assert(pthread_create(&target, NULL, target_main, NULL) == 0);

  pthread_t caller;
  void* ret;

  assert(pthread_create(&caller, NULL, sync_caller, NULL) == 0);
  while (!slow_started)
    usleep(1000);
  assert(pthread_cancel(caller) == 0);
  usleep(20000); // the cancel must not complete while `slow` still runs
  assert(!slow_done);
  slow_release = true;
  assert(pthread_join(caller, &ret) == 0);
  assert(ret == PTHREAD_CANCELED);
  assert(slow_done);

  assert(pthread_create(&caller, NULL, ctx_caller, NULL) == 0);
  while (!stashed)
    usleep(1000);
  assert(pthread_cancel(caller) == 0);
  usleep(20000); // the cancel must not complete while the ctx is unfinished
  assert(!ctx_caller_exiting);
  assert(emscripten_proxy_async(q, target, finish_stashed, NULL));
  assert(pthread_join(caller, &ret) == 0);
  assert(ret == PTHREAD_CANCELED);
  assert(ctx_caller_exiting);

  // The queue is still healthy for ordinary work.
  assert(emscripten_proxy_sync(q, target, noop, NULL));

  stop_target = true;
  assert(pthread_join(target, NULL) == 0);
  em_proxying_queue_destroy(q);
  printf("done\n");
  return 0;
}
