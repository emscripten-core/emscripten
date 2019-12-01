/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

jmp_buf jb;

__attribute__((noinline)) void foo(int64_t x) {
  printf("foo: %" PRId64 ".\n", x);
  longjmp(jb, 1);
}

__attribute__((noinline)) int64_t bar() {
  return (uint64_t)-2;
}

int main()
{
  if (!setjmp(jb)) {
    foo((uint64_t)-1);
    return 0;
  } else {
    printf("bar: %" PRId64 ".\n", bar());
    return 1;
  }
}
