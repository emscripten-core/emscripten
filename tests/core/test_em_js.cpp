#include <emscripten.h>
#include <stdio.h>

EM_JS(void, noarg, (), { Module['print']("no args works"); });
EM_JS(int, noarg_int, (), {
  Module['print']("no args returning int");
  return 12;
});
EM_JS(double, noarg_double, (), {
  Module['print']("no args returning double");
  return 12.25;
});
EM_JS(void, intarg, (int x), { Module['print']("  takes ints: " + x);});
EM_JS(void, doublearg, (double d), { Module['print']("  takes doubles: " + d);});
EM_JS(double, stringarg, (char* str), {
  Module['print']("  takes strings: " + Pointer_stringify(str));
  return 7.75;
});
EM_JS(int, multi_intarg, (int x, int y), {
  Module['print']("  takes multiple ints: " + x + ", " + y);
  return 6;
});
EM_JS(double, multi_mixedarg, (int x, const char* str, double d), {
  Module['print']("  mixed arg types: " + x + ", " + Pointer_stringify(str) + ", " + d);
  return 8.125;
});
EM_JS(int, unused_args, (int unused), {
  Module['print']("  ignores unused args");
  return 5.5;
});
EM_JS(double, skip_args, (int x, int y), {
  Module['print']("  skips unused args: " + y);
  return 6;
});
EM_JS(double, add_outer, (double x, double y, double z), {
  Module['print']("  " + x + " + " + z);
  return x + z;
});
EM_JS(int, user_separator, (), {
  Module['print']("  can use <::> separator in user code");
  return 15;
});

int main() {
  printf("BEGIN\n");
  noarg();
  printf("    noarg_int returned: %d\n", noarg_int());
  printf("    noarg_double returned: %f\n", noarg_double());

  intarg(5);
  doublearg(5.0675);
  printf("    stringarg returned: %f\n", stringarg("string arg"));
  printf("    multi_intarg returned: %d\n", multi_intarg(5, 7));
  printf("    multi_mixedarg returned: %f\n", multi_mixedarg(3, "hello", 4.75));
  printf("    unused_args returned: %d\n", unused_args(0));
  printf("    skip_args returned: %f\n", skip_args(5, 7));
  printf("    add_outer returned: %f\n", add_outer(5.5, 7.0, 14.375));
  printf("    user_separator returned: %d\n", user_separator());

  printf("END\n");
  return 0;
}
