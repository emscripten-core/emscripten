#include <assert.h>
#include <stdio.h>
#include <pthread.h>

int main() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  size_t stacksize;
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("%zu\n", stacksize);
  // Should match DEFAULT_PTHREAD_STACK_SIZE = 64*1024;
  assert(stacksize == 64*1024);
  return 0;
}
