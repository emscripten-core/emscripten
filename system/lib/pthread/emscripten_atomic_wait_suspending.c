/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/promise.h>
#include <emscripten/atomic.h>
#include "threading_internal.h"

ATOMICS_WAIT_TOKEN_T emscripten_atomic_wait_suspending(volatile void * _Nonnull addr,
                                                       uint32_t value,
                                                       double maxWaitMilliseconds) {
  intptr_t res = _emscripten_atomic_wait_promise(addr, value, maxWaitMilliseconds);
  // A negative value is a synchronous result code.
  if (res < 0) {
     return (ATOMICS_WAIT_TOKEN_T)-res;
  }
  // Otherwise a positive value is a promise ID, and we can then `await` using
  // ASYNCIFY/JSPI.
  em_promise_t promise = (em_promise_t)res;
  void* await_result = emscripten_promise_await_unchecked(promise);
  return (ATOMICS_WAIT_TOKEN_T)(intptr_t)await_result;
}
