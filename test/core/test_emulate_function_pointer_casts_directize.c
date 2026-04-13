/*
 * Regression test: fpcast-emu must run before directize (inside -O2).
 * Otherwise directize sees the type-mismatched call_indirect (calling a
 * 1-arg function through a 2-arg pointer) and replaces it with unreachable.
 */
#include <stdio.h>

typedef void (*two_arg_fn)(void *, void *);

static void one_arg(void *p) { printf("OK\n"); }

static two_arg_fn hide_cast(void (*fn)(void *)) {
  two_arg_fn result = (two_arg_fn)fn;
  __asm__("" : "+r"(result));
  return result;
}

int main(void) {
  two_arg_fn fn = hide_cast(one_arg);
  fn((void *)1, (void *)2);
  return 0;
}
