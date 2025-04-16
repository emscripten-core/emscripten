#include <stdio.h>
#include <emscripten.h>

volatile int x;

int bar() { return 1; }

__attribute__((noinline))
int foo(const char *str, int &i)
{
  if (x == 1337) return bar(); // don't inline me
  printf("foo %s\n", str);
  emscripten_sleep(1);
  printf("foo %d\n", i);
  return 42;
}

__attribute__((noinline))
int foo2()
{
  if (x == 1337) return bar(); // don't inline me
  printf("foo2 1\n");
  emscripten_sleep(1);
  printf("foo2 2\n");
  return 43;
}

int main()
{
  printf("main 1\n");
  int j = 42;
  const char *str = "hello";
  int ret = foo(str, j);
  printf("main %d\n", ret);
  int ret2 = foo2();
  printf("ret2 %d\n", ret2);
  REPORT_RESULT(ret + ret2);
}
