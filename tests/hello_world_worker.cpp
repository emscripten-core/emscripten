#include <stdio.h>
#include <emscripten.h>

int main()
{
  printf("you should not see this text when in a worker!\n"); // this should not crash, but also should not show up anywhere if you are in a worker
  emscripten_run_script("if (typeof postMessage !== 'undefined') { postMessage('hello from worker!') }");
}

