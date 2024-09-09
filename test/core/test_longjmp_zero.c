/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <setjmp.h>

int main() {
  printf("start\n");
  jmp_buf b1;
  int val = setjmp(b1);
  if (val) {
    assert(val == 1);
    printf("success\n");
    return 0;
  }
  /*
   * C standard:
   * > The longjmp function cannot cause the setjmp macro to return
   * > the value 0; if val is 0, the setjmp macro returns the value 1.
   */
  printf("longjmp\n");
  longjmp(b1, 0);
  __builtin_trap();
  return 0;
}
