/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "pthread_impl.h"
#include <pthread.h>

extern int __pthread_join_js(pthread_t t, void **res, int tryjoin);

static int __pthread_join_internal(pthread_t t, void **res, int tryjoin) {
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
  return __pthread_join_js(t, res, tryjoin);
}

int __pthread_join(pthread_t t, void **res) {
  return __pthread_join_internal(t, res, 0);
}

// Taken directly from system/lib/libc/musl/src/thread/pthread_join.c
int __pthread_tryjoin_np(pthread_t t, void **res)
{
  return t->detach_state==DT_JOINABLE ? EBUSY : __pthread_join_internal(t, res, 1);
}

weak_alias(__pthread_join, emscripten_builtin_pthread_join);
weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_join, pthread_join);
