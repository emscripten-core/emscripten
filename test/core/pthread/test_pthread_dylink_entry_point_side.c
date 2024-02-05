#include <stdio.h>
#include <emscripten.h>
#include <pthread.h>

EMSCRIPTEN_KEEPALIVE void* side_module_thread_main(void* arg) {
  printf("call thread fn in side: %s\n", (char*)arg);
  return NULL;
}
