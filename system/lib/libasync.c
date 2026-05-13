/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/promise.h>
#include <emscripten/atomic.h>

int _emscripten_atomic_wait_promise(volatile void * _Nonnull addr,
                                    uint32_t value,
                                    double maxWaitMilliseconds);

ATOMICS_WAIT_TOKEN_T emscripten_atomic_wait_suspending(volatile void * _Nonnull addr,
                                                       uint32_t value,
                                                       double maxWaitMilliseconds) {
  int res = _emscripten_atomic_wait_promise(addr, value, maxWaitMilliseconds);
  // A native value is a synchronous result code.
  if (res < 0) {
     return (ATOMICS_WAIT_TOKEN_T)-res;
  }
  // Otherwise a positive value is a promise ID.
  em_promise_t promise = (em_promise_t)res;
  em_settled_result_t result = emscripten_promise_await(promise);
  if (result.result != EM_PROMISE_FULFILL) {
    __builtin_abort();
  }
  return (ATOMICS_WAIT_TOKEN_T)result.value;
}
