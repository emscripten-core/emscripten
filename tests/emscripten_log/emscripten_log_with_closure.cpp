#include<emscripten.h>

int main() {
  emscripten_log(EM_LOG_CONSOLE, "hello, world!");
  return 0;
}

