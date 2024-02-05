/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

#include <emscripten/stack.h>

#define abs(x) ((x) < 0 ? (-x) : (x))

// A minor stack change is ignorable (e.g., printf)
#define MINOR 100
// A major stack change must not happen
#define MAJOR 1000

jmp_buf jb;

__attribute__((noinline)) void bar()
{
  printf("before longjmp: %ld\n", emscripten_stack_get_current());
  longjmp(jb, 1);
}

volatile void* temp;

__attribute__((noinline)) void foo()
{
  temp = alloca(MAJOR);
  printf("major allocation at: %ld\n", (long)temp);
  assert(abs(emscripten_stack_get_current() - (long)temp) >= MAJOR);
  bar();
}

int main()
{
  int before = emscripten_stack_get_current();
  printf("before setjmp: %d\n", before);

  if (!setjmp(jb)) {
    foo();
    return 0;
  } else {
    int after = emscripten_stack_get_current();
    printf("before setjmp: %d\n", after);
    assert(abs(after - before) < MINOR); // stack has been unwound (but may have minor printf changes
    printf("ok.\n");
    return 1;
  }
}
