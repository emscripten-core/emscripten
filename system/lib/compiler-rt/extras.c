/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Support functions for emscripten setjmp/longjmp and exception handling
 * support.
 * See: https://llvm.org/doxygen/WebAssemblyLowerEmscriptenEHSjLj_8cpp.html
 */

/* References to these globals are generated in the llvm backend so they
 * cannot be static */
int __THREW__ = 0;
int __threwValue = 0;

void setThrew(int threw, int value) {
  if (__THREW__ == 0) {
    __THREW__ = threw;
    __threwValue = value;
  }
}
