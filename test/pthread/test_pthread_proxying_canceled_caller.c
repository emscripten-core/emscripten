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
 * work completes or the ctx is finished, or (c) the task releases the argument
 * with emscripten_proxy_release_arg, letting the canceled caller exit early,
 * after which emscripten_proxy_acquire_arg must fail.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

em_proxying_queue* q;
pthread_t target;

// The target runs its event loop, which is where proxied work is dispatched.
void* target_main(void* arg) { emscripten_exit_with_live_runtime(); }

void stop_target(void* arg) { emscripten_runtime_keepalive_pop(); }

// (a) Work in progress on the target when the caller is canceled. The target
// keeps reading the caller's stack argument until released, so the canceled
// caller must not have unwound yet.
_Atomic bool slow_started, slow_release, slow_done;

void slow(void* arg) {
  slow_started = true;
  while (!slow_release) {
    usleep(1000);
  }
  assert(*(int*)arg == 42 && "caller stack freed while target still reads it");
  slow_done = true;
}

void* sync_caller(void* arg) {
  int local = 42;
  emscripten_proxy_sync(q, target, slow, &local);
  assert(false && "should have been canceled");
  return NULL;
}

// (b) The target returns from the proxied function without finishing the ctx;
// the canceled caller is held until the ctx is finished.
em_proxying_ctx* _Atomic stashed;
_Atomic bool ctx_caller_exiting;

void stash(em_proxying_ctx* ctx, void* arg) { stashed = ctx; }

void note_exit(void* arg) { ctx_caller_exiting = true; }

void* ctx_caller(void* arg) {
  int local = 42;
  pthread_cleanup_push(note_exit, NULL);
  emscripten_proxy_sync_with_ctx(q, target, stash, &local);
  pthread_cleanup_pop(0);
  assert(false && "should have been canceled");
  return NULL;
}

void finish_stashed(void* arg) { emscripten_proxy_finish(stashed); }

// (c) The task releases `arg` early: a canceled caller may then exit before
// the task is finished, after which `arg` can no longer be reacquired.
em_proxying_ctx* _Atomic released;

void release_early(em_proxying_ctx* ctx, void* arg) {
  // The caller is still waiting, so the arg can be acquired and accessed.
  assert(emscripten_proxy_acquire_arg(ctx));
  assert(*(int*)arg == 42);
  emscripten_proxy_release_arg(ctx);
  released = ctx;
}

void* release_caller(void* arg) {
  int local = 42;
  emscripten_proxy_sync_with_ctx(q, target, release_early, &local);
  assert(false && "should have been canceled");
  return NULL;
}

void finish_released(void* arg) {
  // The canceled caller has exited, so the arg can no longer be acquired.
  assert(!emscripten_proxy_acquire_arg(released));
  emscripten_proxy_finish(released);
}

void noop(void* arg) {}

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

  assert(pthread_create(&caller, NULL, release_caller, NULL) == 0);
  while (!released)
    usleep(1000);
  assert(pthread_cancel(caller) == 0);
  // The caller can exit even though the ctx is not yet finished.
  assert(pthread_join(caller, &ret) == 0);
  assert(ret == PTHREAD_CANCELED);
  assert(emscripten_proxy_async(q, target, finish_released, NULL));

  // The queue is still healthy for ordinary work.
  assert(emscripten_proxy_sync(q, target, noop, NULL));

  assert(emscripten_proxy_async(q, target, stop_target, NULL));
  assert(pthread_join(target, NULL) == 0);
  em_proxying_queue_destroy(q);
  printf("done\n");
  return 0;
}
