#include <string.h>
#include <stdio.h>
#include <emscripten.h>

int main()
{
  printf("you should not see this text when in a worker!\n"); // this should not crash, but also should not show up anywhere if you are in a worker
  FILE *f = fopen("file.dat", "r");
  char buffer[100];
  memset(buffer, 0, 100);
  buffer[0] = 0;
  if (f) fread(buffer, 10, 1, f);
  char buffer2[100];
  int n = sprintf(buffer2, "if (typeof postMessage !== 'undefined') { postMessage('hello from worker, and |%s|') }", buffer);
  printf("sprintf: %d\n", n);
  emscripten_run_script(buffer2);
}

