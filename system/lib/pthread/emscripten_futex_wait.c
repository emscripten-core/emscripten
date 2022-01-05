/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <emscripten/threading.h>
#include "threading_internal.h"

int _emscripten_thread_supports_atomics_wait(void);
int _emscripten_futex_wait_non_blocking(volatile void *addr, uint32_t val, double max_wait_ms);

int emscripten_futex_wait(volatile void *addr, uint32_t val, double max_wait_ms) {
  if ((((intptr_t)addr)&3) != 0) {
    return -EINVAL;
  }

  int ret;
  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_WAITFUTEX);

  // For threads that cannot block (i.e. the main browser thread) we can't use
  // __builtin_wasm_memory_atomic_wait32 so we call out the JS function that
  // will busy wait.
  if (!_emscripten_thread_supports_atomics_wait()) {
    ret = _emscripten_futex_wait_non_blocking(addr, val, max_wait_ms);
    emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);
    return ret;
  }

  // -1 (or any negative number) means wait indefinitely.
  int64_t max_wait_ns = -1;
  if (max_wait_ms != INFINITY) {
    max_wait_ns = (int64_t)(max_wait_ms*1000*1000);
  }
  ret = __builtin_wasm_memory_atomic_wait32((int*)addr, val, max_wait_ns);
  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);

  // memory.atomic.wait32 returns:
  //   0 => "ok", woken by another agent.
  //   1 => "not-equal", loaded value != expected value
  //   2 => "timed-out", the timeout expired
  if (ret == 1) {
    return -EWOULDBLOCK;
  }
  if (ret == 2) {
    return -ETIMEDOUT;
  }
  assert(ret == 0);
  return 0;
}
