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

#define abs(x) ((x) < 0 ? (-x) : (x))

// A minor stack change is ignorable (e.g., printf)
#define MINOR 100
// A major stack change must not happen
#define MAJOR 1000

jmp_buf jb;

__attribute__((noinline)) int get_stack()
{
  int dummy;
  int ptr = (int)&dummy;
  return ptr;
}

__attribute__((noinline)) void bar()
{
  printf("before longjmp: %d\n", get_stack());
  longjmp(jb, 1);
}

volatile void* temp;

__attribute__((noinline)) void foo()
{
  temp = alloca(MAJOR);
  printf("major allocation at: %d\n", (int)temp);
#ifdef __asmjs__
  // asmjs stack grows up, but wasm backend stack grows down, so the delta
  // between get_stack() and temp isn't related to the size of the alloca for
  // wasm backend
  assert(abs(get_stack() - (int)temp) >= MAJOR);
#endif
  bar();
}

int main()
{
  int before = get_stack();
  printf("before setjmp: %d\n", before);

  if (!setjmp(jb)) {
    foo();
    return 0;
  } else {
    int after = get_stack();
    printf("before setjmp: %d\n", after);
    assert(abs(after - before) < MINOR); // stack has been unwound (but may have minor printf changes
    printf("ok.\n");
    return 1;
  }
}
