/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Much of this code comes from:
// https://github.com/CraneStation/wasi-libc/blob/master/libc-bottom-half/crt/crt1.c
// Converted malloc() calls to alloca() to avoid including malloc in all programs.

#include <alloca.h>
#include <stdlib.h>
#include <sysexits.h>
#include <wasi/wasi.h>
#include <stdio.h>

extern void __wasm_call_ctors(void) __attribute__((weak));

// TODO(sbc): We shouldn't even link this file if there is no main:
// https://github.com/emscripten-core/emscripten/issues/9640
extern int __original_main(void) __attribute__((weak));
extern int main(int argc, char** argv) __attribute__((weak));

void _start(void) {
  if (__wasm_call_ctors) {
    __wasm_call_ctors();
  }

  if (!main) {
    return;
  }

  int r = __original_main();

  /* If main exited successfully, just return, otherwise call _Exit.
   * TODO(sbc): switch to _Exit */
  if (r != 0) {
    __wasi_proc_exit(r);
  }
}
