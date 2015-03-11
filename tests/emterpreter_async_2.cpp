#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

int calc(int x) {
  printf("..%d..\n", x);
  if (x < 5) {
    char buffer[10000];
    emscripten_get_callstack(EM_LOG_JS_STACK, buffer, 10000);
    int newlines = 0;
    char *b = buffer;
    while (*b) {
      if (*b == '\n') newlines++;
      b++;
    }
    if (newlines > 40) newlines = 40;
    printf("stack: %s => %d, sleeping...\n", buffer, newlines);
    emscripten_sleep(1000);
    printf("..and we're back, returning %d!\n", newlines);
    return newlines;
  }
  if (x % 6 == 0) return calc(5*x/6);
  if (x % 4 == 1) return calc(x-2);
  return calc(x-1);
}

int main() {
  volatile int x = 100;
  volatile int result = calc(x);
  printf("calc(%d) = %d\n", x, result);
  REPORT_RESULT();
}

