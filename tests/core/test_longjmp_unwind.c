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
  assert(abs(get_stack() - (int)temp) >= MAJOR);
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
