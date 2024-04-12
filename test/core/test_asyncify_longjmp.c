/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void second(void) {
  printf("second\n");
  emscripten_sleep(1);
  longjmp(buf, -1);
}

void first(void) {
  printf("first\n");  // prints
  emscripten_sleep(1);
  longjmp(buf, 1);  // jumps back to where setjmp was called - making setjmp now
                    // return 1
}

int main() {
  volatile int x = 0;
  emscripten_sleep(1);
  int jmpval = setjmp(buf);
  if (!jmpval) {
    x++;                  // should be properly restored once longjmp jumps back
    first();              // when executed, setjmp returns 1
    printf("skipped\n");  // does not print
  } else if (jmpval == 1) {  // when first() jumps back, setjmp returns 1
    printf("result: %d %d\n", x, jmpval);  // prints
    x++;
    emscripten_sleep(1);
    second();                 // when executed, setjmp returns -1
    emscripten_sleep(1);
  } else if (jmpval == -1) {  // when second() jumps back, setjmp returns -1
    printf("result: %d %d\n", x, jmpval);  // prints
  }
  emscripten_sleep(1);
  assert(x == 2);
  return 0;
}
