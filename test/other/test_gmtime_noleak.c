#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
  // Verify that it doesn't leak memory on first use (if we call it via
  // via gmtime thn this can hide the leak because the result is stored
  // in static data).
  time_t xmas2002 = 1040786563ll;

  // Avoid using a stack allocated struct here to avoid leaving the
  // tm->tm_zone pointer value on the stack. The fact that this is necessary
  // seem like a bug in lsan:
  // https://github.com/emscripten-core/emscripten/issues/15887
  struct tm* tm = (struct tm*)malloc(sizeof(struct tm));
  gmtime_r(&xmas2002, tm);
  printf("zone: %s\n", tm->tm_zone);
  free(tm);
  return 0;
}
