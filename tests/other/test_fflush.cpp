#include <emscripten.h>
#include <stdio.h>
int main()
{
  printf("Hey1\nHey2");
  fflush(stdout); // flushes libc, but does not send out a newline for our JS emitting code, so Hey2 is not printed
  return 0;
}

