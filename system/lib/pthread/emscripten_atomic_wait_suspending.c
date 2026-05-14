/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/atomic.h>
#include <emscripten/console.h>
#include <emscripten/promise.h>
#include "threading_internal.h"

// Internal promise-returning API used to implement
// emscripten_atomic_wait_syspending.
__externref_t _emscripten_atomic_wait_promise_asm(volatile void *addr,
                                                  uint32_t value,
                                                  double maxWaitMilliseconds,
                                                  int32_t* result);

ATOMICS_WAIT_TOKEN_T emscripten_atomic_wait_suspending(volatile void * _Nonnull addr,
                                                       uint32_t value,
                                                       double maxWaitMilliseconds) {
  int32_t result = 42;
  __externref_t promise = _emscripten_atomic_wait_promise_asm(addr, value, maxWaitMilliseconds, &result);
  // A negative value is a synchronous result code.
  if (result != -1) {
    return (ATOMICS_WAIT_TOKEN_T)result;
  }
  // Otherwise a positive value is a promise ID, and we can then `await` using
  // ASYNCIFY/JSPI.
  void* await_result = emscripten_promise_ref_await_unchecked(promise);
  return (ATOMICS_WAIT_TOKEN_T)(intptr_t)await_result;
}
