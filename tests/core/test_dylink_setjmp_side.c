/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

static jmp_buf buf;

void first() {
  printf("first\n");  // prints
  longjmp(buf, 1);  // jumps back to where setjmp was called - making setjmp now
                    // return 1
}

void second() {
  printf("second\n");
  longjmp(buf, 2);
}

void side() {
  int jmpval = setjmp(buf);
  if (!jmpval) {
    first(); // when executed, setjmp returns 1
    abort(); // should never get here
  } else if (jmpval == 1) {  // when first() jumps back, setjmp returns 1
    second();                // when executed, setjmp returns -1
  } else if (jmpval == 2) {  // when second() jumps back, setjmp returns -1
    return;
  }

  // should never get here
  abort();
}
