/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "assert.h"
#include "pthread_impl.h"
#include <pthread.h>

extern int __pthread_join_js(pthread_t t, void **res, int tryjoin);
extern void __emscripten_thread_cleanup(pthread_t t);

static int __pthread_join_internal(pthread_t t, void **res) {
  if (t->self != t) {
    // attempt to join a thread which does not point to a valid thread, or does
    // not exist anymore.
    return ESRCH;
  }
  // TODO(sbc): IIUC __pthread_join_js currently doesn't handle the case
  // when the thread becomes detached/joined *during* the join.  This pre-check
  // can potentially be removed once it does.
  int state = t->detach_state;
  if (state == DT_DETACHED || state == DT_EXITED) {
    // The thread is detached or already joined, and therefore not joinable
    return EINVAL;
  }
  if (t == __pthread_self()) {
    // thread is attempting to join to itself.
    return EDEADLK;
  }
  int is_runtime_thread = emscripten_is_main_runtime_thread();
  while (1) {
    // The thread we are joining with must be either DT_JOINABLE or
    // DT_EXITING.  If its DT_EXITING then we move it to DT_EXITED and
    // we are done.   If its DT_JOINABLE we keep waiting.
    int old_state = a_cas(&t->detach_state, DT_EXITING, DT_EXITED);
    if (old_state == DT_EXITING) {
      // We successfully marked the tread as DT_EXITED
      if (res) *res = t->result;
      __emscripten_thread_cleanup(t);
      return 0;
    }
    assert(old_state == DT_JOINABLE);
    if (old_state != DT_JOINABLE) return EINVAL;

    __pthread_testcancel();
    // In main runtime thread (the thread that initialized the Emscripten C
    // runtime and launched main()), assist pthreads in performing operations
    // that they need to access the Emscripten main runtime for.
    if (is_runtime_thread) emscripten_main_thread_process_queued_calls();
    emscripten_futex_wait(&t->detach_state, old_state, is_runtime_thread ? 100 : 1);
  }
}

int __pthread_join(pthread_t t, void **res) {
  emscripten_check_blocking_allowed();
  return __pthread_join_internal(t, res);
}

// Taken directly from system/lib/libc/musl/src/thread/pthread_join.c
int __pthread_tryjoin_np(pthread_t t, void **res)
{
  return t->detach_state==DT_JOINABLE ? EBUSY : __pthread_join_internal(t, res);
}

weak_alias(__pthread_join, emscripten_builtin_pthread_join);
weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_join, pthread_join);
