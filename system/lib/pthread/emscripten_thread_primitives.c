/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Low level threading primitives for lock, condvar and mutex, build on the
 * emscripten atomics API.
 */
#include <emscripten/threading.h>

#include "emscripten_internal.h"

void emscripten_lock_init(emscripten_lock_t *lock) {
  emscripten_atomic_store_u32((void*)lock, EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER);
}

bool emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds) {
  emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  if (!val) return true;
  int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6) + maxWaitNanoseconds;
  while (maxWaitNanoseconds > 0) {
    emscripten_atomic_wait_u32((int32_t*)lock, val, maxWaitNanoseconds);
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (!val) return true;
    maxWaitNanoseconds = waitEnd - (int64_t)(emscripten_performance_now() * 1e6);
  }
  return false;
}

void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock) {
  emscripten_lock_t val;
  do {
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (val) {
      emscripten_atomic_wait_u32((int32_t*)lock, val, ATOMICS_WAIT_DURATION_INFINITE);
    }
  } while (val);
}

bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds) {
  // TODO: we changed the performance_now calls to get_now, which can be applied
  // to the remaining code (since all calls defer to the best internal option).
  emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  if (!val) return true;

  double t = emscripten_get_now();
  double waitEnd = t + maxWaitMilliseconds;
  while (t < waitEnd) {
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (!val) return true;
    t = emscripten_get_now();
  }
  return false;
}

void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock) {
  emscripten_lock_t val;
  do {
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  } while (val);
}

bool emscripten_lock_try_acquire(emscripten_lock_t *lock) {
  emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  return !val;
}

void emscripten_lock_release(emscripten_lock_t *lock) {
  emscripten_atomic_store_u32((void*)lock, 0);
  emscripten_atomic_notify((int32_t*)lock, 1);
}

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num) {
  emscripten_atomic_store_u32((void*)sem, num);
}

int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num) {
  uint32_t val = num;
  for (;;) {
    uint32_t ret = emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    if (ret < num) return -1;
    val = ret;
  }
}

int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds) {
  int val = emscripten_atomic_load_u32((void*)sem);
  for (;;) {
    while (val < num) {
      // TODO: Shave off maxWaitNanoseconds
      ATOMICS_WAIT_RESULT_T waitResult = emscripten_atomic_wait_u32((int32_t*)sem, val, maxWaitNanoseconds);
      if (waitResult == ATOMICS_WAIT_TIMED_OUT) return -1;
      val = emscripten_atomic_load_u32((void*)sem);
    }
    int ret = (int)emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    val = ret;
  }
}

int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num) {
  int val = emscripten_atomic_load_u32((void*)sem);
  for (;;) {
    while (val < num) {
      emscripten_atomic_wait_u32((int32_t*)sem, val, ATOMICS_WAIT_DURATION_INFINITE);
      val = emscripten_atomic_load_u32((void*)sem);
    }
    int ret = (int)emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    val = ret;
  }
}

uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num) {
  uint32_t ret = emscripten_atomic_add_u32((void*)sem, num);
  emscripten_atomic_notify((int*)sem, num);
  return ret;
}

void emscripten_condvar_init(emscripten_condvar_t *condvar) {
  *condvar = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
}

void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock) {
  int val = emscripten_atomic_load_u32((void*)condvar);
  emscripten_lock_release(lock);
  emscripten_atomic_wait_u32((int32_t*)condvar, val, ATOMICS_WAIT_DURATION_INFINITE);
  emscripten_lock_waitinf_acquire(lock);
}

bool emscripten_condvar_wait(emscripten_condvar_t* condvar,
                             emscripten_lock_t* lock,
                             int64_t maxWaitNanoseconds) {
  int val = emscripten_atomic_load_u32((void*)condvar);
  emscripten_lock_release(lock);
  int waitValue = emscripten_atomic_wait_u32((int32_t*)condvar, val, maxWaitNanoseconds);
  if (waitValue == ATOMICS_WAIT_TIMED_OUT) {
    return false;
  }

  return emscripten_lock_wait_acquire(lock, maxWaitNanoseconds);
}

ATOMICS_WAIT_TOKEN_T emscripten_condvar_wait_async(emscripten_condvar_t *condvar,
                                                   emscripten_lock_t *lock,
                                                   emscripten_async_wait_callback_t asyncWaitFinished,
                                                   void *userData,
                                                   double maxWaitMilliseconds) {
  int val = emscripten_atomic_load_u32((void*)condvar);
  emscripten_lock_release(lock);
  return emscripten_atomic_wait_async((int32_t *)condvar, val, asyncWaitFinished, userData, maxWaitMilliseconds);
}

void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal) {
  emscripten_atomic_add_u32((void*)condvar, 1);
  emscripten_atomic_notify((int*)condvar, numWaitersToSignal);
}
