/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <wasi/api.h>

__attribute__((__weak__)) void __wasm_call_ctors(void);

int __main_void(void);

void _start(void) {
  if (__wasm_call_ctors) {
    __wasm_call_ctors();
  }

  /*
   * Will either end up calling the user's original zero argument main directly
   * or our __original_main fallback in __original_main.c which handles
   * populating argv.
   */
  int r = __main_void();

  exit(r);
}
