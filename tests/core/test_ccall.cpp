// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <emscripten.h>

extern "C" {
int get_int() { return 5; }
float get_float() { return 3.14; }
bool get_bool() { return true; }
const char *get_string() { return "hello world"; }
void print_int(int x) { printf("%d\n", x); }
void print_float(float x) { printf("%.2f\n", x); }
void print_string(char *x) { printf("%s\n", x); }
void print_bool(bool x) {
  if (x) printf("true\n");
  else if (!x) printf("false\n");
}
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
static intptr_t* stackChecker = 0;
__attribute__((noinline))
intptr_t get_stack() { int i; return (intptr_t)&i; }
int uses_stack(test_struct* t1) {
  if (stackChecker == 0) stackChecker = (intptr_t*)malloc(sizeof(intptr_t));
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

