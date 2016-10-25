#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

extern "C" {
int get_int() { return 5; }
float get_float() { return 3.14; }
const char *get_string() { return "hello world"; }
void print_int(int x) { printf("%d\n", x); }
void print_float(float x) { printf("%.2f\n", x); }
void print_string(char *x) { printf("%s\n", x); }
int multi(int x, float y, int z, char *str) {
  if (x) puts(str);
  return (x + y) * z;
}
int *pointer(int *in) {
  printf("%d\n", *in);
  static int ret = 21;
  return &ret;
}

// This test checks that ccall restores the stack correctly to whatever it was
// when ccall was entered.  We save the stack pointer on the heap in stackChecker
// and verify that ccall restores it correctly.
struct test_struct {
  int arg1, arg2, arg3;
};
static int* stackChecker = 0;
int get_stack() { int i; return (int)&i; }
int uses_stack(test_struct* t1) {
  if (stackChecker == 0) stackChecker = (int*)malloc(sizeof(int));
  *stackChecker = get_stack();
  EM_ASM(Module['ccall']('get_int', 'number'));
  printf("stack is %s.\n", *stackChecker == get_stack() ? "ok" : "messed up");
  return 0;
}
void call_ccall_again() {
  test_struct t1 = { 1, 2, 3 };
  uses_stack(&t1);
}
}

