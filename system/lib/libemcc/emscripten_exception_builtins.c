/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Support functions for emscripten setjmp/longjmp and exception handling
 * support. References to the things below are generated in the LLVM backend.
 * See: https://llvm.org/doxygen/WebAssemblyLowerEmscriptenEHSjLj_8cpp.html
 */

#include <stdint.h>
#include <threads.h>

#include "emscripten_internal.h"

thread_local uintptr_t __THREW__ = 0;
thread_local int __threwValue = 0;

void setThrew(uintptr_t threw, int value) {
  if (__THREW__ == 0) {
    __THREW__ = threw;
    __threwValue = value;
  }
}
