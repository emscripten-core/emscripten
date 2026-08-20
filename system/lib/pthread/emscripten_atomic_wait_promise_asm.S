/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Helper function for emscripten_atomic_wait_suspending.
 *
 * This function uses multi-value so it cannot be expressed in C code.  W
 * call out to the `_emscripten_atomic_wait_promise` JS library function which
 * returns a pair of [ Promise|undefined, result code ].   We then write the
 * result code to an out parameter and return the Promise as an externref
 */

#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.functype _emscripten_atomic_wait_promise (PTR, i32, f64) -> (externref, i32)

.globl _emscripten_atomic_wait_promise_asm
_emscripten_atomic_wait_promise_asm:
  .functype _emscripten_atomic_wait_promise_asm (PTR, i32, f64, PTR) -> (externref)
  .local i32
  local.get 0
  local.get 1
  local.get 2
  call _emscripten_atomic_wait_promise
  local.set 4
  local.get 3
  local.get 4
  i32.store 0
  end_function
