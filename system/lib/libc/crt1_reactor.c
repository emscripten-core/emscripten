/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

extern void __wasm_call_ctors(void) __attribute__((weak));

void _initialize(void) {
  if (__wasm_call_ctors) {
    __wasm_call_ctors();
  }
}
